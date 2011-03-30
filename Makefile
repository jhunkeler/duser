CC=gcc
CFLAGS=-g -Wall -Wextra
LDFLAGS=-ltre
PROG="duser"
SRC=duser.c duser.h log.c
all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(SRC)

	
