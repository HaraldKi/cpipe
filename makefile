## The other day I might consider learning autoconf. In the meantime I
## hope this is trivial enough to be installed without. If someone
## feels like sending me an autoconf-script for cpipe, I'll use it.
##
## $Revision$, $Date$
########################################################################

prefix=/usr
exec_prefix=$(prefix)

## This is where the executable will be installed
BINDIR=$(exec_prefix)/bin

## Here we install the manual page
MANDIR=$(prefix)/man/man1

## Your favorite compiler flags.
CFLAGS = -O

########################################################################
cpipe: cpipe.o cmdline.o

cpipe.o: cpipe.c cmdline.h

cmdline.o: cmdline.c cmdline.h


cmdline.c cmdline.h cpipe.1: cmdline.cli
	clig cmdline.cli || {\
	echo "*****"; \
	echo "Get clig at http://wsd.iitb.fhg.de/~kir/clighome"; \
	echo "or use cmdline.c, cmdline.h and cpipe.1 as they come"; \
	echo "in the distribution by touching them."; \
	echo "*****"; \
	exit 1; \
        }

clean:
	rm cmdline.o cpipe.o cpipe


install: cpipe cpipe.1
	mkdir -p $(BINDIR) $(MANDIR)
	cp cpipe $(BINDIR); chmod 755 $(BINDIR)/cpipe
	cp cpipe.1 $(MANDIR); chmod 744 $(MANDIR)/cpipe.1

