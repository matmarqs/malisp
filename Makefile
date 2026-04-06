CC=gcc
CFLAGS=-g -std=c99 -Wall
LDFLAGS=-lreadline -lm

all: parsing.out doge.out

parsing.out: parsing.c mpc.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

doge.out: doge_code.c mpc.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.out
