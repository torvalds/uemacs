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

struct video {
	int v_flag;				/* Flags */
	unicode_t v_text[1];			/* Screen data. */
};

#define VFCHG   0x0001				/* Changed flag                 */
#define	VFEXT	0x0002				/* extended (beyond column 80)  */
#define	VFREQ	0x0008				/* reverse video request        */

static struct video **vscreen;			/* Virtual screen. */

static int displaying = TRUE;
#include <signal.h>
#include <sys/ioctl.h>
/* for window size changes */
int chg_width, chg_height;

static int reframe(struct window *wp);
static void updone(struct window *wp);
static void updall(struct window *wp);
static void updext(void);
static int updateline(int row, struct video *vp);
static void modeline(struct window *wp);
static void mlputi(int i, int r);
static void mlputli(long l, int r);
static void mlputf(int s);
static int newscreensize(int h, int w);

/*
 * Initialize the data structures used by the display code. The edge vectors
 * used to access the screens are set up. The operating system's terminal I/O
 * channel is set up. All the other things get initialized at compile time.
 * The original window has "WFCHG" set, so that it will get completely
 * redrawn on the first call to "update".
 */
void vtinit(void)
{
	int i;
	struct video *vp;

	TTopen();				/* open the screen */
	TTkopen();				/* open the keyboard */
	TTrev(FALSE);
	vscreen = xmalloc(term.t_mrow * sizeof(struct video *));

	for (i = 0; i < term.t_mrow; ++i) {
		vp = xmalloc(sizeof(struct video) + term.t_mcol * 4);
		vp->v_flag = 0;
		vscreen[i] = vp;
	}
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
	TTflush();
	TTclose();
	TTkclose();
	write(1, "\r", 1);
}

/*
 * Set the virtual cursor to the specified row and column on the virtual
 * screen. There is no checking for nonsense values; this might be a good
 * idea during the early stages.
 */
void vtmove(int row, int col)
{
	vtrow = row;
	vtcol = col;
}

/*
 * Write a character to the virtual screen. The virtual row and
 * column are updated. If we are not yet on left edge, don't print
 * it yet. If the line is too long put a "$" in the last column.
 *
 * This routine only puts printing characters into the virtual
 * terminal buffers. Only column overflow is checked.
 */
static void vtputc(int c)
{
	struct video *vp;			/* ptr to line being updated */

	/* In case somebody passes us a signed char.. */
	if (c < 0) {
		c += 256;
		if (c < 0)
			return;
	}

	vp = vscreen[vtrow];

	if (vtcol >= term.t_ncol) {
		++vtcol;
		vp->v_text[term.t_ncol - 1] = '$';
		return;
	}

	if (c == '\t') {
		do {
			vtputc(' ');
		} while (((vtcol + taboff) & tabmask) != 0);
		return;
	}

	if (c < 0x20) {
		vtputc('^');
		vtputc(c ^ 0x40);
		return;
	}

	if (c == 0x7f) {
		vtputc('^');
		vtputc('?');
		return;
	}

	if (c >= 0x80 && c <= 0xA0) {
		static const char hex[] = "0123456789abcdef";
		vtputc('\\');
		vtputc(hex[c >> 4]);
		vtputc(hex[c & 15]);
		return;
	}

	if (vtcol >= 0)
		vp->v_text[vtcol] = c;
	++vtcol;
}

/*
 * Erase from the end of the software cursor to the end of the line on which
 * the software cursor is located.
 */
static void vteeol(void)
{
	unicode_t *vcp = vscreen[vtrow]->v_text;

	while (vtcol < term.t_ncol)
		vcp[vtcol++] = ' ';
}

/*
 * upscreen:
 *	user routine to force a screen update
 *	always finishes complete update
 */
int upscreen(int f, int n)
{
	update(TRUE);
	return TRUE;
}

/*
 * Make sure that the display is right. This is a three part process. First,
 * scan through all of the windows looking for dirty ones. Check the framing,
 * and refresh the screen. Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the virtual and physical
 * screens the same.
 *
 * int force;		force update past type ahead?
 */
