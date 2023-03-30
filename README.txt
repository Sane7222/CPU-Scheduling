CS460 Project 2: CPU Scheduling
Team: Matias Moseley & Bradley Dowling

######################################################
## How we share data between parts of the program:


######################################################
## Approach to synchronization issues:


######################################################
## How to switch between scheduling algorithms:

The scheduling algorithm to be used is specified via the command 
line arguments that are layed out in the project instructions.

When the ready-queue is created, it takes on a 'queue-type'
specified by scheduling command line argument. Processes are inserted into 
the ready-queue in the order specified by the scheduling algorithm given.
When dequeuing off of the ready-queue, it is always the process at
the head of the ready-queue that is removed. Thus, scheduling is 
handled by the way that processes are INSERTED into the ready-queue.

Process are inserted into the ready-queue the following ways:

  FCFS: Processes are inserted in FIFO manner
  SJF: Processes are inserted in order of their total burst time
  PR: Processes are inserted in order of their priority
  RR: Processes are inserted in FIFO manner, but the cpuThread will 
      check how long the current burst is, compare that to the quantum,
      and then sleep for whichever is shorter (readding the process
      to the ready-queue if burst time is longer than quantum).


######################################################
## How we generate data for required measurements:

Within each process struct, we track how long a process has been in the
ready queue as well as how long the process has been 'alive'. Once the
process terminates, it is placed onto a terminated queue. Once all
processes have terminated, the average time in the ready queue and average
turnaround time is calculated based on the values of the processes in the
terminated queue.

The total number of processes that have completed, as well as the total
time that processing those processes has taken, is tracked via several
globally shared variables. When processing all processes has completed,
we calculate the throughput by finding: 

    total number of processes completed / total time processing in ms

These values are then printed to STDOUT in the required format.


######################################################
## The purposes of threads beyond the 3 required:
