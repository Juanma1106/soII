#include "thread_channel_test.hh"

static Channel *chan;

void ThreadChannelTest() {
	chan = new Channel();
	Thread *thread = new Thread("testChannel");
	thread->Fork(ReceiveSendFromChannel,(void *) (thread->GetName()) );
	int m = 1 ;
	for (int k=0; k < 10; k++){
	    DEBUG('s', "%s: Sent %d \n", currentThread->GetName(),m);
		chan->Send(m*2);
		chan->Receive(&m);
	}
}

void ReceiveSendFromChannel(void *name_){
	int m;
	for (int k=0; k < 10; k++){
		chan->Receive(&m);
	    DEBUG('s', "%s: Received %d\n", currentThread->GetName(),m);
		chan->Send(m *2);
	}
}
