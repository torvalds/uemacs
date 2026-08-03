/*	display.c
 *
 *      The functions in this file handle redisplay. There are two halves, the
 *      ones that update the virtual display screen, and the ones that make the
 *      physical display screen the same as the virtual display screen. These
 *      functions use hints that are left in the windows by the commands.
 *
 *	Modified by Petri Kutvonen
 */

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"
#include "version.h"
#include "wrapper.h"
#include "utf8.h"
#include "util.h"

#include <signal.h>
#include <sys/ioctl.h>

/*
 * A window size change, as noticed by the signal handler.  Nothing is
 * done about it there; checkwinsize() picks it up from somewhere the
 * editor is allowed to paint from.
 */
volatile sig_atomic_t chg_width, chg_height;

static int reframe(struct window *wp);
static void updpos(void);
static void paint_window(struct window *wp, bool check);
static void modeline(struct window *wp);
static void mlputi(int i, int r);
static void mlputli(long l, int r);
static void mlputf(int s);

/*
 * Open the terminal.  The operating system's terminal I/O channel is set
 * up; there is nothing else to initialize, because the screen is painted
 * from the buffers and there is no image of it to allocate.
 */
void vtinit(void)
{
	tcapopen();				/* open the screen */
	tcapkopen();				/* open the keyboard */
	tcaprev(FALSE);
}

/*
 * Clean up the virtual terminal system, in anticipation for a return to the
 * operating system. Move down to the last line and clear it out (the next
 * system prompt will be written in the line). Shut down the channel to the
 * terminal.
 */
void vttidy(void)
{
	mlerase();
	movecursor(term.t_nrow, 0);
	ttflush();
	tcapclose();
	tcapkclose();
	write(1, "\r", 1);
}

static void ttputs(const char *s)
{
	for (char c; (c = *s) != 0; s++)
		ttputc(c);
}

/*
 * Painting a row.
 *
 * 'col' counts columns in the line, which is not the same as columns on
 * the screen: a line the cursor has run off the right of is shown
 * scrolled sideways, and everything left of 'offset' is dropped on the
 * floor on its way past.  'out' is what actually reached the terminal,
 * which is where the cursor ends up.
 */
struct paint {
	int col;				/* column in the line */
	int offset;				/* leftmost column shown */
	int out;				/* columns actually painted */
	bool overflow;				/* ran off the right edge */
};

/*
 * One column.  Everything that expands to several of them comes through
 * here one at a time, so the clipping and the counting are only written
 * once.
 */
static void paint_raw(struct paint *p, unicode_t c)
{
	if (p->col >= p->offset + term.t_ncol) {
		p->overflow = true;
		p->col++;
		return;
	}
	if (p->col >= p->offset) {
		ttputc(c);
		p->out++;
	}
	p->col++;
}

/*
 * One character, expanded the way the screen shows it.  util.h's
 * next_column() says how wide each of these comes out, and the two have
 * to keep agreeing or the cursor lands in the wrong place.
 */
static void paint_char(struct paint *p, unicode_t c)
{
	if (c == '\t') {
		do {
			paint_raw(p, ' ');
		} while ((p->col & tabmask) != 0);
		return;
	}

	if (c < 0x20) {
		paint_raw(p, '^');
		paint_raw(p, c ^ 0x40);
		return;
	}

	if (c == 0x7f) {
		paint_raw(p, '^');
		paint_raw(p, '?');
		return;
	}

	if (c >= 0x80 && c <= 0xA0) {
		static const char hex[] = "0123456789abcdef";
		paint_raw(p, '\\');
		paint_raw(p, hex[c >> 4]);
		paint_raw(p, hex[c & 15]);
		return;
	}

	paint_raw(p, c);
}

static void paint_bytes(struct paint *p, char *text, int from, int to)
{
	while (from < to) {
		unicode_t c;

		from += utf8_to_unicode(text, from, to, &c);
		paint_char(p, c);
	}
}

/*
 * Words are handed to hunspell as the UTF-8 they already are.  The
 * buffer is only here because Hunspell_spell() wants a C string; 128
 * bytes is about forty accented letters, and anything longer is called
 * correct rather than guessed at.
 */
