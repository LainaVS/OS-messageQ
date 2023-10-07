//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------


#ifndef PCB_H
#define PCB_H

struct PCB {
  int occupied;    // either true or false
  pid_t pid;       // process id of this child
  int startSeconds;// time when it was forked
  int startNano;   // time when it was forked
};

#endif