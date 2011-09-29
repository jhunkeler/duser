sbin_PROGRAMS=	duser
duser_POSIX=-D_GNU_SOURCE
AM_CFLAGS=${duser_POSIX} --std=c99 -O2 -pedantic -Wall -Wextra
if HAVE_COMPAT
duser_LDFLAGS=-L$(top_srcdir)/compat -ldusercompat
else
duser_LDFLAGS=-lpcre
endif

duser_SOURCES=	duser.c \
		user.c \
		util.c \
		mklist.c \
		log.c \
		cfg.c