int update(int force)
{
	struct window *wp;

	if (force == FALSE && kbdmode == PLAY)
		return TRUE;

	displaying = TRUE;

	/* update any windows that need refreshing */
	wp = curwp;
	if (wp->w_flag) {
		/* if the window has changed, service it */
		reframe(wp);		/* check the framing */
		if ((wp->w_flag & ~WFMODE) == WFEDIT)
			updone(wp);	/* update EDITed line */
		else if (wp->w_flag & ~WFMOVE)
			updall(wp);	/* update all lines */
		if (wp->w_flag & WFMODE)
			modeline(wp);	/* update modeline */
		wp->w_flag = 0;
		wp->w_force = 0;
	}

	/* recalc the current hardware cursor location */
	updpos();

	/* check for lines to de-extend */
	upddex();

	/* if screen is garbage, re-plot it */
	if (sgarbf != FALSE)
		updgar();

	/* update the virtual screen to the physical screen */
	updupd(force);

	/* update the cursor and flush the buffers */
	movecursor(currow, curcol - lbound);
	TTflush();
	displaying = FALSE;
	while (chg_width || chg_height)
		newscreensize(chg_height, chg_width);
	return TRUE;
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

static void show_line(struct line *lp)
{
	int i = 0, len = llength(lp);

	while (i < len) {
		unicode_t c;
		i += utf8_to_unicode(lp->l_text, i, len, &c);
		vtputc(c);
	}
}

/*
 * updone:
 *	update the current line	to the virtual screen
 *
 * struct window *wp;		window to update current line in
 */
static void updone(struct window *wp)
{
	struct line *lp;			/* line to update */
	int sline;				/* physical screen line to update */

	/* search down the line we want */
	lp = wp->w_linep;
	sline = 0;
	while (lp != wp->w_dotp) {
		++sline;
		lp = lforw(lp);
	}

	/* and update the virtual line */
	vscreen[sline]->v_flag |= VFCHG;
	vscreen[sline]->v_flag &= ~VFREQ;
	vtmove(sline, 0);
	show_line(lp);
	vteeol();
}

/*
 * updall:
 *	update all the lines in a window on the virtual screen
 *
 * struct window *wp;		window to update lines in
 */
static void updall(struct window *wp)
{
	struct line *lp;			/* line to update */
	int sline;				/* physical screen line to update */

	/* search down the lines, updating them */
	lp = wp->w_linep;
	sline = 0;
	while (sline < term.t_nrow - 1) {

		/* and update the virtual line */
		vscreen[sline]->v_flag |= VFCHG;
		vscreen[sline]->v_flag &= ~VFREQ;
		vtmove(sline, 0);
		if (lp != wp->w_bufp->b_linep) {
			/* if we are not at the end */
			show_line(lp);
			lp = lforw(lp);
		}

		/* on to the next one */
		vteeol();
		++sline;
	}

}

/*
 * updpos:
 *	update the position of the hardware cursor and handle extended
 *	lines. This is the only update for simple moves.
 */
void updpos(void)
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

	/* if extended, flag so and update the virtual line image */
	if (curcol >= term.t_ncol - 1) {
		vscreen[currow]->v_flag |= (VFEXT | VFCHG);
		updext();
	} else
		lbound = 0;
}

/*
 * upddex:
 *	de-extend any line that derserves it
 */
void upddex(void)
{
	struct window *wp;
	struct line *lp;
	int i;

	wp = curwp;
	lp = wp->w_linep;
	i = 0;

	while (i < term.t_nrow - 1) {
		if (vscreen[i]->v_flag & VFEXT) {
			if ((wp != curwp) || (lp != wp->w_dotp) ||
			    (curcol < term.t_ncol - 1)) {
				vtmove(i, 0);
				show_line(lp);
				vteeol();

				/* this line no longer is extended */
				vscreen[i]->v_flag &= ~VFEXT;
				vscreen[i]->v_flag |= VFCHG;
			}
		}
		lp = lforw(lp);
		++i;
	}
}

/*
 * updgar:
 *	if the screen is garbage, clear the physical screen and
 *	the virtual screen and force a full update
 */
void updgar(void)
{
	int i;

	for (i = 0; i < term.t_nrow; ++i)
		vscreen[i]->v_flag |= VFCHG;

	movecursor(0, 0);			/* Erase the screen. */
	(*term.t_eeop) ();
	sgarbf = FALSE;				/* Erase-page clears */
	mpresf = FALSE;				/* the message area. */
}

/*
 * updupd:
 *	update the physical screen from the virtual screen
 *
 * int force;		forced update flag
 */
int updupd(int force)
{
	struct video *vp1;
	int i;

	for (i = 0; i < term.t_nrow; ++i) {
		vp1 = vscreen[i];

		/* for each line that needs to be updated */
		if ((vp1->v_flag & VFCHG) != 0) {
			updateline(i, vp1);
		}
	}
	return TRUE;
}

/*
 * updext:
 *	update the extended line which the cursor is currently
 *	on at a column greater than the terminal width. The line
 *	will be scrolled right or left to let the user see where
 *	the cursor is
 */
