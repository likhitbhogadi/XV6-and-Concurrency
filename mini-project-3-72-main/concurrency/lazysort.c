#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define MAX_THREADS 4

typedef struct {
    char name[129];  // Name of the file (128 chars + null terminator)
    int id;
    char timestamp[20]; // ISO 8601 timestamp
} File;


typedef struct {
    int hash;
    File* file;
} FileHash;

typedef struct {
    long long int seconds;
    File* file;
} Fileseconds;

typedef struct {
    Fileseconds *arr2;
    FileHash *arr;
    File *files;
    int start;
    int end;
    int *local_count;
    int max;
    int min;
    int range;
} ThreadData;

int power_of_26(int exponent) {
    int result = 1;
    for (int i = 0; i < exponent; i++) {
        result *= 26;
    }
    return result;
}

int hash_string(char *str) {
    int len = strlen(str);
    int hash = 0;
    int multiply = 1;
    int ans = 4;
    if (len < ans) {
        multiply = power_of_26(ans - len);
        ans = len;
    }
    for (int i = 0; i < ans; i++) {
        if (str[i] != '.') {
            hash = hash + (str[i] - 'a' + 1);
        }
        hash = hash * 26;
    }
    hash = hash / 26;
    hash = hash * multiply;
    return hash;
}

long long int hash_time(char *datetime)
{
    struct tm tm = {0};
    if (sscanf(datetime, "%4d-%2d-%2dT%2d:%2d:%2d",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6)
    {
        fprintf(stderr, "Error: Incorrect date-time format.\n");
        return -1;
    }
    tm.tm_mon -= 1; // Months are 0-11
    time_t seconds = mktime(&tm);
    if (seconds == -1)
    {
        fprintf(stderr, "Error: mktime conversion failed.\n");
        return -1;
    }

    return (long long int)seconds;
}

void *count_elements_for_Name(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = data->start; i < data->end; i++) {
        data->local_count[((data->arr)[i]).hash - data->min]++;
    }
    return NULL;
}

void *count_elements_for_ID(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = data->start; i < data->end; i++) {
        // data->local_count[((data->arr)[i]).hash - data->min]++;
        data->local_count[((data->files)[i]).id - data->min]++;
    }
    return NULL;
}

void *count_elements_for_Timestamp(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = data->start; i < data->end; i++) {
        data->local_count[(data->arr2[i]).seconds - data->min]++;
    }
    return NULL;
}


// typedef struct {
//     char name[129];  // Name of the file (128 chars + null terminator)
//     int id;
//     char timestamp[20]; // ISO 8601 timestamp
// } File;

typedef struct {
    // int* arr;
    File* files;
    int left;
    int right;
    int (*cmp)(const File*, const File*);
} ThreadArgs;


