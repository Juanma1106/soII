    #include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	
	int m[1000*1000*1000];
	
	int main(){
	    long i;
	    printf("Arranqué, durmiendo, pid: %d\n", getpid());
	    sleep(20);
	    printf("llenando matriz\n");
	    for (i=0;i<1000*1000*1000;i++){
	    	 m[i]=i;
	    }
	    printf("durmiendo\n");
	    sleep(20);
	    printf("Pidiendo memoria\n");
	    char *a = (char *) malloc(1000*1000*1000);
	    printf("malloc retorno: %p, durmiendo\n", a);
	    sleep(20);
	    printf("Usando memoria\n");
	    for (i=0;i<1000*1000*1000;i++){
	    	 a[i]=i;
	    }
	    printf("Fin, durmiendo\n");
	    sleep(20);
	    return 0;
	}
