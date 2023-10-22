//---------------------------------------------------
//  Elaina Rohlfing
//  October 12th 2023
//  4760 001 
//  Project 3 - Message Queues
//---------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "errorUtils.h"
#include "pcb.h"
#include "macros.h"

//clock globals
static int * psysClock_seconds;
static int * psysClock_nanoseconds;
static int pshmid_seconds;
static int pshmid_nanoseconds;

static void incrementClock(int*, int*);
static void generateArgs(int, char*, char*);
static void help();

int main(int argc, char** argv) {
  //set default args for options
  int proc = 1;
  int simul = 1;
  int timelim = 3;
  char * logfile = "log.txt";

  //parse options
  int option;
  while ((option = getopt(argc, argv, "hn:s:t:f: ")) != -1) {
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
      case 'f':
        logfile = optarg;              //set name for file that will store output
        break;
      case '?':

      return 1;
    }
  }
  
  if (VERBOSE == 1) { 
    printf("\n\n\tOSS will run %d workers. Workers will run up to %d seconds.\n\tMaximum of %d simultaneous workers allowed in system.\n\tOSS output will be recorded in %s file.\n\n", proc, timelim, simul, logfile); 
    }
    
  /**********************************************************
   create and intialize system clock to simulate time.
   If any steps fail, print a descriptive error.
   **********************************************************/  
    
  //allocate memory to shared keys
  pshmid_seconds = shmget(SYSCLK_SKEY, BUFF_SZ, IPC_CREAT | 0666);
  pshmid_nanoseconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0666);

  if(pshmid_seconds <= 0 || pshmid_nanoseconds <= 0)
    fatal("Parent failed to create System Clock in shared memory");
  
  //Attach to shared memory
  psysClock_seconds = shmat(pshmid_seconds, 0, 0);
  psysClock_nanoseconds = shmat(pshmid_nanoseconds, 0, 0);
  
  if (psysClock_seconds <= 0 || psysClock_nanoseconds <= 0) 
    fatal("Parent failed to attach to System Clock in shared memory");
    
  //initialize system clock to zero
  *psysClock_seconds = 0;
  *psysClock_nanoseconds = 0;

  /***********************************************************/ //testing clock in shared file

  //declare and initialize program variables
  initializeProcTable(processTable);
  char s_arg[50];            //char arrays to store time arguments for worker process
  char ns_arg[50];  
  int terminatedWorker_pid;  //wait pid
  int wstatus;               //wait status 
  int activeWorkers = 0;         //counter variables
  int workersToLaunch = proc;
  
  /****************************************
   signal handling and timeout
   ****************************************/
 		if (setupinterrupt() == -1) {
				perror("Failed to set up handler for SIGPROF");
				return 1;
		}
		if (setupitimer() == -1) {
				perror("Failed to set up the ITIMER_PROF interval timer");
				return 1;
		}
    
  /*******************************************************************
   main operation: Loop to fork child processes until all Processes have 
   completed. Process launching must obey simultaneous restriction provided
   by user. uses a simulated time function to increment shared clock.
   *******************************************************************/

  
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
        sleep(1);
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


/******************************************************** 
 The following functions were provided by Mark Hauschild 
 Course: 4760 Fall23
 File: periodicasterik.c 
 Kills processes and terminates program successfully at 
 timeout and ^C 
 ********************************************************/
 /*******************************************************
  PENDING ISSUES 
  --DOES NOT CLEAR SHARED MEM SEG - MUST RUN MEMCLEAR.SH
  --ALSO DOES NOT OUTPUT ERROR INFORMATION AT TIMEOUT
  *******************************************************/
void myhandler(int s) {
	int errsave;
	errsave = errno;
 
  write(STDERR_FILENO, &errsave, 1);
  printf("terminating"); ////////////////////THIS SHOULD CHANGE
 
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

int setupinterrupt(void) { /* set up myhandler for SIGPROF */
  struct sigaction act;
  act.sa_handler = myhandler;
  act.sa_flags = 0;
  return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL) || sigaction(SIGINT, &act, NULL) || sigaction(SIGTERM, &act, NULL));
}

int setupitimer(void) { /* set ITIMER_PROF for 60-second intervals */
  struct itimerval value;
  value.it_interval.tv_sec = 60;
  value.it_interval.tv_usec = 0;
  value.it_value = value.it_interval;
  return (setitimer(ITIMER_PROF, &value, NULL));
}