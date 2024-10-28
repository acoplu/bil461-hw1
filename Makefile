CC = gcc
CFLAGS = -Wall -g

all: osh

osh: osh.c
	$(CC) $(CFLAGS) -o osh osh.c

clean:
	rm -f osh