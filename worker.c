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
  
  //read (and store) current time
  printf("current time: %ds %dns", *sysClock_seconds, *sysClock_nanoseconds);
  
  //calculate terminationTime (future)
  printf("terminate at %ds %dns", *sysClock_seconds + sec, *sysClock_nanoseconds + nano);
  
  
  /************************************************
  loop from P1
  *************************************************
  for (int i = 0; i < sec; i++) { // not correct, just a placeholder so I can eat lunch.
    printf("\n\tWORKER PID:%d  PPID:%d  Iteration:%d  before sleeping\n", getpid(), getppid(), i);
    sleep(1);
    printf("\n\tWORKER PID:%d  PPID:%d  Iteration:%d  after sleeping\n", getpid(), getppid(), i);
  }
  printf("\n\n");
  ************************************************/
  
  return EXIT_SUCCESS;
}

static void help() {
  printf("\n\n\tusage: ./worker seconds nanoseconds\n\n\n");
  exit(0);
}