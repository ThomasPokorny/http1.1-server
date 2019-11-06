CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean

all : client

client : client.o util.o
	$(CC) $(CFLAGS) -o client client.o util.o

client.o : client.c client.h util.h
	$(CC) $(CFLAGS) -c -o client.o client.c

util.o : util.c util.h
	$(CC) $(CFLAGS) -c -o util.o util.c

clean:
	rm -rf *.o client