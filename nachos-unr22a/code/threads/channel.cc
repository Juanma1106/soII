// Channel
#include "channel.hh"

Channel::Channel() {
	lockSender = new Lock("lockSender");
	semReceiver = new Semaphore("semReceiver", 0);
	semFinish= new Semaphore("semFinish", 0);
}

Channel::~Channel() {
	lockSender->~Lock();
	semReceiver->~Semaphore();
	semFinish->~Semaphore();
}

// Circuito:
// 1) Llega alguien que quiere enviar un mensaje, adquiere el lockSender
// 2) Escribimos mensaje en el buffer
// 3) Avisamos a quien va a recibir el mensaje que ya escribimos el mensaje
// 4) Esperamos a que el otro hilo lo reciba y nos avise
// 5) Soltamos el lockSender
void Channel::Send (int message) {
	lockSender->Acquire();
	myMessage = message;
  	semReceiver->V();
  	semFinish->P();
	lockSender->Release();
}

// Circuito:
// 1) Llega alguien que quiere recibir un mensaje, se queda esperando a que le avisen que hay un mensaje
// 2) Leemos el mensaje del buffer
// 3) Avisamos que ya leimos el mensaje
void Channel::Receive (int *message ) {
	semReceiver->P();
  	*message = myMessage;
  	semFinish->V();
}
