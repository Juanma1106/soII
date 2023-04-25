#include "thread_comensales_test.hh"

static Lock *l;

void ThreadComensalesTest() {
    
	l = new Lock();
	Thread *thread = new Thread("testChannel");
	thread->Fork(ReceiveSendFromChannel,(void *) (thread->GetName()) );
	// int *message;
	int m = 1 ;
	for (int k=0; k < 10; k++){
	    DEBUG('s', "%s: Sent %d \n", currentThread->GetName(),m);
		chan->Send(m*2);
		chan->Receive(&m);
	}
	
	
}

void ReceiveSendFromChannel(void *name_){
	// int *message;
	int m;
	for (int k=0; k < 10; k++){
		chan->Receive(&m);
		// m = *message;
	    DEBUG('s', "%s: Received %d\n", currentThread->GetName(),m);
		chan->Send(m *2);
	}
}
