/// Copyright (c) 2019-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"


void ReadBufferFromUser(int userAddress, char *outBuffer,
                        unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(outBuffer != nullptr);
    ASSERT(byteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
        *outBuffer = (unsigned char) temp;
        outBuffer++;
    } while (count < byteCount);
}

bool checkEndString(char outString){
    return (outString != '\0' && outString != '\n');
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
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
        *outString = (unsigned char) temp;
    } while (checkEndString(*outString++) && count < maxByteCount);

    return *(outString - 1) == '\0'  || *(outString - 1) == '\n';
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

    unsigned int temp = 0;
    do {
        // DEBUG('e', "Escribiendo %d de %d .\n", temp, byteCount );
        ASSERT(machine->WriteMem(userAddress++, 1, buffer[temp]));
        temp++;
    } while (temp < byteCount);
}

void WriteStringToUser(const char *string, int userAddress) {
    ASSERT(userAddress != 0);
    if(string != nullptr){
        int temp = 0;
        do {
            ASSERT(machine->WriteMem(userAddress++, 1, string[temp]));
            temp++;
        } while (string[temp] != '\0');
    } else {
        printf("Puntero nulo \n");
    }
}
