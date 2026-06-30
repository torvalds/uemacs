
/*	TERMIO.C
 *
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 *
 *	modified by Petri Kutvonen
 */

#ifndef POSIX

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if     VMS
#include        <stsdef.h>
#include        <ssdef.h>
#include        <descrip.h>
#include        <iodef.h>
#include        <ttdef.h>
#include	<tt2def.h>

#define NIBUF   128		/* Input buffer size            */
#define NOBUF   1024		/* MM says bug buffers win!     */
#define EFN     0		/* Event flag                   */

char obuf[NOBUF];		/* Output buffer                */
int nobuf;			/* # of bytes in above          */
char ibuf[NIBUF];		/* Input buffer                 */
int nibuf;			/* # of bytes in above          */
int ibufi;			/* Read index                   */
int oldmode[3];			/* Old TTY mode bits            */
int newmode[3];			/* New TTY mode bits            */
short iochan;			/* TTY I/O channel              */
#endif

#if     MSDOS & (MSC | TURBO)
union REGS rg;			/* cpu register for use of DOS calls */
int nxtchar = -1;		/* character held from type ahead    */
#endif

#if	USG			/* System V */
#include	<signal.h>
#include	<termio.h>
#include	<fcntl.h>
int kbdflgs;			/* saved keyboard fd flags      */
int kbdpoll;			/* in O_NDELAY mode                     */
int kbdqp;			/* there is a char in kbdq      */
char kbdq;			/* char we've already read      */
struct termio otermio;		/* original terminal characteristics */
struct termio ntermio;		/* charactoristics to use inside */
#if	XONXOFF
#define XXMASK	0016000
#endif
#endif

#if	V7 | BSD
#include        <sgtty.h>	/* for stty/gtty functions */
#include	<signal.h>
struct sgttyb ostate;		/* saved tty state */
struct sgttyb nstate;		/* values for editor mode */
struct tchars otchars;		/* Saved terminal special character set */
#if	XONXOFF
struct tchars ntchars = { 0xff, 0xff, 0x11, 0x13, 0xff, 0xff };

				/* A lot of nothing and XON/XOFF */
#else
struct tchars ntchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

				/* A lot of nothing */
#endif
#if	BSD & PKCODE
struct ltchars oltchars;	/* Saved terminal local special character set */
struct ltchars nltchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

				/* A lot of nothing */
#endif

#if BSD
#include <sys/ioctl.h>		/* to get at the typeahead */
extern int rtfrmshell();	/* return from suspended shell */
#define	TBUFSIZ	128
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif
#endif

#if	__hpux | SVR4
extern int rtfrmshell();	/* return from suspended shell */
#define TBUFSIZ 128
char tobuf[TBUFSIZ];		/* terminal output buffer */
#endif

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */
void ttopen(void)
{
#if     VMS
	struct dsc$descriptor idsc;
	struct dsc$descriptor odsc;
	char oname[40];
	int iosb[2];
	int status;

	odsc.dsc$a_pointer = "TT";
	odsc.dsc$w_length = strlen(odsc.dsc$a_pointer);
	odsc.dsc$b_dtype = DSC$K_DTYPE_T;
	odsc.dsc$b_class = DSC$K_CLASS_S;
	idsc.dsc$b_dtype = DSC$K_DTYPE_T;
	idsc.dsc$b_class = DSC$K_CLASS_S;
	do {
		idsc.dsc$a_pointer = odsc.dsc$a_pointer;
		idsc.dsc$w_length = odsc.dsc$w_length;
		odsc.dsc$a_pointer = &oname[0];
		odsc.dsc$w_length = sizeof(oname);
		status = LIB$SYS_TRNLOG(&idsc, &odsc.dsc$w_length, &odsc);
		if (status != SS$_NORMAL && status != SS$_NOTRAN)
			exit(status);
		if (oname[0] == 0x1B) {
			odsc.dsc$a_pointer += 4;
			odsc.dsc$w_length -= 4;
		}
	} while (status == SS$_NORMAL);
	status = SYS$ASSIGN(&odsc, &iochan, 0, 0);
	if (status != SS$_NORMAL)
		exit(status);
	status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL)
		exit(status);
	newmode[0] = oldmode[0];
	newmode[1] = oldmode[1] | TT$M_NOECHO;
#if	XONXOFF
#else
	newmode[1] &= ~(TT$M_TTSYNC | TT$M_HOSTSYNC);
