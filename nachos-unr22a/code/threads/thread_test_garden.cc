/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_garden.hh"
#include "system.hh"
#include "semaphore.hh"
#include "lock.hh"

#include <stdio.h>


static const unsigned NUM_TURNSTILES = 2;
static const unsigned ITERATIONS_PER_TURNSTILE = 50;
static bool done[NUM_TURNSTILES];
static int count;
static Semaphore *mySemaphore;
static Lock *l;

static void Turnstile(void *n_) {
    unsigned *n = (unsigned *) n_;

    for (unsigned i = 0; i < ITERATIONS_PER_TURNSTILE; i++) {
        // int temp = count;
        currentThread->Yield();
        // count = temp + 1;
        count++;
    }
    printf("Turnstile %u finished. Count is now %u.\n", *n, count);
    done[*n] = true;
    delete n;
}

void ThreadTestGarden() {
    // Launch a new thread for each turnstile.
    for (unsigned i = 0; i < NUM_TURNSTILES; i++) {
        printf("Launching turnstile %u.\n", i);
        char *name = new char [16];
        sprintf(name, "Turnstile %u", i);
        unsigned *n = new unsigned;
        *n = i;
        Thread *t = new Thread(name);
        t->Fork(Turnstile, (void *) n);
    }

    // Wait until all turnstile threads finish their work.  `Thread::Join` is
    // not implemented at the beginning, therefore an ad-hoc workaround is
    // applied here.
    for (unsigned i = 0; i < NUM_TURNSTILES; i++) {
        while (!done[i]) {
            currentThread->Yield();
        }
    }
    printf("All turnstiles finished. Final count is %u (should be %u).\n",
           count, ITERATIONS_PER_TURNSTILE * NUM_TURNSTILES);
}

/////////////////////////
// SEMAPHORES
/////////////////////////

static void TurnstileSemaphores(void *n_) {
    unsigned *n = (unsigned *) n_;

    for (unsigned i = 0; i < ITERATIONS_PER_TURNSTILE; i++) {
	    DEBUG('s', "Turnstile %u want make P. Count is now %u.\n", *n, count);
	    // DEBUG('s', "%s, %s", currentThread->GetName(), l->GetName());
        
	    l->Acquire();
        DEBUG('s', "Turnstile %u make P. Count is now %u.\n", *n, count);
        int temp = count;
        currentThread->Yield();
        count = temp + 1;
	    l->Release();
	    DEBUG('s', "Turnstile %u make V. Count is now %u.\n", *n, count);
	    currentThread->Yield();
    }
    printf("Turnstile %u finished. Count is now %u.\n", *n, count);
    done[*n] = true;
    delete n;
}

void ThreadTestGardenSemaphores() {
	l = new Lock("lock");
	// Launch a new thread for each turnstile.
    for (unsigned i = 0; i < NUM_TURNSTILES; i++) {
        printf("Launching turnstile %u.\n", i);
        char *name = new char [16];
        sprintf(name, "Turnstile %u", i);
        unsigned *n = new unsigned;
        *n = i;
        Thread *t = new Thread(name);
        t->Fork(TurnstileSemaphores, (void *) n);
    }

    // Wait until all turnstile threads finish their work.  `Thread::Join` is
    // not implemented at the beginning, therefore an ad-hoc workaround is
    // applied here.
    for (unsigned i = 0; i < NUM_TURNSTILES; i++) {
        while (!done[i]) {
            currentThread->Yield();
        }
    }
    printf("All turnstiles finished. Final count is %u (should be %u).\n",
           count, ITERATIONS_PER_TURNSTILE * NUM_TURNSTILES);
}
