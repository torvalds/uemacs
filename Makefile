# makefile for emacs, updated Sun Apr 28 17:59:07 EET DST 1996

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

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

PROGRAM=em

SRC=	basic.c bind.c buffer.c display.c eval.c exec.c file.c fileio.c \
	globals.c input.c isearch.c line.c lock.c main.c names.c \
	pklock.c posix.c random.c region.c search.c spawn.c tcap.c \
	usage.c utf8.c version.c window.c word.c wrapper.c

OBJ=	basic.o bind.o buffer.o display.o eval.o exec.o file.o fileio.o \
	globals.o input.o isearch.o line.o lock.o main.o names.o \
	pklock.o posix.o random.o region.o search.o spawn.o tcap.o \
	usage.o utf8.o version.o window.o word.o wrapper.o

HDR=	ebind.h edef.h efunc.h epath.h estruct.h evar.h line.h usage.h \
	utf8.h util.h version.h wrapper.h

# DO NOT ADD OR MODIFY ANY LINES ABOVE THIS -- make source creates them

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

clean:
	$(E) "  CLEAN"
	$(Q) rm -f $(PROGRAM) core lintout makeout tags makefile.bak *.o

install: $(PROGRAM)
	install em ${BINDIR}
	cp emacs.hlp ${LIBDIR}
	cp emacs.rc ${LIBDIR}/.emacsrc
	chmod 755 ${BINDIR}/em
	chmod 644 ${LIBDIR}/emacs.hlp ${LIBDIR}/.emacsrc

source:
	@mv makefile makefile.bak
	@echo "# makefile for emacs, updated `date`" >makefile
	@echo '' >>makefile
	@echo SRC=`ls *.c` >>makefile
	@echo OBJ=`ls *.c | sed s/c$$/o/` >>makefile
	@echo HDR=`ls *.h` >>makefile
	@echo '' >>makefile
	@sed -n -e '/^# DO NOT ADD OR MODIFY/,$$p' <makefile.bak >>makefile

depend: ${SRC}
	@for i in ${SRC}; do $(CC) ${DEFINES} -MM $$i; done >makedep
	@echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	@echo '$$r ./makedep' >>eddep
	@echo 'w' >>eddep
	@cp makefile makefile.bak
	@ed - makefile <eddep
	@rm eddep makedep
	@echo '' >>makefile
	@echo '# DEPENDENCIES MUST END AT END OF FILE' >>makefile
	@echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >>makefile
	@echo '# see make depend above' >>makefile

# DO NOT DELETE THIS LINE -- make depend uses it

basic.o: basic.c estruct.h edef.h efunc.h line.h utf8.h
bind.o: bind.c estruct.h edef.h efunc.h epath.h line.h utf8.h util.h
buffer.o: buffer.c estruct.h edef.h efunc.h line.h utf8.h
display.o: display.c estruct.h edef.h efunc.h line.h utf8.h version.h wrapper.h
eval.o: eval.c estruct.h edef.h efunc.h evar.h line.h utf8.h util.h version.h
exec.o: exec.c estruct.h edef.h efunc.h line.h utf8.h
file.o: file.c estruct.h edef.h efunc.h line.h utf8.h util.h
fileio.o: fileio.c estruct.h edef.h efunc.h
input.o: input.c estruct.h edef.h efunc.h wrapper.h
isearch.o: isearch.c estruct.h edef.h efunc.h line.h utf8.h
line.o: line.c line.h utf8.h estruct.h edef.h efunc.h
lock.o: lock.c estruct.h edef.h efunc.h
main.o: main.c estruct.h edef.h efunc.h ebind.h line.h utf8.h version.h
pklock.o: pklock.c estruct.h edef.h efunc.h
posix.o: posix.c estruct.h edef.h efunc.h utf8.h
random.o: random.c estruct.h edef.h efunc.h line.h utf8.h
region.o: region.c estruct.h edef.h efunc.h line.h utf8.h
search.o: search.c estruct.h edef.h efunc.h line.h utf8.h
spawn.o: spawn.c estruct.h edef.h efunc.h
tcap.o: tcap.c estruct.h edef.h efunc.h
window.o: window.c estruct.h edef.h efunc.h line.h utf8.h wrapper.h
word.o: word.c estruct.h edef.h efunc.h line.h utf8.h
names.o: names.c estruct.h edef.h efunc.h line.h utf8.h
globals.o: globals.c estruct.h edef.h
version.o: version.c version.h
usage.o: usage.c usage.h
wrapper.o: wrapper.c usage.h
utf8.o: utf8.c utf8.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
