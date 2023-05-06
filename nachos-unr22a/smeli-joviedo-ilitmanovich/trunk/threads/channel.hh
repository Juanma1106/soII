// Channel
#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "lock.hh"
#include "condition.hh"

class Channel {
public:
	
	Channel();
	~Channel();
	
	void Send (int message);
	void Receive (int *message );
	
private:
	Lock *lockSender;
	Semaphore *semReceiver;
	Semaphore *semFinish;
	int myMessage;
};

#endif