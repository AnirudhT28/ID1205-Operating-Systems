#define _GNU_SOURCE  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <number of pages>\n", argv[0]);
        return 1;
    }

    int num_pages = atoi(argv[1]);
    int page_size = getpagesize(); 
    size_t total_size = (size_t)num_pages * page_size;

    printf("Allocating %d pages of %d bytes (Total: %zu bytes)\n", num_pages, page_size, total_size);

    char *addr;

    // Start Timer 
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Option 1:
    addr = (char*) mmap(NULL, total_size, PROT_READ | PROT_WRITE, 
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);



    // Option 2

    // addr = (char*) mmap(NULL, total_size, PROT_READ | PROT_WRITE, 
    //                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);


    // Check for failure
    if (addr == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    char c = 'a';
    for(int i = 0; i < num_pages; i++) {
        addr[i * page_size] = c;
        c++;
    }

    //End timer
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate time
    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;

    printf("Elapsed time: %.9f seconds\n", elapsed);

    
    for(int i = 0; (i < num_pages && i < 16); i++) {
        printf("%c ", addr[i * page_size]);
    }
    printf("\n");

    munmap(addr, total_size);

    return 0;
}
