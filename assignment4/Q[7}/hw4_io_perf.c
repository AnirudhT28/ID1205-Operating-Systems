#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
          

void *reader_thread_func(/*@input paramters*/) { 
     
     // @Add code for reader threads
     // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
     // @for each: read bytes_i from offset_i

     pthread_exit(0);
}


void *writer_thread_func(/*@input paramters*/) { 
     
     // @Add code for writer threads
     // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
     // @for each: write bytes_i to offset_i

     pthread_exit(0);
}


int main(int argc, char *argv[])
{
     int n = atoi(argv[1]); //number of bytes
     int p = atoi(argv[2]); //number of threads

     // @create a file for saving the data


     // @allocate a buffer and initialize it


     
     // @create two lists of 100 requests in the format of [offset, bytes]

     // @List 1: sequtial requests of 16384 bytes, where offset_n = offset_(n-1) + 16384
     // @e.g., [0, 16384], [16384, 16384], [32768, 16384] ...
     // @ensure no overlapping among these requests.


     // @List 2: random requests of 128 bytes, where offset_n = random[0,N/4096] * 4096
     // @e.g., [4096, 128], [16384, 128], [32768, 128], etc.
     // @ensure no overlapping among these requests.


     // @start timing 

     /* Create writer workers and pass in their portion of list1 */   
     
     
     /* Wait for all writers to finish */ 
     

     // @close the file 


     // @end timing 


     //@Print out the write bandwidth
     printf("Write %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", /**/);
     
     
     // @reopen the file 

     // @start timing 

     /* Create reader workers and pass in their portion of list1 */   
     
     
     /* Wait for all reader to finish */ 
     

     // @close the file 


     // @end timing 


     //@Print out the read bandwidth
     printf("Read %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", /**/);


     // @Repeat the write and read test now using List2 


     /*free up resources properly */
}
