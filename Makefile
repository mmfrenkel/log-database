CC = gcc
CFLAGS = -g -Wall -std=c11 -fmax-errors=10
LDFLAGS = -g

all: clean main

main: binary_tree.o custom_io.o user_io.o error.o

binary_tree.o: binary_tree.h

custom_io.o: custom_io.h

user_io.o: user_io.h

error.o: error.h

.PHONY: clean
clean:
	rm -rf main *.o a.out
	
