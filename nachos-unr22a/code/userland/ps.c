/// Outputs arguments entered on the command line.

// #include "syscall.h"
#include "../userprog/syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	PrintProcesses();
}
