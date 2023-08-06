/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "exception.hh"

SpaceId StartProcess(char** args, bool joinable);

static void IncrementPC() {
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

void StartProcess2(void *args) {
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();
    if(args != nullptr){
		machine->WriteRegister(4, WriteArgs((char**) args));
		int sp = machine->ReadRegister(STACK_REG);
		machine->WriteRegister(5, sp);
		machine->WriteRegister(STACK_REG, sp - 16);
    }

    machine->Run();  // Jump to the user progam.
    ASSERT(false);   // `machine->Run` never returns; the address space
                    // exits by doing the system call `Exit`.
}

/// Do some default behavior for an unexpected exception.
///
/// NOTE: this function is meant specifically for unexpected exceptions.  If
/// you implement a new behavior for some exception, do not extend this
/// function: assign a new handler instead.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void PageFaultHandler(ExceptionType _et) {
    // DEBUG('d', "Hilo que está manejando la excepción: %s . \n", currentThread->GetName());
    int virtAddr = machine->ReadRegister(BAD_VADDR_REG);
    uint32_t vpn = (unsigned) virtAddr / PAGE_SIZE;
    unsigned numPages = currentThread->space->getNumPages();
    ASSERT(vpn < numPages);
    DEBUG('v', "Fallo de paginación con vpn %d.\n", vpn);
    int indexTLB = currentThread->space->getToReplace();
    // DEBUG('v', "virtAddr: %u . indexTLB: %d \n", virtAddr, indexTLB);
    TranslationEntry *pageTable = currentThread->space->getPageTable();
    // machine->GetMMU()->PrintTLB();
    if(pageTable[vpn].valid){
        // DEBUG('d', "Cargada la vpn desde memoria: %d.\n", vpn);
        machine->GetMMU()->tlb[indexTLB] = pageTable[vpn];
    } else {
        machine->GetMMU()->tlb[indexTLB] = currentThread->space->loadPage(vpn);
        DEBUG('d', "Cargada la vpn desde swap o disco: %d.\n", vpn);
    }

#ifdef USE_TLB
    machine->GetMMU()->sumMiss();
#endif
    // machine->GetMMU()->PrintTLB();
    DEBUG('d', "Se cargó ok la página %d.\n", vpn);

}


static void ReadOnlyHandler(ExceptionType _et){ 
    int virtAddr = machine->ReadRegister(BAD_VADDR_REG);
    unsigned vpn = (unsigned) virtAddr / PAGE_SIZE;
    DEBUG('v', "La entrada %d se quiso modificar y es ReadOnly.\n", vpn);
}


