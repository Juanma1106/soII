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
	Lock *lockSender;
	Semaphore *semReceiver;
	Semaphore *semFinish;
	int myMessage;
};
