#include "syscall.h"

// deberíamos definir un max size?
// #define SIZE 200

int 
main(int argc, char **argv)
{
    int CONSOLE_OUTPUT = 0;
    // Valido que sean dos argumentos
    if (argc < 3) {
        Write("Argumentos invalidos\n", 30, CONSOLE_OUTPUT);
        Write("USO CORRECTO: $cp archivo_origen archivo_destino\n", 60, CONSOLE_OUTPUT);
        Exit(1);
    }
        
    char *inputFile = argv[1];
    char *outputFile = argv[2];

    int inputFileID = Open(inputFile);
    if (inputFileID < 1) {
        Write("El archivo de origen no existe\n", 50, CONSOLE_OUTPUT);
        Exit(1);
    }

    int outputFileID = Open(outputFile);
    if (outputFileID < 1) {
        Create(outputFile);
        outputFileID = Open(outputFile);
    }
    char buffer[1];
    int read = Read(buffer, 1, inputFileID);
    // tanto en el cat como en el cp, copiamos la idea de ir leyendo de a 1 caracter
    while (read == 1) {
        Write(buffer, 1, outputFileID);
        read = Read(buffer, 1, inputFileID);
    }
    Close(inputFileID);
    Close(outputFileID);

}