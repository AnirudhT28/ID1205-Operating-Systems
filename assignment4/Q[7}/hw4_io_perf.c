#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

/* Global variables to match reference style logic */
int n_bytes = 0;           // buffer size from argv[1]
int p_threads = 0;         // number of threads from argv[2]
char *data_buffer;         // main memory buffer
int file_desc;             // file descriptor for I/O
int num_requests = 100;    // fixed number of requests

/* structure for request data */
typedef struct {
    long offset;
    int bytes;
} request_t;

request_t *current_list;   // pointer to whichever list we are currently processing

void *reader_thread_func(void *arg);
void *writer_thread_func(void *arg);

/* helper for time calculation */
double get_elapsed(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <buffer_size> <num_threads>\n", argv[0]);
        return 1;
    }

    n_bytes = atoi(argv[1]);
    p_threads = atoi(argv[2]);

    // @create a file for saving the data
    char *filename = "test_data.bin";

    // @allocate a buffer and initialize it
    data_buffer = (char *)malloc(n_bytes);
    if (!data_buffer) {
        perror("Malloc failed");
        return 1;
    }
    memset(data_buffer, 'B', n_bytes); // fill with dummy data

    // @create two lists of 100 requests in the format of [offset, bytes]
    request_t *list1 = (request_t *)malloc(num_requests * sizeof(request_t));
    request_t *list2 = (request_t *)malloc(num_requests * sizeof(request_t));

    // @List 1: sequtial requests of 16384 bytes
    for (int i = 0; i < num_requests; i++) {
        list1[i].offset = i * 16384;
        list1[i].bytes = 16384;
    }

    // @List 2: random requests of 128 bytes
    /* ensure no overlapping using a simple usage map */
    int max_slots = n_bytes / 4096;
    if (max_slots < 1) max_slots = 1;
    char *slot_used = (char *)calloc(max_slots, sizeof(char));
    
    srand(time(NULL));
    for (int i = 0; i < num_requests; i++) {
        int idx;
        int attempts = 0;
        do {
            idx = rand() % max_slots;
            attempts++;
        } while (slot_used[idx] && attempts < 1000); // simple collision check
        
        slot_used[idx] = 1;
        list2[i].offset = idx * 4096;
        list2[i].bytes = 128;
    }
    free(slot_used);

    /* shared variables for threading */
    pthread_t *workers = (pthread_t *)malloc(p_threads * sizeof(pthread_t));
    int *thread_ids = (int *)malloc(p_threads * sizeof(int));
    struct timeval start, end;
    double elapsed;
    double total_mb;


    // 1. Sequential Write (List 1)
   
    file_desc = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (file_desc < 0) { perror("open failed"); return 1; }
    current_list = list1; // set global pointer to list 1

    // @start timing 
    gettimeofday(&start, NULL);

    /* Create writer workers and pass in their portion of list1 */   
    for (int i = 0; i < p_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&workers[i], NULL, writer_thread_func, &thread_ids[i]);
    }
     
    /* Wait for all writers to finish */ 
    for (int i = 0; i < p_threads; i++) {
        pthread_join(workers[i], NULL);
    }

    // @close the file 
    fsync(file_desc); // ensure write to disk
    close(file_desc);

    // @end timing 
    gettimeofday(&end, NULL);
    elapsed = get_elapsed(start, end);
    
    // calculate size for list 1
    total_mb = (double)(num_requests * 16384) / (1024 * 1024);

    //@Print out the write bandwidth
    printf("List 1 (Sequential): Write %.2f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", 
            total_mb, p_threads, elapsed, total_mb / elapsed);


    // 2. Sequential Read (List 1)
 
    // @reopen the file 
    file_desc = open(filename, O_RDONLY);
    
    // @start timing 
    gettimeofday(&start, NULL);

    /* Create reader workers and pass in their portion of list1 */   
    for (int i = 0; i < p_threads; i++) {
        pthread_create(&workers[i], NULL, reader_thread_func, &thread_ids[i]);
    }
     
    /* Wait for all reader to finish */ 
    for (int i = 0; i < p_threads; i++) {
        pthread_join(workers[i], NULL);
    }

    // @close the file 
    close(file_desc);

    // @end timing 
    gettimeofday(&end, NULL);
    elapsed = get_elapsed(start, end);

    //@Print out the read bandwidth
    printf("List 1 (Sequential): Read %.2f MB, use %d threads, elapsed time %f s, read bandwidth: %f MB/s \n \n", 
            total_mb, p_threads, elapsed, total_mb / elapsed);


    // 3. Random Write (List 2)

    // @Repeat the write and read test now using List2 
    file_desc = open(filename, O_WRONLY); // open existing
    current_list = list2; // switch global pointer to list 2

    gettimeofday(&start, NULL);
    for (int i = 0; i < p_threads; i++) {
        pthread_create(&workers[i], NULL, writer_thread_func, &thread_ids[i]);
    }
    for (int i = 0; i < p_threads; i++) {
        pthread_join(workers[i], NULL);
    }
    fsync(file_desc);
    close(file_desc);
    gettimeofday(&end, NULL);
    
    elapsed = get_elapsed(start, end);
    total_mb = (double)(num_requests * 128) / (1024 * 1024); // recalc size for list 2

    printf("List 2 (Random): Write %.4f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", 
            total_mb, p_threads, elapsed, total_mb / elapsed);

    // 4. Random Read (List 2)
   
    file_desc = open(filename, O_RDONLY);
    
    gettimeofday(&start, NULL);
    for (int i = 0; i < p_threads; i++) {
        pthread_create(&workers[i], NULL, reader_thread_func, &thread_ids[i]);
    }
    for (int i = 0; i < p_threads; i++) {
        pthread_join(workers[i], NULL);
    }
    close(file_desc);
    gettimeofday(&end, NULL);
    
    elapsed = get_elapsed(start, end);

    printf("List 2 (Random): Read %.4f MB, use %d threads, elapsed time %f s, read bandwidth: %f MB/s \n", 
            total_mb, p_threads, elapsed, total_mb / elapsed);


    /*free up resources properly */
    free(data_buffer);
    free(list1);
    free(list2);
    free(workers);
    free(thread_ids);

    return 0;
}

