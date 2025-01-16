#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <semaphore.h>

#define MAX_REQUESTS 1000
#define MAX_FILES 100
#define MAX_LINE 256

// Color Codes
#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define PINK "\033[1;35m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"

//GLOBALS
int read_time, write_time, delete_time;
int num_files, max_concurrent, timeout;
time_t start_time;

sem_t limit[MAX_FILES];
sem_t write_sem[MAX_FILES];
sem_t delete[MAX_FILES];

void init_semaphores();

typedef enum {
    READ,
    WRITE,
    DELETE
} Operation;

typedef struct {
    int user_id;
    int file_id;
    Operation operation;
    int request_time;
    bool processed;
    bool cancelled;
    long time_limit;
} Request;

typedef struct {
    int access_count;
    bool isbeingwritten;
    bool isbeingdeleted;
    bool deleted;
} FileStatus;

FileStatus files[MAX_FILES];

void* request_thread_func(void* arg) {
    Request* req = (Request*)arg;
    int arrival_time = req->request_time;

    sleep(arrival_time);
    printf(YELLOW "User %d has made request for performing ", req->user_id);
    switch (req->operation) {
        case READ: printf("READ "); break;
        case WRITE: printf("WRITE "); break;
        case DELETE: printf("DELETE "); break;
    }
    printf("on file %d at %d seconds" RESET "\n", req->file_id, req->request_time);

    sleep(1); // Simulate the 1 second delay before processing

    while (!req->processed && !req->cancelled) {
        if (req->operation == DELETE) {
            sem_wait(&delete[req->file_id - 1]);

            if (time(NULL) > req->time_limit){
            // if (time(NULL) - req->request_time > req->time_limit) {
                printf(RED "User %d canceled the request due to no response at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
                req->cancelled = true;
                sem_post(&delete[req->file_id - 1]);
                return NULL;
            }

            files[req->file_id - 1].isbeingdeleted = true;
            files[req->file_id - 1].access_count++;
            printf(PINK "LAZY has taken up the request of User %d at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
            sleep(delete_time);
            files[req->file_id - 1].deleted = true;
            files[req->file_id - 1].access_count--;
            req->processed = true;
            printf(GREEN "The request for User %d was completed at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
            files[req->file_id - 1].isbeingdeleted = false;

            sem_post(&delete[req->file_id - 1]);
            return NULL;
        } else if (req->operation == READ) {
            int sem_value;
            sem_getvalue(&limit[req->file_id - 1], &sem_value);

            if (sem_value == max_concurrent) {
                sem_wait(&delete[req->file_id - 1]);
            }
            sem_wait(&limit[req->file_id - 1]);

            if (time(NULL) > req->time_limit){
            // if (time(NULL) - req->request_time > req->time_limit) {
                printf(RED "User %d canceled the request due to no response at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
                req->cancelled = true;
                sem_post(&limit[req->file_id - 1]);
                if (sem_value == max_concurrent) {
                    sem_post(&delete[req->file_id - 1]);
                }
                return NULL;
            }

            files[req->file_id - 1].access_count++;
            printf(PINK "LAZY has taken up the request of User %d at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
            sleep(read_time);
            req->processed = true;
            printf(GREEN "The request for User %d was completed at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
            files[req->file_id - 1].access_count--;

            sem_post(&limit[req->file_id - 1]);
            if (sem_value == max_concurrent) {
                sem_post(&delete[req->file_id - 1]);
            }
            return NULL;
        } else if (req->operation == WRITE) {
            int sem_value;
            sem_getvalue(&limit[req->file_id - 1], &sem_value);

            if (sem_value == max_concurrent) {
                sem_wait(&delete[req->file_id - 1]);
            }

            sem_wait(&write_sem[req->file_id - 1]);
            sem_wait(&limit[req->file_id - 1]);

            // if (time(NULL) - req->request_time > req->time_limit) {
            if (time(NULL) > req->time_limit){
                printf(RED "User %d canceled the request due to no response at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
                req->cancelled = true;
                sem_post(&limit[req->file_id - 1]);
                sem_post(&write_sem[req->file_id - 1]);
                if (sem_value == max_concurrent) {
                    sem_post(&delete[req->file_id - 1]);
                }
                return NULL;
            }

            files[req->file_id - 1].isbeingwritten = true;
            files[req->file_id - 1].access_count++;
            printf(PINK "LAZY has taken up the request of User %d at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
            sleep(write_time);
            req->processed = true;
            printf(GREEN "The request for User %d was completed at %ld seconds" RESET "\n\n", req->user_id, time(NULL) - start_time);
            files[req->file_id - 1].access_count--;
            files[req->file_id - 1].isbeingwritten = false;

            sem_post(&limit[req->file_id - 1]);
            sem_post(&write_sem[req->file_id - 1]);
            if (sem_value == max_concurrent) {
                sem_post(&delete[req->file_id - 1]);
            }
            return NULL;
        }
    }
    return NULL;
}

int main() {
    char line[MAX_LINE];
    char** input_lines = NULL;
    int line_count = 0;
    
    while (fgets(line, MAX_LINE, stdin)) {
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "STOP") == 0) break;
        
        input_lines = realloc(input_lines, (line_count + 1) * sizeof(char*));
        input_lines[line_count] = strdup(line);
        line_count++;
    }

    int request_count = line_count - 2;
    Request requests[request_count];

    for (int i = 0; i < request_count; i++) {
        Request req;
        char operation_str[10];
        sscanf(input_lines[i + 2], "%d %d %s %d", &req.user_id, &req.file_id,
               operation_str, &req.request_time);

        if (strcmp(operation_str, "READ") == 0) req.operation = READ;
        else if (strcmp(operation_str, "WRITE") == 0) req.operation = WRITE;
        else req.operation = DELETE;

        req.processed = false;
        req.cancelled = false;
        requests[i] = req;
    }

    sscanf(input_lines[0], "%d %d %d", &read_time, &write_time, &delete_time);
    sscanf(input_lines[1], "%d %d %ld", &num_files, &max_concurrent, &timeout);

    for (int i = 0; i < request_count; i++) {
        requests[i].time_limit = timeout + time(NULL);
    }

    for (int i = 0; i < num_files; i++) {
        files[i].access_count = 0;
        files[i].deleted = false;
        files[i].isbeingdeleted = false;
        files[i].isbeingwritten = false;
    }

    start_time = time(NULL);
    pthread_t request_threads[request_count];
    int rc;

    printf("\nLAZY has woken up!\n\n");

    init_semaphores(num_files, max_concurrent);

    for (int i = 0; i < request_count; i++) {
        rc = pthread_create(&request_threads[i], NULL, request_thread_func, &requests[i]);
        if (rc) {
            printf("Error creating thread %d\n", i);
            exit(-1);
        }
    }

    for (int i = 0; i < request_count; i++) {
        rc = pthread_join(request_threads[i], NULL);
        if (rc) {
            printf("Error joining thread %d\n", i);
            exit(-1);
        }
    }

    printf("LAZY has gone to sleep!\n");

    free(input_lines);
    return 0;
}

void init_semaphores(int num_files, int max_concurrent) {
    for (int i = 0; i < num_files; i++) {
        sem_init(&limit[i], 0, max_concurrent);
        sem_init(&write_sem[i], 0, 1);
        sem_init(&delete[i], 0, 1);
    }
}
