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
#ifdef USE_TLB
        if(!machine->ReadMem(userAddress++, 1, &temp)) {
            // Se genero un fallo en la TLB.
            // ver de reintentar tmb este
            ASSERT(machine->ReadMem(userAddress++, 1, &temp));
        }
#else
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
#endif
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
#ifdef USE_TLB
/*
        if(!machine->ReadMem(userAddress++, 1, &temp)) {
            ASSERT(machine->ReadMem(userAddress++, 1, &temp));
        } 
*/
        for(int i=0;i<5 || machine->ReadMem(userAddress++, 1, &temp)==false; i++); 
#else
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
#endif
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
#ifdef USE_TLB
/*
        if(!machine->WriteMem(userAddress++, 1, buffer[temp])) {
            ASSERT(machine->WriteMem(userAddress++, 1, buffer[temp]));
        }
*/
        for(int i=0; i<5 || (machine->WriteMem(userAddress++, 1, buffer[temp])==false); i++);
#else
        ASSERT(machine->WriteMem(userAddress++, 1, buffer[temp]));
#endif
        temp++;
    } while (temp < byteCount);
}

void WriteStringToUser(const char *string, int userAddress) {
    ASSERT(userAddress != 0);
    int temp = 0;
    do {
#ifdef USE_TLB
        if(!machine->WriteMem(userAddress++, 1, string[temp])) {
            ASSERT(machine->WriteMem(userAddress++, 1, string[temp]));
        }
#else
        ASSERT(machine->WriteMem(userAddress++, 1, string[temp]));
#endif
        temp++;
    } while (*string++ != '\0');
}
