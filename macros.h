//---------------------------------------------------
//  Elaina Rohlfing
//  October 12th 2023
//  4760 001 
//  Project 3 - Message Queues
//---------------------------------------------------

#ifndef OSS_H
#define OSS_H

//shared memory
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//shared memory
#define SYSCLK_SKEY 6548964
#define SYSCLK_NSKEY 1564523
#define BUFF_SZ sizeof(int)

//toggle output for development: 1 for MORE output
#define VERBOSE 0

//time constants
#define ONESECOND_NS 1000000000
#define HALFSECOND_NS 500000000
#define TIMEOUT 60
#define INCREMENTCLK 500000000

#endif