static bool word_ok(char *text, int from, int to)
{
	char word[128];
	int len = to - from;

	// We're not doing German or Finnish...
	if (len >= sizeof(word))
		return true;

	memcpy(word, text + from, len);
	word[len] = 0;
	return spellcheck(word);
}

#define SPELLSTART "\033[1m"
#define SPELLSTOP "\033[22m"

static void paint_word(struct paint *p, char *text, int from, int to, bool check)
{
	bool bad = check && !word_ok(text, from, to);

	if (bad)
		ttputs(SPELLSTART);
	paint_bytes(p, text, from, to);
	if (bad)
		ttputs(SPELLSTOP);
}

// A letter is any byte that is not something else.  That is the whole
// trick: the bytes of a UTF-8 character are all >= 0x80, so words in
// other alphabets fall out of a plain byte scan already encoded the way
// hunspell wants them, without ever being decoded or re-encoded.
static bool is_letter(unsigned char c)
{
	return c >= 0x80 || isalpha(c);
}

// Mixed letters and digits or underscores are hex numbers and variable
// names rather than words, so the whole token goes unchecked.
static bool is_notaword(unsigned char c)
{
	return c == '_' || (c >= '0' && c <= '9');
}

static bool is_token(unsigned char c)
{
	return is_letter(c) || is_notaword(c);
}

/*
 * Paint one row of the screen, straight from the line it shows.
 *
 * 'lp' is NULL for a row past the end of the buffer, 'offset' is the
 * first column to show, and 'check' asks for the spell checking.
 *
 * The word scanning walks the line's own UTF-8 rather than anything the
 * layout has been through, so a tab or the right margin cannot break a
 * word in half before hunspell sees it.
 */
static void paint_line(int row, struct line *lp, int offset, bool check)
{
	struct paint p = { .offset = offset };
	char *text = lp ? lp->l_text : NULL;
	int len = lp ? llength(lp) : 0;
	int i = 0;

	movecursor(row, 0);

	while (i < len) {
		if (!is_token(text[i])) {
			paint_bytes(&p, text, i, i + 1);
			i++;
			continue;
		}

		int start = i;
		bool word = true;

		while (i < len) {
			unsigned char c = text[i];

			if (is_letter(c)) {
				i++;
			} else if (is_notaword(c)) {
				word = false;
				i++;
			} else if (c == '\'' && word && i + 1 < len &&
				   isalpha((unsigned char)text[i + 1])) {
				i++;		/* an abbreviation, not an end */
			} else
				break;
		}

		paint_word(&p, text, start, i, check && word);
	}

	ttcol = p.out;
	tcapeeol();

	/* the markers that say the line carries on past the edge */
	if (p.overflow) {
		movecursor(row, term.t_ncol - 1);
		ttputc('$');
		ttcol = term.t_ncol;
	}
	if (offset) {
		movecursor(row, 0);
		ttputc('$');
		ttcol = 1;
	}
}

/*
 * Paint every row of the window, and the mode line under it.
 */
static void paint_window(struct window *wp, bool check)
{
	struct line *end = wp->w_bufp->b_linep;
	struct line *lp = wp->w_linep;

	for (int row = 0; row < term.t_nrow - 1; row++) {
		bool eob = lp == end;

		paint_line(row, eob ? NULL : lp,
			   row == currow ? lbound : 0, check);
		if (!eob)
			lp = lforw(lp);
	}
	modeline(wp);
}

/*
 * upscreen:
 *	user routine to force a screen update
 *	always finishes complete update
 */
int upscreen(int f, int n)
{
	update_now();
	return TRUE;
}

/*
 * Refresh the screen, unless a keyboard macro is replaying - the
 * intermediate states of a macro are not worth painting, since what was
 * asked for is the state it finishes in.  Anything that has to be seen
 * whatever is going on calls update_now() instead.
 */
void update(void)
{
	if (kbdmode == PLAY)
		return;
	update_now();
}

/*
 * Make sure that the display is right. Check the framing, work out where
 * the cursor has ended up, and paint whatever the change reaches.
 *
 * There is no image of the screen to compare against, so what gets
 * painted is decided from the buffer alone: everything, unless the
 * window flags say the change cannot have reached further than the line
 * the cursor is on.
 */
