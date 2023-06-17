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
    lockerThread = nullptr;
}

Lock::~Lock() {
    s->~Semaphore();
}

const char * Lock::GetName() const {
    return s->GetName();
}

void Lock::Acquire() {
    DEBUG('s', "*** Thread `%s` acquired %s\n", currentThread->GetName(), GetName());
    ASSERT(!IsHeldByCurrentThread()) ;
    if(lockerThread != nullptr) {
        int priorityLockerThread = lockerThread->getPriority();
        int priorityCurrentThread = currentThread->getPriority();
        if(priorityCurrentThread > priorityLockerThread) {
            /*
            La solución al problema de inversión de prioridades está mal, 
            modifican el valor numérico de la prioridad dentro de la estructura del thread, 
            pero en el scheduler sigue estando en la misma cola de prioridad y 
            la solución no es efectiva, tienen que sacar al thread de la cola de menor 
            prioridad y ubicarlo en la cola de mayor prioridad. 
            También tienen que tener en cuenta de restaurar la prioridad original una vez 
            que el thread de menor prioridad suelte el lock.
            */
            lockerThread->setPriorityTemp(priorityCurrentThread);
            // tienen que sacar al thread de la cola de menor 
            // prioridad y ubicarlo en la cola de mayor prioridad
            scheduler->removeFromPriorityList(lockerThread, priorityLockerThread);
            scheduler->addToPriorityList(lockerThread, priorityCurrentThread);

        }
    }
    s->P();
    lockerThread = currentThread;
}


void Lock::Release() {
    DEBUG('s', "*** Thread `%s` released %s\n", currentThread->GetName(), GetName());
    ASSERT(IsHeldByCurrentThread()) ;
    if(lockerThread->getPriorityTemp() != -1) {
        // También tienen que tener en cuenta de restaurar la prioridad original una vez 
        // que el thread de menor prioridad suelte el lock 
        int priorityTemp = lockerThread->getPriorityTemp(); // original del thread
        int priority     = lockerThread->getPriority();     // modificada por inversion
        scheduler->removeFromPriorityList(lockerThread, priority);
        scheduler->addToPriorityList(lockerThread, priorityTemp);
        lockerThread->setPriority(priorityTemp);
        lockerThread->setPriorityTemp(-1);
    }
    lockerThread = nullptr;
    s->V();
}

bool Lock::IsHeldByCurrentThread() const {
    return lockerThread == currentThread;
}
