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
  int secondsPassed; //counter to keep track of worker progress //used in loop
  
  //set time that worker should be allowed to stay in system
  int sec = arraytoint(argv[1]);
  int nano = arraytoint(argv[2]);
  
  //allocate memory, assign read only permissions
  int shmid_seconds = shmget(SYSCLK_SKEY, BUFF_SZ, IPC_CREAT | 0444);
  int shmid_nanoseconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0444);

  if(shmid_seconds <= 0 || shmid_nanoseconds <= 0)
    fatal("Child failed to create sys clock in shared memory");
  
  //attach to shared memory
  int * sysClock_seconds = shmat(shmid_seconds, 0, 0);
  int * sysClock_nanoseconds = shmat(shmid_nanoseconds, 0, 0);
   
  //store (?start and?) current time
  int curr_sec = *sysClock_seconds;
  int curr_nano = *sysClock_nanoseconds;
  int start_sec = curr_sec;    //used in loop
  //int start_nano = curr_nano; //might not need //used to start loop  
 
  //calculate termination time
  int term_sec = curr_sec + sec;
  int term_nano = curr_nano + nano;

  //output a status update at start-up
  msg = "--Just Starting";
  printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d \n\t%s\n\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano, msg);
    
  
  /************************************************
  loop from P1 - testing worker's output ->> not sure if loop correct/placeholder for testing
  loop until termination time, then final output.
  *************************************************/
  while (*sysClock_seconds <= term_sec) {
    //check to see if time exceeded by nanoseconds and if yes, exit while loop
    if (*sysClock_seconds == term_sec && *sysClock_nanoseconds >= term_nano)
      break; 
      
    //if at least 1 second has passed, output status update.
    if (curr_sec < *sysClock_seconds) {
      //update current time vars
      curr_sec = *sysClock_seconds;
      curr_nano = *sysClock_nanoseconds;
      
      //either increment secondsPassed or subtract curr_sec - start_sec
      secondsPassed = curr_sec - start_sec;
      
      //print output
      printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano);
      printf("\t--%d seconds have passed since starting\n", secondsPassed);
    }
  }
  //final output
  printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d \n\t--Terminating\n\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano);
  /************************************************/
  
  //detach (in parent and in child)
  shmdt(sysClock_seconds);
  shmdt(sysClock_nanoseconds);
  
  return EXIT_SUCCESS;
}

static void help() {
  printf("\n\n\tusage: ./worker seconds nanoseconds\n\n\n");
  exit(0);
}