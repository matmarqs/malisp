CC=gcc
CFLAGS=-g -std=c99 -Wall $(shell pkg-config --cflags libeditline)
LDFLAGS=$(shell pkg-config --libs libeditline) -lm

all: test

parsing.out: parsing.c mpc.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

doge.out: doge_code.c mpc.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.out

test: parsing.out
	cat tests.txt | ./parsing.out
