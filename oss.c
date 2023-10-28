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
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include "errorUtils.h"
#include "pcb.h"
#include "macros.h" 


//message queue
#define PERMS 0644
typedef struct {
	long mtype;  //used by msg send and recieve (but not SENT)
	int intData; //int data sent by child
} msgbuffer;

//message queue globals
static int msqid;

//prototypes
static void incrementClock(int*, int*);
static void generateArgs(int, char*, char*);
static void help();
void myhandler(int);
int setupinterrupt(void);
int setupitimer(void);

//clock globals
static int pshmid_seconds;
static int pshmid_nanoseconds;
static int * psysClock_seconds;
static int * psysClock_nanoseconds;

int main(int argc, char** argv) {
	//set default args for options
	int proc = 1;
	int simul = 1;
	int timelim = 3;
	char * logfile = "log.txt";

	//message queue variables
	msgbuffer buf0, buf1;
	key_t key;

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
	  Setting up message queue
	 **********************************************************/

	// get a key for our message queue - use program file to create unique key
	if ((key = ftok("oss", 1)) == -1) {
		perror("ftok");
		exit(1);
	}

	// create our message queue
	if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {
		perror("msgget in parent");
		exit(1);
	}

	printf("\n\n\t******\n\tMessage queue set up\n");


	/********************************************************************
	  create and intialize system clock to simulate time.
	  If any steps fail, print a descriptive error.
	 ********************************************************************/  

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

	/**********************end clock initialization*********************/

	//declare and initialize program variables
	initializeProcTable(processTable);
	char s_arg[50];            //char arrays to store time arguments for worker process
	char ns_arg[50];  
	int terminatedWorker_pid;  //wait pid
	int wstatus;               //wait status 
	int activeWorkers = 0;     //counter variables
	int workersToLaunch = proc;

	/********************************************************************
	  signal handling and timeout 
	  -- cleans up procs and detaches from mem. Therefore is placed after 
	  process table initialization and attaching to shared memory.
	 ********************************************************************/
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

	while((workersToLaunch > 0 || activeWorkers > 0)) {    
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


		/**********************************************************
		  Sending message to child
		  (send messages to children in order of process table)
		 **********************************************************/    
		if (processTable[0].occupied == OCCUPIED) {
			buf1.mtype = processTable[0].pid;		//set sending message type
			buf1.intData = processTable[0].pid; //we give it the pid we are sending to, so we know it received it

			if (msgsnd(msqid, &buf1, sizeof(msgbuffer)-sizeof(long), 0) == -1) {
				perror("msgsnd to child 1 failed\n");
				exit(1);
			}
			sleep(1); /////////////////////////////////////////////////REMOVE BEFORE SUBMIT
      
			/**********************************************************
			  Recieve message from child
			  (send messages to children in order of process table)
			 **********************************************************/
			msgbuffer rcvbuf;
			// Then let me read a message, but only one meant for me ie: the one the child just is sending back to me
			if (msgrcv(msqid, &rcvbuf,sizeof(msgbuffer), getpid(),0) == -1) {
				perror("failed to receive message in parent\n");
				exit(1);
			}	
			if (VERBOSE ==1 ) { printf("\n\t******\n\tParent %d received message: my int data was %d\n",getpid(),rcvbuf.intData); }

			// check to see if that worker (that sent me a msesage) is terminating?
      terminatedWorker_pid = waitpid(processTable[0].pid, &wstatus, WNOHANG);
      if (terminatedWorker_pid == processTable[0].pid) {
        activeWorkers--;
        terminatePCB(processTable, processTable[0].pid);
        if (VERBOSE == 1) { printf("\n\t******\n\tActive workers: %d\n\tClearing out process table...\n\n", activeWorkers); }
        wait(&wstatus);
      } else if (terminatedWorker_pid < 0) {
        perror("msgsnd to child 1 failed\n");
				exit(1);
      }
			//
			// if so, clear it out of the process table
			// then, do a wait(0);
			// the reason is to just clear it out of the system
			//
		}
		//}
	} //end while (main loop)
 
  if (VERBOSE ==1 ) { printf("\n\t******\n\tActive Workers: %d Pending Workers: %d\n\tReleasing memory and exiting ...\n\n", activeWorkers, workersToLaunch); }

	//clear message queue
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
		perror("msgctl to get rid of queue in parent failed");
		exit(1);
	}

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
	printf("\tusage: ./oss [-h help -n proc -s simul  -t timelimit -f logfile]\n\n");
	exit(0);
}

static void incrementClock(int * sys_sec, int * sys_nano){
	if (*sys_nano > ONESECOND_NS) {
		*sys_sec += 1;
		*sys_nano = 0;
	} else {
		*sys_nano += INCREMENTCLK; 
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
Kills processes successfully at timeout and ^C 
All processes cleaned up before exiting program
 ********************************************************/
void myhandler(int s) {
	int errsave;
	errsave = errno;

	fprintf(stderr, "\n\n\tCleaning up processes and terminating...\n\n"); 

	//clear message queue  
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
		perror("msgctl to get rid of queue in parent failed");
		exit(1);
	}

	//find and kill any running children
	for(int i = 0; i < PROCBUFF; i++) {
		if (processTable[i].occupied == OCCUPIED)
			kill(processTable[i].pid, SIGTERM);
	}

	//detach from shared memory 
	shmdt(psysClock_seconds);
	shmdt(psysClock_nanoseconds);

	//free shared memory segment
	shmctl(pshmid_seconds, IPC_RMID, NULL);
	shmctl(pshmid_nanoseconds, IPC_RMID, NULL);


	errno = errsave;
	exit(1); 
}

int setupinterrupt(void) { /* set up myhandler for SIGPROF */
	struct sigaction act;
	act.sa_handler = myhandler;
	act.sa_flags = 0;
	return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL) || sigaction(SIGINT, &act, NULL) || sigaction(SIGTERM, &act, NULL));
}

int setupitimer(void) { /* set ITIMER_PROF for 60-second intervals */
	struct itimerval value;
	value.it_interval.tv_sec = 3;
	value.it_interval.tv_usec = 0;
	value.it_value = value.it_interval;
	return (setitimer(ITIMER_PROF, &value, NULL));
}