static void updext(void)
{
	int rcursor;				/* real cursor location */
	struct line *lp;			/* pointer to current line */

	/* calculate what column the real cursor will end up in */
	rcursor = ((curcol - term.t_ncol) % term.t_scrsiz) + term.t_margin;
	taboff = lbound = curcol - rcursor + 1;

	/* scan through the line outputing characters to the virtual screen */
	/* once we reach the left edge                                  */
	vtmove(currow, -lbound);		/* start scanning offscreen */
	lp = curwp->w_dotp;			/* line to output */
	show_line(lp);

	/* truncate the virtual line, restore tab offset */
	vteeol();
	taboff = 0;

	/* and put a '$' in column 1 */
	vscreen[currow]->v_text[0] = '$';
}

static void TTputs(const char *s)
{
	for (char c; (c = *s) != 0; s++)
		TTputc(c);
}

static bool is_letter(unicode_t ch)
{
	return ch > 128 || isalpha(ch);
}

static bool is_notaword(unicode_t ch)
{
	return ch == '_' || (ch >= '0' && ch <= '9');
}

static int find_letter(unicode_t *line, size_t len, int pos)
{
	while (pos < len) {
		if (is_letter(line[pos]))
			return pos;
		pos++;
	}
	return -1;
}

static int find_not_letter(unicode_t *line, size_t len, int pos)
{
	while (pos < len) {
		if (!is_letter(line[pos]))
			return pos;
		pos++;
	}
	return len;
}

#define BAD_WORD_BEGIN 1
#define BAD_WORD_END 2

static size_t findwords(unicode_t *line, size_t len, unsigned char *result, size_t size)
{
	int pos = 0;

	if (len < size)
		size = len;
	memset(result, 0, size);

	while ((pos = find_letter(line, len, pos)) >= 0) {
		int start = pos;
		int end = find_not_letter(line, len, pos + 1);

		// Special case: allow (one) apostrophe for abbreviations
		if (end + 1 < len && line[end] == '\'' && is_letter(line[end + 1]))
			end = find_not_letter(line, len, end + 2);

		pos = end + 1;

		// A word with adjacent numbers or underscores is
		// not a word, it's a hex number or a variable name
		if (start && is_notaword(line[start - 1]))
			continue;
		if (end < len && is_notaword(line[end]))
			continue;

		// We found something that may be a real word.
		// Check it, and mark it in the result
		if (end > size)
			break;

		char word_buffer[80];
		int word_len = end - start;
		if (word_len >= sizeof(word_buffer) - 1)
			continue;
		for (int i = 0; i < word_len; i++)
			word_buffer[i] = line[start + i];
		word_buffer[word_len] = 0;
		if (spellcheck(word_buffer))
			continue;

		// We found something that hunspell doesn't like
		result[start] = BAD_WORD_BEGIN;
		result[end - 1] |= BAD_WORD_END;
	}
	return size;
}

/*
 * Update a single line. This does not know how to use insert or delete
 * character sequences; we are using VT52 functionality. Update the physical
 * row and column variables. It does try an exploit erase to end of line.
 */

/*
 * updateline()
 *
 * int row;		row of screen to update
 * struct video *vp;	virtual screen image
 */
