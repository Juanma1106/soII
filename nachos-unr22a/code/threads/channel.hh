// Channel
#include "lock.hh"
#include "condition.hh"

class Channel {
public:
	
	Channel();
	~Channel();
	
	void Send (int message);
	void Receive (int *message );
	
private:
	Lock *lockForCondSender;
	Lock *lockForCondReceiver;
	Condition *condSender;
	Condition *condReceiver;
	int countReceiveWaiting;
	int countSendWaiting;
	Lock *lockSender;
	Lock *lockReceiver;
	Semaphore *semRead;
	Semaphore *semWrite;
	int myMessage;
};
