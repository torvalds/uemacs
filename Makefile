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
	posix.c random.c region.c search.c spawn.c tcap.c \
	usage.c utf8.c version.c window.c word.c wrapper.c

OBJ=	$(SRC:.c=.o)

HDR=	ebind.h edef.h efunc.h epath.h estruct.h evar.h line.h usage.h \
	utf8.h util.h version.h wrapper.h

CC=gcc
WARNINGS=-Wall -Wstrict-prototypes
DEFINES=-DPOSIX -D_GNU_SOURCE

CFLAGS=-O2 $(WARNINGS) $(DEFINES)

LIBS=ncurses hunspell
BINDIR=$(HOME)/bin
LIBDIR=$(HOME)/lib

CFLAGS += $(shell pkg-config --cflags $(LIBS))
LDLIBS += $(shell pkg-config --libs $(LIBS))

$(PROGRAM): $(OBJ)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(LDFLAGS) $(DEFINES) -o $@ $(OBJ) $(LDLIBS)

.c.o:
	$(E) "  CC      " $@
	$(Q) ${CC} ${CFLAGS} -c $<

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
