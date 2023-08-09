#include "filesystem_lock.hh"

FileSystemLock::FileSystemLock(const char *name){
	ASSERT(name != NULL);
	
	lock = new Lock(name);
	condition = new Condition(name, lock);
	readersCount = 0;
	isWriting = false;
}

FileSystemLock::~FileSystemLock(){
	delete lock;
	delete condition;
}

void FileSystemLock::addReader(){
	lock->Acquire();
	while(isWriting){
        // DEBUG('f', "Esperando para leer.\n");
		condition->Wait();
	}
	readersCount++;
	lock->Release();
}

void FileSystemLock::removeReader() {
	lock->Acquire();
	readersCount--;
	if (readersCount == 0) {
        DEBUG('f', "readers = 0.\n");
        condition->Broadcast(); 
    }
    lock->Release();
}

void FileSystemLock::addWriter() {
	lock->Acquire();
	while (isWriting || readersCount > 0){
        // DEBUG('f', "Esperando para escribir.\n");
		condition->Wait();
    }
	isWriting = true;
	lock->Release();
}

void FileSystemLock::removeWriter() {
    lock->Acquire();
    isWriting = false;
    condition->Broadcast();
    lock->Release();
}
