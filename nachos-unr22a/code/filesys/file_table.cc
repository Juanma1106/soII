#include "file_table.hh"

FileTable::FileTable(){
    table = new DirectoryEntry[NUM_DIR_ENTRIES] ;
    lock = new Lock("FileTable");
}

FileTable::~FileTable(){
    delete table;
    delete lock;
}

bool FileTable::readyToRemove(unsigned sector){
    return true;
}

void FileTable::addOpenFile(unsigned sector){
    table[sector].inUse = true;
    table[sector].openCounter ++;
}

void FileTable::addReadFile(unsigned sector){
    table[sector].readCounter ++;
}
