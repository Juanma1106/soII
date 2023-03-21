/* Compillar con : 
 gcc -c lhw.c -fPIC -o lhw.o
 gcc   lhw.o -shared -o lhw.so.1.0.1 -lc 
*/

#include <stdio.h>
void helloworld(){
	printf("Hello world\n");
}
