CC	= gcc -g3
CFLAGS  = -g3 -Wall -Wshadow
TARGET1 = worker
TARGET2 = oss 

OBJS1	= worker.o validate.o
OBJS2	= oss.o validate.o

all:	$(TARGET1) $(TARGET2)

$(TARGET1):	$(OBJS1)
	$(CC) -o $(TARGET1) $(OBJS1)

$(TARGET2):	$(OBJS2)
	$(CC) -o $(TARGET2) $(OBJS2)

worker.o:	worker.c macros.h pcb.h
	$(CC) $(CFLAGS) -c worker.c macros.h pcb.h

oss.o:		oss.c macros.h pcb.h
	$(CC) $(CFLAGS) -c oss.c macros.h pcb.h
 
validate.o:	validate.c validate.h pcb.h
	$(CC) $(CFLAGS) -c validate.c validate.h pcb.h

clean:
	/bin/rm -f *.o *.gch $(TARGET1) $(TARGET2)