#endif
	newmode[2] = oldmode[2] | TT2$M_PASTHRU;
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL)
		exit(status);
	term.t_nrow = (newmode[1] >> 24) - 1;
	term.t_ncol = newmode[0] >> 16;

#endif

#if     MSDOS & (TURBO | (PKCODE & MSC))
	/* kill the CONTROL-break interupt */
	rg.h.ah = 0x33;		/* control-break check dos call */
	rg.h.al = 1;		/* set the current state */
	rg.h.dl = 0;		/* set it OFF */
	intdos(&rg, &rg);	/* go for it! */
#endif

#if	USG
	ioctl(0, TCGETA, &otermio);	/* save old settings */
	ntermio.c_iflag = 0;	/* setup new settings */
#if	XONXOFF
	ntermio.c_iflag = otermio.c_iflag & XXMASK;	/* save XON/XOFF P.K. */
#endif
	ntermio.c_oflag = 0;
	ntermio.c_cflag = otermio.c_cflag;
	ntermio.c_lflag = 0;
	ntermio.c_line = otermio.c_line;
	ntermio.c_cc[VMIN] = 1;
	ntermio.c_cc[VTIME] = 0;
#if	PKCODE
	ioctl(0, TCSETAW, &ntermio);	/* and activate them */
#else
	ioctl(0, TCSETA, &ntermio);	/* and activate them */
#endif
	kbdflgs = fcntl(0, F_GETFL, 0);
	kbdpoll = FALSE;
#endif

#if     V7 | BSD
	gtty(0, &ostate);	/* save old state */
	gtty(0, &nstate);	/* get base of new state */
#if	XONXOFF
	nstate.sg_flags |= (CBREAK | TANDEM);
#else
	nstate.sg_flags |= RAW;
#endif
	nstate.sg_flags &= ~(ECHO | CRMOD);	/* no echo for now... */
	stty(0, &nstate);	/* set mode */
	ioctl(0, TIOCGETC, &otchars);	/* Save old characters */
	ioctl(0, TIOCSETC, &ntchars);	/* Place new character into K */
#if	BSD & PKCODE
	ioctl(0, TIOCGLTC, &oltchars);	/* Save old local characters */
	ioctl(0, TIOCSLTC, &nltchars);	/* New local characters */
#endif
#if	BSD
	/* provide a smaller terminal output buffer so that
	   the type ahead detection works better (more often) */
	setbuffer(stdout, &tobuf[0], TBUFSIZ);
	signal(SIGTSTP, SIG_DFL);	/* set signals so that we can */
	signal(SIGCONT, rtfrmshell);	/* suspend & restart emacs */
#endif
#endif

#if	__hpux | SVR4
	/* provide a smaller terminal output buffer so that
	   the type ahead detection works better (more often) */
	setvbuf(stdout, &tobuf[0], _IOFBF, TBUFSIZ);
	signal(SIGTSTP, SIG_DFL);	/* set signals so that we can */
	signal(SIGCONT, rtfrmshell);	/* suspend & restart emacs */
	TTflush();
#endif				/* __hpux */

	/* on all screens we are not sure of the initial position
	   of the cursor                                        */
	ttrow = 999;
	ttcol = 999;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
void ttclose(void)
{
#if     VMS
	int status;
	int iosb[1];

	ttflush();
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status != SS$_NORMAL || (iosb[0] & 0xFFFF) != SS$_NORMAL)
		exit(status);
	status = SYS$DASSGN(iochan);
	if (status != SS$_NORMAL)
		exit(status);
#endif
#if     MSDOS & (TURBO | (PKCODE & MSC))
	/* restore the CONTROL-break interupt */
	rg.h.ah = 0x33;		/* control-break check dos call */
	rg.h.al = 1;		/* set the current state */
	rg.h.dl = 1;		/* set it ON */
	intdos(&rg, &rg);	/* go for it! */
#endif

#if	USG
#if	PKCODE
	ioctl(0, TCSETAW, &otermio);	/* restore terminal settings */
#else
	ioctl(0, TCSETA, &otermio);	/* restore terminal settings */
#endif
	fcntl(0, F_SETFL, kbdflgs);
#endif

#if     V7 | BSD
	stty(0, &ostate);
	ioctl(0, TIOCSETC, &otchars);	/* Place old character into K */
