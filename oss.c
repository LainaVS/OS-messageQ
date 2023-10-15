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
#include "validate.h"
#include "macros.h" //system clock keys - might rename file

typedef struct {
  int occupied;    // either true or false
  pid_t pid;       // process id of this child
  int startSeconds;// time when it was forked
  int startNano;   // time when it was forked
} PCB;

PCB processTable[20];

static void incrementClock(int*, int*);
static void help();
static int activeWorkers;
static int workersToLaunch;

int main(int argc, char** argv) {
  //set default args for options //RESET TO ONE BEFORE SUBMISSION
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
    fatal("Parent failed to create System Clock in shared memory");
  
  //Attach to shared memory
  int * psysClock_seconds = shmat(pshmid_seconds, 0, 0);
  int * psysClock_nanoseconds = shmat(pshmid_nanoseconds, 0, 0);
  
  if (psysClock_seconds <= 0 || psysClock_nanoseconds <= 0) 
    fatal("Parent failed to attach to System Clock in shared memory");
    
  //initialize system clock to zero
  *psysClock_seconds = 0;
  *psysClock_nanoseconds = 0;

  /****************************************************
   actual option processing needs to happen at worker launch
   (random s and random ns)
   **max int: 2,147,483,647
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
  
  //While PCB table contains waiting processes //SECONDS COUNTER TO BE REPLACED WITH TIMEOUT FUNCTION
  while(*psysClock_seconds < 60 && (workersToLaunch > 0 || activeWorkers > 0)) {
    incrementClock(psysClock_seconds, psysClock_nanoseconds);
    
    //Print PCB table every half second
    if (*psysClock_nanoseconds % 500000000 == 0)
      printf("\n\twhile pcb table\n\tsec: %d ns: %d\n\n", *psysClock_seconds, *psysClock_nanoseconds);
    
    //Whenever possible, launch a new process
    if (activeWorkers < simul && workersToLaunch > 0) {
    
      //Update PCB Table
      activeWorkers++;    // REPLACE WITH PCB FUNTIONALITY
      workersToLaunch--; //remove from PCB
      
      if (VERBOSE == 1) { printf("\n\tworkers waiting: %d workers active: %d\n\t--worker launched\n", workersToLaunch, activeWorkers); }
      
      //Fork new process
      pid_t workerPid = fork();
      
      //In child process: exec to worker or terminate on failure
      if (workerPid == 0) {
        char* args[] = {"./worker", s_arg, ns_arg, NULL};
        if (execvp(args[0], args) == -1) {
          perror("execvp");
          fatal("exec call failed");
        }
      } 
    }
    
    //In parent: as long as there are active workers, monitor their progress
    if (activeWorkers > 0) {
      w_pid = waitpid(-1, &wstatus, WNOHANG);
      
      //While worker is active, increment system clock
      if (w_pid == 0) {
        incrementClock(psysClock_seconds, psysClock_nanoseconds);
        
        if (*psysClock_nanoseconds % 500000000 == 0)
          printf("\n\tif pcb table\n\tsec: %d ns: %d\n\n", *psysClock_seconds, *psysClock_nanoseconds);
      } 
      //When a worker terminates, update PCB Table
      else if (w_pid > 0) {          
        activeWorkers--; //remove from active processes //INSERT PCB
        
        if (VERBOSE == 1) {
          printf("Child process exited with status: %d\n", WEXITSTATUS(wstatus));
          printf("\n\tworkers waiting: %d workers active: %d\n\t--worker finished\n", workersToLaunch, activeWorkers);
        }
        
      } 
      //Error in waitpid
      else {
        perror("waitpid");
        fatal("waitpid failed");
      }
    }
  }
  
  if (VERBOSE == 1) { printf("\n\touter: workers waiting: %d workers active: %d\n", workersToLaunch, activeWorkers); }

  //detach from shared memory
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
  if (*sys_nano > 1000000000) {
    *sys_sec += 1;
    *sys_nano = 0;
  } else {
    *sys_nano += 1000;
  }
}
