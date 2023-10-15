//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "pcb.h"
#include "validate.h"
#include "macros.h" //system clock keys - might rename file

static void incrementClock(int*, int*);
static void help();
//static struct PCB processTable[20]; //not yet
static int activeWorkers;
static int workersToLaunch;

int main(int argc, char** argv) {
  //set default args for options
  int proc = 3;
  int simul = 2;
  int timelim = 2;

  //parse options
  int option;
  while ((option = getopt(argc, argv, "hn:s:t: ")) != -1) {
    switch(option) {
      case 'h':  
        help();
        break;
      case 'n':
        proc = arraytoint(optarg);     //set number of worker processes to launch
        break;
      case 's':
        simul = arraytoint(optarg);    //set max number of processes to run simultaneously
        break;
      case 't':
        timelim = arraytoint(optarg);  //set timelimit for worker runtime
        break;
      case '?':

      return 1;
    }
  }
    
  //allocate memory to shared keys
  int pshmid_seconds = shmget(SYSCLK_SKEY, BUFF_SZ, IPC_CREAT | 0666);
  int pshmid_nanoseconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0666);

  if(pshmid_seconds <= 0 || pshmid_nanoseconds <= 0)
    fatal("Parent failed to create sys clock in shared memory");
  
  //Attach to shared memory
  int * psysClock_seconds = shmat(pshmid_seconds, 0, 0);
  int * psysClock_nanoseconds = shmat(pshmid_nanoseconds, 0, 0);
  
  if (psysClock_seconds <= 0 || psysClock_nanoseconds <= 0) 
    fatal("Parent failed to attach to Sysclock in shared memory");
    
  //initialize system clock to zero
  *psysClock_seconds = 0;
  *psysClock_nanoseconds = 0;

  /****************************************************
   1: create fake args for first worker test
   2: then process oss input (s and random ns)
   **max int: 2147483647
   **ns to s: 1,000,000,000
   ****************************************************/
   char * s_arg = "4";
   char * ns_arg = "2000";
   /****************************************************/    

  /****************************************************
   for testing with worker: fork and exec single worker
   ****************************************************/
  int workersInPCB = proc;
  activeWorkers = 0;
  workersToLaunch = workersInPCB;
  int w_pid; //wait id
  int wstatus;
  
  while(*psysClock_seconds < 60 && workersToLaunch > 0) {
    incrementClock(psysClock_seconds, psysClock_nanoseconds);
    
    //output pcb table every .5 sec
    if (*psysClock_nanoseconds == 500000)
      printf("\n\twhile pcb table\n\tsec: %d ns: %d\n\n", *psysClock_seconds, *psysClock_nanoseconds);
    
    if (activeWorkers < simul) {
      activeWorkers++;
      printf("\n\tlaunched worker");
      
      //fork 1 worker
      pid_t workerPid = fork();
      
      if (workerPid == 0) {  // in child
        // terminate if exec call fails
        char* args[] = {"./worker", s_arg, ns_arg, NULL};
        if (execvp(args[0], args) == -1) {
          perror("execvp");
          fatal("exec call failed");
        }
      } 
    }
    //for every child process, parent checks whether child is running or finished
    w_pid = waitpid(-1, &wstatus, WNOHANG); 
    if(w_pid == 0) { //when a child is running
      incrementClock(psysClock_seconds, psysClock_nanoseconds);
      
      if (*psysClock_nanoseconds == 500000)
        printf("\n\tif pcb table\n\tsec: %d ns: %d\n\n", *psysClock_seconds, *psysClock_nanoseconds);
        
      //has child terminated?
      w_pid = waitpid(-1, &wstatus, WNOHANG); 
    }  
    if (w_pid > 0) { //when a child is finished
      printf("Child process exited with status: %d\n", WEXITSTATUS(wstatus));
      workersToLaunch--; //remove from PCB
      activeWorkers--; //remove from active processes
      printf("\n\tworkers waiting: %d workers active: %d\n", workersToLaunch, activeWorkers);
    }
  }


 /****************************************************/
 
  //detach (in parent and in child)
  shmdt(psysClock_seconds);
  shmdt(psysClock_nanoseconds);
  
  //free shared memory segment (in parent only)
  shmctl(pshmid_seconds, IPC_RMID, NULL);
  shmctl(pshmid_nanoseconds, IPC_RMID, NULL);
  
  return EXIT_SUCCESS;
}

static void help() {
  printf("\n\n\tinfo.\n");
  printf("\tusage: ./oss [-h help -n proc -s simul  -t timelimit]\n\n");
  exit(0);
}

static void incrementClock(int * sys_sec, int * sys_nano){
  if (*sys_nano > 10000000) {
    //restart nano and add a second
    *sys_sec += 1;
    *sys_nano = 0;
  } else {
    *sys_nano += 50;
  }
}
