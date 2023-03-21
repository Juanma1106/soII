#include <stdio.h>
main(){
	char a = 3; 
	char b = 6; 
	char c = -1; 
	char d = -20; 
	unsigned char e = 255;
	printf("a: %hhd %hhd\n", a,a>>1);
	printf("b: %hhd %hhd\n", b,b>>1);
	printf("c: %hhd %hhd, como unsigned: %hhu %hhu\n", c,c>>1, c, c>>1);
	printf("d: %hhd %hhd\n", d,d>>1);
	printf("d: %hhu %hhu, como signed:   %hhd %hhd\n", e,e>>1, e, e>>1);
	return 0;
}
