/* Compilar con "-ldl */
/* Antes de ejecutar compilar lhw, ver comentarios en lhw.c */

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

int main(int argc, char **argv) {
    void *handle;
    void (*helloworld)();
    char *error;

    handle = dlopen ("lhw.so.1.0.1", RTLD_LAZY);
    if (!handle) {
        fputs (dlerror(), stderr);
        exit(1);
    }

    helloworld = dlsym(handle, "helloworld");
    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        exit(1);
    }

    (*helloworld)();
    dlclose(handle);
}


