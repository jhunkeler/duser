CC=gcc
CFLAGS=-g -Wall -Wextra
LDFLAGS=
PROG="duser"
SRC=duser.c duser.h log.c cfg.c
all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(SRC)

	
