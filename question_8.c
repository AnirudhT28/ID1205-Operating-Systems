#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <time.h>       
#include <sys/wait.h>   
#include <sys/types.h>
#include <math.h>

int main(int argcount, char *arglist[]) {

   
    if (argcount != 2) { // check if there was any more or less than 1 argument passed
      
       fprintf(stderr, "Usage: %s <N>\n", arglist[0]);   // give out an error message
        exit(1); 
    }
    
    int N = atoi(arglist[1]); 
    
    double *array = (double *)malloc(N * sizeof(double)); // allocate memory for array of size N
    

    srand((unsigned int)time(NULL)); // seed the random function

     for (int i = 0; i < N; i++) {
        array[i] = (double)rand() / (double)RAND_MAX; // RAND_MAX is always > rand, cannot be 0 aswell
     }








    // 3. This is the corrected print statement
 for (int i = 0; i < N; i++) {
        printf("%f ", array[i]);
    }

    free(array);
    return 0;
}