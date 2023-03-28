#ifndef PROC_QUEUES_H
#define PROC_QUEUES_H

#define DEFAULT_PROC_QUEUE 0
#define SJF_PROC_QUEUE 1
#define PR_PROC_QUEUE 2
#define RR_PROC_QUEUE 3

typedef struct process {
    int priority;               // priority of the process
    int time;                   // total burst time (cpu + io)
    double totalTimeInReadyQueue;  // total time in ready queue (starts at 0) (in ms)
    int *cpu;                   // array of cpu burst times (in ms)
    int totalCPU_Bursts;        // length of cpu burst array
    int currentCPU_Burst;       // index of current cpu burst
    int *io;                    // array of io burst times (in ms)
    int totalIO_Bursts;         // length of io burst array
    int currentIO_Burst;        // index of current io burst
    clock_t start;              // process start time
    clock_t end;                // process end time
    clock_t enter_ready;        // time entering ready queue

    struct process *prev;
    struct process *next;
} Process;

typedef struct processQueue {
    Process *head;
    int type;
} ProcessQueue;

int getCurrentCPUBurstTime(Process *);
int getCurrentIOBurstTime(Process *);
ProcessQueue *initProcessQueue(Process *, int);
void enqueue(ProcessQueue *, Process *);
Process *dequeue(ProcessQueue *);
void freeProcess(Process *);
void freeProcessQueue(ProcessQueue *);
void printProcess(Process *);
void printProcessQueue(ProcessQueue *);

#endif