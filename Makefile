# makefile for emacs

# Make the build silent by default
V =

ifeq ($(strip $(V)),)
	E = @echo
	Q = @
else
	E = @\#
	Q =
endif
export E Q

PROGRAM=em

SRC=	basic.c bind.c buffer.c display.c eval.c exec.c file.c fileio.c \
	globals.c input.c isearch.c line.c lock.c main.c names.c \
	posix.c random.c region.c search.c spawn.c spell.c tcap.c \
	usage.c utf8.c version.c window.c word.c wrapper.c

OBJ=	$(SRC:.c=.o)

HDR=	ebind.h efunc.h epath.h estruct.h evar.h globals.h line.h usage.h \
	utf8.h util.h version.h wrapper.h

CC=gcc
WARNINGS=-Wall -Wstrict-prototypes
DEFINES=-DPOSIX -D_GNU_SOURCE

CFLAGS=-O2 $(WARNINGS)

LIBS=ncurses
BINDIR=$(HOME)/bin
LIBDIR=$(HOME)/lib

# The spell checking is hunspell, and it is the only optional part of
# this.  HUNSPELL=0 - or anything that is not 1 - builds an editor that
# has spell mode and never finds a misspelling.  Asking for it without
# the library installed says so and builds that editor too, rather than
# failing halfway through with a missing header.
HUNSPELL=1
HAVE_HUNSPELL := $(shell pkg-config --exists hunspell && echo 1)
ifeq ($(HUNSPELL),1)
ifeq ($(HAVE_HUNSPELL),1)
LIBS += hunspell
DEFINES += -DHUNSPELL
else
$(warning hunspell not installed, not building with spell checking)
endif
endif

PKG_CONFIG_CFLAGS=$(shell pkg-config --cflags $(LIBS))
ALL_CFLAGS=$(CFLAGS) $(DEFINES) $(PKG_CONFIG_CFLAGS)
LDLIBS += $(shell pkg-config --libs $(LIBS))

$(PROGRAM): $(OBJ)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(LDFLAGS) $(DEFINES) -o $@ $(OBJ) $(LDLIBS)

.c.o:
	$(E) "  CC      " $@
	$(Q) ${CC} ${ALL_CFLAGS} -c $<

# The whole thing builds in a couple of seconds, so it is not worth
# knowing which object needs which header - they all depend on all of
# them.  If that ever stops being true, generate the real dependencies
# during the build with -MMD and include them, rather than going back
# to keeping a list by hand.
$(OBJ): $(HDR)

clean:
	$(E) "  CLEAN"
	$(Q) rm -f $(PROGRAM) core *.o

install: $(PROGRAM)
	install em ${BINDIR}
	cp emacs.hlp ${LIBDIR}
	cp emacs.rc ${HOME}/.emacsrc
	chmod 755 ${BINDIR}/em
	chmod 644 ${LIBDIR}/emacs.hlp ${HOME}/.emacsrc
