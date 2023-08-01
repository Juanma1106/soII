#include "write_read_mem_test.hh"

void WriteReadMemTest() {
    char i[50] = {} ;
    WriteStringToUser("hola\0", 10);

    ReadStringFromUser(10, i, 50);
    DEBUG('t', "Read from user %s\n", i);
}