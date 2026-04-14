CC = gcc
CFLAGS = -std=c99 -g -Wall $(shell pkg-config --cflags libeditline libpcre2-8)
LDFLAGS =  $(shell pkg-config --libs libeditline libpcre2-8)

all: step0_repl step1_read_print

step0_repl: step0_repl.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

step1_read_print: step1_read_print.c reader.c mal_obj.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f step0_repl step1_read_print
