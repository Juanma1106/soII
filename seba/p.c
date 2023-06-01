#include <stdio.h>
#include <stdlib.h>

int b;

int main(){
    char a* = malloc(10);
    printf(" %p %p %p ", &b, main, a);
// b es una variable global, entonces va al .data (un-init-data)
// main es .code
// a esta en el heap

}


/*

con free o top se pueden ver los ocnsumos de cada proceso,
se puede ver cuanto swap hay utilizado, etc.

objdump para inspeccionar un .o
ldd para ver las librerias dinamicas del programa

-pic  Position Independent Code

echo $$ te muestra el pid de bash
con vi /prox/<pid>/maps se puede ver donde estan las direcciones

Vemos dlopen y mmap dentro de la carpeta de Codigo
no es necesario saber todos los comandos de mmap, pero sí hay que saber que se puede mapear y compartir memoria entre procesos

*/
