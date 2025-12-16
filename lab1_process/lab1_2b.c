#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main( int argc, char *argv[], char *env[] )
{
    int x =1;
    if(fork()==0)
	    printf("printf1: x=%d\n", ++x);
    printf("printf2: x=%d\n", --x);
    return 0;
}
