TARGETS = cleaner prog
CC = gcc
CFLAGS = -g -Wall -pedantic -D_POSIX_C_SOURCE
LIB=ar rcs

all: $(TARGETS)

prog: prog.o libloglib.a loglib.h memory.h
	$(CC) $(CFLAGS) memory.c -o prog prog.o -L. -lloglib

prog.o: prog.c
	$(CC) $(CFLAGS) -c prog.c

cleaner: cleaner.o libloglib.a loglib.h memory.h
	$(CC) $(CFLAGS) memory.c -o cleaner cleaner.o -L. -lloglib

cleaner.o: cleaner.c
	$(CC) $(CFLAGS) -c cleaner.c

log.o: log.c
	$(CC) $(CFLAGS) -c log.c

libloglib.a: log.o
	$(LIB) libloglib.a log.o

clean:
	-rm -f $(TARGETS) *.o *.a
