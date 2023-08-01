/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_simple.hh"

static Semaphore *mySemaphore;
/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void SimpleThread(void *name_) {
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
#ifdef SEMAPHORE_TEST
	    mySemaphore->P();
	    DEBUG('s', "*** Thread `%s` makes P()\n",name);
#endif
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
#ifdef SEMAPHORE_TEST
        mySemaphore->V();
	    DEBUG('s', "*** Thread `%s` makes V()\n",name);
#endif
        currentThread->Yield();
    }
    printf("!!! Thread `%s` has finished\n", name);
}



/// Set up a ping-pong between several threads.
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void ThreadTestSimple() {
#ifdef SEMAPHORE_TEST
	mySemaphore = new Semaphore("MySemaphore", 3);
#endif
	char names[4][64] = {"2nd", "3rd", "4th", "5th"};
    currentThread->setPriority(0);
    Thread** allThreads = new Thread*[4];
	for(int i = 0; i < 4; i++){
		allThreads[i] = new Thread(names[i], true);
        allThreads[i]->setPriority(4-i);
		allThreads[i]->Fork(SimpleThread, (void *) names[i]);
	}
	SimpleThread((void *) "1st");

}

/// Set up a ping-pong between several threads.
/// Add join
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void ThreadTestSimpleWithJoin() {
#ifdef SEMAPHORE_TEST
	mySemaphore = new Semaphore("MySemaphore", 3);
#endif
	char names[4][64] = {"2nd", "3rd", "4th", "5th"};
    currentThread->setPriority(4);
    Thread** allThreads = new Thread*[4];
	for(int i = 0; i < 4; i++){
		allThreads[i] = new Thread(names[i], true);
        allThreads[i]->setPriority(4-i-1);
		allThreads[i]->Fork(SimpleThread, (void *) names[i]);
	}
	SimpleThread((void *) "1st");
    for(int i = 0; i < 4; i++){
		allThreads[i]->Join();
	}

}


