/// Routines for synchronizing threads.
///
/// The implementation for this primitive does not come with base Nachos.
/// It is left to the student.
///
/// When implementing this module, keep in mind that any implementation of a
/// synchronization routine needs some primitive atomic operation.  The
/// semaphore implementation, for example, disables interrupts in order to
/// achieve this; another way could be leveraging an already existing
/// primitive.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "condition.hh"



/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Condition::Condition(const char *debugName, Lock *conditionLock)
{
    name = debugName;
    cl = conditionLock;
    sem = new Semaphore(debugName, 0);
    countWaiters = 0;
}

Condition::~Condition()
{
    cl->~Lock();
    sem->~Semaphore();
}

const char *
Condition::GetName() const
{
    return name;
}

void
Condition::Wait()
{
    /*
    En Wait el proceso debe soltar el lock, esperar a una señal que lo despierte y 
    tomar el lock de nuevo antes de salir.
    cl->Acquire();
    countWaiters++;
    cl->Release();
    sem->P();
    */
    ASSERT(!IsHeldByCurrentThread()) ;
    cl->Release();
    countWaiters++;
    sem->P();
    cl->Acquire();

}

void
Condition::Signal()
{
    /*
    Signal y Broadcast están bien pero no deben tomar y soltar el lock, 
    solo deben verificar que el thread que llame a estas funciones ya lo posea
    */
    ASSERT(!IsHeldByCurrentThread()) ;
    if(countWaiters > 0) {
		sem->V();
		countWaiters--;
	}
}

void
Condition::Broadcast()
{
    /* Signal y Broadcast están bien pero no deben tomar y soltar el lock, 
    solo deben verificar que el thread que llame a estas funciones ya lo posea
    cl->Acquire();
    while(countWaiters > 0) {
		sem->V();
		countWaiters--;
	}
    cl->Release();
    */
    ASSERT(!IsHeldByCurrentThread());
    while(countWaiters > 0) {
        sem->V();
		countWaiters--;
	}


}
