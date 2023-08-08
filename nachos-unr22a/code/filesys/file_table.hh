#ifndef NACHOS_FILESYS_FILE_TABLE_HH
#define NACHOS_FILESYS_FILE_TABLE_HH

#include "threads/lock.hh"
#include "lib/list.hh"
#include "directory_entry.hh"

class FileTable {
public:
    FileTable();

    ~FileTable();

    bool readyToRemove(unsigned sector);

    void addOpenFile(unsigned sector);

    void addReadFile(unsigned sector);

private:
    List<DirectoryEntry*> *table;
    Lock *lock; //< Table lock.
};

#endif