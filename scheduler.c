#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#define THREADS 3
#define BUFFER 256

typedef struct process {
    int priority;
    int time;
    int *cpu;
    int *io;
    struct process *next;
} Process;

pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;

void errorWithMessage (char *message){ // Output error message and exit program with failure
    printf("%s\n", message);
    exit(1);
}

void unalloc(Process *process){ // Frees allocated memory of a struct process
    free(process->cpu);
    free(process->io);
    free(process);
}

void readThread(void *arg){ // Thread function for reading from the input file
    printf("I will read from input file\n");

    FILE *fp = fopen((char *) arg, "r"); // Open file
    if (!fp){
        printf("%s\n", strerror(errno));
        pthread_exit(NULL);
    }

    int bursts, time, seek, pos;
    char line[BUFFER];
    while (fgets(line, BUFFER, fp)){
        if (line[1] == 'r'){ // Proc
            Process *process = malloc(sizeof(Process));
            sscanf(line, "proc %d %d%n", &process->priority, &bursts, &seek);

            process->cpu = malloc(((bursts >> 1) + 1) * sizeof(int));
            process->io = malloc((bursts >> 1) * sizeof(int));
            process->time = 0;
            process->next = NULL;

            pos = seek;
            for (int i = 0; i < bursts; pos += seek){ // Update process information
                sscanf(line + pos, "%d%n", &time, &seek);
                process->time += time;

                if (i % 2 == 0) process->cpu[i++ >> 1] = time;
                else process->io[i++ >> 1] = time;
            }

            unalloc(process);
        }
        else if (line[1] == 'l'){ // Sleep
            sscanf(line, "sleep %d", &time);
            usleep(time);
        }
        else if (line[1] == 't'){ // Stop
            fclose(fp);
            pthread_exit(NULL);
        }
    }
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
