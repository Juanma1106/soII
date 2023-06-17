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


#include "lock.hh"
#include "system.hh"

/// Dummy functions -- so we can compile our later assignments.

Lock::Lock(const char *debugName) {
    s = new Semaphore (debugName, 1); 
    myThread = nullptr;
}

Lock::~Lock() {
    s->~Semaphore();
}

const char * Lock::GetName() const {
    return s->GetName();
}

void Lock::Acquire() {
    ASSERT(!IsHeldByCurrentThread()) ;
    if(myThread != nullptr) {
        int priorityThreadLocked = myThread->getPriority();
        int priorityCurrentThread = currentThread->getPriority();
        if(priorityCurrentThread > priorityThreadLocked) {
            /*
            La solución al problema de inversión de prioridades está mal, 
            modifican el valor numérico de la prioridad dentro de la estructura del thread, 
            pero en el scheduler sigue estando en la misma cola de prioridad y 
            la solución no es efectiva, tienen que sacar al thread de la cola de menor 
            prioridad y ubicarlo en la cola de mayor prioridad. 
            También tienen que tener en cuenta de restaurar la prioridad original una vez 
            que el thread de menor prioridad suelte el lock.
            */
            myThread->setPriorityTemp(priorityCurrentThread);
            // tienen que sacar al thread de la cola de menor 
            // prioridad y ubicarlo en la cola de mayor prioridad
            scheduler->removeFromPriorityList(myThread, priorityThreadLocked);
            scheduler->addToPriorityList(myThread, priorityCurrentThread);

        }
    }
    s->P();
    myThread = currentThread;
}


void Lock::Release() {
    ASSERT(IsHeldByCurrentThread()) ;
    myThread = nullptr;
    if(! (myThread->getPriorityTemp()== -1) ) {
        // También tienen que tener en cuenta de restaurar la prioridad original una vez 
        // que el thread de menor prioridad suelte el lock 
        int priorityTemp = myThread->getPriorityTemp(); // original del thread
        int priority     = myThread->getPriority();     // modificada por inversion
        scheduler->removeFromPriorityList(myThread, priority );
        scheduler->addToPriorityList(myThread, priorityTemp);
        myThread->setPriority(priorityTemp);
        myThread->setPriorityTemp(-1);
    }
    s->V();
}

bool Lock::IsHeldByCurrentThread() const {
    return myThread == currentThread;
}
