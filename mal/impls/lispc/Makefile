CC = gcc
CFLAGS = -std=c99 -g -Wall $(shell pkg-config --cflags libeditline)
LDFLAGS =  $(shell pkg-config --libs libeditline)

step0_repl: step0_repl.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
