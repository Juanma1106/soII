#include "thread_comensales_test.hh"

Lock** forksLocks = new Lock*[5];
static const int COMENSALES = 5;

void ComensalFailedLocked(void *forkRight) {
	unsigned *fR = (unsigned *) forkRight;
	// unsigned fR = *n;
	int fL;
	if(*fR == 0) {
		fL = COMENSALES -1;
	} else {
		fL = *fR-1;
	}
	DEBUG('s', "El comensal %s quiere el tenedor %d.\n", currentThread->GetName(), *fR);
	forksLocks[*fR]->Acquire();
	DEBUG('s', "El comensal %s tomó el tenedor %d.\n", currentThread->GetName(), *fR);
	currentThread->Yield();
	
	DEBUG('s', "El comensal %s quiere el tenedor %d.\n", currentThread->GetName(), fL);
	

	forksLocks[fL]->Acquire();
	DEBUG('s', "El comensal %s tomó el tenedor %d.\n", currentThread->GetName(), fL);
	// quedan todos esperando, nunca se imprime la siguiente linea
	DEBUG('s', "Termino de comer el comensal %s .\n", currentThread->GetName());
	forksLocks[*fR]->Release();
	forksLocks[fL]->Release();
}


void ThreadComensalesTestFailedLocked() {
	char names[COMENSALES][64] = {"Zero", "1st", "2nd", "3rd", "4th"};
    Thread** allThreads = new Thread*[COMENSALES -1];

	for (int i=0; i < COMENSALES; i++){
		forksLocks[i] = new Lock(names[i]);
	}

	for(int i = 0; i < COMENSALES - 1; i++){
		allThreads[i] = new Thread(names[i+1], true);
		unsigned *n = new unsigned;
		*n = (i+2) % COMENSALES;
		allThreads[i]->Fork(ComensalFailedLocked, (void *) n);
	}
	unsigned *n = new unsigned;
	*n = 1;
	ComensalFailedLocked((void *) n);

	for(int i = 0; i < COMENSALES -1; i++){
		allThreads[i]->Join();
	}

	// free memory
	delete allThreads;
	for(int i = 0; i < 5; i++){
		forksLocks[i]->~Lock();
	}
	delete forksLocks;
}

void ComensalTestFixed(void *forkRight) {
	unsigned *fR = (unsigned *) forkRight;
	// unsigned fR = *n;
	int fL;
	if(*fR == 0) {
		fL = 0;
		*fR = COMENSALES -1;
	} else {
		fL = *fR-1;
	}

	DEBUG('s', "El comensal %s quiere el tenedor %d.\n", currentThread->GetName(), *fR);
	forksLocks[*fR]->Acquire();
	DEBUG('s', "El comensal %s tomó el tenedor %d.\n", currentThread->GetName(), *fR);
	currentThread->Yield();
	
	DEBUG('s', "El comensal %s quiere el tenedor %d.\n", currentThread->GetName(), fL);
	

	forksLocks[fL]->Acquire();
	DEBUG('s', "El comensal %s tomó el tenedor %d.\n", currentThread->GetName(), fL);
	// quedan todos esperando, nunca se imprime la siguiente linea
	DEBUG('s', "Termino de comer el comensal %s .\n", currentThread->GetName());
	forksLocks[*fR]->Release();
	forksLocks[fL]->Release();
}


void ThreadComensalesTestFixed() {
	char names[COMENSALES][64] = {"Zero", "1st", "2nd", "3rd", "4th"};
    Thread** allThreads = new Thread*[COMENSALES -1];

	for (int i=0; i < COMENSALES; i++){
		forksLocks[i] = new Lock(names[i]);
	}

	for(int i = 0; i < COMENSALES - 1; i++){
		allThreads[i] = new Thread(names[i+1], true);
		unsigned *n = new unsigned;
		*n = (i+2) % COMENSALES;
		allThreads[i]->Fork(ComensalTestFixed, (void *) n);
	}
	unsigned *n = new unsigned;
	*n = 1;
	ComensalTestFixed((void *) n);

	for(int i = 0; i < COMENSALES -1; i++){
		allThreads[i]->Join();
	}

	// free memory
	delete allThreads;
	for(int i = 0; i < 5; i++){
		forksLocks[i]->~Lock();
	}
	delete forksLocks;
}