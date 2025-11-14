#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <time.h>       
#include <sys/wait.h>   
#include <sys/types.h>
#include <math.h>
#include <string.h>     

#define READ_END 0
#define WRITE_END 1

int main(int argcount, char *arglist[]) {

    if (argcount != 2) { // check if there was any more or less than 1 argument passed
        fprintf(stderr, "Usage: %s <N>\n", arglist[0]);   // give out an error message
        exit(1); 
    }
    
    int N = atoi(arglist[1]); 
    if (N <= 0) {
        fprintf(stderr, "Error: N must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }
    
    double *array = (double *)malloc(N * sizeof(double)); // allocate memory for array of size N
    if (array == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    srand((unsigned int)time(NULL)); // seed the random function

     for (int i = 0; i < N; i++) {
         array[i] = (double)rand() / (double)RAND_MAX; // RAND_MAX is always > rand, cannot be 0 aswell
     }



    int pipe_1_fd[2]; // create pipes for each child process
    if (pipe(pipe_1_fd) == -1) {
        perror("pipe1 failed");
        exit(EXIT_FAILURE);
    }

    int pipe_2_fd[2]; 
    if (pipe(pipe_2_fd) == -1) {
        perror("pipe2 failed");
        exit(EXIT_FAILURE);
    }

    // Declare variables for PIDs and timing
    pid_t pid1, pid2;
    struct timespec start_time, end_time;

     // Start the timer *just before* process creation
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Fork the first child
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork1 failed");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) { // child 1
    
        close(pipe_1_fd[READ_END]);
        close(pipe_2_fd[READ_END]);
        close(pipe_2_fd[WRITE_END]);

        double sum1 = 0.0;
        int end_index = N / 2; // index at half of the array
        for (int i = 0; i < end_index; i++) {  // iterate and summate all values
            sum1 += array[i];
        }

        if (write(pipe_1_fd[WRITE_END], &sum1, sizeof(sum1)) == -1) {
            perror("Child 1 write failed");
        }

        close(pipe_1_fd[WRITE_END]);
        free(array);
        exit(EXIT_SUCCESS);
    }

    // Fork the second child
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork2 failed");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) { // child 2
    
        close(pipe_2_fd[READ_END]);
        close(pipe_1_fd[READ_END]);
        close(pipe_1_fd[WRITE_END]);

        double sum2 = 0.0;
        int start_index = N / 2;   
        for (int i = start_index; i < N; i++) {
            sum2 += array[i];
        }

        if (write(pipe_2_fd[WRITE_END], &sum2, sizeof(sum2)) == -1) {
            perror("Child 2 write failed");
        }

        close(pipe_2_fd[WRITE_END]);
        free(array);
        exit(EXIT_SUCCESS);
    }

    

    // Parent closes *all* write ends of the pipes
    close(pipe_1_fd[WRITE_END]);
    close(pipe_2_fd[WRITE_END]);

    // Wait for both children to terminate
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // Read the results from the pipes
    double child1_sum, child2_sum; // read from processes
    if (read(pipe_1_fd[READ_END], &child1_sum, sizeof(child1_sum)) == -1) {
        perror("Parent read from child 1 failed");
    }
    if (read(pipe_2_fd[READ_END], &child2_sum, sizeof(child2_sum)) == -1) {
        perror("Parent read from child 2 failed");
    }

  
    close(pipe_1_fd[READ_END]); // close read ends 
    close(pipe_2_fd[READ_END]);

    double total_sum = child1_sum + child2_sum;
   
    clock_gettime(CLOCK_MONOTONIC, &end_time); // stop timer after all processes and computation ends 

    // Calculate and print the final sum
   

    printf("\n--- Results ---\n");
    printf("Sum from Child 1:  %f\n", child1_sum);
    printf("Sum from Child 2: %f\n", child2_sum);
    printf("Total sum (parent): %f\n", total_sum);

    // Calculate elapsed time in seconds
    double time_elapsed = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("------------------------------------------\n");
    printf("Total time elapsed: %f seconds\n", time_elapsed);

  

    free(array); //free parent memory
    return 0;
}
