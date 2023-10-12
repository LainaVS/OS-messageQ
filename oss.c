//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------


#include <stdlib.h>
//#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "pcb.h"
#include "validate.h"
#include "macros.h" //system clock keys - might rename file
/*
  //initialize the system clock (alternately use #define)
  const int sysClock_skey = ftok("oss.c", 1);   //generate seconds key based on file
  const int sysClock_nskey = ftok("oss.c", 2);  //generate nanoseconds key based on file  
*/

static void incrementClock();
static void parent();
static void child();
static void help();

static struct PCB processTable[20];


int main(int argc, char** argv) {
  //set default args for options
  int proc = 1;
  int simul = 1;
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
  int shmid_seconds = shmget(SYSCLK_SKEY, BUFF_SZ, IPC_CREAT | 0666);
  int shmid_nanoseconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0666);

  if(shmid_seconds <= 0 || shmid_nanoseconds <= 0)
    fatal("Parent failed to create sys clock in shared memory");
  
  //Attach to shared memory (in parent and in child)
  int * sysClock_seconds = shmat(shmid_seconds, 0, 0);
  int * sysClock_nanoseconds = shmat(shmid_nanoseconds, 0, 0);
  
  if (sysClock_seconds <= 0 || sysClock_nanoseconds <= 0) 
    fatal("Parent failed to attach to Sysclock in shared memory");
  
/***************************************************
 * 2) go into 60 second loop ->
 *    3) increment clock every iteration (should roughly match real time)
 *       >> output process table every half second
 *    4) if -s < max: fork -> exec worker (child) and update processTable: (or while/until?)
 *       OCCUPIED 1/0, PID chProcId, STARTSEC timeAtLaunch, STARTNANO timeAtLaunch
 *    5) if -s > max: WAIT until a child terminates ((nonblocking wait() call)) 
 *       int pid = waitpid(-1, &status, WNOHANG); -> ((return 0 if none term else pid of term'ed))
 *       6) when child term: update childPCB entry
 *          7) if proc (worker) remaining: loop
 *          8) else: fin
  ***************************************************/
  
  /****************************************************
   ****************************************************
   * From psuedocode: loop to fork and exec call to workers. 
   ****************************************************/
  bool stillChildrenToLaunch = true;
  bool childHasTerminated = true;
  while(stillChildrenToLaunch) {
    // incrementClock();
    sleep(1);
    *sysClock_seconds += 1;
    *sysClock_nanoseconds += 1000;
    //print every half second (simulated clock time):
    printf("Parent: Incrementing SysClock at %ds %dns\n", *sysClock_seconds, *sysClock_nanoseconds);
    printf("process table"); //remove
     
    if(childHasTerminated) {
      //update process table of terminated child
      childHasTerminated = false;
      //launch new child (but must obey process limits)
      pid_t workerPid = fork();
      
      stillChildrenToLaunch = false;
    }
  }
  
  /***************************************************
  ***************************************************/
  
  
  
  /****************************************
  //convert iter variable to char * for processing 
  //(source https://www.geeksforgeeks.org/sprintf-in-c/)
  char iterChar[50];
  sprintf(iterChar, "%d", iter); 
  ****************************************/

  
  return EXIT_SUCCESS;
}

static void incrementClock() {

}

/**********************************************************
 * parent desc (oss tasks)
 ********************************************************** 
 * launch n (proc) number of workers (children)
 * uses parameters !--->(probably needs to be moved back into main)<---!
 * -t (timelim) sets worker args at random: 
 * ** Example: -t 7, all n workers are given a random time 
 * ** ** sec: between 1 and 7 
 * ** ** nano: between 1000 - 10000 (these values are hard coded - choose wisely)
 * parent steps:
 * 1) initialize clock (done)
 * 2) go into 60 second loop ->
 *    3) increment clock every iteration (should roughly match real time)
 *       >> output process table every half second
 *    4) if -s < max: fork -> exec worker (child) and update processTable: (or while/until?)
 *       OCCUPIED 1/0, PID chProcId, STARTSEC timeAtLaunch, STARTNANO timeAtLaunch
 *    5) if -s > max: WAIT until a child terminates ((nonblocking wait() call)) 
 *       int pid = waitpid(-1, &status, WNOHANG); -> ((return 0 if none term else pid of term'ed))
 *       6) when child term: update childPCB entry
 *          7) if proc (worker) remaining: loop
 *          8) else: fin
 *********************************************************/
