PREFIX=/usr
BINDIR=$(PREFIX)/bin

CC=gcc
CFLAGS=-Wextra -Wall -O2
INSTALL=ginstall

all:	simpletun
distclean:	clean

clean:
	rm simpletun

simpletun:
	$(CC) -o simpletun simpletun.c $(CFLAGS)

install: all
	$(INSTALL) -D simpletun $(DESTDIR)$(BINDIR)/simpletun

macmask:
	$(CC) simpletun.c -o simpletun
