// Private mapping, anonymous.

#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>

#define SIZE  32

int main(void)
{
    // Map a new region into memory.
    char *ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Read and write in the region.
    for (unsigned i = 0; i < SIZE; i++) {
        int x = rand() % 256;
        ptr[i] = x;
        printf("Value at ptr[%u]: %hhu\n", i, ptr[i]);
    }

    return 0;
}
