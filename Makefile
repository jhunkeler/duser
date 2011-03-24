CC=gcc
CFLAGS=-O2 -fomit-frame-pointer -Wall -Wextra
LDFLAGS=-ltre
PROG="duser"
SRC=duser.c
all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(SRC)

	
