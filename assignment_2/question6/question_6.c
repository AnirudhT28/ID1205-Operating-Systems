#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1000000

/* Global variables to be accessible by threads */
int num_threads = 0;
float *data_array; // The array of data

void *thread_func(void *arg); /* the thread function */

/* Helper for timing */
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
    if (num_threads <= 0) {
        printf("Error: num_threads must be > 0\n");
        return 1;
    }

    /* Initialize an array of random values */
    data_array = (float *)malloc(ARRAY_SIZE * sizeof(float));
    if (data_array == NULL) {
        perror("Failed to allocate array");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = (float)rand() / (float)RAND_MAX; // Random float in [0,1]
    }

    /* Perform Serial Sum */
    double sum_serial = 0.0;
    double time_serial = 0.0;
    struct timespec start_serial, end_serial;

    // Timer Begin
    clock_gettime(CLOCK_MONOTONIC, &start_serial);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum_serial += data_array[i];
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_serial);
    // Timer End

    time_serial = get_time_diff(start_serial, end_serial);
    printf("Serial Sum = %.2f, time = %.5f \n", sum_serial, time_serial);

    /* Create a pool of num_threads workers and keep them in workers */
    pthread_t *workers = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    int *thread_ids = (int *)malloc(num_threads * sizeof(int)); // Memory to pass IDs safely
    
    double time_parallel = 0.0;
    double sum_parallel = 0.0;
    struct timespec start_parallel, end_parallel;

    // Timer Begin
    clock_gettime(CLOCK_MONOTONIC, &start_parallel);

    for (int i = 0; i < num_threads; i++) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        
        // Assign ID
        thread_ids[i] = i; 
        
        // Pass the address of the ID to the thread
        if (pthread_create(&workers[i], &attr, thread_func, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
        }
    }

    for (int i = 0; i < num_threads; i++) {
        void *retval;
        pthread_join(workers[i], &retval);
        
        // Aggregate results
        if (retval != NULL) {
            sum_parallel += *(double*)retval;
            free(retval); // Free the memory allocated inside thread_func
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_parallel);
    // Timer End

    time_parallel = get_time_diff(start_parallel, end_parallel);
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
    // allocating on heap to return safely
    double *my_sum_ptr = (double *)malloc(sizeof(double)); 
    *my_sum_ptr = 0.0;

    for (int i = start_index; i < end_index; i++) {
        *my_sum_ptr += data_array[i];
    }

    // Optional: Print debug info (comment out for cleaner output on large thread counts)
    // printf("Thread %d sum = %f (Range: %d - %d)\n", my_id, *my_sum_ptr, start_index, end_index);
    
    pthread_exit((void*)my_sum_ptr);
}
