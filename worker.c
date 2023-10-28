//---------------------------------------------------
//  Elaina Rohlfing
//  October 12th 2023
//  4760 001 
//  Project 3 - Message Queues
//---------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "errorUtils.h"
#include "macros.h"

//message queue
#define PERMS 0644
typedef struct {
	long mtype;  //used by msg send and recieve (but not SENT)
	int intData; //int data sent by child
} msgbuffer;

static void help();

int main(int argc, char** argv) {
	//message queue variables
	msgbuffer buf;
	buf.mtype = 1;
	int msqid = 0;
	key_t key;

	//check for appropriate usage
	if(argc < 3) {
		help();
	}

	//set time that worker should be allowed to stay in system
	int sec = arraytoint(argv[1]);
	int nano = arraytoint(argv[2]);

	/***********************************************************************
	  Attaching worker to the system clock for monitoring. 
	  If any steps fail, print a descriptive error.
	 ***********************************************************************/

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

	/***********************************************************************
	  Set up message queue
	 ***********************************************************************/

	// get a key for our message queue
	if ((key = ftok("oss", 1)) == -1) {
		perror("ftok");
		exit(1);
	}

	// create message queue
	if ((msqid = msgget(key, PERMS)) == -1) {
		perror("msgget in child");
		exit(1);
	}

	if (VERBOSE == 1) { printf("\n\t******\n\tChild %d has access to the queue\n",getpid()); }

	/***********************************************************************
	  Worker will continuously print status updates until it 
	  notices the system clock has reached the calculated 
	  termination time. It will print a final update then exit.
	 ***********************************************************************/
	int secondsPassed; //counter to keep track of worker progress

	//output a status update at start-up
	printf("\n\tWORKER PID:%d  PPID:%d  SysClockS:%d  SysClockNano:%d  TermTimeS:%d  TermTimeNano:%d \n\t--Just Starting\n\n", getpid(), getppid(), *sysClock_seconds, *sysClock_nanoseconds, term_sec, term_nano);

	//loop until system clock exceeds worker's termination time
	while (*sysClock_seconds <= term_sec) {    
   //recieving message from parent    
		if ( msgrcv(msqid, &buf, sizeof(msgbuffer), getpid(), 0) == -1) {
			perror("failed to receive message from parent\n");
			exit(1);
		}

		if (VERBOSE ==1 ) { printf("\n\t******\n\tChild %d received message: my int data was %d\n",getpid(), buf.intData); }

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

		buf.mtype = getppid();  //set msg type to get PARENT PID
    buf.intData = CONTINUING;       //continue
    
    //Worker operations completed: send message indicating continue
		if (msgsnd(msqid,&buf,sizeof(msgbuffer)-sizeof(long),0) == -1) {
			perror("msgsnd to parent failed\n");
			exit(1);
		}

		if (VERBOSE == 1 ) { printf("\n\t******\n\tChild %d is done examining clock ... \n",getpid()); }
	}

  //notify parent of termination
  buf.mtype = getppid();  //set msg type to get PARENT PID
  buf.intData = TERMINATING;        //about to terminate	
  if (VERBOSE == 1 ) { printf("\n\t******\n\tSending termination message to parent ...\n"); }
  
  if (msgsnd(msqid,&buf,sizeof(msgbuffer)-sizeof(long),0) == -1) {
    perror("msgsnd to parent failed\n");
    exit(1);
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
