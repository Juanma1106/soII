// Private mapping, file-backed.

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdio.h>

#define PATH  "hola.txt"

int main(void)
{
    // Open the file.
    int fd = open(PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Get the file size.
    struct stat buf;
    int rv = fstat(fd, &buf);
    if (rv == -1) {
        perror("fstat");
        return 2;
    }
    size_t size = buf.st_size;

    // Map file into memory.
    char *ptr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 3;
    }

    // Write file contents.
    printf("File contents:\n%*s", (int) size, ptr);

    return 0;
}
