//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------


#ifndef PCB_H
#define PCB_H

#include <unistd.h>

#define PROCBUFF 20
#define OCCUPIED 1
#define EMPTY 0
#define TERMINATED 0

typedef struct {
  int occupied;    // true (1) if active process, false (0) if empty or if terminated
  pid_t pid;       // process id of this child
  int startSeconds;// time when it was forked
  int startNano;   // time when it was forked
} PCB;

PCB newPCB(int, int, int, int); 
PCB updatePCB(PCB, int, int, int*, int*);
void initializeProcTable(PCB*);
void printProcTable(PCB*, int, int*, int *);

int findPCBEntry(PCB*, int);
void activatePCB(PCB*, int, int*, int*);
void terminatePCB(PCB*, int);

int numOfActiveWorkers(PCB*);

#endif