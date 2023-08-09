#ifndef NACHOS_FILESYS_OPENFILE_ENTRY__HH
#define NACHOS_FILESYS_OPENFILE_ENTRY__HH

#include "filesystem_lock.hh"

class OpenFileEntry {
public:
	OpenFileEntry(const char *name, unsigned sector);
	~OpenFileEntry();
	
	bool mustRemove;
	unsigned openFileCount;
	unsigned sector;
	const char *name;
	FileSystemLock *fileSystemLock;
};

#endif