void merge(File* files, int l, int m, int r, int (*cmp)(const File*, const File*))
{
    int n1 = m - l + 1;
    int n2 = r - m;
    File L[n1], R[n2];

    // Copy data to temp arrays L[] and R[]
    for (int i = 0; i < n1; i++) {
        L[i] = files[l + i];
    }
    for (int j = 0; j < n2; j++) {
        R[j] = files[m + 1 + j];
    }

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (cmp(&L[i], &R[j]) <= 0) {
            files[k] = L[i];    
            i++;
        } else {
            files[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        files[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        files[k] = R[j];
        j++;
        k++;
    }
}

void* mergeSortThread(void* args)
{
    ThreadArgs* threadArgs = (ThreadArgs*)args;
    // int* arr = threadArgs->arr;
    File* files = threadArgs->files;
    int l = threadArgs->left;
    int r = threadArgs->right;
    int (*cmp)(const File*, const File*) = threadArgs->cmp;

    if (l < r) {
        int m = l + (r - l) / 2;

        pthread_t thread1, thread2;
        ThreadArgs args1 = {files, l, m, cmp};
        ThreadArgs args2 = {files, m + 1, r, cmp};

        // Create threads for left and right halves
        pthread_create(&thread1, NULL, mergeSortThread, &args1);
        pthread_create(&thread2, NULL, mergeSortThread, &args2);

        // Wait for both threads to finish
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

        // Merge the sorted halves
        merge(files, l, m, r, cmp);
    }
    return NULL;
}

void mergeSort(File* files, int l, int r, int (*cmp)(const File*, const File*))
{
    ThreadArgs args = {files, l, r, cmp};
    mergeSortThread(&args);
}

int compareByName(const File* a, const File* b) {
    return strcmp(a->name, b->name);
}

// Comparison function for sorting by ID
int compareByID(const File* a, const File* b) {
    return a->id - b->id;
}

// Comparison function for sorting by Timestamp
int compareByTimestamp(const File* a, const File* b) {
    return strcmp(a->timestamp, b->timestamp);
}


int main(){

    int n;
    scanf("%d", &n);

    if(n > 42){

        char sortColumn[20];

        // Read number of files
        getchar(); // Consume newline character

        // Allocate array to hold file information
        File files[n];

        // Read file information
        for (int i = 0; i < n; i++) {
            scanf("%128s %d %19s", files[i].name, &files[i].id, files[i].timestamp);
        }

        // Read the column name to sort by
        scanf("%s", sortColumn);

        // Start the clock for performance measurement
        clock_t start_time = clock();

        // Sort the files based on the specified column using MergeSort
        if (strcmp(sortColumn, "Name") == 0) {
            mergeSort(files, 0, n - 1, compareByName);
        } else if (strcmp(sortColumn, "ID") == 0) {
            mergeSort(files, 0, n - 1, compareByID);
        } else if (strcmp(sortColumn, "Timestamp") == 0) {
            mergeSort(files, 0, n - 1, compareByTimestamp);
        }

        // End the clock after sorting
        clock_t end_time = clock();

        // Output the sorting column
        printf("%s\n", sortColumn);


        // clock_t start_time = clock();
        // mergeSort(arr, 0, n - 1);
        // clock_t end_time = clock();

        // for (int i = 0; i < n; i++) {
        //     printf("%d ", arr[i]);
        // }
        // printf("\n");

        for (int i = 0; i < n; i++) {
            printf("%s %d %s\n", files[i].name, files[i].id, files[i].timestamp);
        }

        double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("Time taken: %.6f seconds\n", time_taken);
    }
    else{

        char sortColumn[20];

        getchar(); // Consume newline character

        File files[n];

        // Read file information
        for (int i = 0; i < n; i++) {
            scanf("%128s %d %19s", files[i].name, &files[i].id, files[i].timestamp);
        }

        // Read the column name to sort by
        scanf("%s", sortColumn);

        if(strcmp(sortColumn, "Name") == 0){
            FileHash *arr = (FileHash *)malloc(n * sizeof(FileHash));

            for (int i = 0; i < n; i++) {
                arr[i].file = &files[i];
                arr[i].hash = hash_string(files[i].name);
            }

            // for(int i = 0; i < n; i++){
            //     printf("%s %d\n", arr[i].file->name, arr[i].hash);
            // }

            int max = arr[0].hash;
            int min = arr[0].hash;

            for (int i = 1; i < n; i++) {
                if (arr[i].hash > max) {
                    max = arr[i].hash;
                }
                if (arr[i].hash < min) {
                    min = arr[i].hash;
                }
            }

            int range = max - min + 1;

            int *global_count = calloc(range, sizeof(int));
            int thread_count[MAX_THREADS][range];
            for (int i = 0; i < MAX_THREADS; i++) {
                for (int j = 0; j < range; j++) {
                    thread_count[i][j] = 0;
                }
            }

            pthread_t threads[MAX_THREADS];
            ThreadData thread_data[MAX_THREADS];


            clock_t start_time = clock();

            int chunk_size = n / MAX_THREADS;
            for (int i = 0; i < MAX_THREADS; i++) {
                thread_data[i].arr = arr;
                thread_data[i].start = i * chunk_size;
                thread_data[i].end = (i == MAX_THREADS - 1) ? n : (i + 1) * chunk_size;
                thread_data[i].local_count = thread_count[i];
                thread_data[i].max = max;
                thread_data[i].min = min;
                thread_data[i].range = range;

                pthread_create(&threads[i], NULL, count_elements_for_Name, &thread_data[i]);
            }

            for (int i = 0; i < MAX_THREADS; i++) {
                pthread_join(threads[i], NULL);
            }

            for (int i = 0; i < MAX_THREADS; i++) {
                for (int j = 0; j < range; j++) {
                    global_count[j] += thread_count[i][j];
                }
            }

            for (int i = 1; i < range; i++) {
                global_count[i] += global_count[i - 1];
            }

            FileHash *sorted = malloc(n * sizeof(FileHash));
            for (int i = n - 1; i >= 0; i--) {
                sorted[global_count[arr[i].hash - min] - 1] = arr[i];
                global_count[arr[i].hash - min]--;
            }

            for (int i = 0; i < n; i++) {
                arr[i] = sorted[i];
            }
            
            clock_t end_time = clock();
            for(int i = 0; i < n; i++){
                printf("%s %d %s\n", arr[i].file->name, arr[i].file->id, arr[i].file->timestamp);
            }
            printf("time for input of size %d: %f\n", n, (double)(end_time - start_time) / CLOCKS_PER_SEC);
            // printf("\n");

            free(global_count);
            free(sorted);
            free(arr);
        }
        else if (strcmp(sortColumn, "Timestamp") == 0) {

            Fileseconds *arr2 = (Fileseconds *)malloc(n * sizeof(FileHash));

            for (int i = 0; i < n; i++) {
                arr2[i].file = &files[i];
                // arr[i].hash = hash_string(files[i].timestamp);
                arr2[i].seconds = hash_time(files[i].timestamp);
            }

            int max = arr2[0].seconds;
            int min = arr2[0].seconds;

            for (int i = 1; i < n; i++) {
                if (arr2[i].seconds > max) {
                    max = arr2[i].seconds;
                }
                if (arr2[i].seconds < min) {
                    min = arr2[i].seconds;
                }
            }

            int range = max - min + 1;

            int *global_count = calloc(range, sizeof(int));
            int thread_count[MAX_THREADS][range];
            for (int i = 0; i < MAX_THREADS; i++) {
                for (int j = 0; j < range; j++) {
                    thread_count[i][j] = 0;
                }
            }

            pthread_t threads[MAX_THREADS];
            ThreadData thread_data[MAX_THREADS];

            clock_t start_time = clock();


            int chunk_size = n / MAX_THREADS;
            for (int i = 0; i < MAX_THREADS; i++) {
                thread_data[i].arr2 = arr2;
                thread_data[i].files = files;
                thread_data[i].start = i * chunk_size;
                thread_data[i].end = (i == MAX_THREADS - 1) ? n : (i + 1) * chunk_size;
                thread_data[i].local_count = thread_count[i];
                thread_data[i].max = max;
                thread_data[i].min = min;
                thread_data[i].range = range;

                pthread_create(&threads[i], NULL, count_elements_for_Timestamp, &thread_data[i]);
            }

            for (int i = 0; i < MAX_THREADS; i++) {
                pthread_join(threads[i], NULL);
            }

            for (int i = 0; i < MAX_THREADS; i++) {
                for (int j = 0; j < range; j++) {
                    global_count[j] += thread_count[i][j];
                }
            }

            for (int i = 1; i < range; i++) {
                global_count[i] += global_count[i - 1];
            }

            Fileseconds *sorted = malloc(n * sizeof(Fileseconds));
            for (int i = n - 1; i >= 0; i--) {
                sorted[global_count[arr2[i].seconds - min] - 1] = arr2[i];
                global_count[arr2[i].seconds - min]--;
            }

            for (int i = 0; i < n; i++) {
                arr2[i] = sorted[i];
            }

            clock_t end_time = clock();
            
            for(int i = 0; i < n; i++){
                printf("%s %d %s\n", arr2[i].file->name, arr2[i].file->id, arr2[i].file->timestamp);
            }
            // printf("\n");

            printf("time for input of size %d: %f\n", n, (double)(end_time - start_time) / CLOCKS_PER_SEC);

            free(global_count);
            free(sorted);
            free(arr2);
        }   
        else if(strcmp(sortColumn, "ID") == 0){
            

            int max = files[0].id;
            int min = files[0].id;

            for(int i = 1; i < n; i++){
                if(files[i].id > max){
                    max = files[i].id;
                }
                if(files[i].id < min){
                    min = files[i].id;
                }
            }

            int range = max - min + 1;

            int *global_count = calloc(range, sizeof(int));
            int thread_count[MAX_THREADS][range];
            for (int i = 0; i < MAX_THREADS; i++) {
                for (int j = 0; j < range; j++) {
                    thread_count[i][j] = 0;
                }
            }

            pthread_t threads[MAX_THREADS];
            ThreadData thread_data[MAX_THREADS];

            clock_t start_time = clock();


            int chunk_size = n / MAX_THREADS;
            for (int i = 0; i < MAX_THREADS; i++) {
                thread_data[i].arr = NULL;
                thread_data[i].files = files;
                thread_data[i].start = i * chunk_size;
                thread_data[i].end = (i == MAX_THREADS - 1) ? n : (i + 1) * chunk_size;
                thread_data[i].local_count = thread_count[i];
                thread_data[i].max = max;
                thread_data[i].min = min;
                thread_data[i].range = range;

                pthread_create(&threads[i], NULL, count_elements_for_ID, &thread_data[i]);
            }

            for (int i = 0; i < MAX_THREADS; i++) {
                pthread_join(threads[i], NULL);
            }

            for (int i = 0; i < MAX_THREADS; i++) {
                for (int j = 0; j < range; j++) {
                    global_count[j] += thread_count[i][j];
                }
            }

            for (int i = 1; i < range; i++) {
                global_count[i] += global_count[i - 1];
            }

            File *sorted = malloc(n * sizeof(File));
            for (int i = n - 1; i >= 0; i--) {
                sorted[global_count[files[i].id - min] - 1] = files[i];
                global_count[files[i].id - min]--;
            }

            for (int i = 0; i < n; i++) {
                files[i] = sorted[i];
            }

            clock_t end_time = clock();


            for(int i = 0; i < n; i++){
                printf("%s %d %s\n", files[i].name, files[i].id, files[i].timestamp);
            }
            // printf("\n");

            printf("time for input of size %d: %f\n", n, (double)(end_time - start_time) / CLOCKS_PER_SEC);

            free(global_count);
            free(sorted);
        }   
    }

    return 0;
}