## The other day I might consider learning autoconf. In the meantime I
## hope this is trivial enough to be installed without. If someone
## feels like sending me an autoconf-script for cpipe, I'll use it.
##
## $Revision: 1.6 $, $Date: 2008/10/18 09:41:00 $
########################################################################

prefix=/usr
exec_prefix=$(prefix)

## This is where the executable will be installed
BINDIR=$(exec_prefix)/bin

## Here we install the manual page
MANDIR=$(prefix)/man/man1

## Your favorite compiler flags.
CFLAGS = -O2 -W -Wall -pedantic

VERSION=$(shell cat .version)
########################################################################
all: cpipe cpipe.1

cpipe: cpipe.o cmdline.o
	$(CC) -o $@ cpipe.o cmdline.o -lm

cpipe.1: cpipe.1.in
	sed -e "s/|VERSION|/${VERSION}/g" <cpipe.1.in >cpipe.1

cpipe.o: cpipe.c cmdline.h

cmdline.o: cmdline.c cmdline.h

clean:
	rm cmdline.o cpipe.o cpipe cpipe.1

install: cpipe cpipe.1
	mkdir -p $(BINDIR) $(MANDIR)
	cp cpipe $(BINDIR); chmod 755 $(BINDIR)/cpipe
	cp cpipe.1 $(MANDIR); chmod 644 $(MANDIR)/cpipe.1

uninstall:
	-rm $(BINDIR)/cpipe
	-rm $(MANDIR)/cpipe.1
