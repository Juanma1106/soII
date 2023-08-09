#ifndef NACHOS_FILESYS_FILESYSTEM_LOCK__HH
#define NACHOS_FILESYS_FILESYSTEM_LOCK__HH

#include "threads/condition.hh"
#include "lib/assert.hh"
#include "lib/utility.hh"


class FileSystemLock {
public:
    FileSystemLock(const char *name);
    
    ~FileSystemLock();
    
    void addReader();
    
    void removeReader();
    
    void addWriter();
    
    void removeWriter();

private:
    Lock *lock;
    Condition *condition;
    unsigned readersCount;
    bool isWriting;
};

#endif