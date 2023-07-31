/// Routines to manage address spaces (memory for executing user programs).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "threads/system.hh"

#include <string.h>

int AddressSpace::getToReplace(){
    return (toReplace++)%TLB_SIZE;
}


/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
AddressSpace::AddressSpace(OpenFile *executable_file) {
    toReplace = 0;
    ASSERT(executable_file != nullptr);

    addressSpaceFile = executable_file;
    Executable exe (executable_file);

    ASSERT(exe.CheckMagic());
    // addressSpaceExecutable = &exe;
    // How big is address space?

    unsigned size = exe.GetSize() + USER_STACK_SIZE;
      // We need to increase the size to leave room for the stack.
    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

    #ifndef SWAP
        ASSERT(numPages <= NUM_PHYS_PAGES);
    #else
        std::string fileNameStr = "SWAP." + std::to_string(currentThread->spaceId);
        const char *fileName = fileNameStr.c_str();
        if (fileSystem->Create(fileName, numPages * PAGE_SIZE)){
            swapFile = fileSystem->Open(fileName);
            DEBUG('s', "Archivo %s creado OK \n", fileName);
        } else
            DEBUG('s', "Error al crear el archivo %s \n", fileName);
    #endif
      // Check we are not trying to run anything too big -- at least until we
      // have virtual memory.

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    // First, set up the translation.

    char *mainMemory = machine->GetMMU()->mainMemory;

    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
        pageTable[i].virtualPage  = i;
        // For now, virtual page number = physical page number.
        // #ifdef DEMAND_LOADING
        //         pageTable[i].physicalPage = -2; // ponemos el -1 así no hace la búsqueda
        // #else
        #ifdef SWAP
            pageTable[i].physicalPage = coremap->Find(i, currentThread);
        #else
            pageTable[i].physicalPage = bitmap->Find();
        #endif
        ASSERT(pageTable[i].physicalPage >= 0); // debería haber espacio
// #endif
        // DEBUG('a', "Page %d to %d\n", i, pageTable[i].physicalPage);
#ifdef DEMAND_LOADING
        pageTable[i].valid        = false;
        // DEBUG('d', "La vpn %d/%d  es se crea como inválida.\n", i, numPages);
#else
        pageTable[i].valid        = true;
#endif
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
          // If the code segment was entirely on a separate page, we could
          // set its pages to be read-only.
        
        // Zero out the entire address space, to zero the unitialized data
        // segment and the stack segment.
        // Ya no podemos inicializar todas las paginas juntas.
        memset(mainMemory, pageTable[i].physicalPage, PAGE_SIZE);
    }

    // memset(mainMemory, 0, size);

#ifndef DEMAND_LOADING

    // Then, copy in the code and data segments into memory.
    uint32_t codeSize = exe.GetCodeSize();
    uint32_t initDataSize = exe.GetInitDataSize();

    if (codeSize > 0) {
        uint32_t virtualAddr = exe.GetCodeAddr();
        DEBUG('a', "Initializing code segment, at 0x%X, size %u\n",
              virtualAddr, codeSize);
        // exe.ReadCodeBlock(&mainMemory[virtualAddr], codeSize, 0);
        for (uint32_t i = 0; i < codeSize; i++) {
            uint32_t frame = pageTable[DivRoundDown(virtualAddr + i, PAGE_SIZE)].physicalPage;
            uint32_t offset = (virtualAddr + i) % PAGE_SIZE;
            uint32_t physicalAddr = frame * PAGE_SIZE + offset;
            exe.ReadCodeBlock(&mainMemory[physicalAddr], 1, i);
        }
    }
    if (initDataSize > 0) {
        uint32_t virtualAddr = exe.GetInitDataAddr();
        DEBUG('a', "Initializing data segment, at 0x%X, size %u\n",
              virtualAddr, initDataSize);
        // exe.ReadDataBlock(&mainMemory[virtualAddr], initDataSize, 0);
        for (uint32_t i = 0; i < initDataSize; i++) {
          uint32_t frame = pageTable[DivRoundDown(virtualAddr + i, PAGE_SIZE)].physicalPage;
          uint32_t offset = (virtualAddr + i) % PAGE_SIZE;
          uint32_t physicalAddr = frame * PAGE_SIZE + offset;
          exe.ReadDataBlock(&mainMemory[physicalAddr], 1, i);
        }
    }
#endif
}


