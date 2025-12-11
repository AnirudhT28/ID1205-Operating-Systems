#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of pages>\n", argv[0]);
        return 1;
    }

    long n_pages = atol(argv[1]);
    int page_size = getpagesize(); 
    size_t total_size = n_pages * page_size;

    printf("Page size: %d bytes\n", page_size);
    printf("Allocating and initializing %ld pages (Total: %zu bytes)\n", n_pages, total_size);

    char *ptr = (char *)malloc(total_size);

    if (ptr == NULL) {
        perror("Malloc failed");
        return 1;
    }

    memset(ptr, 0, total_size);

    free(ptr);
    return 0;
}
