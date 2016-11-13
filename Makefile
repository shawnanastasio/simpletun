PREFIX=/usr
BINDIR=$(PREFIX)/bin

CC=gcc
CFLAGS=-std=gnu99 $(shell pkg-config zlib --libs)
INSTALL=install

DEPS=src/iputils.c src/compression.c

all:	simpletun
distclean:	clean

clean:
	rm -f simpletun
	rm -f *.o

simpletun: clean $(DEPS)
	$(CC) -o simpletun src/simpletun.c $(DEPS) $(CFLAGS)

install: all
	$(INSTALL) -D simpletun $(DESTDIR)$(BINDIR)/simpletun

macmask:
	$(CC) simpletun.c -o simpletun
