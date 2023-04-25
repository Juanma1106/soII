#include "thread_comensales_test.hh"

static Lock *l; 
Lock** forksLocks = new Lock*[5];

void Comensal(void *forkRight) {
	unsigned *fR = (unsigned *) forkRight;
	// unsigned fR = *n;
	int fL;
	DEBUG('s', "El comensal %s quiere el tenedor %d.\n", currentThread->GetName(), *fR);
	forksLocks[*fR]->Acquire();
	DEBUG('s', "El comensal %s tomó el tenedor %d.\n", currentThread->GetName(), *fR);
	currentThread->Yield();
	if  (*fR==0){
		fL = 4;
	} else {
		fL = *fR-1;
	}
	DEBUG('s', "El comensal %s quiere el tenedor %d.\n", currentThread->GetName(), fL);
	forksLocks[fL]->Acquire();
	DEBUG('s', "El comensal %s tomó el tenedor %d.\n", currentThread->GetName(), fL);
	// quedan todos esperando, nunca se imprime la siguiente linea
	DEBUG('s', "Termino de comer el comensal %s .\n", currentThread->GetName());
}


void ThreadComensalesTest() {
	char names[4][64] = {"2nd", "3rd", "4th", "5th"};
	for (int i=0; i<5; i++){
		forksLocks[i] = new Lock(names[i]);
	}
    Thread** threads = new Thread*[4];
	for(int i = 0; i < 4; i++){
		threads[i] = new Thread(names[i], true);
		unsigned *n = new unsigned;
		*n = i;
		threads[i]->Fork(Comensal, (void *) n);
	}
	unsigned *n = new unsigned;
	*n = 4;
	Comensal((void *) n);
}



