#ifndef NACHOS_USERPROG_SYNCH_CONSOLE__HH
#define NACHOS_USERPROG_SYNCH_CONSOLE__HH

#include "machine/console.hh"
#include "threads/semaphore.hh"
#include "threads/lock.hh"

class SynchConsole {
public:

    /// Initialize a synchronous console.
    SynchConsole(const char *in, const char *out);

    /// De-allocate the synch console data.
    ~SynchConsole();

    /// Read/write a console sector, returning only once the data is actually
    /// read or written.  These call `Console::ReadRequest`/`WriteRequest` and
    /// then wait until the request is done.

    char getChar();
    void putChar(char ch);
private: 
    Console *console;  ///< Raw console device.
    Lock *readLock;
    Lock *writeLock;
};

#endif