/// Copyright (c) 2019-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"

#define MAX_RETRY 3

void ReadBufferFromUser(int userAddress, char *outBuffer,
                        unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(outBuffer != nullptr);
    ASSERT(byteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
        int retry=0;
        while(machine->ReadMem(userAddress++, 1, &temp) == false || retry>MAX_RETRY){
            retry++;
            DEBUG('a', "Fallo en el acceso. ReadBufferFromUser\n");
        }
        *outBuffer = (unsigned char) temp;
    } while (count < byteCount);
}

bool ReadStringFromUser(int userAddress, char *outString, 
                        unsigned maxByteCount) {
    ASSERT(userAddress != 0);
    ASSERT(outString != nullptr);
    ASSERT(maxByteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
        int retry=0;
        while(machine->ReadMem(userAddress++, 1, &temp) == false || retry>MAX_RETRY){
            retry++;
            DEBUG('a', "Fallo en el acceso. ReadStringFromUser\n");
        }
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}


/*
/// * `addr` is the virtual address to write to.
/// * `size` is the number of bytes to be written (1, 2, or 4).
/// * `value` is the data to be written.
*/
void WriteBufferToUser(const char *buffer, int userAddress,
                       unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(buffer != nullptr);
    ASSERT(byteCount != 0);

    int temp = 0;
    do {
        int retry=0;
        while(machine->WriteMem(userAddress++, 1, buffer[temp]) == false || retry>MAX_RETRY){
            retry++;
            DEBUG('a', "Fallo en el acceso. WriteBufferToUser\n");
        }
        temp++;
    } while (temp < byteCount);
}

void WriteStringToUser(const char *string, int userAddress) {
    ASSERT(userAddress != 0);
    int temp = 0;
    do {
        int retry=0;
        while(machine->WriteMem(userAddress++, 1, string[temp]) == false || retry>MAX_RETRY) {
            retry++;
            DEBUG('a', "Fallo en el acceso. WriteStringToUser\n");
        }
        temp++;
    } while (*string++ != '\0');
}
