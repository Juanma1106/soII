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
char digitToChar(int n){
    return 65+(n-10);
}
void itoa (int n , char *s) {
    int pos = 0;
    int r ;
    while (n >= 10) {
        r = n%10;
        s[pos++] = digitToChar(r);
        n = (n - r)/10;
    }
    s[pos++] = n;
    s[pos] = '\0';
    // resta hacerle un reverse
}


int main(int argc, char *argv[]) {

/*
    for (unsigned i = 1; i < argc; i++) {

        if (i != 1) {
            PrintChar(' ');
        }
        PrintString(argv[i]);
    }
    PrintChar('\n');
*/    
    char *s;
    itoa(65841,s );
    puts(s);
}

