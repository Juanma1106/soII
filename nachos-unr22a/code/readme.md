
4. El timer que se setea en system.cc, toma un argumento randomYield que viene por defecto en false (excepto caso -rs)
Le modificamos para que si hay programas de usuario #ifdef USER_PROGRAM siempre sea true.
El constructor del Timer, planifica una interrupción que ahora con la función TimeOfNextInterrupt.
Por defecto era 100, lo pusimos en 25.

Nos resta terminar el ejercicio 6 y testear a fondo toda la plancha.
De todas maneras, lo entregamos así y lo seguimos avanzando en paralelo.
