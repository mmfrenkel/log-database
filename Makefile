CC = gcc
CFLAGS = -g -Wall -std=c11 -fmax-errors=10
LDFLAGS = -g

all: main

main: two_three_tree.o binary_tree.o custom_io.o

binary_tree.o: binary_tree.h

two_three_tree.o: two_three_tree.h

custom_io.o: custom_io.h

.PHONY: clean
clean:
	rm -rf main *.o a.out

	