static void parent() {
  int i;
  
  //allocate memory to shared keys
  int shmid_seconds = shmget(SYSCLK_SKEY, BUFF_SZ, IPC_CREAT | 0666);
  int shmid_nanoseconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0666);

  if(shmid_seconds <= 0 || shmid_nanoseconds <= 0)
    fatal("Parent failed to create sys clock in shared memory");
  
  //Attach to shared memory (in parent and in child)
  int * sysClock_seconds = shmat(shmid_seconds, 0, 0);
  int * sysClock_nanoseconds = shmat(shmid_nanoseconds, 0, 0);
  
  if (sysClock_seconds <= 0 || sysClock_nanoseconds <= 0) 
    fatal("Parent failed to attach to Sysclock in shared memory");
    
  //increment the clock (make separate function)
  for (i = 0; i < 10; i++) {
    sleep(2);
    *sysClock_seconds += 1;
    *sysClock_nanoseconds += 100;
    printf("Parent: Incrementing SysClock at %ds %dns\n", *sysClock_seconds, *sysClock_nanoseconds);
  }
  //detach and free up shared memory
  //detach (in parent and in child)
  shmdt(sysClock_seconds);
  shmdt(sysClock_nanoseconds);
  
  //free shared memory segment (in parent only)
  shmctl(shmid_seconds, IPC_RMID, NULL);
  shmctl(shmid_nanoseconds, IPC_RMID, NULL);
  
  printf("Parent Terminating\n");
}

/**********************************************************
 * Child will be worker - exec'd in parent, (need to refactor)
 * worker attaches to shared memory
 * worker args: time to stay in system: s ns (allotedTime)
 *   worker args selected randomly between 1 and -t
 * worker tasks:
 * **save current sysClock time
 * **calculate terminationTarget: currTime + workerArgs
 *   >> output currTime, terminationTarget, 'just starting'
 *   worker loops, 
 *     check sysClock to update currTime
 *     calculate elapsed time
 *     if 1sec elapsed (if prevSec < currSec or more likely: if elapsedSec > 0):
 *        >> output currTime, terminationTarget, timeElapsed
 *     if terminationTarget reached 
 *        >> output currTime, terminationTarget, 'terminating'
 *        terminate
 *********************************************************/
static void child() {
  int i;
  
  sleep(5);
  
  //allocate memory to shared keys (shild only needs read permissions)
  int shmid_seconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0444);
  int shmid_nanoseconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0444);

  if(shmid_seconds <= 0 || shmid_nanoseconds <= 0)
    fatal("Child failed to create sys clock in shared memory");
  
  //Attach to shared memory (in parent and in child)
  int * sysClock_seconds = shmat(shmid_seconds, 0, 0);
  int * sysClock_nanoseconds = shmat(shmid_nanoseconds, 0, 0);
  
  if (sysClock_seconds <= 0 || sysClock_nanoseconds <= 0) 
    fatal("Child failed to attach to Sysclock in shared memory");
    
  //look at the clock
  for(i=0; i < 10; i++) {
    sleep(1);
    printf("Child: Checking SysClock at %ds %dns\n", *sysClock_seconds, *sysClock_nanoseconds);
  }
  
  
  //detach (in parent and in child)
  shmdt(sysClock_seconds);
  shmdt(sysClock_nanoseconds);
}

static void help() {
  printf("\n\n\tinfo.\n");
  printf("\tusage: ./oss [-h help -n proc -s simul  -t timelimit]\n\n");
  exit(0);
}

