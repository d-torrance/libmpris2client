AM_LDFLAGS =  -Wl,--as-needed -static
AM_CPPFLAGS = -g -Wall

LDADD = libmpris2client.la
lib_LTLIBRARIES = libmpris2client.la

libmpris2client_la_SOURCES = \
	libmpris2client.c    \
	libmpris2client.h    \
	mpris2-metadata.c    \
	mpris2-metadata.h

libmpris2client_la_CPPFLAGS = \
	$(GIO_CFLAGS)             \
	-g -Wall

libmpris2client_la_LDFLAGS = \
	$(GIO_LIBS)              \
	-Wl,--as-needed

# Public header files
libmpris2client_includedir = $(includedir)/libmpris2client
pkginclude_HEADERS =  \
	libmpris2client.h \
	mpris2-metadata.h
