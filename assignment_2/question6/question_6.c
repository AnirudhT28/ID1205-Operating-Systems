#define _POSIX_C_SOURCE 199309L
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1000000

int num_threads = 0;
float *data_array; 
void *thread_func(void *arg); /* the thread function */
double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);

    /* Initialize an array of random values */
    data_array = (float *)malloc(ARRAY_SIZE * sizeof(float));
  

    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = (float)rand() / (float)RAND_MAX; // Random float in [0,1] resuing code from last assignment
    }

   
    double sum_serial = 0.0;
    double time_serial = 0.0;
    struct timespec start_serial, end_serial;

    clock_gettime(CLOCK_MONOTONIC, &start_serial); // start timer
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum_serial += data_array[i];
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_serial); //end serial timer
  

time_serial = (end_serial.tv_sec - start_serial.tv_sec) + 
              (end_serial.tv_nsec - start_serial.tv_nsec) / 1000000000.0;
    printf("Serial Sum = %.2f, time = %.5f \n", sum_serial, time_serial);

    /* Create a pool of num_threads workers and keep them in workers */
    pthread_t *workers = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    int *thread_ids = (int *)malloc(num_threads * sizeof(int)); 
    
    double time_parallel = 0.0;
    double sum_parallel = 0.0;
    struct timespec start_parallel, end_parallel;
    clock_gettime(CLOCK_MONOTONIC, &start_parallel); // start parallel timer

    for (int i = 0; i < num_threads; i++) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
    
        thread_ids[i] = i; 
        
       
        if (pthread_create(&workers[i], &attr, thread_func, &thread_ids[i]) != 0) {  // Pass the address of the ID to the thread
            perror("Failed to create thread");
        }
    }

    for (int i = 0; i < num_threads; i++) {
        void *retval;
        pthread_join(workers[i], &retval);
        
        if (retval != NULL) {
            sum_parallel += *(double*)retval; // sum results
            free(retval); // Free the memory inside threads
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_parallel); // end parallel timer



time_parallel = (end_parallel.tv_sec - start_parallel.tv_sec) + 
              (end_parallel.tv_nsec - start_parallel.tv_nsec) / 1000000000.0;


    printf("Parallel Sum = %.2f, time = %.5f \n", sum_parallel, time_parallel);

    /* free up resources properly */
    free(data_array);
    free(workers);
    free(thread_ids);

    return 0;
}

void *thread_func(void *arg) {
    /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
    int my_id = *(int*)arg;

    /* Calculate bounds for this thread */
    int chunk_size = ARRAY_SIZE / num_threads;
    int start_index = my_id * chunk_size;
    int end_index = start_index + chunk_size;

    /* Handle the remainder for the last thread */
    if (my_id == num_threads - 1) {
        end_index = ARRAY_SIZE;
    }

    /* Perform Partial Parallel Sum Here */
    double *my_sum_ptr = (double *)malloc(sizeof(double)); 
    *my_sum_ptr = 0.0;

    for (int i = start_index; i < end_index; i++) {
        *my_sum_ptr += data_array[i];
    }
    pthread_exit((void*)my_sum_ptr);
}