void update_now(void)
{
	struct window *wp = curwp;
	bool check = (wp->w_bufp->b_mode & MDSPELL) != 0;
	int oldbound = lbound;

	if (wp->w_flag)
		reframe(wp);			/* check the framing */

	updpos();				/* currow, curcol and lbound */

	if (sgarbf != FALSE) {
		/* the screen is not what we think it is; start over */
		movecursor(0, 0);
		tcapeeop();
		sgarbf = FALSE;
		mpresf = FALSE;
		paint_window(wp, check);
	} else if ((wp->w_flag & ~WFMODE) == WFEDIT && !lbound && !oldbound) {
		/*
		 * The case that happens on every keystroke: a character
		 * went into the line the cursor is on, the line count did
		 * not change, and nothing is scrolled sideways.  No other
		 * row can have moved, so no other row is worth painting.
		 */
		paint_line(currow, wp->w_dotp, 0, check);
		if (wp->w_flag & WFMODE)
			modeline(wp);
	} else if (!(wp->w_flag & ~(WFMOVE | WFMODE)) && !lbound && !oldbound) {
		/*
		 * The dot moved and the frame did not have to follow it -
		 * reframe() sets WFHARD when it does - so no text changed
		 * and every row still says what it already said.  Only the
		 * mode line can differ, and the cursor has to move.
		 */
		modeline(wp);
	} else if (wp->w_flag || lbound != oldbound)
		paint_window(wp, check);

	wp->w_flag = 0;
	wp->w_force = 0;

	/* update the cursor and flush the buffers */
	movecursor(currow, curcol - lbound);
	ttflush();

	/* a resize that arrived while we were painting */
	while (chg_width || chg_height)
		checkwinsize();
}

/*
 * reframe:
 *	check to see if the cursor is on in the window
 *	and re-frame it if needed or wanted
 */
static int reframe(struct window *wp)
{
	struct line *lp, *lp0;
	int i = 0;

	/* if not a requested reframe, check for a needed one */
	if ((wp->w_flag & WFFORCE) == 0) {
		/* loop from one line above the window to one line after */
		lp = wp->w_linep;
		lp0 = lback(lp);
		if (lp0 == wp->w_bufp->b_linep)
			i = 0;
		else {
			i = -1;
			lp = lp0;
		}
		for (; i < term.t_nrow; i++) {
			/* if the line is in the window, no reframe */
			if (lp == wp->w_dotp) {
				/* if not _quite_ in, we'll reframe gently */
				if (i < 0 || i == term.t_nrow - 1) {
					break;
				}
				return TRUE;
			}

			/* if we are at the end of the file, reframe */
			if (lp == wp->w_bufp->b_linep)
				break;

			/* on to the next line */
			lp = lforw(lp);
		}
	}
	if (i == -1) {				/* we're just above the window */
		i = scrollcount;		/* put dot at first line */
	} else if (i == term.t_nrow - 1) {	/* we're just below the window */
		i = -scrollcount;		/* put dot at last line */
	} else					/* put dot where requested */
		i = wp->w_force;		/* (is 0, unless reposition() was called) */

	wp->w_flag |= WFMODE;

	/* how far back to reframe? */
	if (i > 0) {				/* only one screen worth of lines max */
		if (--i >= term.t_nrow - 1)
			i = term.t_nrow - 2;
	} else if (i < 0) {			/* negative update???? */
		i += term.t_nrow - 1;
		if (i < 0)
			i = 0;
	} else
		i = (term.t_nrow - 1) / 2;

	/* backup to new line at top of window */
	lp = wp->w_dotp;
	while (i != 0 && lback(lp) != wp->w_bufp->b_linep) {
		--i;
		lp = lback(lp);
	}

	/* and reset the current line at top of window */
	wp->w_linep = lp;
	wp->w_flag |= WFHARD;
	wp->w_flag &= ~WFFORCE;
	return TRUE;
}

/*
 * updpos:
 *	update the position of the hardware cursor and handle extended
 *	lines. This is the only update for simple moves.
 */
