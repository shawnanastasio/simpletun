PREFIX=/usr
BINDIR=$(PREFIX)/bin

CC=gcc
CFLAGS=-Wextra -Wall -O2
INSTALL=ginstall

DEPS=iputils.c

all:	simpletun
distclean:	clean

clean:
	rm -f simpletun
	rm -f *.o

simpletun: clean $(DEPS)
	$(CC) -o simpletun $(DEPS) simpletun.c $(CFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

install: all
	$(INSTALL) -D simpletun $(DESTDIR)$(BINDIR)/simpletun

macmask:
	$(CC) simpletun.c -o simpletun
