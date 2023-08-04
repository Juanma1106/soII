/// Outputs arguments entered on the command line.

// #include "syscall.h"
#include "../userprog/syscall.h"
#include "lib.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
    //Halt();
    for (unsigned i = 1; i < argc; i++) {

        if (i != 1) {
            PrintChar(' ');
        }
        PrintString(argv[i]);
    }
    PrintChar('\n');
    Halt();
}
