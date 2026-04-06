CC=gcc
CFLAGS=-g -std=c99 -Wall
LDFLAGS=-lreadline -lm
TARGET=a.out

all: $(TARGET) doge.out

$(TARGET): parsing.c mpc.c
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

doge.out: doge_code.c mpc.c
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@
