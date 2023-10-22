//---------------------------------------------------
//  Elaina Rohlfing
//  October 12th 2023
//  4760 001 
//  Project 3 - Message Queues
//  This file contains functions to check the validity
//  of user input, signal handlers, timeouts, and
//  other error types.
//---------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "pcb.h"
#include "errorUtils.h"

void fatal(char * msg) {
  fprintf(stderr, "\n\n\t%s\n\tExiting program.\n\n", msg);
  exit(1);
}

int arraytoint(char *val) {
  if(atoi(val) < 1)
    fatal("Options require int values greater than zero.");
  return atoi(val);
}

/******************************************************** 
 The following functions were provided by Mark Hauschild 
 Course: 4760 Fall23
 File: periodicasterik.c 
 ********************************************************
void myhandler(int s) {
	int errsave;
	errsave = errno;
  //find and kill any running children
  for(int i = 0; i < PROCBUFF; i++) {
    if (processTable[i].occupied == OCCUPIED)
      kill(processTable[i].pid, SIGINT);
  }
  //free shared memory segment
  shmctl(pshmid_seconds, IPC_RMID, NULL);
  shmctl(pshmid_nanoseconds, IPC_RMID, NULL);
  errno = errsave;
}
int setupinterrupt(void) { /* set up myhandler for SIGPROF *
  struct sigaction act;
  act.sa_handler = myhandler;
  act.sa_flags = 0;
  return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL) || sigaction(SIGINT, &act, NULL) || sigaction(SIGTERM, &act, NULL));
}
int setupitimer(void) { /* set ITIMER_PROF for 60-second intervals *
  struct itimerval value;
  value.it_interval.tv_sec = 1;
  value.it_interval.tv_usec = 0;
  value.it_value = value.it_interval;
  return (setitimer(ITIMER_PROF, &value, NULL));
}
 ********************************************************/ //testing in oss.c