#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h> // Required for intptr_t

int num_threads = 0;
pthread_mutex_t stack_lock = PTHREAD_MUTEX_INITIALIZER;
int node_counter = 0;

typedef struct node {
    int node_id;
    struct node *next;
} Node;

Node *top = NULL; // Initialize top to NULL

/* Helper Functions (Added these to fix implicit declaration errors) */
void print_remaining_nodes() {
    Node *current = top;
    if (current == NULL) {
        printf("Stack is empty.\n");
        return;
    }
    while (current != NULL) {
        printf("Node ID: %d\n", current->node_id);
        current = current->next;
    }
}

void cleanup_stack() {
    Node *current = top;
    Node *next_node;
    while (current != NULL) {
        next_node = current->next;
        free(current);
        current = next_node;
    }
    top = NULL; // Reset top
}

/* Option 1: Mutex Lock */
void push_mutex() {
    Node *old_node;
    Node *new_node;

    new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
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

int pop_mutex() {
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

/* Option 2: Compare-and-Swap (CAS) */
void push_cas() {
    Node *old_node;
    Node *new_node;
    new_node = malloc(sizeof(Node));

    if (new_node == NULL) {
        perror("malloc failed");
        return;
    }

    // Assign unique ID atomically
    new_node->node_id = __sync_fetch_and_add(&node_counter, 1);

    do {
        old_node = top;
        new_node->next = old_node;
        // Loop while the CAS operation FAILS
    } while (!__sync_bool_compare_and_swap(&top, old_node, new_node));
}

int pop_cas() {
    Node *old_node;
    Node *new_node;
    int id = -1;

    while (1) {
        old_node = top;
        
        // Check if stack is empty
        if (old_node == NULL) {
            return -1; 
        }

        new_node = old_node->next;

        // Try to swap top with next node
        if (__sync_bool_compare_and_swap(&top, old_node, new_node)) {
            id = old_node->node_id;
            // Note: Freeing memory in CAS is complex (ABA problem). 
            // For this assignment logic, we free, but in production, 
            // this requires hazard pointers.
            free(old_node); 
            return id;
        }
        // If swap failed, loop retries with updated 'top'
    }
}

/* The thread function */
// FIXED: Signature must be void *func(void *arg)
void *thread_func(void *arg) {
    // Cast the void* argument back to int
    int opt = (int)(intptr_t)arg;
    
    /* NOTE: The prompt code did not pass a unique thread ID (like i), 
       it only passed the option (0 or 1). So 'my_id' cannot be 
       determined here without changing main(). 
       We will print the option instead to satisfy the compiler.
    */

    if (opt == 0) {
        push_mutex();
        push_mutex();
        pop_mutex();
        pop_mutex();
        push_mutex();
    } else {
        push_cas();
        push_cas();
        pop_cas();
        pop_cas();
        push_cas();
    }

    // printf("Thread exit\n"); // Removed my_id printing as it was uninitialized
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }
    
    num_threads = atoi(argv[1]);

    pthread_t *workers = malloc(sizeof(pthread_t) * num_threads);
    if (workers == NULL) {
        perror("malloc failed for workers");
        return 1;
    }

    /* Option 1: Mutex */
    printf("--- Testing Mutex ---\n");
    node_counter = 0; // Reset counter
    
    for (int i = 0; i < num_threads; i++) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        // Pass 0 as (void*)
        pthread_create(&workers[i], &attr, thread_func, (void *)(intptr_t)0);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(workers[i], NULL);
    }

    printf("Mutex: Remaining nodes \n");
    print_remaining_nodes();
    cleanup_stack();

    /* Option 2: CAS */
    printf("\n--- Testing CAS ---\n");
    node_counter = 0; // Reset counter for clean test

    for (int i = 0; i < num_threads; i++) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        // Pass 1 as (void*)
        pthread_create(&workers[i], &attr, thread_func, (void *)(intptr_t)1);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(workers[i], NULL);
    }

    printf("CAS: Remaining nodes \n");
    print_remaining_nodes();
    cleanup_stack();
    
    free(workers);
    return 0;
}
