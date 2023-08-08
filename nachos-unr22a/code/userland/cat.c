#include "syscall.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        puts("cat: Debe haber 2 argumentos");
        return -1;
    }
    int OUTPUT = 0;
    int INPUT =  Open(argv[1]);
    char buff[1];
    int read = Read(buff, 1, INPUT);
    while (read == 1) {
        Write(buff, 1, OUTPUT);
        read = Read(buff, 1, INPUT);
    }
    buff[0] = '\n';
    Write(buff, 1, OUTPUT);
    return 0;
}