static void updpos(void)
{
	struct line *lp;
	int i;

	/* find the current row */
	lp = curwp->w_linep;
	currow = 0;
	while (lp != curwp->w_dotp) {
		++currow;
		lp = lforw(lp);
	}

	/* find the current column */
	curcol = 0;
	i = 0;
	while (i < curwp->w_doto) {
		unicode_t c;
		int bytes;

		bytes = utf8_to_unicode(lp->l_text, i, curwp->w_doto, &c);
		i += bytes;
		curcol = next_column(curcol, c);
	}

	/*
	 * If the cursor has run off the right, scroll the line sideways
	 * far enough to show it.  lbound is the leftmost column that
	 * still fits, and paint_line() drops everything to the left of
	 * it.
	 */
	if (curcol >= term.t_ncol - 1) {
		int rcursor = ((curcol - term.t_ncol) % term.t_scrsiz)
			      + term.t_margin;
		lbound = curcol - rcursor + 1;
	} else
		lbound = 0;
}

/*
 * Add one character to the mode line image, expanding it the way the
 * screen would, and marking an overlong line with a '$' in the last
 * column.  The column counter is carried in *np.
 */
static void mladd(unsigned char *mline, int *np, int c)
{
	/* In case somebody passes us a signed char.. */
	if (c < 0) {
		c += 256;
		if (c < 0)
			return;
	}

	if (*np >= term.t_ncol) {
		mline[term.t_ncol - 1] = '$';
		(*np)++;
		return;
	}

	if (c == '\t') {
		do {
			mladd(mline, np, ' ');
		} while ((*np & tabmask) != 0);
		return;
	}

	if (c < 0x20) {
		mladd(mline, np, '^');
		mladd(mline, np, c ^ 0x40);
		return;
	}

	if (c == 0x7f) {
		mladd(mline, np, '^');
		mladd(mline, np, '?');
		return;
	}

	if (c >= 0x80 && c <= 0xA0) {
		static const char hex[] = "0123456789abcdef";
		mladd(mline, np, '\\');
		mladd(mline, np, hex[c >> 4]);
		mladd(mline, np, hex[c & 15]);
		return;
	}

	mline[(*np)++] = c;
}

/*
 * Redisplay the mode line for the window pointed to by the "wp". This is the
 * only routine that has any idea of how the modeline is formatted. You can
 * change the modeline format by hacking at this routine. Called by "update"
 * any time there is a dirty window.
 */
static void modeline(struct window *wp)
{
	char *cp;
	int c;
	int n;					/* cursor position count */
	struct buffer *bp;
	int i;					/* loop index */
	int lchar;				/* character to draw line in buffer with */
	int firstm;				/* is this the first mode? */
	char tline[NLINE];			/* buffer for part of mode line */
	unsigned char mline[MAXCOL];		/* the assembled mode line */

	memset(mline, ' ', sizeof(mline));
	n = 0;
	if (wp == curwp)			/* mark the current buffer */
		lchar = '-';
	else if (revexist)
		lchar = ' ';
	else
		lchar = '-';

	bp = wp->w_bufp;
	mladd(mline, &n, lchar);

	if ((bp->b_flag & BFCHG) != 0)		/* "*" if changed. */
		mladd(mline, &n, '*');
	else
		mladd(mline, &n, lchar);

	strcpy(tline, " ");
	strcat(tline, PROGRAM_NAME_LONG);
	strcat(tline, " ");
	strcat(tline, VERSION);
	strcat(tline, ": ");
	cp = &tline[0];
	while ((c = *cp++) != 0)
		mladd(mline, &n, c);

	cp = &bp->b_bname[0];
	while ((c = *cp++) != 0)
		mladd(mline, &n, c);

	strcpy(tline, " (");

	/* display the modes */

	firstm = TRUE;
	if ((bp->b_flag & BFTRUNC) != 0) {
		firstm = FALSE;
		strcat(tline, "Truncated");
	}
	for (i = 0; i < NUMMODES; i++)		/* add in the mode flags */
		if (wp->w_bufp->b_mode & (1 << i)) {
			if (firstm != TRUE)
				strcat(tline, " ");
			firstm = FALSE;
			strcat(tline, mode2name[i]);
		}
	strcat(tline, ") ");

	cp = &tline[0];
	while ((c = *cp++) != 0)
		mladd(mline, &n, c);

	if (bp->b_fname[0] != 0 && strcmp(bp->b_bname, bp->b_fname) != 0) {
		cp = &bp->b_fname[0];

		while ((c = *cp++) != 0)
			mladd(mline, &n, c);

		mladd(mline, &n, ' ');
	}

	while (n < term.t_ncol)			/* Pad to full width. */
		mladd(mline, &n, lchar);

	{					/* determine if top line, bottom line, or both are visible */
		struct line *lp = wp->w_linep;
		int rows = term.t_nrow - 1;
		char *msg = NULL;

		n -= 7;				/* strlen(" top ") plus a couple */
		while (rows--) {
			lp = lforw(lp);
			if (lp == wp->w_bufp->b_linep) {
				msg = " Bot ";
				break;
			}
		}
		if (lback(wp->w_linep) == wp->w_bufp->b_linep) {
			if (msg) {
				if (wp->w_linep == wp->w_bufp->b_linep)
					msg = " Emp ";
				else
					msg = " All ";
			} else {
				msg = " Top ";
			}
		}
		if (!msg) {
			struct line *lp;
			int numlines, predlines, ratio;

			lp = lforw(bp->b_linep);
			numlines = 0;
			predlines = 0;
			while (lp != bp->b_linep) {
				if (lp == wp->w_linep) {
					predlines = numlines;
				}
				++numlines;
				lp = lforw(lp);
			}
			if (wp->w_dotp == bp->b_linep) {
				msg = " Bot ";
			} else {
				ratio = 0;
				if (numlines != 0)
					ratio = (100L * predlines) / numlines;
				if (ratio > 99)
					ratio = 99;
				sprintf(tline, " %2d%% ", ratio);
				msg = tline;
			}
		}

		cp = msg;
		while ((c = *cp++) != 0)
			mladd(mline, &n, c);
	}

	/* and paint it, in reverse video across the full width */
	movecursor(term.t_nrow - 1, 0);
	tcaprev(TRUE);
	for (i = 0; i < term.t_ncol; i++)
		ttputc(mline[i]);
	tcaprev(FALSE);
	ttcol = term.t_ncol;
}