static void DefaultHandler(ExceptionType et) {
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);
    ASSERT(false);
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void SyscallHandler(ExceptionType _et) {
    int scid = machine->ReadRegister(2);

    switch (scid) {

        case SC_HALT:{
            DEBUG('e', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;
        }
        
        case SC_EXIT:{
            int status = machine->ReadRegister(4);
            DEBUG('a', "Exited with status %d\n", status);
            // interrupt->Halt();
            currentThread->Finish(status);
            break;
        }

        case SC_EXEC: {
            // Seteo -1 en el registro por cualquier fallo que pueda salir.
            // DEBUG('e', "Arranca el exec.\n");
            machine->WriteRegister(2, -1);

            int filenameAddr = machine->ReadRegister(4);
            int argsAddr = machine->ReadRegister(5);
            bool joinable = machine->ReadRegister(6) ? true : false;
            
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
                break;
            } 
            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n", FILE_NAME_MAX_LEN);
                break;
            } 
            
            OpenFile *exe = fileSystem->Open(filename);
            if (exe == nullptr) {
                DEBUG('e', "No se pudo abrir el archivo\n");
                break;
            }

            // Caso feliz
            DEBUG('e', "`Exec` requested for file `%s`.\n", filename);
            Thread *execThread = new Thread(filename, joinable, currentThread, currentThread->getPriority());
            #ifdef USER_PROGRAM
                AddressSpace *execSpace = new AddressSpace(exe);
	    	    execThread->space = execSpace;
                threads->Add(execThread);
            #endif

            char **args = SaveArgs(argsAddr);
            execThread->Fork(StartProcess2, (void *) args);            

            break;
        }

        case SC_JOIN: {
            // SpaceId pid = machine->ReadRegister(4);
            // DEBUG('p', "Haciendo el join del %d.\n", pid);

            // if(threads->HasKey(pid)) {
            //     Thread *thread = threads->Get(pid);
            //     int joinned = thread->Join();
            //     machine->WriteRegister(2, joinned);
            //     DEBUG('e', "Join terminado.\n");
            // }
            break;
        }

        //case SC_FORK: {
        //    break;
        //}

        //case SC_YIELD: {
        //    break;
        //}

        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);

            // Seteo -1 en el registro por cualquier fallo que pueda salir.
            machine->WriteRegister(2, -1);

            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
            } else {
                char filename[FILE_NAME_MAX_LEN + 1];
                if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
                    DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                        FILE_NAME_MAX_LEN);
                } else {
                    DEBUG('e', "`Create` requested for file `%s`.\n", filename);
                    int fileCreated = fileSystem->Create(filename,1000);
                    machine->WriteRegister(2, fileCreated);
                }
            }
            break;
        }
        
        case SC_REMOVE: {
            // int filenameAddr = machine->ReadRegister(4);

            // // Seteo -1 en el registro por cualquier fallo que pueda salir.
            // machine->WriteRegister(2, -1);

            // if (filenameAddr == 0) {
            //     DEBUG('e', "Error: address to filename string is null.\n");
            //     break;
            // } 

            // char filename[FILE_NAME_MAX_LEN + 1];
            // if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
            //     DEBUG('p', "Error: filename string too long (maximum is %u bytes).\n", FILE_NAME_MAX_LEN);
            //     machine->WriteRegister(2, 0);
            // } else {
            //     DEBUG('p', "`Remove` requested for file `%s`.\n", filename);
            //     int fileRemoved = fileSystem->Remove(filename);
            //     machine->WriteRegister(2, fileRemoved);
            // }
            // break;
        }

        case SC_OPEN: {
            /*
            Cuando abren correctamente el archivo, puede darse el caso de que no haya lugar en la tabla
             de archivos abiertos, además de que el OpenFile seguiría abierto ocupando espacio en memoria
            */
            int filenameAddr = machine->ReadRegister(4);

            // Seteo -1 en el registro por cualquier fallo que pueda salir.
            machine->WriteRegister(2, -1);

            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
            } else {
                char filename[FILE_NAME_MAX_LEN + 1];
                if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
                    DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                        FILE_NAME_MAX_LEN);
                } else {
                    DEBUG('e', "`Open` requested for file `%s`.\n", filename);
                    OpenFile *fileOpened = fileSystem->Open(filename);
                    if(fileOpened == nullptr) {
                        DEBUG('e', "Error: file does not exist with name %s.\n", filename);
                    } else {
                        OpenFileId fileId = currentThread->openFile(fileOpened);
                        if (fileId < 0){
                            DEBUG('e', "No hay espacio en la tabla de archivos %s.\n", filename);
                            fileOpened->~OpenFile();
                        } else {
                            machine->WriteRegister(2, fileId);
                        }
                            
                    }
                }
            }

            
            break;
        }

        case SC_CLOSE: {
            OpenFileId fileId = machine->ReadRegister(4);

            // Seteo -1 en el registro por cualquier fallo que pueda salir.
            machine->WriteRegister(2, -1);

            if(fileId < 0) {
                DEBUG('e', "Error: fileId is null.\n");
            } else if(fileId < 2) {
                DEBUG('e', "Error: fileId %d is reserved and can't be closed.\n", fileId);
            } else {
                DEBUG('e', "`Close` requested for fileId `%d`.\n", fileId);
                bool isOpenedFile = currentThread->isOpenedFile(fileId);
                if(!isOpenedFile) {
                    DEBUG('e', "Error: fileId %d is not valid.\n", fileId);
                } else {
                    OpenFile *fileOpened = currentThread->closeFile(fileId);
                    // Esto cierra el archivo. Adentro del OpenFile tenemos en fileDescriptor.
                    fileOpened->~OpenFile();
                    machine->WriteRegister(2, 0);
                }
            }
            break;
        }

        case SC_READ: {
            bool errorOcurred = false;
            // DEBUG('e', "Empieza a leer.\n");
            int bufferAddr = machine->ReadRegister(4);
            if (bufferAddr == 0) {
                DEBUG('e', "Read - Error: address for buffer is null.\n");
                errorOcurred=true;
            }
            int size = machine->ReadRegister(5);
            if (size == 0) {
                DEBUG('e', "Error: size is 0.\n");
                errorOcurred=true;
            }
            OpenFileId fileId = machine->ReadRegister(6);

            // DEBUG('e', "Lectura: Caso feliz.\n");

            if(!errorOcurred) {
                char *buffer = new char[size]; 
                // DEBUG('e', "fileId %d. CONSOLE_OUTPUT %d. CONSOLE_INPUT %d. Compare wit OUT\n", fileId, CONSOLE_OUTPUT, CONSOLE_INPUT);
                if(fileId == CONSOLE_INPUT) {
                    int temp = 0;
                    char c;
                    do {
                        // DEBUG('e', "Leyendo %d de %d.\n", temp, size );
                        c = synchConsole->getChar();
                        // DEBUG('e', "Caracter %c leído.\n", c );
                        // if(c != '\n' && c != '\0'){
                        // }
                            buffer[temp] = c;
                            temp++;
                    } while (temp < size && c != '\n' && c != '\0');
                    // if(c != '\n' && c != '\0'){
                    buffer[temp]='\0';
                    // DEBUG('e', "Por entrar al WriteBufferToUser con buffer: %s\n", buffer );
                    WriteBufferToUser(buffer, bufferAddr, size);
                    // } else {
                    //     --size;
                    // }
                } else if (fileId > CONSOLE_OUTPUT) {
                    if (currentThread->isOpenedFile(fileId)) {
                        OpenFile *file = currentThread->getFileOpened(fileId);
                        int sizeRead = file->Read(buffer, size);
                        WriteBufferToUser(buffer, bufferAddr, sizeRead);
                    } else {
                        DEBUG('e', "Error: file is not opened.\n");
                        errorOcurred=true;
                    }
                } else {
                    DEBUG('e', "Error: Invalid fileId %d.\n", fileId);
                    errorOcurred=true;
                }
                delete buffer;
            }
            
            if(!errorOcurred) {
                // DEBUG('e', "Read OK.\n");
                machine->WriteRegister(2, 0);
            //     machine->WriteRegister(2, -1);
            // } else {
            }
            break;
        }

        case SC_WRITE: {
            bool errorOcurred = false;

            int bufferAddr = machine->ReadRegister(4);
            if (bufferAddr == 0) {
                DEBUG('e', "Write - Error: address for buffer is null.\n");
                errorOcurred=true;
            }
            int size = machine->ReadRegister(5);
            if (size == 0) {
                DEBUG('e', "Error: size is 0.\n");
                errorOcurred=true;
            }
            OpenFileId fileId = machine->ReadRegister(6);
            char *buffer = new char[size];

            if(!errorOcurred) {
                if (!ReadStringFromUser(bufferAddr, buffer, 100)) {
                    DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                    FILE_NAME_MAX_LEN);
                    errorOcurred=true;
                }
            }
            if(!errorOcurred) {
                // DEBUG('e', "Escritura: Caso feliz.\n");
                // DEBUG('e', "buffer : %s\n", buffer);
                
                if(fileId == CONSOLE_OUTPUT) {
                    int temp = 0;
                    char c;
                    do {
                        c = buffer[temp];
                        synchConsole->putChar(c);
                        temp++;
                    } while (temp < size && c != '\n' && c != '\0'); 
                } else if(fileId > CONSOLE_OUTPUT) {
                    // archivo 
                    if (currentThread->isOpenedFile(fileId)) {
                        OpenFile *file = currentThread->getFileOpened(fileId);
                        int numBytesWrited = file->Write(buffer, size);
                        if (numBytesWrited == 0) {
                            errorOcurred=true;
                        }
                    } else {
                        DEBUG('e', "Error: file is not opened.\n");
                        errorOcurred=true;
                    }
                } else {
                    DEBUG('e', "Error: Invalid fileId %d.\n", fileId);
                    errorOcurred=true;
                }
            }
            delete buffer;
            
            if(errorOcurred) {
                machine->WriteRegister(2, -1);
            } else {
                // DEBUG('e', "Escritura OK.\n");
                machine->WriteRegister(2, 0);
            }
            break;
        }

        case SC_PS: {
            #ifdef USER_PROGRAM
                scheduler->PrintAllThreads();
            #endif
            break;
        }

        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
}

/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void SetExceptionHandlers() {
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &PageFaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &ReadOnlyHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
