# David Töyrä
# Makefile for mfind
FLAGS= -std=gnu11 -Wall -pthread
CC= gcc
all:    mfind

mfind:   mfind.o list.o
	$(CC) $(FLAGS) -o  mfind mfind.o list.o

list.o:      list.c list.h
	$(CC) $(FLAGS) -c list.c

mfind.o:      mfind.c mfind.h
	$(CC) $(FLAGS) -c mfind.c

clean:
	rm -f *.o mfind
