/*	POSIX.C
 *
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 *
 *	modified by Petri Kutvonen
 *
 *	based on termio.c, with all the old cruft removed, and
 *	fixed for termios rather than the old termio.. Linus Torvalds
 */

#ifdef POSIX

#include <stdio.h>
#include <unistd.h>
#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

static int kbdflgs;			/* saved keyboard fd flags      */
static int kbdpoll;			/* in O_NDELAY mode             */

static struct termios otermios;		/* original terminal characteristics */
static struct termios ntermios;		/* charactoristics to use inside */

#define TBUFSIZ 128
static char tobuf[TBUFSIZ];		/* terminal output buffer */


/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */
void ttopen(void)
{
	tcgetattr(0, &otermios);	/* save old settings */

	/*
	 * base new settings on old ones - don't change things
	 * we don't know about
	 */
	ntermios = otermios;

	/* raw CR/NL etc input handling, but keep ISTRIP if we're on a 7-bit line */
	ntermios.c_iflag &= ~(IGNBRK | BRKINT | IGNPAR | PARMRK
			      | INPCK | INLCR | IGNCR | ICRNL);

	/* raw CR/NR etc output handling */
	ntermios.c_oflag &=
	    ~(OPOST | ONLCR | OLCUC | OCRNL | ONOCR | ONLRET);

	/* No signal handling, no echo etc */
	ntermios.c_lflag &= ~(ISIG | ICANON | XCASE | ECHO | ECHOE | ECHOK
			      | ECHONL | NOFLSH | TOSTOP | ECHOCTL |
			      ECHOPRT | ECHOKE | FLUSHO | PENDIN | IEXTEN);

	/* one character, no timeout */
	ntermios.c_cc[VMIN] = 1;
	ntermios.c_cc[VTIME] = 0;
	tcsetattr(0, TCSADRAIN, &ntermios);	/* and activate them */

	/*
	 * provide a smaller terminal output buffer so that
	 * the type ahead detection works better (more often)
	 */
	setbuffer(stdout, &tobuf[0], TBUFSIZ);

	kbdflgs = fcntl(0, F_GETFL, 0);
	kbdpoll = FALSE;

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
	tcsetattr(0, TCSADRAIN, &otermios);	/* restore terminal settings */
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
int ttputc(int c)
{
	fputc(c, stdout);
	return (TRUE);
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
void ttflush(void)
{
/*
 * Add some terminal output success checking, sometimes an orphaned
 * process may be left looping on SunOS 4.1.
 *
 * How to recover here, or is it best just to exit and lose
 * everything?
 *
 * jph, 8-Oct-1993
 * Jani Jaakkola suggested using select after EAGAIN but let's just wait a bit
 *
 */
	int status;

	status = fflush(stdout);
	while (status < 0 && errno == EAGAIN) {
		sleep(1);
		status = fflush(stdout);
	}
	if (status < 0)
		exit(15);
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
int ttgetc(void)
{
	return (255 & fgetc(stdin));	/* 8BIT P.K. */
}

/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/

int typahead(void)
{
	int x;			/* holds # of pending chars */

#ifdef FIONREAD
	if (ioctl(0, FIONREAD, &x) < 0)
		x = 0;
#else
	x = 0;
#endif
	return x;
}

#endif				/* POSIX */
