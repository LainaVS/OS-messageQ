#include <stdio.h>
#include "pcb.h"

/********************************************************
 * Create and initialize a new Process Control Block
 ********************************************************/
PCB newPCB(int occ, int id, int ss, int sns) {
  PCB new_PCB;
  new_PCB.occupied = occ;
  new_PCB.pid = id;
  new_PCB.startSeconds = ss;
  new_PCB.startNano = sns;
  
  return new_PCB;
}

/********************************************************
 * Update a Process Control Block
 ********************************************************/
PCB updatePCB(PCB pcb, int occ, int id, int * ss, int * sns) {
  pcb.occupied = occ;
  pcb.pid = id;
  pcb.startSeconds = *ss;
  pcb.startNano = *sns;
  
  return pcb;
}

/********************************************************
 * Initialize a Process table with empty PCBs
 ********************************************************/
void initializeProcTable(PCB * table) {
  for(int i = 0; i < PROCBUFF; i++)
    table[i] = newPCB(EMPTY, EMPTY, EMPTY, EMPTY);
}

/********************************************************
 * Search Process table for a PCB using a specific pid
 ********************************************************/
int findPCBEntry(PCB * table, int id) {
  for(int i = 0; i < PROCBUFF; i++) {
      if (table[i].pid == id)
        return i;
  }
  return -1;  
}

/********************************************************
 * Function to update PCB entry in process table after a
 * new process is launched.
 * Find a PCB that is waiting to be launched.
 /////////////////////////////////////////////////////////////////////////////////////
 * If a waiting block is not found, output error information and exit OR return -1
 /////////////////////////////////////////////////////////////////////////////////////
 ********************************************************
void activatePCBInTable(PCB * table, int id, int * ss, int * sns) {
  int index = findPCBEntry(table, EMPTY);
  if (index >= 0) {
    table[index] = updatePCB(table[index], OCCUPIED, id, ss, sns); 
  } else {
//////////////////////////////////////////////////////////////////////////////////////
    printf("process table full\n");
//////////////////////////////////////////////////////////////////////////////////////
  }
}
 ********************************************************/

/********************************************************
 * Function to indicate the process attached to a specific 
 * PCB has terminated. Search process table for the id of 
 * terminated process. Set occupied to 0 (TERMINATED) but 
 * keep process data.
  /////////////////////////////////////////////////////////////////////////////////////
 * If the id is not found, output error information and exit OR return -1
 /////////////////////////////////////////////////////////////////////////////////////
 ********************************************************/
void removeFromProcTable(PCB * table, int id) {
  int index = findPCBEntry(table, id);
  if (index > -1) {
    table[index].occupied = TERMINATED;
  } else {
//////////////////////////////////////////////////////////////////////////////////////
    printf("process does not exist");
//////////////////////////////////////////////////////////////////////////////////////
  }
}

/********************************************************
 * Functions to check status of processes in PCB
 ********************************************************/
/********************************************************
 * count the number of active processes in the process table
 * look for occupied PCBs that are not waiting for a child 
 * to attach. (pid > 0) Return the total active procs
 ********************************************************
int numOfActiveWorkers(PCB * table) {
  int active = 0;
  for(int i = 0; i < PROCBUFF; i++) {
    if (table[i].occupied == OCCUPIED) {
      if (table[i].pid > EMPTY)
        active++;
    }
  } 
  return active;
}
 ********************************************************/

/********************************************************
 * Search Process Table for any occupied PCBs that have a 
 * wait status (pid = 0). If any PCBs are found waiting
 * return occupied (1) (at least one occupied block is waiting), 
 * else return empty (0) (any remaining PCBs are unoccupied)
 ********************************************************
int stillWorkersToLaunch(PCB* table) {
  int waitingProc = findPCBEntry(table, WAITING);
  if (waitingProc >= 0) //if index is found
    return OCCUPIED;
  else
    return EMPTY;
}
 ********************************************************/


/********************************************************
 * Function to print ProcTable in table format
 * ////for testing allow limiting output - in proj use PROCBUFF////////////////////////
 ********************************************************/
void printProcTable(PCB * table, int maxEntries) {
  //printf("PID:%d SysClockS: %d SysClockNano: %d", id, sec, nano);
  printf("\n\tProcess Table:");

  // Print column headers
  printf("\n\t%-5s %-8s %-8s %-6s %-12s", "Entry", "Occupied", "PID", "StartS", "StartN");
  
  // Print table rows
  for (int row = 0; row < maxEntries; row++)
    printf("\n\t%-5d %-8d %-8d %-6d %-12d", row, table[row].occupied, table[row].pid, table[row].startSeconds, table[row].startNano);
  
  printf("\n");
}

