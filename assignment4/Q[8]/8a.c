#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

int main() {
    const char *filepath = "file_to_map.txt";
    int fd = open(filepath, O_RDWR);
    
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    size_t size = 1024 * 1024; 

    void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (map == MAP_FAILED) {
        perror("Error mapping the file");
        close(fd);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } 
    else if (pid == 0) {
        printf("Child process (pid = %d); mmap address = %p\n", getpid(), map);
    } 
    else {
        printf("Parent process (pid = %d); mmap address = %p\n", getpid(), map);
        wait(NULL);
    }

    munmap(map, size);
    close(fd);

    return 0;
}
