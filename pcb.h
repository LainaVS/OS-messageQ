//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------


#ifndef PCB_H
#define PCB_H

#include <unistd.h>
#include <stdbool.h>

#define PROCBUFF 20
#define OCCUPIED 1
#define WAITING 0
#define EMPTY 0
#define TERMINATE 0

typedef struct {
  int occupied;    // either true or false
  pid_t pid;       // process id of this child
  int startSeconds;// time when it was forked
  int startNano;   // time when it was forked
} PCB;

PCB newPCB(int, int, int, int); 
PCB updatePCB(PCB, int, int, int*, int*);
void initializeProcTable(PCB*, int);
void printProcTable(PCB*, int, int, int);

int findPCBEntry(PCB*, int);
void activatePCBInTable(PCB*, int, int*, int*);
void removeFromProcTable(PCB*, int);

int numOfActiveWorkers(PCB*);
//int stillWorkersToLaunch(PCB*);

#endif