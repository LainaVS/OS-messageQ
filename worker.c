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
#include "validate.h"
#include "macros.h" //system clock keys - might rename file

static void help();

int main(int argc, char** argv) {
  //check for appropriate usage
  if(argc < 3) {
    help();
  }
  
  /***************************************************
   Attaching worker to the system clock for monitoring. 
   If any steps fail, print a descriptive error.
   ***************************************************/
   
  //set time that worker should be allowed to stay in system
  int sec = arraytoint(argv[1]);
  int nano = arraytoint(argv[2]);
  
  //allocate memory, assign read only permissions
  int shmid_seconds = shmget(SYSCLK_SKEY, BUFF_SZ, IPC_CREAT | 0444);
  int shmid_nanoseconds = shmget(SYSCLK_NSKEY, BUFF_SZ, IPC_CREAT | 0444);

  if(shmid_seconds <= 0 || shmid_nanoseconds <= 0)
    fatal("Worker failed to create System Clock in shared memory");
  
  //attach to shared memory
  int * sysClock_seconds = shmat(shmid_seconds, 0, 0);
  int * sysClock_nanoseconds = shmat(shmid_nanoseconds, 0, 0);
  
  if (sysClock_seconds <= 0 || sysClock_nanoseconds <= 0) 
    fatal("Worker failed to attach to System Clock in shared memory");
   
  //look at system clock and record current time
  int curr_sec = *sysClock_seconds;
  int curr_nano = *sysClock_nanoseconds;
  int start_sec = curr_sec; 
 
  //calculate termination time
  int term_sec;
  int term_nano;
  if ((curr_nano + nano) > ONESECOND_NS) {
    term_sec = (curr_sec + sec) + 1;
    term_nano = (curr_nano + nano) - ONESECOND_NS;
  } else {
    term_sec = (curr_sec + sec);
    term_nano = (curr_nano + nano);
  }

  /***************************************************
   Worker will continuously print status updates until 
   it notices the system clock has reached the calculated 
   termination time. It will print a final update then 
   exit.
   ***************************************************/
  int secondsPassed; //counter to keep track of worker progress

  //output a status update at start-up
  printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d \n\t--Just Starting\n\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano);
    
  //loop until system clock exceeds worker's termination time
  while (*sysClock_seconds <= term_sec) {
  
    //special case: if termination second is reached, check whether termination nano has been exceeded
    if (*sysClock_seconds == term_sec && *sysClock_nanoseconds >= term_nano)
      break; 
      
    //look at system clock and print status update
    if (curr_sec < *sysClock_seconds) {
      
      curr_sec = *sysClock_seconds;
      curr_nano = *sysClock_nanoseconds;
      secondsPassed = curr_sec - start_sec;

      printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d\n\t--%d seconds have passed since starting\n", getpid(), getppid(), curr_sec, curr_nano, term_sec, term_nano, secondsPassed);
    }
  }
  
  //termination condition was met - print final status update
  printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d \n\t--Terminating\n\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano);
  
  //detach from shared memory
  shmdt(sysClock_seconds);
  shmdt(sysClock_nanoseconds);
  
  return EXIT_SUCCESS;
}

static void help() {
  printf("\n\n\tusage: ./worker seconds nanoseconds\n\n\n");
  exit(0);
}