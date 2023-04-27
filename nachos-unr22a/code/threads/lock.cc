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

    // if(myThread != nullptr) {
    //     DEBUG('v', "Thread %s wants to acquired lock. Lock %s is acquired by %s\n", currentThread->GetName(), GetName(), myThread->GetName());
    // } else {
    //     DEBUG('v', "Thread %s wants to acquired lock. Lock %s is free\n", currentThread->GetName(), GetName());
    // }
    

    ASSERT(!IsHeldByCurrentThread()) ;
    if(myThread != nullptr) {
        int priorityTheadLocked = myThread->getPriority();
        int priorityCurrentThread = currentThread->getPriority();
        if(priorityCurrentThread > priorityTheadLocked) {
            myThread->setPriorityTemp(priorityCurrentThread);
        }
    }
    s->P();
    myThread = currentThread;
}

void Lock::Release() {
    // if(myThread != nullptr) {
    //     DEBUG('v', "Thread %s wants to release lock. Lock %s is acquired by %s\n", currentThread->GetName(), GetName(), myThread->GetName());
    // } else {
    //     DEBUG('v', "Thread %s wants to release lock. Lock %s is not acquired by anyone\n", currentThread->GetName(), GetName());
    // }
    ASSERT(IsHeldByCurrentThread()) ;
    myThread = nullptr;
    s->V();
}

bool Lock::IsHeldByCurrentThread() const {
    return myThread == currentThread;
}
