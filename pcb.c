//---------------------------------------------------
//  Elaina Rohlfing
//  October 12th 2023
//  4760 001 
//  Project 3 - Message Queues
//---------------------------------------------------

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
 * new process is launched. Find first proccess block without 
 * a child attached (PID is 0 (EMPTY)). Swith PCB to OCCUPIED (1)
 * record child pid, start seconds, and start ns. If all 
 * process blocks have a child attached, output error information. //change later
 ********************************************************/
void activatePCB(PCB * table, int id, int * ss, int * sns) {
  int index = findPCBEntry(table, EMPTY);
  if (index >= 0) {
    table[index] = updatePCB(table[index], OCCUPIED, id, ss, sns); 
  } else {
    printf("process table full\n");
  }
}


/********************************************************
 * Function to indicate the process attached to a specific 
 * PCB has terminated. Search process table for the id of 
 * terminated process. Set occupied to 0 (TERMINATED) but 
 * keep process data.
 * If the id is not found, output error information. //change later
 ********************************************************/
void terminatePCB(PCB * table, int id) {
  int index = findPCBEntry(table, id);
  if (index > -1) {
    table[index].occupied = TERMINATED;
  } else {
    printf("process does not exist");
  }
}

/********************************************************
 * Function to count the number of active processes in the 
 * process table. Count only occupied PCBs.
 ********************************************************/
int numOfActiveWorkers(PCB * table) {
  int active = 0;
  for(int i = 0; i < PROCBUFF; i++) {
    if (table[i].occupied == OCCUPIED) {
      active++;
    }
  } 
  return active;
}


/********************************************************
 * Function to print complete ProcTable in table format
 ********************************************************/
void printProcTable(PCB * table, int id, int * sec, int * nano) {
  //print parent information
  printf("\tOSS PID:%d SysClockS: %d SysClockNano: %d\n\tProcess Table:", id, *sec, *nano);
  
  //print table
  printf("\n\t%-5s %-8s %-8s %-6s %-12s", "Entry", "Occupied", "PID", "StartS", "StartN");
  
  for (int row = 0; row < PROCBUFF; row++)
    printf("\n\t%-5d %-8d %-8d %-6d %-12d", row, table[row].occupied, table[row].pid, table[row].startSeconds, table[row].startNano);
  
  printf("\n");
}

