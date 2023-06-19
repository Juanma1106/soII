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
    ASSERT(!IsHeldByCurrentThread()) ;
    if(lockerThread != nullptr) {
        int priorityLockerThread = lockerThread->getPriority();
        int priorityCurrentThread = currentThread->getPriority();
        if(priorityCurrentThread > priorityLockerThread) {
            lockerThread->setPriorityTemp(priorityCurrentThread);
            if(scheduler->isReady(lockerThread, priorityLockerThread)) {
                scheduler->removeFromPriorityList(lockerThread, priorityLockerThread);
                scheduler->addToPriorityList(lockerThread, priorityCurrentThread);
            }
        }
    }
    s->P();
    DEBUG('s', "*** Thread `%s` acquired %s\n", currentThread->GetName(), GetName());
    lockerThread = currentThread;
}



void Lock::Release() {
    ASSERT(IsHeldByCurrentThread()) ;
    if(lockerThread->getPriorityTemp() != -1) {
        int priorityTemp = lockerThread->getPriorityTemp(); // original del thread
        int priority     = lockerThread->getPriority();     // modificada por inversion
        // No hace falta chequear si el hilo está ready, ya que lockerThread es currentThread
        // y por lo tanto está ejecutando (es decir está ready)
        scheduler->removeFromPriorityList(lockerThread, priority);
        scheduler->addToPriorityList(lockerThread, priorityTemp);
        lockerThread->setPriority(priorityTemp);
        lockerThread->setPriorityTemp(-1);
    }
    lockerThread = nullptr;
    s->V();
    DEBUG('s', "*** Thread `%s` released %s\n", currentThread->GetName(), GetName());
}

bool Lock::IsHeldByCurrentThread() const {
    return lockerThread == currentThread;
}