static int updateline(int row, struct video *vp)
{
	int maxchar = 0, analyzed = 0;
	unsigned char array[256];
	bool spellcheck = curwp->w_bufp->b_mode & MDSPELL;

	movecursor(row, 0);			/* Go to start of line. */

	/* scan through the line and dump it to the the
	   virtual screen array, finding where the last non-space is  */
	for (int i = 0; i < term.t_ncol; i++) {
		unicode_t ch = vp->v_text[i];
		if (ch != ' ')
			maxchar = i + 1;
	}

	/* set rev video if needed, and fill all the way */
	if (vp->v_flag & VFREQ) {
		maxchar = term.t_ncol;
		TTrev(TRUE);
		spellcheck = false;
	}

	if (spellcheck)
		analyzed = findwords(vp->v_text, maxchar, array, sizeof(array));

#define SPELLSTART "\033[1m"
#define SPELLSTOP "\033[22m"

	int started = 0;
	for (int i = 0; i < maxchar; i++) {
		if (i < analyzed && (array[i] & BAD_WORD_BEGIN)) {
			started = 1;
			TTputs(SPELLSTART);
		}
		TTputc(vp->v_text[i]);
		if (i < analyzed && (array[i] & BAD_WORD_END)) {
			TTputs(SPELLSTOP);
			started = 0;
		}
	}
	if (started)
		TTputs(SPELLSTOP);
	ttcol = term.t_ncol;

	TTeeol();
	/* turn rev video off */
	TTrev(FALSE);

	/* update the needed flags */
	vp->v_flag &= ~VFCHG;
	return TRUE;
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

	n = term.t_nrow - 1;			/* Location. */
	vscreen[n]->v_flag |= VFCHG | VFREQ;		/* Redraw next time. */
	vtmove(n, 0);				/* Seek to right line. */
	if (wp == curwp)			/* mark the current buffer */
		lchar = '-';
	else if (revexist)
		lchar = ' ';
	else
		lchar = '-';

	bp = wp->w_bufp;
	vtputc(lchar);

	if ((bp->b_flag & BFCHG) != 0)		/* "*" if changed. */
		vtputc('*');
	else
		vtputc(lchar);

	n = 2;

	strcpy(tline, " ");
	strcat(tline, PROGRAM_NAME_LONG);
	strcat(tline, " ");
	strcat(tline, VERSION);
	strcat(tline, ": ");
	cp = &tline[0];
	while ((c = *cp++) != 0) {
		vtputc(c);
		++n;
	}

	cp = &bp->b_bname[0];
	while ((c = *cp++) != 0) {
		vtputc(c);
		++n;
	}

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
	while ((c = *cp++) != 0) {
		vtputc(c);
		++n;
	}

	if (bp->b_fname[0] != 0 && strcmp(bp->b_bname, bp->b_fname) != 0) {
		cp = &bp->b_fname[0];

		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}

		vtputc(' ');
		++n;
	}

	while (n < term.t_ncol) {		/* Pad to full width. */
		vtputc(lchar);
		++n;
	}

	{					/* determine if top line, bottom line, or both are visible */
		struct line *lp = wp->w_linep;
		int rows = term.t_nrow - 1;
		char *msg = NULL;

		vtcol = n - 7;			/* strlen(" top ") plus a couple */
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
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
	}
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
		TTmove(row, col);
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
		TTeeol();
	else {
		for (i = 0; i < term.t_ncol - 1; i++)
			TTputc(' ');
		movecursor(term.t_nrow, 1);	/* force the move! */
		movecursor(term.t_nrow, 0);
	}
	TTflush();
	mpresf = FALSE;
}

/*
 * Write a message into the message line. Keep track of the physical cursor
 * position. A small class of printf like format items is handled. Assumes the
 * stack grows down; this assumption is made by the "++" in the argument scan
 * loop. Set the "message line" flag TRUE.
 *
 * char *fmt;		format string for output
 * char *arg;		pointer to first argument to print
 */
void mlwrite(const char *fmt, ...)
{
	int c;					/* current char in format string */
	va_list ap;

	/* if we are not currently echoing on the command line, abort this */
	if (discmd == FALSE) {
		movecursor(term.t_nrow, 0);
		return;
	}

	/* if we can not erase to end-of-line, do it manually */
	if (eolexist == FALSE) {
		mlerase();
		TTflush();
	}

	movecursor(term.t_nrow, 0);
	va_start(ap, fmt);
	while ((c = *fmt++) != 0) {
		if (c != '%') {
			TTputc(c);
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
				TTputc(c);
				++ttcol;
			}
		}
	}
	va_end(ap);

	/* if we can, erase to the end of screen */
	if (eolexist == TRUE)
		TTeeol();
	TTflush();
	mpresf = TRUE;
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
	mlwrite(s);				/* write the string out */
	discmd = oldcmd;			/* and restore the original setting */
}

/*
 * Write out a string. Update the physical cursor position. This assumes that
 * the characters in the string all have width "1"; if this is not the case
 * things will get screwed up a little.
 */
void mlputs(char *s)
{
	int c;

	while ((c = *s++) != 0) {
		TTputc(c);
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
		TTputc('-');
	}

	q = i / r;

	if (q != 0)
		mlputi(q, r);

	TTputc(hexdigits[i % r]);
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
		TTputc('-');
	}

	q = l / r;

	if (q != 0)
		mlputli(q, r);

	TTputc((int)(l % r) + '0');
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
	TTputc('.');
	TTputc((f / 10) + '0');
	TTputc((f % 10) + '0');
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

void sizesignal(int signr)
{
	int w, h;
	int old_errno = errno;

	getscreensize(&w, &h);

	if (h && w && (h - 1 != term.t_nrow || w != term.t_ncol))
		newscreensize(h, w);

	signal(SIGWINCH, sizesignal);
	errno = old_errno;
}

static int newscreensize(int h, int w)
{
	/* do the change later */
	if (displaying) {
		chg_width = w;
		chg_height = h;
		return FALSE;
	}
	chg_width = chg_height = 0;
	if (h - 1 < term.t_mrow)
		newsize(TRUE, h);
	if (w < term.t_mcol)
		newwidth(TRUE, w);

	update(TRUE);
	return TRUE;
}