void *reader_thread_func(void *arg) { 
    int my_id = *(int*)arg;
     
    // @Add code for reader threads
    // bound calculation for each thread, similar to reference logic
    int chunk_size = num_requests / p_threads;
    int start_index = my_id * chunk_size;
    int end_index = start_index + chunk_size;

    //remainder for the last thread
    if (my_id == p_threads - 1) {
        end_index = num_requests;
    }

    // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
    // @for each: read bytes_i from offset_i
    for (int i = start_index; i < end_index; i++) {
        long off = current_list[i].offset;
        int b = current_list[i].bytes;
        // pread is thread-safe, doesn't rely on file pointer position
        if (pread(file_desc, data_buffer + off, b, off) < 0) {
            perror("read error");
        }
    }

    pthread_exit(NULL);
}


void *writer_thread_func(void *arg) { 
    int my_id = *(int*)arg;
     
    // @Add code for writer threads
    // bound calculation for each thread
    int chunk_size = num_requests / p_threads;
    int start_index = my_id * chunk_size;
    int end_index = start_index + chunk_size;

    if (my_id == p_threads - 1) {
        end_index = num_requests;
    }

    // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
    // @for each: write bytes_i to offset_i
    for (int i = start_index; i < end_index; i++) {
        long off = current_list[i].offset;
        int b = current_list[i].bytes;
        // pwrite is thread-safe
        if (pwrite(file_desc, data_buffer + off, b, off) < 0) {
            perror("write error");
        }
    }

    pthread_exit(NULL);
}