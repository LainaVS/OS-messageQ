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
#include "macros.h" 

PCB processTable[PROCBUFF];

static void incrementClock(int*, int*);
static void generateArgs(int, char*, char*);
static void help();
static int activeWorkers;
static int workersToLaunch;

int main(int argc, char** argv) {
  //set default args for options
  int proc = 1;
  int simul = 1;
  int timelim = 3;

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
  
  /**********************************************************
   create and intialize system clock to simulate time.
   If any steps fail, print a descriptive error.
   **********************************************************/  
    
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

  
  char s_arg[50];            //char arrays to store time arguments for worker process
  char ns_arg[50];  
  int terminatedWorker_pid;  //wait pid
  int wstatus;               //wait status 
  activeWorkers = 0;         //counter variables
  workersToLaunch = proc;
    
  /*******************************************************************
   main operation: Loop to fork child processes until all Processes have 
   completed. Process launching must obey simultaneous restriction provided
   by user. uses a simulated time function to increment shared clock.
   *******************************************************************/
  initializeProcTable(processTable);
  
  time_t startTime = time(NULL);
  while((workersToLaunch > 0 || activeWorkers > 0)) {
  
    //timeout - should handle process clean up - should be implemented with timeoutsignal
    time_t progRunTime = time(NULL) - startTime;
    if (progRunTime > 60) {
      fatal("timeout");
    }
    
    incrementClock(psysClock_seconds, psysClock_nanoseconds);
     
    //Print process table every half second
    if (*psysClock_nanoseconds % HALFSECOND_NS == 0)
      printProcTable(processTable, getpid(), psysClock_seconds, psysClock_nanoseconds);
    
    //Whenever possible, launch a new process
    if (activeWorkers < simul && workersToLaunch > 0) {
    
      //Keep track of active and waiting worker processes
      activeWorkers++;    
      workersToLaunch--;
            
      //Fork new process
      pid_t runningWorker_pid = fork();
      
      //In child process: exec to worker
      if (runningWorker_pid == 0) {
        generateArgs(timelim, s_arg, ns_arg);
        char* args[] = {"./worker", s_arg, ns_arg, NULL};
        if (execvp(args[0], args) == -1) {
          perror("execvp");
          fatal("exec call failed");
        }
      }
      //In parent process: update process table
      else if (runningWorker_pid > 0) {
        activatePCB(processTable, runningWorker_pid, psysClock_seconds, psysClock_nanoseconds);
      }
    }
    
    //In parent: as long as there are active workers, monitor their progress
    if (activeWorkers > 0) {
      terminatedWorker_pid = waitpid(-1, &wstatus, WNOHANG);
      
      //When parent sees worker has terminated, update the process Table
      if (terminatedWorker_pid > 0) {       
        activeWorkers--;
        terminatePCB(processTable, terminatedWorker_pid);
      } 
      //Error in waitpid
      else if (terminatedWorker_pid < 0){
        perror("waitpid");
        fatal("waitpid failed");
      }
    }
  } //end while (main loop)

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
    *sys_nano += 1000;
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
