#define _POSIX_C_SOURCE 199309L
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define NUM_BINS 30

int num_threads = 0;
long array_size = 0;
double *data_array; 

void *thread_func(void *arg); /* the fucntion that each created thread executes individually */

void print_histogram(int *hist) { /*helper for printing histogram on terminal*/
    printf("Histogram:\n");
    for (int i = 0; i < NUM_BINS; i++) {
        printf("Bin %2d: %d\n", i, hist[i]);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <num_threads> <array_size>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);
    array_size = atol(argv[2]);

    /*random doubles values array */
    data_array = (double *)malloc(array_size * sizeof(double));

    srand(time(NULL));
    for (long i = 0; i < array_size; i++) {
        data_array[i] = (double)rand() / (double)RAND_MAX; // Random double in [0,1]
    }

    // --- Serial Histogram ---
    printf("--- Serial Calculation ---\n");
    int serial_hist[NUM_BINS] = {0};
    struct timespec start_serial, end_serial;

    clock_gettime(CLOCK_MONOTONIC, &start_serial); // start time for serially generating histogram
    
    for (long i = 0; i < array_size; i++) {
        int bin = (int)(data_array[i] * NUM_BINS);
        if (bin >= NUM_BINS) bin = NUM_BINS - 1; 
        serial_hist[bin]++;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_serial);// end time for serially generating histogram

    double time_serial = (end_serial.tv_sec - start_serial.tv_sec) + 
                         (end_serial.tv_nsec - start_serial.tv_nsec) / 1e9;

    print_histogram(serial_hist);
    printf("Serial time = %.5f seconds\n\n", time_serial);

    // --- Parallel Histogram ---
    printf("--- Parallel Calculation ---\n");
    pthread_t *workers = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    int *thread_ids = (int *)malloc(num_threads * sizeof(int)); 
    int parallel_hist[NUM_BINS] = {0};
    
    struct timespec start_parallel, end_parallel;
    clock_gettime(CLOCK_MONOTONIC, &start_parallel); // start time for parallelly generating histogram

    //creates an individual thread for each worker, that runs thread_func
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i; 
        if (pthread_create(&workers[i], NULL, thread_func, &thread_ids[i]) != 0) { 
            perror("Failed to create thread");
        }
    }

    for (int i = 0; i < num_threads; i++) {
        void *retval;
        pthread_join(workers[i], &retval);// pause main to wait for worker threads to finish execution
        // all returned values from finished threads are collected  + stored in the retval variable.

        if (retval != NULL) {
            int *partial_hist = (int *)retval;
            // Aggregate results
            for (int j = 0; j < NUM_BINS; j++) { // aggregate partial histogram results into final histogram
                parallel_hist[j] += partial_hist[j];
            }
            free(partial_hist); // Free the memory from the thread
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_parallel); // end time for parallelly generating histogram

    double time_parallel = (end_parallel.tv_sec - start_parallel.tv_sec) + 
                           (end_parallel.tv_nsec - start_parallel.tv_nsec) / 1e9;

    print_histogram(parallel_hist);
    printf("Parallel time = %.5f seconds\n", time_parallel);

    /* free up resources properly */
    free(data_array);
    free(workers);
    free(thread_ids);

    return 0;
}

void *thread_func(void *arg) {
    int my_id = *(int*)arg;

    // bound calculation for each thread
    long chunk_size = array_size / num_threads;
    long start_index = my_id * chunk_size;
    long end_index = start_index + chunk_size;

    //remainder for the last thread needs to be treated if the number of threads dont evenly divide the array size
    if (my_id == num_threads - 1) {
        end_index = array_size;
    }

    //partial histogram for current thread
    int *my_hist = (int *)calloc(NUM_BINS, sizeof(int));
    if (my_hist == NULL) {
        pthread_exit(NULL);
    }

    for (long i = start_index; i < end_index; i++) {
        int bin = (int)(data_array[i] * NUM_BINS);
        if (bin >= NUM_BINS) bin = NUM_BINS - 1; // Handle case where value is 1.0
        my_hist[bin]++;
    }
    
    pthread_exit((void*)my_hist);
}