TranslationEntry AddressSpace::loadPage(unsigned vpn){
    DEBUG('d', "Empezando con el loadpagede la vpn: %d.\n", vpn);

    // chequear si la pagina corresponde a codigo, datos o stack
    #ifdef SWAP
        int ppn = coremap->Find(vpn, currentThread);
    #else
        int ppn = bitmap->Find();
    #endif

    Executable exec (addressSpaceFile);

    ASSERT(ppn >= 0); // debería haber espacio

    TranslationEntry newPage = TranslationEntry();
    newPage.virtualPage  = vpn;
    newPage.physicalPage = ppn;
    newPage.valid        = true;
    newPage.use          = false;
    newPage.dirty        = false;

    char * mainMemory = machine->GetMMU()->mainMemory;
    char frame = mainMemory[ppn * PAGE_SIZE];


    DEBUG('d', "Cargando la página virtual %d en la física %d.\n", vpn, ppn);

    uint32_t codeSize = exec.GetCodeSize();
    uint32_t codePages = DivRoundUp(codeSize, PAGE_SIZE);
    uint32_t initDataSize = exec.GetInitDataSize();
    uint32_t dataPages = DivRoundUp(initDataSize, PAGE_SIZE);
    uint32_t totalSize = exec.GetSize();
    uint32_t totalPages = DivRoundUp(totalSize, PAGE_SIZE);
    // ver si es tan trivial como asumir la siguiente

    DEBUG('d', "Páginas de código: %d.\n", codePages);
    DEBUG('d', "Páginas de datos: %d.\n", dataPages);

    ASSERT(vpn < totalPages);

    if (vpn > codePages+dataPages) { 
        // es stack
        newPage.readOnly = false;
        DEBUG('d', "DemandLoading. La vpn %d es un segmento del STACK.\n", vpn);
        // memset(&frame , 0, PAGE_SIZE);
    } else if(vpn < codePages){
        if (codePages>0){
            // es codigo
            newPage.readOnly = true;
            DEBUG('d', "DemandLoading. La vpn %d es un segmento de CODIGO.\n", vpn);
            for (uint32_t alreadyRead=0; alreadyRead<PAGE_SIZE; alreadyRead++){
                exec.ReadCodeBlock(&frame, 1, alreadyRead); 
            }
        }
    } else if(vpn < codePages+dataPages){
        if (dataPages>0){
            // es dato
            newPage.readOnly = false;
            DEBUG('d', "DemandLoading. La vpn %d es un segmento de DATOS.\n", vpn);
            for (uint32_t alreadyRead=0; alreadyRead<PAGE_SIZE; alreadyRead++){
                exec.ReadDataBlock(&frame, 1, alreadyRead);
            }
        }
    }

    return newPage;
}

void saveInSwap(int ppn){
    CoremapEntry entry = coremap->GetEntry(ppn);
    OpenFile * swapfile = entry.thread->space->getSwapFile();
    unsigned position = entry.vpn * PAGE_SIZE;
    unsigned numBytes = PAGE_SIZE;
    const char *from = &(machine->GetMMU()->mainMemory[ppn]);
    swapfile->WriteAt(from, numBytes, position);
}
#ifdef SWAP
#endif

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace() {
    for(unsigned i = 0; i < numPages; i++) {
        bitmap->Clear(pageTable[i].physicalPage);
    }
    delete [] pageTable;
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++) {
        machine->WriteRegister(i, 0);
    }

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);
}

void AddressSpace::InitRegisters(int argc, char** argv, int sizeArgs) {
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++) {
        machine->WriteRegister(i, 0);
    }

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(ARGC, argc);
    machine->WriteRegister(ARGV, numPages * PAGE_SIZE - 16);

    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16 - sizeArgs);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);

    for(int i = 0; i < argc; i++) {
        char* arg = argv[i];
        for(int j = 0; j != '\0'; j++) {
            machine->WriteMem(numPages * PAGE_SIZE - 16, 1, arg[j]);
        }
        
    }
    
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void
AddressSpace::SaveState()
{}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void
AddressSpace::RestoreState()
{
//    machine->GetMMU()->pageTable     = pageTable;
//    machine->GetMMU()->pageTableSize = numPages;
    invalidateTLB();
    toReplace = 0;
}

void AddressSpace::invalidateTLB(){
    for(unsigned i=0; i<TLB_SIZE; i++)
        machine->GetMMU()->tlb[i].valid = false;
}

TranslationEntry* AddressSpace::getPageTable(){
    return pageTable;
}


#ifdef SWAP
OpenFile * AddressSpace::getSwapFile(){
    return swapFile;
}
#endif