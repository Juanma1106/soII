/// Outputs arguments entered on the command line.

// #include "syscall.h"
#include "../userprog/syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned StringLength(const char *s) {
    // What if `s` is null?

    unsigned i;
    for (i = 0; s[i] != '\0'; i++) {}
    return i;
}

int PrintString(const char *s) {
    // What if `s` is null?

    unsigned len = StringLength(s);
    return Write(s, len, CONSOLE_OUTPUT);
}

int PrintChar(char c) {
    return Write(&c, 1, CONSOLE_OUTPUT);
}

int main(int argc, char *argv[]) {
	Exec("echo", 1, "hola");
    for (unsigned i = 1; i < argc; i++) {

        if (i != 1) {
            PrintChar(' ');
        }
        PrintString(argv[i]);
    }
    PrintChar('\n');
    Halt();
}
