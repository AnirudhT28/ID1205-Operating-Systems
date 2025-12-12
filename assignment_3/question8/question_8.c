#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define REFERENCE_STRING_LENGTH 1000
#define ACTIVE_LIST_THRESHOLD 0.7
#define MOVE_COUNT_PERCENT 0.2
#define PLAYER_SLEEP_US 10

typedef struct page { 
     int page_id;
     int reference_bit;
     struct page *next;
} Page;

// Global shared data
Page *active_list_head = NULL;
Page *active_list_tail = NULL;
Page *inactive_list_head = NULL;
Page *inactive_list_tail = NULL;

int active_list_size = 0;
int *reference_string;
int *page_stats;
int N; // Total number of unique pages
int M; // Checker sleep time in microseconds

pthread_mutex_t list_mutex;
volatile int player_finished = 0;

// Helper function to find and remove a page from any list
Page* find_and_remove_page(int page_id) {
    Page *current = active_list_head;
    Page *prev = NULL;
    // Search in active list
    while (current != NULL && current->page_id != page_id) {
        prev = current;
        current = current->next;
    }
    if (current != NULL) { // IF Found in active list
        if (prev == NULL) active_list_head = current->next;
        else prev->next = current->next;
        if (active_list_tail == current) active_list_tail = prev;
        active_list_size--;
        current->next = NULL;
        return current;
    }

    // Search inside inactive list
    current = inactive_list_head;
    prev = NULL;
    while (current != NULL && current->page_id != page_id) {
        prev = current;
        current = current->next;
    }
    if (current != NULL) { // IF Found in inactive list
        if (prev == NULL) inactive_list_head = current->next;
        else prev->next = current->next;
        if (inactive_list_tail == current) inactive_list_tail = prev;
        current->next = NULL;
        return current;
    }
    return NULL; // Should not happen if lists are initialized correctly
}

// Helper function to add a page to the tail of the active list
void add_to_active_tail(Page *p) {
    p->next = NULL;
    if (active_list_head == NULL) {
        active_list_head = p;
        active_list_tail = p;
    } else {
        active_list_tail->next = p;
        active_list_tail = p;
    }
    active_list_size++;
}

// Helper function to add a page to the tail of the inactive list
void add_to_inactive_tail(Page *p) {
    p->next = NULL;
    if (inactive_list_head == NULL) {
        inactive_list_head = p;
        inactive_list_tail = p;
    } else {
        inactive_list_tail->next = p;
        inactive_list_tail = p;
    }
}

void *player_thread_func() { 
    for (int i = 0; i < REFERENCE_STRING_LENGTH; i++) {
        int page_id = reference_string[i];

        pthread_mutex_lock(&list_mutex);

        // Find page, remove it from its current list
        Page *p = find_and_remove_page(page_id);
        if (p) {
            // Set reference bit and move to active list's tail
            p->reference_bit = 1;
            add_to_active_tail(p);
        }

        // Check for active list being way too big
        if (active_list_size > (int)(N * ACTIVE_LIST_THRESHOLD)) {
            int num_to_move = (int)(N * MOVE_COUNT_PERCENT);
            for (int k = 0; k < num_to_move && active_list_head != NULL; k++) {
                Page *page_to_move = active_list_head;
                active_list_head = active_list_head->next;
                active_list_size--;
                if (active_list_head == NULL) active_list_tail = NULL;
                
                add_to_inactive_tail(page_to_move);
            }
        }

        pthread_mutex_unlock(&list_mutex);
        usleep(PLAYER_SLEEP_US);
    }
    player_finished = 1;
    pthread_exit(0);
}

void *checker_thread_func() { 
    while (!player_finished) {
        usleep(M);
        pthread_mutex_lock(&list_mutex);
        Page *current = active_list_head;
        while (current != NULL) {
            if (current->reference_bit == 1) {
                page_stats[current->page_id]++;
                current->reference_bit = 0;
            }
            current = current->next;
        }
        pthread_mutex_unlock(&list_mutex);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <N_pages> <M_microseconds>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    M = atoi(argv[2]);

    if (N <= 0 || M <= 0) {
        fprintf(stderr, "N and M must be positive integers.\n");
        return 1;
    }

    srand(time(NULL));
    reference_string = malloc(REFERENCE_STRING_LENGTH * sizeof(int));
    page_stats = calloc(N, sizeof(int));

    // Random reference string of 1000 accesses
    for (int i = 0; i < REFERENCE_STRING_LENGTH; i++) {
        reference_string[i] = rand() % N;
    }

    // Initialization of pages + putting them in the inactive list
    for (int i = 0; i < N; i++) {
        Page *p = malloc(sizeof(Page));
        p->page_id = i;
        p->reference_bit = 0;
        p->next = NULL;
        add_to_inactive_tail(p);
    }

    pthread_mutex_init(&list_mutex, NULL);

    // Two workers should be created respectively for player and checker
    pthread_t player;   
    pthread_t checker;    

    pthread_create(&player, NULL, player_thread_func, NULL); 
    pthread_create(&checker, NULL, checker_thread_func, NULL); 
     
    pthread_join(player, NULL);
    pthread_join(checker, NULL);

    printf("Page_Id, Total_Referenced\n");
    for (int i = 0; i < N; i++) {
        printf("%d,%d\n", i, page_stats[i]);
    }
     
    printf("\nPages in active list: ");
    Page *current = active_list_head;
    while (current != NULL) {
        printf("%d%s", current->page_id, current->next ? ", " : "");
        current = current->next;
    }
    printf("\n");

    printf("Pages in inactive list: ");
    current = inactive_list_head;
    while (current != NULL) {
        printf("%d%s", current->page_id, current->next ? ", " : "");
        current = current->next;
    }
    printf("\n");

    /*free up resources properly */
    free(reference_string);
    free(page_stats);
    pthread_mutex_destroy(&list_mutex);
    
    current = active_list_head;
    while (current != NULL) {
        Page *temp = current;
        current = current->next;
        free(temp);
    }
    current = inactive_list_head;
    while (current != NULL) {
        Page *temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}
