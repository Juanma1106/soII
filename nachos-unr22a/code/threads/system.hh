/// All global variables used in Nachos are defined here.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_SYSTEM__HH
#define NACHOS_THREADS_SYSTEM__HH


#include "thread.hh"
#include "scheduler.hh"
#include "lib/utility.hh"
#include "machine/interrupt.hh"
#include "machine/statistics.hh"
#include "machine/timer.hh"


/// Initialization and cleanup routines.

// Initialization, called before anything else.
extern void Initialize(int argc, char **argv);

// Cleanup, called when Nachos is done.
extern void Cleanup();


extern Thread *currentThread;        ///< The thread holding the CPU.
extern Thread *threadToBeDestroyed;  ///< The thread that just finished.
extern Scheduler *scheduler;         ///< The ready list.
extern Interrupt *interrupt;         ///< Interrupt status.
extern Statistics *stats;            ///< Performance metrics.
extern Timer *timer;                 ///< The hardware alarm clock.

#ifdef USER_PROGRAM
#include "machine/machine.hh"
extern Machine *machine;  // User program memory and registers.
#include "userprog/SynchConsole.hh"
extern SynchConsole *synchConsole;
#include "lib/table.hh"
#include "threads/thread.hh"
extern Table<Thread*> *threads;
#include "lib/bitmap.hh"
extern Bitmap *bitmap;
#endif



#ifdef SWAP
    #include "vmem/coremap.hh"
    extern Coremap *coremap;
#endif

#ifdef FILESYS_NEEDED  // *FILESYS* or *FILESYS_STUB*.
class FileSystem;
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "filesys/synch_disk.hh"
extern SynchDisk *synchDisk;
#include "filesys/openfile_entry.hh"
#include "lib/table.hh"
extern Table<OpenFileEntry*> *opennedFilesTable;
#endif

#ifdef NETWORK
#include "network/post.hh"
extern PostOffice *postOffice;
#endif


#endif
