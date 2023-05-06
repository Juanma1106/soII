#include "SynchConsole.hh"

/// Console interrupt handler.  Need this to be a C routine, because C++ cannot
/// handle pointers to member functions.
static void ConsoleRequestDone(void *arg) {
    ASSERT(arg != nullptr);
    SynchConsole *console = (SynchConsole *) arg;
    console->RequestDone();
}

/// Initialize the synchronous interface to the physical console, in turn
/// initializing the physical console.
///
/// * `name` is a UNIX file name to be used as storage for the console data
///   (usually, `DISK`).
SynchConsole::SynchConsole(const char *in, const char *out) {
    semaphore = new Semaphore("synch console", 0);
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

void ReadAvail(void *arg) {
    readAvail->V();
}

void WriteDone(void *arg) {
    writeDone->V();
}

/// De-allocate data structures needed for the synchronous console abstraction.
SynchConsole::~SynchConsole() {
    delete console;
    delete semaphore;
}

/// Read the contents of a console sector into a buffer.  Return only after the
/// data has been read.
///
/// * `sectorNumber` is the console sector to read.
/// * `data` is the buffer to hold the contents of the console sector.
void SynchConsole::ReadSector(int sectorNumber, char *data) {
    ASSERT(data != nullptr);
    console->ReadRequest(sectorNumber, data);
    semaphore->P();   // Wait for interrupt.
}

/// Write the contents of a buffer into a console sector.  Return only
/// after the data has been written.
///
/// * `sectorNumber` is the console sector to be written.
/// * `data` are the new contents of the console sector.
void SynchConsole::WriteSector(int sectorNumber, const char *data) {
    ASSERT(data != nullptr);
    console->WriteRequest(sectorNumber, data);
    semaphore->P();   // wait for interrupt
}

/// Console interrupt handler.  Wake up any thread waiting for the console
/// request to finish.
void SynchConsole::RequestDone() {
    semaphore->V();
}