#if	BSD & PKCODE
	ioctl(0, TIOCSLTC, &oltchars);	/* Place old local character into K */
#endif
#endif
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
void ttputc(c)
{
#if     VMS
	if (nobuf >= NOBUF)
		ttflush();
	obuf[nobuf++] = c;
#endif

#if	MSDOS & ~IBMPC
	bdos(6, c, 0);
#endif

#if     V7 | USG | BSD
	fputc(c, stdout);
#endif
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
int ttflush(void)
{
#if     VMS
	int status;
	int iosb[2];

	status = SS$_NORMAL;
	if (nobuf != 0) {
		status =
		    SYS$QIOW(EFN, iochan, IO$_WRITELBLK | IO$M_NOFORMAT,
			     iosb, 0, 0, obuf, nobuf, 0, 0, 0, 0);
		if (status == SS$_NORMAL)
			status = iosb[0] & 0xFFFF;
		nobuf = 0;
	}
	return status;
#endif

#if     MSDOS
#endif

#if     V7 | USG | BSD
/*
 * Add some terminal output success checking, sometimes an orphaned
 * process may be left looping on SunOS 4.1.
 *
 * How to recover here, or is it best just to exit and lose
 * everything?
 *
 * jph, 8-Oct-1993
 */

#include <errno.h>

	int status;

	status = fflush(stdout);

	if (status != 0 && errno != EAGAIN) {
		exit(errno);
	}
#endif
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
ttgetc()
{
#if     VMS
	int status;
	int iosb[2];
	int term[2];

	while (ibufi >= nibuf) {
		ibufi = 0;
		term[0] = 0;
		term[1] = 0;
		status = SYS$QIOW(EFN, iochan, IO$_READLBLK | IO$M_TIMED,
				  iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0);
		if (status != SS$_NORMAL)
			exit(status);
		status = iosb[0] & 0xFFFF;
		if (status != SS$_NORMAL && status != SS$_TIMEOUT &&
		    status != SS$_DATAOVERUN)
			exit(status);
		nibuf = (iosb[0] >> 16) + (iosb[1] >> 16);
		if (nibuf == 0) {
			status = SYS$QIOW(EFN, iochan, IO$_READLBLK,
					  iosb, 0, 0, ibuf, 1, 0, term, 0,
					  0);
			if (status != SS$_NORMAL
			    || (status = (iosb[0] & 0xFFFF)) != SS$_NORMAL)
				if (status != SS$_DATAOVERUN)
					exit(status);
			nibuf = (iosb[0] >> 16) + (iosb[1] >> 16);
		}
	}
	return ibuf[ibufi++] & 0xFF;	/* Allow multinational  */
#endif

#if	MSDOS & (MSC | TURBO)
	int c;			/* character read */

	/* if a char already is ready, return it */
	if (nxtchar >= 0) {
		c = nxtchar;
		nxtchar = -1;
		return c;
	}

	/* call the dos to get a char */
	rg.h.ah = 7;		/* dos Direct Console Input call */
	intdos(&rg, &rg);
	c = rg.h.al;		/* grab the char */
	return c & 255;
#endif

#if     V7 | BSD
	return 255 & fgetc(stdin);	/* 8BIT P.K. */
#endif

#if	USG
	if (kbdqp)
		kbdqp = FALSE;
	else {
		if (kbdpoll && fcntl(0, F_SETFL, kbdflgs) < 0)
			return FALSE;
		kbdpoll = FALSE;
		while (read(0, &kbdq, 1) != 1);
	}
	return kbdq & 255;
#endif
}

#if	TYPEAH
/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/

typahead()
{
#if	MSDOS & (MSC | TURBO)
	if (kbhit() != 0)
		return TRUE;
	else
		return FALSE;
#endif

#if	BSD
	int x;			/* holds # of pending chars */

	return (ioctl(0, FIONREAD, &x) < 0) ? 0 : x;
#endif

#if	PKCODE & VMS
	return ibufi < nibuf;
#endif

#if	USG
	if (!kbdqp) {
		if (!kbdpoll && fcntl(0, F_SETFL, kbdflgs | O_NDELAY) < 0)
			return FALSE;
#if	PKCODE
		kbdpoll = 1;
#endif
		kbdqp = (1 == read(0, &kbdq, 1));
	}
	return kbdqp;
#endif

#if !UNIX & !VMS & !MSDOS
	return FALSE;
#endif
}
#endif

#endif				/* not POSIX */
