#include <stdio.h>
#include <stdlib.h>
#include "proc-queues.h"

Process *initProcess(int priority, int time) {
    Process *newProcess = malloc(sizeof(Process));
    newProcess->priority = priority;
    newProcess->time = time;
    newProcess->timeInReady = 0;
    newProcess->cpu = NULL;
    newProcess->io = NULL;
    newProcess->prev = NULL;
    newProcess->next = NULL;
    return newProcess;
}

ProcessQueue *initProcessQueue(Process *head, int queueType) {
    ProcessQueue *newQueue = malloc(sizeof(ProcessQueue));
    newQueue->head = head;
    newQueue->type = queueType;
    return newQueue;
}

// enqueue will add a process to a queue based on the queue's type.
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
            // handle end of queue scenario
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
            // handle end of queue scenario
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
Process *dequeue(ProcessQueue *queue) {
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

void freeProcess(Process *process){
    if (process->cpu != NULL) {
        free(process->cpu);
    }

    if (process->io != NULL) {
        free(process->io);
    }

    free(process);
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
