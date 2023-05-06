#include "SynchConsole.hh"

static Semaphore *readAvail;
static Semaphore *writeDone;

/// Initialize Synch Console
SynchConsole::SynchConsole(const char *in, const char *out) {
    readLock = new Lock("synch disk lock");
    writeLock = new Lock("synch disk lock");
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

/// Console interrupt handlers.
///
/// Wake up the thread that requested the I/O.
static void ReadAvail(void *arg) {
    readAvail->V();
}

static void WriteDone(void *arg) {
    writeDone->V();
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
    readLock->Acquire();
    readAvail->P();
    char ch = console->GetChar();
    readLock->Release();
    return ch;
}