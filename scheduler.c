#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void errorWithMessage (char *message){
    printf("%s\n", message);
    exit(1);
}

void main (int argc, char *argv[]){

    if (argc < 5 || argc > 7) errorWithMessage("Invalid parameters");
    if (strcmp(argv[1], "-alg")) errorWithMessage("Missing algorithm");
    if (strcmp(argv[argc - 2], "-input")) errorWithMessage("Missing input file");

    char *algo = argv[2];
    char *file = argv[argc - 1];

    if (!strcmp(algo, "FCFS")){ // First Come First Serve
        printf("First Come First Serve\n");
    }
    else if (!strcmp(algo, "SJF")){ // Shortest Job First
        printf("Shortest Job First\n");
    }
    else if (!strcmp(algo, "PR")){ // Priority Queue
        printf("Priority Queue\n");
    }
    else if (!strcmp(algo, "RR")){ // Round Robin
        int quantum;
        if (strcmp(argv[3], "-quantum") || (quantum = atoi(argv[4])) < 1) errorWithMessage("Missing or invalid quantum");
        printf("Round Robin: Quantum = %d\n", quantum);
    }
    else errorWithMessage("Invalid algorithm");

    exit(0);
}
