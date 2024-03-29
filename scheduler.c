#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include "proc-queues.h"

#define THREADS 3
#define BUFFER 256

pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;

ProcessQueue *ready_queue = NULL; 
ProcessQueue *io_queue = NULL;
ProcessQueue *terminated_queue = NULL;

int quantum = 0;

int parsing_complete = 0;
int total_processes = 0;
int processes_completed = 0;
long long avgTurnAround_t = 0, avgReadyWaiting_t = 0; // ms
struct timeval start_processing;
struct timeval end_processing;

void errorWithMessage (char *message){ // Output error message and exit program with failure
    printf("%s\n", message);
    exit(1);
}

void readThread(void *arg){ // Thread function for reading from the input file
    FILE *fp = fopen((char *) arg, "r"); // Open file
    if (!fp){
        parsing_complete = 1;
        total_processes = 0;
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
            process->currentCPU_Burst = 0;
            process->totalCPU_Bursts = (bursts >> 1) + 1;
            process->io = malloc((bursts >> 1) * sizeof(int));
            process->currentIO_Burst = 0;
            process->totalIO_Bursts = (bursts >> 1);
            process->time = 0;
            process->totalTimeInReadyQueue = 0;
            process->next = NULL;
            process->prev = NULL;
            gettimeofday(&process->start, NULL); // Begin time
            gettimeofday(&process->enter_ready, NULL);

            pos = seek;
            for (int i = 0; i < bursts; pos += seek){ // Update process information
                sscanf(line + pos, "%d%n", &time, &seek);
                process->time += time;

                if (i % 2 == 0) process->cpu[i++ >> 1] = time;
                else process->io[i++ >> 1] = time;
            }

            // add the process to the ready queue
            pthread_mutex_lock(&ready_mutex);
            total_processes++;
            enqueue(ready_queue, process);
            pthread_mutex_unlock(&ready_mutex);
            if (total_processes == 1) gettimeofday(&start_processing, NULL);
        }
        else if (line[1] == 'l'){ // Sleep
            sscanf(line, "sleep %d", &time);
            usleep(time * 1000);
        }
        else if (line[1] == 't'){ // Stop
            fclose(fp);
            //printf("PARSING THREAD: exiting\n");
            parsing_complete = 1;
            pthread_exit(NULL);
        }
    }
}

void cpuThread(void *arg){ // Thread function for simulating CPU bursts based on scheduling algorithm
    int cpuBurstTime = 0;

    while (!parsing_complete || processes_completed < total_processes) {
        // grab a process from the ready queue
        pthread_mutex_lock(&ready_mutex);
        Process *currentProcess = dequeue(ready_queue);
        pthread_mutex_unlock(&ready_mutex);

        if (currentProcess == NULL) continue; // ready queue empty, don't sleep

        gettimeofday(&currentProcess->end, NULL);
        currentProcess->totalTimeInReadyQueue += (currentProcess->end.tv_sec - currentProcess->enter_ready.tv_sec) * 1000LL + (currentProcess->end.tv_usec - currentProcess->enter_ready.tv_usec) / 1000LL;

        cpuBurstTime = getCurrentCPUBurstTime(currentProcess);

        if (ready_queue->type == RR_PROC_QUEUE) {
            if (cpuBurstTime > quantum) {
                // this cpuBurst won't finish within 1 quantum.
                // need to process for 1 quantum then readd to the ready queue.
                currentProcess->cpu[currentProcess->currentCPU_Burst] = cpuBurstTime - quantum;
                //printf("CPU THREAD: sleeping for 1 quantum (%d ms)\n", quantum);
                usleep(quantum * 1000);

                // add process back into ready queue
                gettimeofday(&currentProcess->enter_ready, NULL);
                pthread_mutex_lock(&ready_mutex);
                enqueue(ready_queue, currentProcess);
                pthread_mutex_unlock(&ready_mutex);

                continue;
            }
        }


        //printf("CPU THREAD: sleeping for %d ms\n", cpuBurstTime);
        usleep(cpuBurstTime * 1000);

        // mark index for next cpu burst and check if process has completed...
        currentProcess->currentCPU_Burst++;
        if (getCurrentCPUBurstTime(currentProcess) == -1) {
            // process has completed!
            //printf("CPU THREAD: process completed\n");
            processes_completed++;

            gettimeofday(&currentProcess->end, NULL);
            enqueue(terminated_queue, currentProcess); // Entering termination
            continue;
        }

        // add process to the ready queue
        pthread_mutex_lock(&io_mutex);
        enqueue(io_queue, currentProcess);
        pthread_mutex_unlock(&io_mutex);
    }

    gettimeofday(&end_processing, NULL);

    //printf("CPU THREAD: completed %d processes\n", processes_completed);
    //printf("CPU THREAD: exiting\n");
    pthread_exit(NULL);
}

