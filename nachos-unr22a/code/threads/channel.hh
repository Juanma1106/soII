// Channel

class Channel {
public:
	
	Channel();
	~Channel();
	
	void Send (int message);
	void Receive (int *message );
	
private
	Lock *lockForCondSender;
	Lock *lockForCondReceiver;
	Condition *condSender;
	Condition *condReceiver;
};
