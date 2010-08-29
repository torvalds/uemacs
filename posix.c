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

/*
 * NOTE NOTE NOTE!
 *
 * Uemacs is currently very much byte-oriented, and not at all UTF8-aware
 * interally. However, this allows it to understand a _terminal_ that is
 * in utf-8 mode, and will turn input into the 8-bit subset, and will turn
 * things back into UTF8 on output.
 *
 * Do _not_ confuse this with the notion of actually being able to edit
 * UTF-8 file _contents_. That's a totally different thing.
 */
#define utf8_mode() \
	(curwp && curwp->w_bufp && (curwp->w_bufp->b_mode & MDUTF8))

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
	/*
	 * We always represent things in 1 byte, but if we output
	 * in UTF-8, we may need to expand that into 2 bytes..
	 *
	 * Some day we might even be able to handle UTF-8 _content_.
	 *
	 * That day is not today.
	 */
	if (utf8_mode()) {
		c &= 0xff;
		if (c >= 0x80) {
			unsigned char first = (c >> 6) | 0xc0;
			fputc(first, stdout);
			c = (c & 0x3f) | 0x80;
		}
	}
	fputc(c, stdout);
	return TRUE;
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
	static unsigned char buffer[32];
	static int pending;
	unsigned char c, second;
	int count;

	count = pending;
	if (!count) {
		count = read(0, buffer, sizeof(buffer));
		if (count <= 0)
			return 0;
		pending = count;
	}

	c = buffer[0];
	if (c >= 32 && c < 128)
		goto done;

	/* Special character - try to fill buffer */
	if (count == 1) {
		int n;
		ntermios.c_cc[VMIN] = 0;
		ntermios.c_cc[VTIME] = 1;		/* A .1 second lag */
		tcsetattr(0, TCSANOW, &ntermios);

		n = read(0, buffer + count, sizeof(buffer) - count);

		/* Undo timeout */
		ntermios.c_cc[VMIN] = 1;
		ntermios.c_cc[VTIME] = 0;
		tcsetattr(0, TCSANOW, &ntermios);

		if (n <= 0)
			goto done;
		pending += n;
	}
	second = buffer[1];

	/* Turn ESC+'[' into CSI */
	if (c == 27 && second == '[') {
		pending -= 2;
		memmove(buffer, buffer+2, pending);
		return 128+27;
	}

	if (!utf8_mode())
		goto done;

	/* Normal 7-bit? */
	if (!(c & 0x80))
		goto done;

	/*
	 * Unexpected UTF-8 continuation character? Maybe
	 * we're in non-UTF mode, or maybe it's a control
	 * character.. Regardless, just pass it on.
	 */
	if (!(c & 0x40))
		goto done;

	/*
	 * Multi-byte sequences.. Right now we only
	 * want to get characters that can be represented
	 * in a single byte, so we're not interested in
	 * anything else..
	 */
	if (c & 0x3c)
		goto done;

	if ((second & 0xc0) != 0x80)
		goto done;

	/*
	 * Ok, it's a two-byte UTF-8 character that can be represented
	 * as a single-byte Latin1 character!
	 */
	c = (c << 6) | (second & 0x3f);
	pending -= 2;
	memmove(buffer, buffer+2, pending);

	return c;

done:
	pending--;
	memmove(buffer, buffer+1, pending);
	return c;
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
