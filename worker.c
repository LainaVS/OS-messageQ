//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------

/**********************************************************
 * Child will be worker - exec'd in parent, (need to refactor)
 * worker attaches to shared memory
 * worker args: time to stay in system: s ns (allotedTime)
 *   worker args selected randomly between 1 and -t
 * worker tasks:
 * **save current sysClock time
 * **calculate terminationTarget: currTime + workerArgs
 * **>> output currTime, terminationTarget, 'just starting'
 * **worker loops, 
 *     check sysClock to update currTime
 *     calculate elapsed time
 *     if 1sec elapsed (if prevSec < currSec or more likely: if elapsedSec > 0):
 *        >> output currTime, terminationTarget, timeElapsed
 *     if terminationTarget reached 
 *        >> output currTime, terminationTarget, 'terminating'
 *        terminate
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <stdbool.h>
#include "validate.h"
#include "macros.h" //system clock keys - might rename file

static void help();

int main(int argc, char** argv) {
  if(argc < 3) {
    help();
  }
  
  char * msg; //declare message variable for worker output statements
  //int secondsPassed = 0; //counter to keep track of worker progress //used in loop
  
  //set time that worker should be allowed to stay in system
  int sec = arraytoint(argv[1]);
  int nano = arraytoint(argv[2]);
  
  //allocate memory, assign read only permissions
  int shmid_seconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0444);
  int shmid_nanoseconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0444);

  if(shmid_seconds <= 0 || shmid_nanoseconds <= 0)
    fatal("Child failed to create sys clock in shared memory");
  
  //attach to shared memory
  int * sysClock_seconds = shmat(shmid_seconds, 0, 0);
  int * sysClock_nanoseconds = shmat(shmid_nanoseconds, 0, 0);
  
  //initialize system clock to zero 
  /*(((for testing worker only - will be done in parent)))*********
  *sysClock_seconds = 0;
  *sysClock_nanoseconds = 0;*/
   
  //store (?start and?) current time
  int curr_sec = *sysClock_seconds;
  int curr_nano = *sysClock_nanoseconds;
  //int start_sec = curr_sec;    //used in loop
  //int start_nano = curr_nano; //might not need //used to start loop  
 
  //calculate terminationTime (future)
  int term_sec = curr_sec + sec;
  int term_nano = curr_nano + nano;
  /***********************************************
   Start here for testing - test to see if worker is attached
  ***********************************************/
  //output a status update at start-up
  msg = "--Just Starting";
  printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d \n\t%s\n\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano, msg);
    
  
  /************************************************
  loop from P1 - test above first
  *************************************************
  for (int i = start_nano; i < term_nano; i+1000) { //not correct, just a placeholder so I can do something else
    //if at least 1 second has passed, output status update.
    if (curr_sec < *sysClock_seconds) {
      //update current time vars
      curr_sec = *sysClock_seconds;
      curr_nano = *sysClock_nanoseconds;
      
      //either increment secondsPassed or subtract curr_sec - start_sec
      secondsPassed = curr_sec - start_sec;
      
      //print output
      msg = "msgDependent-on-prog";
      printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano);
      printf("--%d seconds have passed since starting", secondsPassed);
    }
    if (*sysClock_seconds >= term_sec && *sysClock_nanoseconds >= term_nano) {
      msg = "--Terminating";
      printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d \n%s\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano, msg);
    }
    
    sleep(1);  //to be removed
  }
  printf("\n\n");
  ************************************************/
  
  return EXIT_SUCCESS;
}

static void help() {
  printf("\n\n\tusage: ./worker seconds nanoseconds\n\n\n");
  exit(0);
}