/// Routines to manage address spaces (memory for executing user programs).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "executable.hh"
#include "threads/system.hh"

#include <string.h>


/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
AddressSpace::AddressSpace(OpenFile *executable_file) {
    ASSERT(executable_file != nullptr);

    Executable exe (executable_file);
//    const char * name = executable_file->name;
//    DEBUG('a', "Archivo: %s \n", executable_file->name );
    DEBUG('a', "Archivo: %u \n", executable_file->Length() );
    ASSERT(exe.CheckMagic());

    // How big is address space?

    unsigned size = exe.GetSize() + USER_STACK_SIZE;
      // We need to increase the size to leave room for the stack.
    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

    ASSERT(numPages <= NUM_PHYS_PAGES);
      // Check we are not trying to run anything too big -- at least until we
      // have virtual memory.

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    // First, set up the translation.

#ifndef DEMAND_LOADING
    char *mainMemory = machine->GetMMU()->mainMemory;
#endif

unsigned tableSize ;
#ifdef USE_TLB
    tableSize = TLB_SIZE;
#else
    tableSize = numPages;
#endif
    TranslationEntry * table = new TranslationEntry[tableSize];

    for (unsigned i = 0; i < tableSize; i++) {
        DEBUG('a', "Page %d to %d\n", i, table[i].physicalPage);
        table[i].virtualPage  = i;
        table[i].use          = false;
        table[i].dirty        = false;
        table[i].readOnly     = false;
        table[i].inSwap       = false;
#ifdef DEMAND_LOADING
        table[i].physicalPage = -1; // todavía no se cargó
        table[i].valid        = false; /*3)*/
#else
        int ppn = bitmap->Find();
        if (ppn<0){
            DEBUG('m',"There is no physical pages available");
        }
        table[i].physicalPage = ppn;
        table[i].valid        = true; 
        memset(mainMemory, table[i].physicalPage, PAGE_SIZE);
#endif
    }

#ifdef USE_TLB
    machine->GetMMU()->tlb = table;
#else
    machine->GetMMU()->pageTable = table;
#endif



#ifdef SWAP
    std::string fileNameStr = std::to_string(currentThread->spaceId);
    fileNameStr = "SWAP." + fileNameStr;
    const char *fileName = fileNameStr.c_str();
    if (fileSystem->Create(fileName, numPages * PAGE_SIZE))
        swapFile = fileSystem->Open(fileName);
    else
        DEBUG('a', "Error al crear el archivo %s \n", fileName);
#endif


// en caso de que esté prendida la bandera DEMAND_LOADING, no hay que hacer nada 
// porque se encarga el page-fault-handler
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
<<<<<<< HEAD
#endif
}

#ifdef USE_TLB
TranslationEntry AddressSpace::loadPage(int posToFree, int physicalPage, int vpn){

    TranslationEntry entryToFree = machine->GetMMU()->tlb[posToFree];
    int freePhysicalPage;
    int physicalAddr = physicalPage * PAGE_SIZE;
    int virtualAddr = vpn * PAGE_SIZE;
    #ifdef SWAP
    freePhysicalPage = coreMap->Find(); 
    // si encuentra una página física libre en memoria, la devuelve
    // si no hay libres, retorna -1
    if (freePhysicalPage < 0) {
        DEBUG('a', "No hay páginas libres\n");
        // Hay que hacer SWAP

        if(!entryToFree.dirty && entryToFree.physicalPage != -1){
        // Si está clean y la página ya fue cargada alguna vez 
        // (-1 es el valor inicial con demand loading)
        // Escribe la página que hay que liberar en el swapFile de mi proceso
            
            int addrToWrite = entryToFree.vpn * PAGE_SIZE;
            // voy a escribir en el bloque vpn del archivo
            int whatToWrite = entryToFree.physicalPage * PAGE_SIZE;
            // voy a escribir el bloque que voy a liberar
            currentThread->getSwapFile()->WriteAt(
                &mainMemory[whatToWrite], PAGE_SIZE, addrToWrite
                );
            
            // Ahora hay que liberar la memoria que ya escribí en el swapFile
            // sería así?
            // &mainMemory[entryToFree.physicalPage] = &mainMemory[physicalPage];
            freePhysicalPage = physicalPage;
        }
    } else {
        DEBUG('a', "Si hay páginas libres\n");
    } 
    //swapFile->ReadAt(&mainMemory[physicalAddr], PAGE_SIZE, vpn * PAGE_SIZE);
    #endif

    #ifdef DEMAND_LOADING
    // chequear si la pagina corresponde a codigo, datos o stack
    // con las funciones dentro del constructor, como GetCodeSize
    Executable exe (executable_file);
    
    uint32_t codeSize = exe.GetCodeSize();
    uint32_t initDataSize = exe.GetInitDataSize();

    if (virtualAddr > codeSize+initDataSize) { 
        // es stack
        memset(mainMemory[physicalAddr] , 0, PAGE_SIZE)
    } else if(virtualAddr < codeSize){
        // es codigo
        if (codeSize > 0) {
            // entiendo que acá hay que leer una página
            // y meterla en nuevaPagFreeMap 
            // exe.ReadCodeBlock(&mainMemory[virtualAddr], PAGE_SIZE, 0); 
            exe.ReadCodeBlock(&mainMemory[freePhysicalPage], PAGE_SIZE, 0); 
        } 
    } else if(virtualAddr < codeSize+initDataSize){
            // es dato
            if (initDataSize > 0) {
                exe.ReadDataBlock(&mainMemory[freePhysicalPage], PAGE_SIZE, 0);
            }
    }
    #endif

    TranslationEntry ret;
    ret.dirty      = false;
    ret.readOnly   = false;
    ret.inSwap     = false;
    ret.valid      = true;
    ret.use        = true;
    ret.physicalPage   = freePhysicalPage;
    ret.virtualPage    = vpn;
    return ret;
    
}
#endif
  
unsigned AddressSpace::getPositionToReplace(){ 
    return (machine->GetMMU()->getToReplace()+1)%TLB_SIZE; 
=======
>>>>>>> 61a2ed4963605f5554004c99c97845e43ef74d68
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace() {
#ifndef USE_TLB
    for(int i = 0; i < numPages; i++) {
        bitmap->Clear(pageTable[i].physicalPage);
    }
    delete [] pageTable;
#endif
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void AddressSpace::InitRegisters() {
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
#ifdef USE_TLB
    machine->GetMMU()->InvalidateTLB();
#else
    machine->GetMMU()->pageTable     = pageTable;
    machine->GetMMU()->pageTableSize = numPages;
#endif
}

unsigned  AddressSpace::getNumPages() {
    return numPages;
}

#ifdef SWAP
OpenFile * AddressSpace::getSwapFile() {return swapFile;}
#endif

