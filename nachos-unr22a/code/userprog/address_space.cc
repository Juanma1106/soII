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

unsigned AddressSpace::getNumPages(){
    return numPages;
}


/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
AddressSpace::AddressSpace(OpenFile *executable_file) {
    DEBUG('d', "Inició el constructor de address space del proceso %s\n", currentThread->GetName());    
    toReplace = 0;
    ASSERT(executable_file != nullptr);

    addressSpaceFile = executable_file;
    Executable exe (executable_file);

    ASSERT(exe.CheckMagic());

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
            DEBUG('d', "Archivo %s creado OK \n", fileName);
        } else
            DEBUG('d', "Error al crear el archivo %s \n", fileName);
    #endif

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    // First, set up the translation.

    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
        // For now, virtual page number = physical page number.
        #ifdef DEMAND_LOADING
            pageTable[i].physicalPage = -1;     // ponemos el -1 así no hace la búsqueda
            pageTable[i].valid        = false;
        #else
            pageTable[i].physicalPage = bitmap->Find();
            pageTable[i].valid        = true;
            ASSERT(pageTable[i].physicalPage >= 0); // debería haber espacio
            memset(machine->GetMMU()->mainMemory + pageTable[i].physicalPage * PAGE_SIZE, 0, PAGE_SIZE);
        #endif

        pageTable[i].virtualPage  = i;
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;       
    }

    #ifndef DEMAND_LOADING
        char *mainMemory = machine->GetMMU()->mainMemory;

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
            DEBUG('a', "Initializing data segment, at 0x%X, size %u\n", virtualAddr, initDataSize);
            for (uint32_t i = 0; i < initDataSize; i++) {
                uint32_t frame = pageTable[DivRoundDown(virtualAddr + i, PAGE_SIZE)].physicalPage;
                uint32_t offset = (virtualAddr + i) % PAGE_SIZE;
                uint32_t physicalAddr = frame * PAGE_SIZE + offset;
                exe.ReadDataBlock(&mainMemory[physicalAddr], 1, i);
            }
        }
    #endif
    DEBUG('d', "Terminó el constructor de address space del proceso %s\n", currentThread->GetName());

}


TranslationEntry AddressSpace::loadPage(unsigned vpn){
    DEBUG('d', "Empezando con el loadpage de la vpn: %d.\n", vpn);
    Executable exec (addressSpaceFile);
    uint32_t virtualPosition = vpn*PAGE_SIZE;
    uint32_t virtualCodeAddr = exec.GetCodeAddr();
    uint32_t virtualDataAddr = exec.GetInitDataAddr();
    uint32_t codeSize = exec.GetCodeSize();
    uint32_t dataSize = exec.GetInitDataSize();

    #ifdef SWAP
        int ppn = coremap->Find(vpn, swapFile, pageTable);
    #else
        // DEMAND LOADING
        int ppn = bitmap->Find();
    #endif
    ASSERT(ppn >= 0);

    if (pageTable[vpn].virtualPage == -1){
        // está en swap
        #ifdef SWAP
            DEBUG('w', "Leyendo la vpn: %d desde el swap.\n", vpn);
            swapFile->ReadAt(&machine->GetMMU()->mainMemory[ppn * PAGE_SIZE], PAGE_SIZE, virtualPosition);
        #endif
        DEBUG('w', "Leida la vpn: %d desde el swap.\n", vpn);
    } else {
        // chequear si la pagina corresponde a codigo, datos o stack
        // se carga desde el disco
        DEBUG('d', "Cargando la página virtual %d en la física %d.\n", vpn, ppn);

        char *mainMemory = machine->GetMMU()->mainMemory;
        int position = ppn * PAGE_SIZE;
        //por las dudas, antes, inicializamos la página completa
        memset(mainMemory + position, 0, PAGE_SIZE);
        uint32_t size;
        uint32_t lastRead = virtualPosition;
        if(virtualPosition < virtualCodeAddr + codeSize){
            if (codeSize>0){
                // es codigo
                DEBUG('d', "DemandLoading. La vpn %d es un segmento del CODIGO.\n", vpn);
                if (virtualCodeAddr + codeSize - virtualPosition < PAGE_SIZE){
                    size = virtualCodeAddr + codeSize - virtualPosition;
                } else {
                    size = PAGE_SIZE;
                }
                exec.ReadCodeBlock(&mainMemory[position], size, virtualPosition-virtualCodeAddr);
                lastRead += size;
            }
        } 

        if(lastRead < virtualPosition + PAGE_SIZE && lastRead < virtualDataAddr + dataSize) {
            if (dataSize>0){
                // es dato
                size = PAGE_SIZE - size;
                if (lastRead + size > virtualDataAddr + dataSize){
                    size = virtualDataAddr + dataSize - lastRead;
                }
                DEBUG('d', "DemandLoading. La vpn %d es un segmento de DATOS.\n", vpn);
                exec.ReadDataBlock(&mainMemory[position], size, lastRead-virtualDataAddr);
            }
        }
    }

    // sería el caso en que no entra en memoria, y asumimos que eso no sucede

    pageTable[vpn].physicalPage = ppn;
    pageTable[vpn].virtualPage  = vpn;
    pageTable[vpn].valid        = true;
    pageTable[vpn].use          = false;
    pageTable[vpn].dirty        = false;
    pageTable[vpn].readOnly     = false; //(vpn*PAGE_SIZE)<virtualCodeAddr + codeSize? true: false;

    return pageTable[vpn];
}


/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace() {
    for(unsigned int i = 0; i < numPages; i++) {
      if(pageTable[i].valid)
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

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void AddressSpace::SaveState() {}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void AddressSpace::RestoreState() {
    #ifndef USE_TLB
        machine->GetMMU()->pageTable     = pageTable;
        machine->GetMMU()->pageTableSize = numPages;
    #else
        invalidateTLB();
        toReplace = 0;
    #endif
}

void AddressSpace::invalidateTLB() {
    #ifdef USE_TLB
    TranslationEntry *tlb = machine->GetMMU()->tlb;
    for(unsigned i=0; i<TLB_SIZE; i++) {
        tlb[i].valid = false;
        }
    #endif
}

TranslationEntry* AddressSpace::getPageTable(){
    return pageTable;
}


#ifdef SWAP
OpenFile * AddressSpace::getSwapFile(){
    return swapFile;
}
#endif