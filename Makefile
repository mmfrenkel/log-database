CC = gcc
CFLAGS = -g -Wall -std=c11 -fmax-errors=10
LDFLAGS = -g

all: main

main: two_three_tree.o

two_three_tree.o: two_three_tree.h

.PHONY: clean
clean:
	rm -rf main *.o a.out

	