void upmode(void)
{						/* update all the mode lines */
	curwp->w_flag |= WFMODE;
}

/*
 * Send a command to the terminal to move the hardware cursor to row "row"
 * and column "col". The row and column arguments are origin 0. Optimize out
 * random calls. Update "ttrow" and "ttcol".
 */
void movecursor(int row, int col)
{
	if (row != ttrow || col != ttcol) {
		ttrow = row;
		ttcol = col;
		tcapmove(row, col);
	}
}

/*
 * Erase the message line. This is a special routine because the message line
 * is not considered to be part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed via a call to the flusher.
 */
void mlerase(void)
{
	int i;

	movecursor(term.t_nrow, 0);
	if (discmd == FALSE)
		return;

	if (eolexist == TRUE)
		tcapeeol();
	else {
		for (i = 0; i < term.t_ncol - 1; i++)
			ttputc(' ');
		movecursor(term.t_nrow, 1);	/* force the move! */
		movecursor(term.t_nrow, 0);
	}
	ttflush();
	mpresf = FALSE;
}

/*
 * The framing every message shares: get to the message line, and tidy
 * up behind whatever was written there.
 */
static int ml_open(void)
{
	/* if we are not currently echoing on the command line, abort this */
	if (discmd == FALSE) {
		movecursor(term.t_nrow, 0);
		return FALSE;
	}

	/* if we can not erase to end-of-line, do it manually */
	if (eolexist == FALSE) {
		mlerase();
		ttflush();
	}

	movecursor(term.t_nrow, 0);
	return TRUE;
}

static void ml_close(void)
{
	/* if we can, erase to the end of screen */
	if (eolexist == TRUE)
		tcapeeol();
	ttflush();
	mpresf = TRUE;
}

/*
 * Write a string to the message line.  The string is text, not a
 * format - which is what a caller with a run-time string wants, and
 * saves it from having to double any '%' the user typed.
 */
void mlputstr(const char *s)
{
	if (!ml_open())
		return;
	mlputs(s);
	ml_close();
}

/*
 * Write a message into the message line. Keep track of the physical cursor
 * position. A small class of printf like format items is handled. Set the
 * "message line" flag TRUE.
 *
 * The format string had better be one: anything built at run time goes
 * to mlputstr() instead.
 *
 * char *fmt;		format string for output
 * char *arg;		pointer to first argument to print
 */
