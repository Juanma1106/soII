/// Test routines for demonstrating that Nachos can load a user program and
/// execute it.
///
/// Also, routines for testing the Console hardware device.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_USEPROG_PROG_TEST__CC
#define NACHOS_USEPROG_PROG_TEST__CC


#include "address_space.hh"
#include "machine/console.hh"
#include "threads/semaphore.hh"
#include "threads/system.hh"
#include "write_read_mem_test.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Args {
public:
    Args(){};
    int argc;
    char** argv;
    int sizeArgs;
};

void StartProcess2(void * args) {
    AddressSpace *space = currentThread->space;
    if(args != nullptr){
        Args *myArgs = (Args *) args;
        //struct args *myArgs = (struct args *)args;
        space->InitRegisters(myArgs->argc, myArgs->argv, myArgs->sizeArgs);  // Set the initial register values.
    } else {
        space->InitRegisters();  // Set the initial register values.
    }
    space->RestoreState();   // Load page table register.

    machine->Run();  // Jump to the user progam.
    ASSERT(false);   // `machine->Run` never returns; the address space
                    // exits by doing the system call `Exit`.
}

/// Run a user program.
///
/// Open the executable, load it into memory, and jump to it.

void StartProcess(const char *filename) {
    ASSERT(filename != nullptr);

    OpenFile *executable = fileSystem->Open(filename);
    if (executable == nullptr) {
        printf("Unable to open file %s\n", filename);
        return;
    }

    AddressSpace *space = new AddressSpace(executable);
    currentThread->space = space;

    delete executable;

    space->InitRegisters();  // Set the initial register values.
    space->RestoreState();   // Load page table register.

    machine->Run();  // Jump to the user progam.
    ASSERT(false);   // `machine->Run` never returns; the address space
                     // exits by doing the system call `Exit`.
}

/// Run a user program.
///
/// Open the executable, load it into memory, and jump to it.
SpaceId StartProcess(char** args, bool joinable) {
    const char *filename = args[0];

    ASSERT(filename != nullptr);

    OpenFile *executable = fileSystem->Open(filename);
    if (executable == nullptr) {
        DEBUG('e', "Unable to open file %s\n", filename);
        return -1;
    }

    int argc =  sizeof(args) / sizeof(args[0]);// - 1;
    int sizeArgs = 0;
    char *argv[argc] = {};
    for (int i = 0; i < argc; i++) {
        argv[i] = args[i+1];
        sizeArgs += sizeof(argv[i]);
    }

    Thread * newThread = new Thread(filename, joinable, 0);

    AddressSpace *space = new AddressSpace(executable);
    newThread->space = space;
    delete executable;

    Args * myArgs = new Args();
    myArgs->argc=argc;
    myArgs->argv=argv;
    myArgs->sizeArgs=sizeArgs;

    newThread->Fork(StartProcess2, (void *) myArgs);

    return newThread->spaceId;
}

/// Data structures needed for the console test.
///
/// Threads making I/O requests wait on a `Semaphore` to delay until the I/O
/// completes.

static Console   *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

/// Console interrupt handlers.
///
/// Wake up the thread that requested the I/O.

static void ReadAvail(void *arg) {
    readAvail->V();
}

static void WriteDone(void *arg) {
    writeDone->V();
}

/// Test the console by echoing characters typed at the input onto the
/// output.
///
/// Stop when the user types a `q`.
void ConsoleTest(const char *in, const char *out) {
    console   = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    for (;;) {
        readAvail->P();        // Wait for character to arrive.
        char ch = console->GetChar();
        console->PutChar(ch);  // Echo it!
        writeDone->P();        // Wait for write to finish.
        if (ch == 'q') {
            return;  // If `q`, then quit.
        }
    }
}

#endif