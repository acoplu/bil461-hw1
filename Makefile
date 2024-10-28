all: osh

osh: osh.c
	gcc -o osh osh.c

clean:
	rm -f osh
