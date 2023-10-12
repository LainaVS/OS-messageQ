//---------------------------------------------------
//  Elaina Rohlfing
//  October 5, 2023
//  4760 001 
//  Clock
//---------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "validate.h"

void fatal(char * msg) {
  fprintf(stderr, "\n\n\t%s\n\tExiting program.\n\n", msg);
  exit(1);
}

int arraytoint(char *val) {
  if(atoi(val) < 1)
    fatal("Options require int values greater than zero.");
  return atoi(val);
}