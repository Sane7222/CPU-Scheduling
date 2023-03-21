#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define THREADS 3

void errorWithMessage (char *message){ // Output error message and exit program with failure
    printf("%s\n", message);
    exit(1);
}

void readThread(void *arg){ // Thread function for reading from the input file
    printf("I will read from input file\n");
    pthread_exit(NULL);
}

void cpuThread(void *arg){ // Thread function for simulating CPU bursts based on scheduling algorithm
    printf("I will manage Ready Queue\n");
    pthread_exit(NULL);
}

void ioThread(void * arg){ // Thread function for simulating IO bursts in FIFO order
    printf("I will manage IO Queue\n");
    pthread_exit(NULL);
}

void cleanUpThreads(pthread_t *threads){ // Clean up threads
    for (int i = 0; i < THREADS; i++) pthread_join(threads[i], NULL);
}

void main (int argc, char *argv[]){
    char *algos[] = {"FCFS", "SJF", "PR", "RR"};
    int valid = 0;
    int quantum = 0;

    if (argc < 5 || argc > 7) errorWithMessage("Invalid parameters"); // Check for valid parameters
    if (strcmp(argv[1], "-alg")) errorWithMessage("Missing algorithm");
    if (strcmp(argv[argc - 2], "-input")) errorWithMessage("Missing input file");

    for (int i = 0; i < 4; i++) { // Check for valid algorithm
        if (!strcmp(argv[2], algos[i])) {
            if (i == 4 && (strcmp(argv[3], "-quantum") || (quantum = atoi(argv[4])) < 1)) errorWithMessage("Missing or invalid quantum");
            valid = 1;
            break;
        }
    } if (!valid) errorWithMessage("Invalid algorithm");

    pthread_t threads[THREADS];
    void *threadFunctions[THREADS] = {readThread, cpuThread, ioThread};
    void *threadArgs[THREADS] = {argv[argc - 1], argv[2], NULL};

    for (int i = 0; i < THREADS; i++){ // Create pthreads
        if (pthread_create(&threads[i], NULL, (void *)threadFunctions[i], threadArgs[i])){
            cleanUpThreads(threads);
            errorWithMessage("Thread creation failed");
        }
    }

    cleanUpThreads(threads);
    exit(0);
}
