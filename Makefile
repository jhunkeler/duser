CC=gcc
CFLAGS=-O2 -Wall -Wextra
LDFLAGS=
PROG="duser"
SRC=duser.c duser.h user.c util.c log.c cfg.c
all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(SRC)
	strip -s $(PROG)
clean:
	rm -rf $(PROG)

