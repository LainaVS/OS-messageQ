#ifndef OSS_H
#define OSS_H

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SYSCLK_SKEY 6548964
#define SYSCLK_NSKEY 1564523
#define BUFF_SZ sizeof(int)
#define VERBOSE 1 //1 for debug, 0 for silent

#endif