void ioThread(void * arg){ // Thread function for simulating IO bursts in FIFO order
    int ioProcessTime = 0;

    while (!parsing_complete || processes_completed < total_processes) {
        // grab process from the io queue
        pthread_mutex_lock(&io_mutex);
        Process *currentProcess = dequeue(io_queue);
        pthread_mutex_unlock(&io_mutex);

        // work the process
        if (currentProcess == NULL) continue;
        ioProcessTime = getCurrentIOBurstTime(currentProcess);
        //printf("IO THREAD: sleeping for %d ms\n", ioProcessTime);
        usleep(ioProcessTime * 1000);

        // mark index for next io burst
        currentProcess->currentIO_Burst++;
        gettimeofday(&currentProcess->enter_ready, NULL);

        // add process to the ready queue
        pthread_mutex_lock(&ready_mutex);
        enqueue(ready_queue, currentProcess);
        pthread_mutex_unlock(&ready_mutex);
    }

    //printf("IO THREAD: exiting\n");
    pthread_exit(NULL);
}

void calculate(ProcessQueue *q){
    Process *curr;
    while ((curr = dequeue(q)) != NULL) {
        avgTurnAround_t += (curr->end.tv_sec - curr->start.tv_sec) * 1000LL + (curr->end.tv_usec - curr->start.tv_usec) / 1000LL;
        avgReadyWaiting_t += curr->totalTimeInReadyQueue;
        freeProcess(curr);
    }
    avgTurnAround_t /= total_processes;
    avgReadyWaiting_t /= total_processes;
}

void cleanUpThreads(pthread_t *threads){ // Clean up threads
    for (int i = 0; i < THREADS; i++) pthread_join(threads[i], NULL);
}

void main (int argc, char *argv[]){
    char *algos[] = {"FCFS", "SJF", "PR", "RR"};
    int valid = 0;

    if (argc < 5 || argc > 7) errorWithMessage("Invalid parameters"); // Check for valid parameters
    if (strcmp(argv[1], "-alg")) errorWithMessage("Missing algorithm");
    if (strcmp(argv[argc - 2], "-input")) errorWithMessage("Missing input file");

    for (int i = 0; i < 4; i++) { // Check for valid algorithm
        if (!strcmp(argv[2], algos[i])) {
            if (i == 3 && (strcmp(argv[3], "-quantum") || (quantum = atoi(argv[4])) < 1)) errorWithMessage("Missing or invalid quantum");
            valid = 1;

            // valid algorithm specified, initialize appropriate queue
            ready_queue = initProcessQueue(NULL, i);
            io_queue = initProcessQueue(NULL, DEFAULT_PROC_QUEUE);
            terminated_queue = initProcessQueue(NULL, DEFAULT_PROC_QUEUE);
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

    // wait on threads
    cleanUpThreads(threads);

    // calculate total process time
    long long total_process_time = (end_processing.tv_sec - start_processing.tv_sec) * 1000LL + (end_processing.tv_usec - start_processing.tv_usec) / 1000LL;
    double throughput = processes_completed / (double) total_process_time;

    freeProcessQueue(ready_queue);
    freeProcessQueue(io_queue);

    calculate(terminated_queue);
    freeProcessQueue(terminated_queue);

    printf("Input File Name : %s\n", argv[argc - 1]);
    printf("CPU Scheduling Alg : %s ", argv[2]);
    if (!strcmp(argv[2], "RR")) printf("(%d)", quantum);
    printf("\nThroughput : %f\n", throughput); // This is for you Brad
    printf("Avg. Turnaround Time : %lld ms\n", avgTurnAround_t);
    printf("Avg. Waiting Time in Ready Queue: %lld ms\n", avgReadyWaiting_t);

    exit(0);
}
