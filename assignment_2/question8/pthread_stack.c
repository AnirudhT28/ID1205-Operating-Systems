#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int num_threads = 0;
pthread_mutex_t stack_lock = PTHREAD_MUTEX_INITIALIZER;
int node_counter = 0;
typedef struct node
 {
    int node_id; //a unique ID assigned to each node
    struct node *next;

 } Node;

Node *top; // top of stack
/*Option 1: Mutex Lock*/
void push_mutex() 
{
    Node *old_node;
    Node *new_node;

    new_node = malloc(sizeof(Node));
    if (new_node == NULL) 
    {
        perror("malloc failed");
        return;
    }

   pthread_mutex_lock(&stack_lock); 

    new_node->node_id = node_counter; 
    node_counter++; 

    old_node = top;
    new_node->next = old_node; 
    top = new_node; 

    pthread_mutex_unlock(&stack_lock);
}

int pop_mutex() 
{
    int id = -1;
    Node *old_node;

    pthread_mutex_lock(&stack_lock);

    old_node = top; 

    if (old_node != NULL) {
    
        top = old_node->next; 
        id = old_node->node_id;
        free(old_node);
    } 

    pthread_mutex_unlock(&stack_lock);
    
    return id;
}

/*Option 2: Compare-and-Swap (CAS)*/
void push_cas() 
{
    Node *old_node;
    Node *new_node;
    new_node = malloc(sizeof(Node));
    
    if (new_node == NULL) {
        perror("malloc failed");
        return;
    }

    // Assign unique ID atomically
    new_node->node_id = __sync_fetch_and_add(&node_counter, 1); 

    // 1. Initial Read (Expected value)
    old_node = top; 
    
    // 2. Initial Link
    new_node->next = old_node; 

    // 3. Attempt the CAS. Loop while the CAS operation FAILS (returns FALSE).
    while (!__sync_bool_compare_and_swap(&top, old_node, new_node)) 
    {
        old_node = top; 
        new_node->next = old_node; 
    }
}
int pop_cas() 
{
    Node *old_node;
    Node *new_node;
    //update top of the stack below
    return old_node->node_id;
}
/* the thread function */
void *thread_func(int opt) 
{
/* Assign each thread an id so that they are unique in range [0, num_thread -1
] */
    int my_id;
    if(opt==0)
    {
        push_mutex();push_mutex();pop_mutex();pop_mutex();push_mutex();
    }
    else
    {
        push_cas();push_cas();pop_cas();pop_cas();push_cas();
    }
    
    printf("Thread %d: exit\n", my_id);
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    num_threads = atoi(argv[1]);

    pthread_t *workers = malloc(sizeof(pthread_t) * num_threads);
    if (workers == NULL) {
        perror("malloc failed for workers");
        return 1;
    }

    /* Option 1: Mutex */
    for (int i = 0; i < num_threads; i++) 
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&workers[i], &attr, thread_func, (void *)(intptr_t)0);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(workers[i], NULL);
    }
   
    //Print out all remaining nodes in Stack
    printf("Mutex: Remaining nodes \n");
    print_remaining_nodes();
    cleanup_stack();

    /*free up resources properly */
    /* Option 2: CAS */

    for (int i = 0; i < num_threads; i++)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&workers[i], &attr, thread_func, (void *)(intptr_t)1);
    }

    for (int i = 0; i < num_threads; i++)
    {
         pthread_join(workers[i], NULL);
    }
   
    //Print out all remaining nodes in Stack
    printf("CAS: Remaining nodes \n");
    /*free up resources properly */
    print_remaining_nodes();
    cleanup_stack();
    free(workers);
    return 0;    

}
