//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------


#ifndef PCB_H
#define PCB_H

#include <unistd.h>

typedef struct {
  int occupied;    // either true or false
  pid_t pid;       // process id of this child
  int startSeconds;// time when it was forked
  int startNano;   // time when it was forked
} PCB;

PCB processTable[20];

#endif