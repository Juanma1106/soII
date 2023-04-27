/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_prod_cons.hh"

static Lock *lockForCondConsumer;
static Lock *lockForCondProducer;
static Condition *condConsumer;
static Condition *condProducer;
static Lock *lockProducer;
static Lock *lockConsumer;
static int itemCount;
static const int NUM_PRODUCER = 3;
static const int NUM_CONSUMER = 3;
static int done = 0;

const int BUFFER_SIZE = 20;

void producer(char *name) {
	int iterations = 10;
	for(int i = 0; i < iterations; i++) {
		lockProducer->Acquire();
		DEBUG('s', "*** Thread `%s` acquired lockProducer\n",name);
	    if (itemCount == BUFFER_SIZE) {
	    	DEBUG('s', "*** Thread `%s` is waiting to produce\n",name);
	    	lockProducer->Release();
	    	// Con esto explota, porque puede ejecutarse un Signal antes de que este thread esté escuchando
	    	currentThread->Yield();
	        condProducer->Wait();
	    } else {
	    	lockProducer->Release();
	    }
	    // produceItem
		itemCount++;
		DEBUG('s', "*** Thread `%s` release lockProducer. ItemCount %d. Iterations pending %d\n", name, itemCount, iterations - i - 1);
		// lockProducer->Release();
		
	    if (itemCount > 0) {
		    DEBUG('s', "*** Thread `%s` send signal. ItemCount > 0\n",name);
	        condConsumer->Signal();
	    }
	    currentThread->Yield();
	}
	DEBUG('s', "*** Thread `%s` finish\n", name);
	done++;
}

void consumer(char *name) {
	int iterations = 10;
	for(int i = 0; i < iterations; i++) {
		lockConsumer->Acquire();
		DEBUG('s', "*** Thread `%s` acquired lockConsumer\n",name);
	    if (itemCount == 0) {
	    	DEBUG('s', "*** Thread `%s` is waiting to consume\n",name);
	        lockConsumer->Release();
	    	// Con esto explota, porque puede ejecutarse un Signal antes de que este thread esté escuchando
	    	currentThread->Yield();
	        condConsumer->Wait();
	    } else {
	    	lockConsumer->Release();
	    }
	    // consumeItem
	    itemCount--;

		DEBUG('s', "*** Thread `%s` release lockConsumer. ItemCount %d. Iterations pending %d\n", name, itemCount, iterations - i - 1);
		// lockConsumer->Release();

	    if (itemCount < BUFFER_SIZE) {
		    DEBUG('s', "*** Thread `%s` send signal. ItemCount < BUFFER_SIZE\n",name);
	        condProducer->Signal();
	    }
		currentThread->Yield();
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
    itemCount = 0;
	
	lockForCondConsumer = new Lock("lockForCondConsumer");
	lockForCondProducer = new Lock("lockForCondProducer");

	condConsumer = new Condition("condConsumer", lockForCondConsumer);
	condProducer = new Condition("condProducer", lockForCondProducer);

	lockProducer = new Lock("producer");
	lockConsumer = new Lock("consumer");

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
	lockConsumer->~Lock();
	lockProducer->~Lock();
	lockForCondConsumer->~Lock();
	lockForCondProducer->~Lock();
	condConsumer->~Condition();
	condProducer->~Condition();
	delete threadsProducers;
	delete threadsConsumers;
}
	
