// Shared mapping, file-backed, two related processes.

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PATH  "compartido.txt"
#define SIZE  16

static void do_child(char *ptr)
{
    srand(time(NULL));
    char x = rand() % 256;
    ptr[0] = x;
    printf("Child wrote: %hhu.\n", x);
}

static void do_parent(char *ptr)
{
    wait(NULL);
    printf("Parent read: %hhu.\n", ptr[0]);
}

int main(void)
{
    // Open the file.
    int fd = open(PATH, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Adjust file size.
    int rv = ftruncate(fd, SIZE);
    if (rv == -1) {
        perror("ftruncate");
        return 2;
    }

    // Map file into memory.
    char *ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 3;
    }
    do_parent(ptr);

    // Fork process.
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 4;
    } else if (child == 0) {
        do_child(ptr);
    } else {
        do_parent(ptr);
    }

    return 0;
}
