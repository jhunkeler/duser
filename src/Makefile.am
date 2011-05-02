sbin_PROGRAMS=	duser

LIBCOMPAT_LDADDS=	$(top_builddir)/compat/libdusercompat.a

AM_CPPFLAGS=	-I$(top_srcdir)/include

duser_LDFLAGS=  -L$(top_srcdir)/compat
duser_SOURCES=	duser.c \
		user.c \
		util.c \
		log.c \
		cfg.c
