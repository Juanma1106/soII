#ifndef NACHOS_USERPROG_SYNCH_CONSOLE__HH
#define NACHOS_USERPROG_SYNCH_CONSOLE__HH

#include "machine/console.hh"
#include "threads/semaphore.hh"

class SynchConsole {
public:

    /// Initialize a synchronous console, by initializing the raw Disk.
    SynchConsole(const char *name);

    /// De-allocate the synch console data.
    ~SynchConsole();

    /// Read/write a console sector, returning only once the data is actually
    /// read or written.  These call `Disk::ReadRequest`/`WriteRequest` and
    /// then wait until the request is done.

    void ReadSector(int sectorNumber, char *data);
    void WriteSector(int sectorNumber, const char *data);

private: 
    Console *console;  ///< Raw console device.
    Semaphore *semaphore;  ///< To synchronize requesting thread with the
                           ///< interrupt handler.
    Semaphore *readAvail;
    Semaphore *writeDone;
};

#endif