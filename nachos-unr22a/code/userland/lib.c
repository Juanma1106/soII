#include "../userprog/syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "echo.c"

unsigned strlen(const char *s) {
    return StringLength(s);
}

int puts(const char *s) {
    return PrintString(s);
}

void itoa (int n , char *str) {
    
}

int main(int argc, char *argv[]) {

    for (unsigned i = 1; i < argc; i++) {

        if (i != 1) {
            PrintChar(' ');
        }
        PrintString(argv[i]);
    }
    PrintChar('\n');
}

