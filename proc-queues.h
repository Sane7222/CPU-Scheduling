#ifndef PROC_QUEUES_H
#define PROC_QUEUES_H

#define DEFAULT_PROC_QUEUE 1
#define SJF_PROC_QUEUE 2
#define PR_PROC_QUEUE 3
#define RR_PROC_QUEUE 4

typedef struct process {
    int priority;
    int time;
    int timeInReady;
    int *cpu;
    int *io;
    struct process *prev;
    struct process *next;
} Process;

typedef struct processQueue {
    Process *head;
    int type;
} ProcessQueue;

Process *initProcess(int, int);
ProcessQueue *initProcessQueue(Process *, int);
void enqueue(ProcessQueue *, Process *);
Process *dequeue(ProcessQueue *);
void freeProcess(Process *);
void freeProcessQueue(ProcessQueue *);

#endif