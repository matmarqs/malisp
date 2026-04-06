CC=gcc
CFLAGS=-g -std=c99 -Wall
LDFLAGS=-lreadline
TARGET=a.out

$(TARGET): main.c
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@
