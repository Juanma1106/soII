// Channel
#include "channel.hh"

Channel::Channel() {
	lockForCondSender = new Lock("lockForCondSender");
	lockForCondReceiver = new Lock("lockForCondReceiver");
	condSender = new Condition("condSender", lockForCondSender);
	condReceiver = new Condition("condReceiver", lockForCondReceiver);
	countReceiveWaiting = 0;
	countSendWaiting = 0;
	lockSender = new Lock("lockSender");
	lockReceiver = new Lock("lockReceiver");
	
	semRead = new Semaphore("semWrite", 0);
	semWrite = new Semaphore("semWrite", 0);
	
	//lockRead = new Lock("lockRead");
	//condRead = new Condition("condRead", lockRead);
	
	//lockWrite = new Lock("lockWrite");
	//condWrite = new Condition("condWrite", lockWrite);
}

Channel::~Channel() {
	
}

// Circuito:
// 1)Llega alguien que quiere enviar un mensaje, adquiere el lockSender
// 2)
//	A- Si no hay nadie escuchando, incrementa countSendWaiting y se pone a esperar.
//		Cuando llega la señal de que alguien se puso a escuchar, continua en el paso 3
//	B- Si hay alguien escuchando, decrementa countReceiveWaiting y continua en el paso 3
// 3) Escribimos mensaje en el buffer
// 4) Avisamos a quien va a recibir el mensaje que ya escribimos el mensaje y esperamos a que el otro lo reciba
// 5) Soltamos el lockSender
void Channel::Send (int message) {
	lockSender->Acquire();
	if(countReceiveWaiting == 0) {
		// No hay nadie escuchando. Incrementamos los que envían mensaje en 1.
		countSendWaiting++;
		// Esperamos a que haya alguien escuchando del otro lado
		condSender->Wait();
	} else {
		// Ya hay alguien escuchando, decrementamos porque le vamos a enviar el mensaje
		countReceiveWaiting--;
	}
	
	// Escribimos mensaje en el buffer
	// TODO
	
	// Avisamos a quien va a recibir el mensaje que ya escribimos el mensaje
	semWrite->V();
	
	// Esperamos a que el otro lo reciba
	semRead->P();
	
	lockSender->Release();
}

// Circuito:
// 1)Llega alguien que quiere recibir un mensaje, adquiere el lockReceiver
// 2)
//	A- Si no hay nadie enviando mensaje, incrementa countReceiveWaiting.
//	B- Si hay alguien enviando mensaje, decrementa countSendWaiting y le avisa a alguno de los que escribe que ya hay alguien escuchando.
// 3) Esperamos a que se guarde el mensaje en el buffer
// 4) Leemos el mensaje del buffer
// 5) Le avisamos al sender que ya leimos el mensaje.
// 6) Soltamos el lockReceiver
void Channel::Receive (int *message ) {
	lockReceiver->Acquire();
	if(countSendWaiting == 0) {
		// No hay nadie intentando enviar mensaje. Incrementamos los que esperan mensajes en 1.
		countReceiveWaiting++;
	} else {
		// Ya hay un Sender intentado enviar mensaje, lo decrementamos.
		countSendWaiting--;
		// Avisarle a sender que ya está escuchando
		condSender->Signal();
	}
		
	// Esperamos a que se guarde el mensaje en el buffer. (Usamos semaforo en vez de condition para evitar que se pierda un signal)
	semWrite->P();

	// Leemos el mensaje del buffer
	// TODO

	// Le avisamos al sender que ya leimos el mensaje.
	semRead->V();

	
	lockReceiver->Release();
}



// Send 1		==> countSendWaiting = 1 y espero
// Receive 1 	==> countSendWaiting = 0, señal para que send escriba el mensaje y espero para leer mensaje
// Send 1		==> escribo mensaje, envío señal a Receive avisando que ya puede leer el mensaje y me quedo esperando a que receive lea el mensaje
// Receive 1	==> Lee el mensaje y envía senal a Receive avisando que ya leyó el mensaje

// Receive 1 	==> countReceiveWaiting = 1 y espero
// Send 1		==> countReceiveWaiting = 0 y envía señal.
// Receive 1

