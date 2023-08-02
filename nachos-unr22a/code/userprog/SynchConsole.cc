#include "SynchConsole.hh"

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

/// Initialize Synch Console
SynchConsole::SynchConsole(const char *in, const char *out) {
    readLock = new Lock("synch disk lock");
    writeLock = new Lock("synch disk lock");
    console   = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
}


/// De-allocate data structures needed for the synchronous console abstraction.
SynchConsole::~SynchConsole() {
    readLock->~Lock();
    writeLock->~Lock();
    console->~Console();
    readAvail->~Semaphore();
    writeDone->~Semaphore();
}

/// Putchar in console
void SynchConsole::putChar(char ch) {
    writeLock->Acquire();
    console->PutChar(ch);
    writeDone->P();
    writeLock->Release();
}

/// Getchar from console
char SynchConsole::getChar() {
    // DEBUG('e', "Obteniendo char\n");
    readLock->Acquire();
    readAvail->P();
    // DEBUG('e', "Leyendo de consola\n");
    char ch = console->GetChar();
    // DEBUG('e', "Leído de consola\n");
    readLock->Release();
    // DEBUG('e', "Char obtenido\n");
    return ch;
}