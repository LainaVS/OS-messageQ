//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------


#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pcb.h"
#include "validate.h"

static void help();

static struct PCB processTable[20];

int main(int argc, char** argv) {
  // set default args for options
  int process = 1;
  int simul = 1;
  int iter = 1;
  
  // initialize loop variables
  int batches = 0;
  int lastWorker = 0;
  int max = 0;

  int option;
  while ((option = getopt(argc, argv, "hn:s:t: ")) != -1) {
    switch(option) {
      case 'h':  
        help();
        break;
      case 'n':
        process = arraytoint(optarg);  //set number of worker processes to launch
        break;
      case 's':
        simul = arraytoint(optarg);    //set max number of processes to run simultaneously
        break;
      case 't':
        iter = arraytoint(optarg);     //set timelimit for worker runtime
        break;
      case '?':

      return 1;
    }
  }

  //initialize the system clock
  int sysClock;
  
  //loop to fork and exec call to workers. From psuedocode:
  /*while(stillChildrenToLaunch) {
    incrementClock();
    //print every half second (simulated clock time):
    printf("process table") //remove
    printf(
    
    if(childHasTerminated) {
      //update process table of terminated child
      
      //launch new child (but must obey process limits)
    }
  }*/
  //convert iter variable to char * for processing (source https://www.geeksforgeeks.org/sprintf-in-c/)
  char iterChar[50];
  sprintf(iterChar, "%d", iter); 
  
  //set up loop conditions
  batches = (process / simul);    // set number of full launches
  if (process % simul > 0) {      // set size of partial launch (if any)
    lastWorker = (process % simul);
    batches += 1;
  }

  //launch worker processes simultaneously 
  for(int launch = batches; launch > 0; launch--) {          // loop to launch all batches
    max = simul;
    if (launch == 1 && lastWorker > 0) {                     // restrict batch size when necessary
      max = lastWorker;
    }
          
    for (int s = 0; s < max; s++) {                          // loop to launch batch of forked workers
      pid_t workerPid = fork();  

      if (workerPid == 0) {                                  // child process to launch one worker
        char* args[] = {"./worker", iterChar};
        execvp(args[0], args);
        
        // terminate if exec call fails
        fatal("exec call failed");
      }
    }
    for (int s = 0; s < max; s++) {                          // end worker batch
      wait(NULL);
    }
  }
  
  return EXIT_SUCCESS;
}

static void help() {
  printf("\n\n\toss launches a series of simultaneous processes.\n");
  printf("\tusage: ./oss [-h help -n proc -s simul  -t iter]\n\n");
  exit(0);
}

