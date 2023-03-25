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
#include "semaphore.hh"
#include "system.hh"

static Semaphore *s;
static const char *isLockedBy = "";

/// Dummy functions -- so we can compile our later assignments.

Lock::Lock(const char *debugName) 
{
    s = new Semaphore (debugName,1); 

}

Lock::~Lock()
{
    s->~Semaphore();
}

const char *
Lock::GetName() const
{
    return s->GetName();
}

const char *
Lock::GetLockedBy() const
{
    return isLockedBy;
}

void
Lock::Acquire()
{
    ASSERT(!IsHeldByCurrentThread()) ;
    s->P();
    isLockedBy = currentThread->GetName();
}

void
Lock::Release()
{
    ASSERT(IsHeldByCurrentThread()) ;
    s->V();
    isLockedBy = "";
}

bool
Lock::IsHeldByCurrentThread() const
{
    return isLockedBy == currentThread->GetName();
}
