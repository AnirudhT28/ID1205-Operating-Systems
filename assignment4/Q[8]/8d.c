#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    const char *filepath = "file_to_map.txt";
    int fd = open(filepath, O_RDWR);
    if (fd < 0) return 1;

    size_t size = 1024 * 1024;
    char *mmaped_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    pid_t pid = fork();

    if (pid == 0) {
      
        memcpy(&mmaped_ptr[0], "01234", 5);
        msync(mmaped_ptr, size, MS_SYNC); 
        
        sleep(1);
        printf("child process (pid %d); read from mmaped_ptr[4096]:%.5s\n", getpid(), &mmaped_ptr[4096]);
    } 
    else {
        // PARENT: Write and Sync
        memcpy(&mmaped_ptr[4096], "56789", 5);
        msync(mmaped_ptr, size, MS_SYNC);
        
        wait(NULL); 
        printf("parent process (pid %d); read from mmaped_ptr[0]:%.5s\n", getpid(), &mmaped_ptr[0]);
    }

    munmap(mmaped_ptr, size);
    close(fd);
    return 0;
}
