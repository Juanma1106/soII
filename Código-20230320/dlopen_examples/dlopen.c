/* From http://tldp.org/HOWTO/Program-Library-HOWTO/dl-libraries.html */

/* Compilar con "-ldl"  */

/* Puede ser necesario ajustar la ubicacion libm.so en la sig. linea */
#define LIBM "/lib/x86_64-linux-gnu/libm.so.6"

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

int main(int argc, char **argv) {
    void *handle;
    double (*cosine)(double);
    char *error;

    //handle = dlopen ("/lib/libm.so.6", RTLD_LAZY);
    handle = dlopen (LIBM, RTLD_LAZY);
    if (!handle) {
        fputs (dlerror(), stderr);
        exit(1);
    }

    cosine = dlsym(handle, "cos");
    if ((error = dlerror()) != NULL)  {
        fputs(error, stderr);
        exit(1);
    }

    printf ("%f\n", (*cosine)(2.0));
    dlclose(handle);
}


