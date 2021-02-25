CFLAGS = -c -Wall
CC = gcc
LIBS = -lm

all: tableIndex

tableIndex : tableIndex.o
	$(CC) -w tableIndex.o -o tableIndex

tableIndex.o: tableIndex.c
	$(CC) -w $(CFLAGS) tableIndex.c

clean:
	rm -f *.o *-
