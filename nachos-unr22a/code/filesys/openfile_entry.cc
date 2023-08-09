#include "openfile_entry.hh"

OpenFileEntry::OpenFileEntry(const char *myName, unsigned mySector){
	ASSERT(myName != NULL);
	mustRemove = false;
	openFileCount = 1;
	sector = mySector;
	name = myName;
	fileSystemLock = new FileSystemLock(myName);
}

OpenFileEntry::~OpenFileEntry(){
	delete fileSystemLock;
}