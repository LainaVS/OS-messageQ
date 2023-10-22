//---------------------------------------------------
//  Elaina Rohlfing
//  October 12th 2023
//  4760 001 
//  Project 3 - Message Queues
//---------------------------------------------------

#ifndef ERRORUTILS_H
#define ERRORUTILS_H

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

void fatal(char*);
int arraytoint(char*);

void myhandler(int);
int setupinterrupt(void);
int setupitimer(void);

#endif