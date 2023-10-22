CC	= gcc -g3
CFLAGS  = -g3 -Wall -Wshadow
TARGET1 = worker
TARGET2 = oss 

OBJS1	= worker.o errorUtils.o
OBJS2	= oss.o errorUtils.o pcb.o

all:	$(TARGET1) $(TARGET2)

$(TARGET1):	$(OBJS1)
	$(CC) -o $(TARGET1) $(OBJS1)

$(TARGET2):	$(OBJS2)
	$(CC) -o $(TARGET2) $(OBJS2)

worker.o:	worker.c macros.h
	$(CC) $(CFLAGS) -c worker.c macros.h

oss.o:		oss.c macros.h
	$(CC) $(CFLAGS) -c oss.c macros.h
 
errorUtils.o:	errorUtils.c errorUtils.h
	$(CC) $(CFLAGS) -c errorUtils.c errorUtils.h

pcb.o:		pcb.c pcb.h
	$(CC) $(CFLAGS) -c pcb.c pcb.h

clean:
	/bin/rm -f *.o *.gch $(TARGET1) $(TARGET2)

