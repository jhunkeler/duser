CC=gcc
CFLAGS=-O2 -Wall -Wextra
LDFLAGS=
PROG="duser"
SRC=duser.c duser.h log.c cfg.c
all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(SRC)

	
