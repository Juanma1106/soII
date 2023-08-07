#include "syscall.h"
#include "lib.c"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        puts("rm: Debe haber 2 argumentos");
        return -1;
    }
    if (Remove(argv[1])) {
        puts("rm: cannot remove: No such file or directory");
    }
    return 0;
}