void mlwrite(const char *fmt, ...)
{
	int c;					/* current char in format string */
	va_list ap;

	if (!ml_open())
		return;

	va_start(ap, fmt);
	while ((c = *fmt++) != 0) {
		if (c != '%') {
			ttputc(c);
			++ttcol;
		} else {
			c = *fmt++;
			switch (c) {
			case 'd':
				mlputi(va_arg(ap, int), 10);
				break;

			case 'o':
				mlputi(va_arg(ap, int), 8);
				break;

			case 'x':
				mlputi(va_arg(ap, int), 16);
				break;

			case 'D':
				mlputli(va_arg(ap, long), 10);
				break;

			case 's':
				mlputs(va_arg(ap, char *));
				break;

			case 'f':
				mlputf(va_arg(ap, int));
				break;

			default:
				ttputc(c);
				++ttcol;
			}
		}
	}
	va_end(ap);
	ml_close();
}

/*
 * Force a string out to the message line regardless of the
 * current $discmd setting. This is needed when $debug is TRUE
 * and for the write-message and clear-message-line commands
 *
 * char *s;		string to force out
 */
void mlforce(char *s)
{
	int oldcmd;				/* original command display flag */

	oldcmd = discmd;			/* save the discmd value */
	discmd = TRUE;				/* and turn display on */
	mlputstr(s);				/* write the string out */
	discmd = oldcmd;			/* and restore the original setting */
}

/*
 * Write out a string. Update the physical cursor position. This assumes that
 * the characters in the string all have width "1"; if this is not the case
 * things will get screwed up a little.
 */
void mlputs(const char *s)
{
	int c;

	while ((c = *s++) != 0) {
		ttputc(c);
		++ttcol;
	}
}

/*
 * Write out an integer, in the specified radix. Update the physical cursor
 * position.
 */
static void mlputi(int i, int r)
{
	int q;
	static char hexdigits[] = "0123456789ABCDEF";

	if (i < 0) {
		i = -i;
		ttputc('-');
	}

	q = i / r;

	if (q != 0)
		mlputi(q, r);

	ttputc(hexdigits[i % r]);
	++ttcol;
}

/*
 * do the same except as a long integer.
 */
static void mlputli(long l, int r)
{
	long q;

	if (l < 0) {
		l = -l;
		ttputc('-');
	}

	q = l / r;

	if (q != 0)
		mlputli(q, r);

	ttputc((int)(l % r) + '0');
	++ttcol;
}

/*
 * write out a scaled integer with two decimal places
 *
 * int s;		scaled integer to output
 */
static void mlputf(int s)
{
	int i;					/* integer portion of number */
	int f;					/* fractional portion of number */

	/* break it up */
	i = s / 100;
	f = s % 100;

	/* send out the integer portion */
	mlputi(i, 10);
	ttputc('.');
	ttputc((f / 10) + '0');
	ttputc((f % 10) + '0');
	ttcol += 3;
}

/* Get terminal size from system.
   Store number of lines into *heightp and width into *widthp.
   If zero or a negative number is stored, the value is not valid.  */

void getscreensize(int *widthp, int *heightp)
{
	struct winsize size;
	*widthp = 0;
	*heightp = 0;
	if (ioctl(0, TIOCGWINSZ, &size) < 0)
		return;
	*widthp = size.ws_col;
	*heightp = size.ws_row;
}

/*
 * The window changed size.  Write it down and get out: this is a signal
 * handler, and everything the display does - newsize()'s mlwrite() on a
 * silly size, ttputc(), the painter itself - would be running on top of
 * whatever the editor was in the middle of.
 *
 * The handler is installed without SA_RESTART, so the read() the editor
 * spends its life blocked in returns EINTR and ttgetc() services this
 * straight away rather than at the next keystroke.
 */
void sizesignal(int signr)
{
	int w, h;
	int old_errno = errno;

	getscreensize(&w, &h);

	if (h && w && (h - 1 != term.t_nrow || w != term.t_ncol)) {
		chg_width = w;
		chg_height = h;
	}

	errno = old_errno;
}

/*
 * Act on a size change the handler noticed, from a context that is
 * allowed to paint.
 */
void checkwinsize(void)
{
	int w = chg_width, h = chg_height;

	if (!w && !h)
		return;

	chg_width = chg_height = 0;
	if (h - 1 < term.t_mrow)
		newsize(TRUE, h);
	if (w < term.t_mcol)
		newwidth(TRUE, w);

	update_now();
}
