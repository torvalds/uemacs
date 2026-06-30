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

PROGRAM=bin/em

BUILD_DIR=build
BIN_DIR=bin

VPATH = src src/tty src/core src/bind src/util include

SRC=src/tty/ansi.c src/core/basic.c src/bind/bind.c src/core/buffer.c \
	src/core/crypt.c src/core/display.c src/core/eval.c \
	src/core/exec.c src/core/file.c src/core/fileio.c \
	src/tty/ibmpc.c src/core/input.c src/core/isearch.c \
	src/core/line.c src/core/lock.c \
	main.c \
	src/util/pklock.c src/tty/posix.c src/core/random.c \
	src/core/region.c src/core/search.c \
	src/util/spawn.c src/tty/tcap.c src/tty/termio.c \
	src/tty/vmsvt.c src/tty/vt52.c \
	src/core/window.c src/core/word.c src/bind/names.c \
	src/core/globals.c src/util/version.c \
	src/util/usage.c src/util/wrapper.c src/util/utf8.c

OBJ=build/ansi.o build/basic.o build/bind.o build/buffer.o \
	build/crypt.o build/display.o build/eval.o build/exec.o \
	build/file.o build/fileio.o build/ibmpc.o build/input.o \
	build/isearch.o build/line.o build/lock.o build/main.o \
	build/pklock.o build/posix.o build/random.o build/region.o \
	build/search.o build/spawn.o build/tcap.o build/termio.o \
	build/vmsvt.o build/vt52.o build/window.o build/word.o \
	build/names.o build/globals.o build/version.o \
	build/usage.o build/wrapper.o build/utf8.o

HDR=include/ebind.h include/edef.h include/efunc.h include/epath.h \
	include/estruct.h include/evar.h include/util.h include/version.h

# DO NOT ADD OR MODIFY ANY LINES ABOVE THIS -- make source creates them

CC=gcc
WARNINGS=-Wall -Wstrict-prototypes
CFLAGS=-O2 $(WARNINGS) -Iinclude
#CC=c89 +O3			# HP
#CFLAGS= -D_HPUX_SOURCE -DSYSV
#CFLAGS=-O4 -DSVR4		# Sun
#CFLAGS=-O -qchars=signed	# RS/6000
ifeq ($(uname_S),Linux)
 DEFINES=-DAUTOCONF -DPOSIX -DUSG -D_BSD_SOURCE -D_SVID_SOURCE -D_XOPEN_SOURCE=600
endif
ifeq ($(uname_S),FreeBSD)
 DEFINES=-DAUTOCONF -DPOSIX -DSYSV -D_FREEBSD_C_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_XOPEN_SOURCE=600
endif
ifeq ($(uname_S),Darwin)
 DEFINES=-DAUTOCONF -DPOSIX -DSYSV -D_DARWIN_C_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_XOPEN_SOURCE=600
endif
#DEFINES=-DAUTOCONF
#LIBS=-ltermcap			# BSD
LIBS=-lcurses			# SYSV
#LIBS=-ltermlib
#LIBS=-L/usr/lib/termcap -ltermcap
LFLAGS=-hbx
BINDIR=/usr/bin
LIBDIR=/usr/lib

$(PROGRAM): $(OBJ) | $(BIN_DIR)
	$(E) "  LINK    " $@
	$(Q) $(CC) $(LDFLAGS) $(DEFINES) -o $@ $(OBJ) $(LIBS)

$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

SPARSE=sparse
SPARSE_FLAGS=-D__LITTLE_ENDIAN__ -D__x86_64__ -D__linux__ -D__unix__

sparse:
	$(SPARSE) $(SPARSE_FLAGS) $(DEFINES) $(SRC)

clean:
	$(E) "  CLEAN"
	$(Q) rm -rf $(BUILD_DIR) $(BIN_DIR) core lintout makeout tags makefile.bak

install: $(PROGRAM)
	strip $(PROGRAM)
	cp bin/em ${BINDIR}
	cp emacs.hlp ${LIBDIR}
	cp emacs.rc ${LIBDIR}/.emacsrc
	chmod 755 ${BINDIR}/em
	chmod 644 ${LIBDIR}/emacs.hlp ${LIBDIR}/.emacsrc

lint:	${SRC}
	@rm -f lintout
	lint ${LFLAGS} ${SRC} >lintout
	cat lintout

errs:
	@rm -f makeout
	make em >makeout

tags:	${SRC}
	@rm -f tags
	ctags ${SRC}

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
	@for i in ${SRC}; do\
	    cc ${DEFINES} -M $$i | sed -e 's, \./, ,' | grep -v '/usr/include' | \
	    awk '{ if ($$1 != prev) { if (rec != "") print rec; \
		rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		else rec = rec " " $$2 } } \
		END { print rec }'; done >makedep
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

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(E) "  CC      " $@
	$(Q) ${CC} ${CFLAGS} ${DEFINES} -c $< -o $@

# DO NOT DELETE THIS LINE -- make depend uses it

build/ansi.o: ansi.c estruct.h edef.h
build/basic.o: basic.c estruct.h edef.h
build/bind.o: bind.c estruct.h edef.h epath.h
build/buffer.o: buffer.c estruct.h edef.h
build/crypt.o: crypt.c estruct.h edef.h
build/display.o: display.c estruct.h edef.h utf8.h
build/eval.o: eval.c estruct.h edef.h evar.h
build/exec.o: exec.c estruct.h edef.h
build/file.o: file.c estruct.h edef.h
build/fileio.o: fileio.c estruct.h edef.h
build/ibmpc.o: ibmpc.c estruct.h edef.h
build/input.o: input.c estruct.h edef.h
build/isearch.o: isearch.c estruct.h edef.h
build/line.o: line.c estruct.h edef.h
build/lock.o: lock.c estruct.h edef.h
build/main.o: main.c estruct.h efunc.h edef.h ebind.h
build/pklock.o: pklock.c estruct.h
build/posix.o: posix.c estruct.h utf8.h
build/random.o: random.c estruct.h edef.h
build/region.o: region.c estruct.h edef.h
build/search.o: search.c estruct.h edef.h
build/spawn.o: spawn.c estruct.h edef.h
build/tcap.o: tcap.c estruct.h edef.h
build/termio.o: termio.c estruct.h edef.h
build/utf8.o: utf8.c utf8.h
build/vmsvt.o: vmsvt.c estruct.h edef.h
build/vt52.o: vt52.c estruct.h edef.h
build/window.o: window.c estruct.h edef.h
build/word.o: word.c estruct.h edef.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
