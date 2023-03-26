#include <stdio.h>
#include <stdlib.h>

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

Process *dequeue(ProcessQueue *);

void freeProcess(Process *process){
    if (process->cpu != NULL) {
        free(process->cpu);
    }

    if (process->io != NULL) {
        free(process->io);
    }

    free(process);
}

Process *initProcess(int priority, int time) {
    Process *newProcess = malloc(sizeof(Process));
    newProcess->priority = priority;
    newProcess->time = time;
    newProcess->timeInReady = 0;
    newProcess->prev = NULL;
    newProcess->next = NULL;
    return newProcess;
}

void freeProcessQueue(ProcessQueue *queue) {
    // first, free all processes in the queue...
    Process *curr = dequeue(queue);
    while (curr != NULL) {
        freeProcess(curr);
        curr = dequeue(queue);
    }

    // now free the queue itself...
    free(queue);
}

ProcessQueue *initProcessQueue(Process *head, int queueType) {
    ProcessQueue *newQueue = malloc(sizeof(ProcessQueue));
    newQueue->head = head;
    newQueue->type = queueType;
    return newQueue;
}

// add a process to the tail of an unsorted queue
void enqueue(ProcessQueue *queue, Process *newProcess) {
    Process *curr = queue->head;

    // handle an empty queue:
    if (curr == NULL) {
        queue->head = newProcess;
        newProcess->prev = NULL;
        return;
    }

    // enqueue based on queue type:
    if (queue->type == SJF_PROC_QUEUE) {

        // handle newProcess time being shortest in queue
        if (newProcess->time < curr->time) {
            newProcess->next = curr;
            curr->prev = newProcess;
            newProcess->prev = NULL;
            queue->head = newProcess;
            return;
        }

        // traverse the queue
        while (curr->time <= newProcess->time) {
            if (curr->next == NULL) {
                curr->next = newProcess;
                newProcess->prev = curr;
                return;
            }

            curr = curr->next;
        }

        // insert location found, add in place
        newProcess->next = curr;
        newProcess->prev = curr->prev;
        curr->prev->next = newProcess;
        curr->prev = newProcess;

    } else if (queue->type == PR_PROC_QUEUE) {

        // handle newProcess priority being largest in queue
        if (newProcess->priority > curr->priority) {
            newProcess->next = curr;
            curr->prev = newProcess;
            newProcess->prev = NULL;
            queue->head = newProcess;
            return;
        }

        // traverse the queue
        while (curr->priority >= newProcess->priority) {
            // handle end of queue
            if (curr->next == NULL) {
                curr->next = newProcess;
                newProcess->prev = curr;
                return;
            }

            curr = curr->next;
        }

        // otherwise add process in place
        newProcess->next = curr;
        newProcess->prev = curr->prev;
        curr->prev->next = newProcess;
        curr->prev = newProcess;
    } else {
        // traverse the queue;
        while (curr->next != NULL) {
            curr = curr->next;
        }

        // tail reached, add new process
        curr->next = newProcess;
        newProcess->prev = curr;
    }

}

// remove a process from the head of the queue
Process * dequeue(ProcessQueue *queue) {
    // handle an empty queue:
    if (queue->head == NULL) {
        return NULL;
    }

    // handle non-empty queue:
    Process *dequeuedProcess = queue->head;
    queue->head = queue->head->next;

    dequeuedProcess->prev = NULL;
    dequeuedProcess->next = NULL;
    return dequeuedProcess;
}

/* FOR DEBUGGING....
void printProcess(Process *proc) {
    if (proc != NULL) {
        printf("<Process time=%d, priority=%d>\n", proc->time, proc->priority);
    }
}

void printProcessQueue(ProcessQueue *queue) {
    // empty queue
    if (queue->head == NULL) {
        return;
    }

    Process *curr = queue->head;
    printf("<ProcessQueue>\n");
    while (curr != NULL) {
        printf("    ");
        printProcess(curr);
        curr = curr->next;
    }
    printf("</ProcessQueue>\n");
}

void main() {
    
    ProcessQueue *queue = initProcessQueue(NULL, PR_PROC_QUEUE);

    Process *proc1 = initProcess(10, 50);
    Process *proc2 = initProcess(1, 20);
    Process *proc3 = initProcess(10, 38);
    Process *proc4 = initProcess(5, 110);
    Process *proc5 = initProcess(3, 29);
    Process *proc6 = initProcess(2, 73);
    Process *proc7 = initProcess(9, 1);
    Process *proc8 = initProcess(10, 2);
    Process *proc9 = initProcess(5, 50);

    enqueue(queue, proc1);
    enqueue(queue, proc2);
    enqueue(queue, proc3);
    enqueue(queue, proc4);
    enqueue(queue, proc5);
    enqueue(queue, proc6);
    enqueue(queue, proc7);
    enqueue(queue, proc8);
    enqueue(queue, proc9);

    printProcessQueue(queue);
    Process *dequeuedProc = dequeue(queue);
    printProcess(dequeuedProc);
    enqueue(queue, dequeuedProc);
    printProcessQueue(queue);
    freeProcessQueue(queue);
}
*/