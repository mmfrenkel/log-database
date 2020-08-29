CC = gcc
CFLAGS = -g -Wall -std=c11 -fmax-errors=10
LDFLAGS = -g

all: clean main

main: lsm_tree.o memtable.o custom_io.o user_io.o error.o

lsm_tree.o: lsm_tree.h

memtable.o: memtable.h

custom_io.o: custom_io.h

user_io.o: user_io.h

error.o: error.h

.PHONY: clean
clean:
	rm -rf main *.o a.out *.log
	
