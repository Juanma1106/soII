/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_prod_cons.hh"

static Lock *lockForCondition;
static Condition *condConsumer;
static Condition *condProducer;
static int itemCount = 0;
static const int NUM_PRODUCER = 3;
static const int NUM_CONSUMER = 3;
static int done = 0;
const int BUFFER_SIZE = 3;

void producer(char *name) {
	int iterations = 10;
	for(int i = 0; i < iterations; i++) {
		lockForCondition->Acquire();
		while (itemCount == BUFFER_SIZE) {
			DEBUG('s', "*** Thread `%s` is waiting to produce\n", name);
			condProducer->Wait();
		}
		// produceItem
		itemCount++;
		DEBUG('s', "*** Thread `%s`. Items Count %d. Iterations pending %d\n", name, itemCount, iterations - i - 1);
	    condConsumer->Signal();
	   	DEBUG('s', "*** Thread `%s` send signal. ItemCount > 0\n",name);
		lockForCondition->Release();
	}
	DEBUG('s', "*** Thread `%s` finish\n", name);
	done++;
}

void consumer(char *name) {
	int iterations = 10;
	for(int i = 0; i < iterations; i++) {
		lockForCondition->Acquire();
	    while (itemCount == 0) {
	    	DEBUG('s', "*** Thread `%s` is waiting to consume\n",name);
	        condConsumer->Wait();
	    }
		// consumeItem
		itemCount--;
		DEBUG('s', "*** Thread `%s`. Items Count %d. Iterations pending %d\n", name, itemCount, iterations - i - 1);
	    condProducer->Signal();
		DEBUG('s', "*** Thread `%s` send signal. ItemCount < BUFFER_SIZE\n", name);
		lockForCondition->Release();
	}
	DEBUG('s', "*** Thread `%s` finish\n", name);
	done++;
}

void ConsThread(void *name_) {
    char *name = (char *) name_;
	consumer(name);
}

void ProdThread(void *name_) {
    char *name = (char *) name_;
	producer(name);
}

void ThreadTestProdCons() {
	lockForCondition = new Lock("lockForCondition");
	condConsumer = new Condition("condConsumer", lockForCondition);
	condProducer = new Condition("condProducer", lockForCondition);

	char namesCons[NUM_CONSUMER - 1][64] = {"2nd cons", "3rd cons"};
    Thread** threadsConsumers = new Thread*[NUM_CONSUMER - 1];
	for(int i = 0; i < NUM_CONSUMER - 1; i++){
		threadsConsumers[i] = new Thread(namesCons[i]);
		threadsConsumers[i]->Fork(ConsThread, (void *) namesCons[i]);
	}
	
	char namesProd[NUM_PRODUCER][64] = {"1st prod", "2nd prod", "3rd prod"};
    Thread** threadsProducers = new Thread*[NUM_PRODUCER];
	for(int i = 0; i < NUM_PRODUCER; i++){
		threadsProducers[i] = new Thread(namesProd[i]);
		threadsProducers[i]->Fork(ProdThread, (void *) namesProd[i]);
	}
	
	ConsThread((void *) "1st cons");
	
	while(done != NUM_PRODUCER + NUM_CONSUMER) {
		currentThread->Yield();
	}

	// free memory
	lockForCondition->~Lock();
	condConsumer->~Condition();
	condProducer->~Condition();
	delete threadsProducers;
	delete threadsConsumers;
}
	
