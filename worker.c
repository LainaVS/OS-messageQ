//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "validate.h"

static void help();

int main(int argc, char** argv) {
  if(argc < 2) {
    help();
  }
  
  int iter = arraytoint(argv[1]);
  
  for (int i = 0; i < iter; i++) {
    printf("\n\tWORKER PID:%d  PPID:%d  Iteration:%d  before sleeping\n", getpid(), getppid(), i);
    sleep(1);
    printf("\n\tWORKER PID:%d  PPID:%d  Iteration:%d  after sleeping\n", getpid(), getppid(), i);
  }
  printf("\n\n");
  
  return EXIT_SUCCESS;
}

static void help() {
  printf("\n\n\tusage: ./worker int\n\n\n");
  exit(0);
}