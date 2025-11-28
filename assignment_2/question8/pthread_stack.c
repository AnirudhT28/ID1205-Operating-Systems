#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h> 

int num_threads = 0;
pthread_mutex_t stack_lock = PTHREAD_MUTEX_INITIALIZER;
int node_counter = 0;

typedef struct node {
    int node_id;
    struct node *next;
} Node;

Node *top = NULL; 

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
    top = NULL; 
}

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


void push_cas() {
    Node *old_node;
    Node *new_node;
    new_node = malloc(sizeof(Node));

    if (new_node == NULL) {
        perror("malloc failed");
        return;
    }

   
    new_node->node_id = __sync_fetch_and_add(&node_counter, 1);

    do {
        old_node = top;
        new_node->next = old_node;
        
    } while (!__sync_bool_compare_and_swap(&top, old_node, new_node));
}

int pop_cas() {
    Node *old_node;
    Node *new_node;
    int id = -1;

    while (1) {
        old_node = top;
        
       
        if (old_node == NULL) {
            return -1; 
        }

        new_node = old_node->next;

        
        if (__sync_bool_compare_and_swap(&top, old_node, new_node)) {
            id = old_node->node_id;
            free(old_node); 
            return id;
        }
    }
}


void *thread_func(void *arg) {
  
    int opt = (int)(intptr_t)arg;
    

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

  
    printf("--- Testing Mutex ---\n");
    node_counter = 0; 
    
    for (int i = 0; i < num_threads; i++) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
       
        pthread_create(&workers[i], &attr, thread_func, (void *)(intptr_t)0);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(workers[i], NULL);
    }

    printf("Mutex: Remaining nodes \n");
    print_remaining_nodes();
    cleanup_stack();

   
    printf("\n--- Testing CAS ---\n");
    node_counter = 0;

    for (int i = 0; i < num_threads; i++) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        
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
