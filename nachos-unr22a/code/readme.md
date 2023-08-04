
4. El timer que se setea en system.cc, toma un argumento randomYield que viene por defecto en false (excepto caso -rs)
Le modificamos para que si hay programas de usuario #ifdef USER_PROGRAM, se setee siempre en true.
El constructor del Timer, planifica una interrupción con la función TimeOfNextInterrupt. Por defecto era 100, lo pusimos en 25.

Nos restan terminar los ejercicios 5 y 6.
Pudimos ejecutar los programas y hacer algunas funciones en lib.c, pero al no poder manejar los argumentos se nos complican los otros ejercicios.
Seguimos investigando, pero cualquier ayuda nos vendría genial.
