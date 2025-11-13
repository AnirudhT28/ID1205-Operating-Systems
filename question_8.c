#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <time.h>       
#include <sys/wait.h>   
#include <sys/types.h>

int main(int argcount, char *arglist[]) {

if (argcount != 2) { // if there is no function argument
        fprintf(stderr, "include function argument", arglist[0]);
        exit(1); // Exit with an error
    }
    
int N = atoi(arglist[1]); // take the value from arglist[1] from the argument and convert it into an integer 


}