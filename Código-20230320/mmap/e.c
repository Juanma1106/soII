// Shared mapping, file-backed, two unrelated processes.
// Link with `-lrt`.

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE  16

int main(void)
{
    // Open the file.
    int fd = shm_open("/compartido", O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("shm_open");
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

    srand(time(NULL));
    for (;;) {
        printf("Read: %hhu.\n", ptr[0]);
        int x = rand() % 256;
        ptr[0] = x;
        printf("Wrote: %hhu.\n", x);
        sleep(1);
    }
    return 0;
}
