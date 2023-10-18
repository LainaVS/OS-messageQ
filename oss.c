//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "validate.h"
#include "pcb.h"
#include "macros.h" //system clock keys - might rename file

PCB processTable[PROCBUFF];

static void incrementClock(int*, int*);
static void generateArgs(int, char*, char*);
static void help();
static int activeWorkers;
static int workersToLaunch;

int main(int argc, char** argv) {
  //set default args for options //RESET TO ONE BEFORE SUBMISSION
  int proc = 3;
  int simul = 2;
  int timelim = 5;

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
   character arrays to hold random time args
    to send to worker
   **max int: 2,147,483,647
   **ns to s: 1,000,000,000
   ****************************************************/
   char s_arg[50];
   char ns_arg[50];
   /****************************************************/    

  /****************************************************
   for testing with worker: fork and exec single worker
   ****************************************************/
  int workersInPCB = proc;
  activeWorkers = 0;
  workersToLaunch = workersInPCB;
  int w_pid; //wait id 
  int wstatus;
  
  initializeProcTable(processTable);
  printProcTable(processTable, 5);
  
  time_t startTime = time(NULL);
  //While PCB table contains waiting processes //SECONDS COUNTER TO BE REPLACED WITH TIMEOUT FUNCTION
  while((workersToLaunch > 0 || activeWorkers > 0)) {
    incrementClock(psysClock_seconds, psysClock_nanoseconds);
    
    time_t endTime = time(NULL);
    if ((endTime - startTime) > 10) {
      //clean up processes and exit
      fatal("timeout");
    }
    //Print PCB table every half second
    if (*psysClock_nanoseconds % HALFSECOND_NS == 0)
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
        generateArgs(timelim, s_arg, ns_arg);
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
        
        if (*psysClock_nanoseconds % HALFSECOND_NS == 0)
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
  if (*sys_nano > ONESECOND_NS) {
    *sys_sec += 1;
    *sys_nano = 0;
  } else {
    *sys_nano += 500;
  }
}

static void generateArgs(int limit, char * arg_s, char * arg_ns) {
  srand(getpid());
  int a_sec = ((rand() % limit) + 1);
  int a_ns = ((rand() % ONESECOND_NS) + 1);
  
  //convert int variables to char * for processing 
  //(source https://www.geeksforgeeks.org/sprintf-in-c/)
  sprintf(arg_s, "%d", a_sec); 
  sprintf(arg_ns, "%d", a_ns);
  
  printf("generated: %ds %dns", a_sec, a_ns);
}
