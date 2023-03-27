// Channel

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
	
	// Escribimos mensaje en buffer
	// TODO
	
	// Avisamos al otro proceso que ya escribimos el mensaje
	semWrite->V();
	
	// Esperamos a que el otro lo reciba
	semRead->P();
	
	lockSender->Release();
}
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
		
	// Esperar a que sender escriba el mensaje. Usamos semaforo en vez de condition para evitar que se pierda un signal.
	semWrite->P();

	// Leer mensaje en buffer		
	// TODO

	// Avisarle a sender que ya leimos el mensaje
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

