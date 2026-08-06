/*	buffer.c
 *
 *	Buffer management.
 *	Some of the functions are internal,
 *	and some are actually attached to user
 *	keys. Like everyone else, they set hints
 *	for the display system
 *
 *	modified by Petri Kutvonen
 */

#include        <stdio.h>

#include "estruct.h"
#include "globals.h"
#include "efunc.h"
#include "line.h"

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
int cmd_select_buffer(int f, int n)
{
	struct buffer *bp;
	int s;
	char bufn[NBUFN];

	if ((s = ask_string("Use buffer: ", bufn, NBUFN)) != TRUE)
		return s;
	if ((bp = find_buffer(bufn, TRUE, 0)) == NULL)
		return FALSE;
	return swbuffer(bp);
}

/*
 * switch to the next buffer in the buffer list
 *
 * int f, n;		default flag, numeric argument
 */
int cmd_next_buffer(int f, int n)
{
	struct buffer *bp = NULL;		/* eligable buffer to switch to */
	struct buffer *bbp;			/* eligable buffer to switch to */

	/* make sure the arg is legit */
	if (f == FALSE)
		n = 1;
	if (n < 1)
		return FALSE;

	bbp = curbp;
	while (n-- > 0) {
		/* advance to the next buffer */
		bp = bbp->b_bufp;

		/* cycle through the buffers to find an eligable one */
		while (bp == NULL || bp->b_flag & BFINVS) {
			if (bp == NULL)
				bp = buffer_head;
			else
				bp = bp->b_bufp;

			/* don't get caught in an infinite loop! */
			if (bp == bbp)
				return FALSE;

		}

		bbp = bp;
	}

	return swbuffer(bp);
}

/*
 * make buffer BP current
 */
int swbuffer(struct buffer *bp)
{
	if (--curbp->b_nwnd == 0) {		/* Last use.            */
		curbp->b_dotp = curwp->w_dotp;
		curbp->b_doto = curwp->w_doto;
		curbp->b_markp = curwp->w_markp;
		curbp->b_marko = curwp->w_marko;
	}
	curbp = bp;				/* Switch.              */
	if (curbp->b_active != TRUE) {		/* buffer not active yet */
		/* read it in and activate it */
		readin(curbp->b_fname, TRUE);
		curbp->b_dotp = line_next(curbp->b_linep);
		curbp->b_doto = 0;
		curbp->b_active = TRUE;
		curbp->b_mode |= global_mode;		/* P.K. */
	}
	curwp->w_bufp = bp;
	curwp->w_linep = bp->b_linep;		/* For macros, ignored. */
	curwp->w_flag |= WFMODE | WFFORCE | WFHARD;	/* Quite nasty.         */
	if (bp->b_nwnd++ == 0) {		/* First use.           */
		curwp->w_dotp = bp->b_dotp;
		curwp->w_doto = bp->b_doto;
		curwp->w_markp = bp->b_markp;
		curwp->w_marko = bp->b_marko;
	} else {
		struct window *wp;

		/*
		 * Somebody else is already showing this buffer, so start
		 * where they are looking rather than where the buffer was
		 * left the last time nobody had it on the screen.
		 */
		wp = window_head;
		while (wp != NULL) {
			if (wp != curwp && wp->w_bufp == bp) {
				curwp->w_dotp = wp->w_dotp;
				curwp->w_doto = wp->w_doto;
				curwp->w_markp = wp->w_markp;
				curwp->w_marko = wp->w_marko;
				break;
			}
			wp = wp->w_wndp;
		}
	}
	shown_buffer_changed();
	return TRUE;
}

/*
 * Tell a macro that the buffer on the screen is not the one that was
 * there before.  Anything bound to META-SPEC-X gets run; nothing is by
 * default.
 *
 * Not to be confused with buffer_changed(), which is about a buffer's
 * contents rather than about which one you are looking at.
 */
void shown_buffer_changed(void)
{
	execute(META | SPEC | 'X', FALSE, 1);
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
int cmd_delete_buffer(int f, int n)
{
	struct buffer *bp;
	int s;
	char bufn[NBUFN];

	if ((s = ask_string("Kill buffer: ", bufn, NBUFN)) != TRUE)
		return s;
	if ((bp = find_buffer(bufn, FALSE, 0)) == NULL)	/* Easy if unknown.     */
		return TRUE;
	if (bp->b_flag & BFINVS)		/* Deal with special buffers        */
		return TRUE;			/* by doing nothing.    */
	return destroy_buffer(bp);
}

/*
 * kill the buffer pointed to by bp
 */
int destroy_buffer(struct buffer *bp)
{
	struct buffer *bp1;
	struct buffer *bp2;
	int s;

	if (bp->b_nwnd != 0) {			/* Error if on screen.  */
		msg_printf("Buffer is being displayed");
		return FALSE;
	}
	if ((s = clear_buffer(bp)) != TRUE)		/* Blow text away.      */
		return s;
	free((char *)bp->b_linep);		/* Release header line. */
	bp1 = NULL;				/* Find the header.     */
	bp2 = buffer_head;
	while (bp2 != bp) {
		bp1 = bp2;
		bp2 = bp2->b_bufp;
	}
	bp2 = bp2->b_bufp;			/* Next one in chain.   */
	if (bp1 == NULL)			/* Unlink it.           */
		buffer_head = bp2;
	else
		bp1->b_bufp = bp2;
	free((char *)bp);			/* Release buffer block */
	return TRUE;
}

/*
 * Rename the current buffer
 *
 * int f, n;		default Flag & Numeric arg
 */
int cmd_name_buffer(int f, int n)
{
	struct buffer *bp;			/* pointer to scan through all buffers */
	char bufn[NBUFN];			/* buffer to hold buffer name */

	/* prompt for and get the new buffer name */
 ask:	if (ask_string("Change buffer name to: ", bufn, NBUFN) != TRUE)
		return FALSE;

	/* and check for duplicates */
	bp = buffer_head;
	while (bp != NULL) {
		if (bp != curbp) {
			/* if the names the same */
			if (strcmp(bufn, bp->b_bname) == 0)
				goto ask;	/* try again */
		}
		bp = bp->b_bufp;		/* onward */
	}

	strcpy(curbp->b_bname, bufn);		/* copy buffer name to structure */
	curwp->w_flag |= WFMODE;		/* make mode line replot */
	msg_erase();
	return TRUE;
}

void ltoa(char *buf, int width, long num)
{
	buf[width] = 0;				/* End of string.       */
	while (num >= 10) {			/* Conditional digits.  */
		buf[--width] = (int)(num % 10L) + '0';
		num /= 10L;
	}
	buf[--width] = (int)num + '0';		/* Always 1 digit.      */
	while (width != 0)			/* Pad with blanks.     */
		buf[--width] = ' ';
}

/*
 * The argument "text" points to
 * a string. Append this line to the
 * buffer list buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 */
int addline(struct buffer *bp, char *text)
{
	struct line *lp;
	int i;
	int ntext;

	ntext = strlen(text);
	if ((lp = line_alloc(ntext)) == NULL)
		return FALSE;
	for (i = 0; i < ntext; ++i)
		lputc(lp, i, text[i]);
	bp->b_linep->l_bp->l_fp = lp;		/* Hook onto the end    */
	lp->l_bp = bp->b_linep->l_bp;
	bp->b_linep->l_bp = lp;
	lp->l_fp = bp->b_linep;
	if (bp->b_dotp == bp->b_linep)		/* If "." is at the end */
		bp->b_dotp = lp;		/* move it to new line  */
	return TRUE;
}

/*
 * Rebuild the text in the *List* buffer: a line per buffer, saying
 * whether it has been read in, whether it has changed, what modes it
 * is in, how big it is, and what file it came from.
 *
 * int iflag;		list the invisible buffers too
 */
int makelist(int iflag)
{
	char *cp1;
	char *cp2;
	int c;
	struct buffer *bp;
	struct line *lp;
	int s;
	int i;
	long nbytes;				/* # of bytes in current buffer */
	char b[7 + 1];
	char line[MAXCOL];

	list_buffer->b_flag &= ~BFCHG;		/* Don't complain!      */
	if ((s = clear_buffer(list_buffer)) != TRUE)	/* Blow old text away   */
		return s;
	strcpy(list_buffer->b_fname, "");
	if (addline(list_buffer, "ACT MODES        Size Buffer        File") == FALSE
	    || addline(list_buffer, "--- -----        ---- ------        ----") == FALSE)
		return FALSE;
	bp = buffer_head;			/* For all buffers      */

	/* build line to report global mode settings */
	cp1 = &line[0];
	*cp1++ = ' ';
	*cp1++ = ' ';
	*cp1++ = ' ';
	*cp1++ = ' ';

	/* output the mode codes */
	for (i = 0; i < NUMMODES; i++)
		if (global_mode & (1 << i))
			*cp1++ = modecode[i];
		else
			*cp1++ = '.';
	strcpy(cp1, "         Global Modes");
	if (addline(list_buffer, line) == FALSE)
		return FALSE;

	/* output the list of buffers */
	while (bp != NULL) {
		/* skip invisable buffers if iflag is false */
		if (((bp->b_flag & BFINVS) != 0) && (iflag != TRUE)) {
			bp = bp->b_bufp;
			continue;
		}
		cp1 = &line[0];			/* Start at left edge   */

		/* output status of ACTIVE flag (has the file been read in? */
		if (bp->b_active == TRUE)	/* "@" if activated       */
			*cp1++ = '@';
		else
			*cp1++ = ' ';

		/* output status of changed flag */
		if ((bp->b_flag & BFCHG) != 0)	/* "*" if changed       */
			*cp1++ = '*';
		else
			*cp1++ = ' ';

		/* report if the file is truncated */
		if ((bp->b_flag & BFTRUNC) != 0)
			*cp1++ = '#';
		else
			*cp1++ = ' ';

		*cp1++ = ' ';			/* space */

		/* output the mode codes */
		for (i = 0; i < NUMMODES; i++) {
			if (bp->b_mode & (1 << i))
				*cp1++ = modecode[i];
			else
				*cp1++ = '.';
		}
		*cp1++ = ' ';			/* Gap.                 */
		nbytes = 0L;			/* Count bytes in buf.  */
		lp = line_next(bp->b_linep);
		while (lp != bp->b_linep) {
			nbytes += (long)line_length(lp) + 1L;
			lp = line_next(lp);
		}
		ltoa(b, 7, nbytes);		/* 6 digit buffer size. */
		cp2 = &b[0];
		while ((c = *cp2++) != 0)
			*cp1++ = c;
		*cp1++ = ' ';			/* Gap.                 */
		cp2 = &bp->b_bname[0];		/* Buffer name          */
		while ((c = *cp2++) != 0)
			*cp1++ = c;
		cp2 = &bp->b_fname[0];		/* File name            */
		if (*cp2 != 0) {
			while (cp1 < &line[3 + 1 + 5 + 1 + 6 + 4 + NBUFN])
				*cp1++ = ' ';
			while ((c = *cp2++) != 0) {
				if (cp1 < &line[MAXCOL - 1])
					*cp1++ = c;
			}
		}
		*cp1 = 0;			/* Add to the buffer.   */
		if (addline(list_buffer, line) == FALSE)
			return FALSE;
		bp = bp->b_bufp;
	}
	return TRUE;				/* All done             */
}

/*
 * The list-buffers command: rebuild the listing and go and look at it.
 * With an argument, the invisible buffers are listed too.
 *
 * This used to put the listing in a second window and leave you in the
 * first.  With one window we simply switch to it, and you come back
 * with C-x b the way you would from any other buffer.
 *
 * The dot has to be put back by hand rather than left to swbuffer():
 * makelist() has just thrown away every line the window was pointing
 * at, so if we were already looking at *List* the window's idea of
 * where it was is a pointer to freed memory.
 */
int cmd_list_buffers(int f, int n)
{
	int s;

	if ((s = makelist(f)) != TRUE)
		return s;

	if (curwp->w_bufp != list_buffer && (s = swbuffer(list_buffer)) != TRUE)
		return s;

	curwp->w_linep = line_next(list_buffer->b_linep);
	curwp->w_dotp = line_next(list_buffer->b_linep);
	curwp->w_doto = 0;
	curwp->w_markp = NULL;
	curwp->w_marko = 0;
	curwp->w_flag |= WFMODE | WFHARD;
	return TRUE;
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Buffers
 * that hold magic internal stuff are
 * not considered; who cares if the
 * list of buffer names is hacked.
 * Return FALSE if no buffers
 * have been changed.
 */
int any_changed_buffers(void)
{
	struct buffer *bp;

	bp = buffer_head;
	while (bp != NULL) {
		if ((bp->b_flag & BFINVS) == 0 && (bp->b_flag & BFCHG) != 0)
			return TRUE;
		bp = bp->b_bufp;
	}
	return FALSE;
}

/*
 * Find a buffer, by name. Return a pointer
 * to the buffer structure associated with it.
 * If the buffer is not found
 * and the "cflag" is TRUE, create it. The "bflag" is
 * the settings for the flags in in buffer.
 */
struct buffer *find_buffer(char *bname, int cflag, int bflag)
{
	struct buffer *bp;
	struct buffer *sb;			/* buffer to insert after */
	struct line *lp;

	bp = buffer_head;
	while (bp != NULL) {
		if (strcmp(bname, bp->b_bname) == 0)
			return bp;
		bp = bp->b_bufp;
	}
	if (cflag != FALSE) {
		if ((bp = (struct buffer *)malloc(sizeof(struct buffer))) == NULL)
			return NULL;
		if ((lp = line_alloc(0)) == NULL) {
			free((char *)bp);
			return NULL;
		}
		/* find the place in the list to insert this buffer */
		if (buffer_head == NULL || strcmp(buffer_head->b_bname, bname) > 0) {
			/* insert at the beginning */
			bp->b_bufp = buffer_head;
			buffer_head = bp;
		} else {
			sb = buffer_head;
			while (sb->b_bufp != NULL) {
				if (strcmp(sb->b_bufp->b_bname, bname) > 0)
					break;
				sb = sb->b_bufp;
			}

			/* and insert it */
			bp->b_bufp = sb->b_bufp;
			sb->b_bufp = bp;
		}

		/* and set up the other buffer fields */
		bp->b_active = TRUE;
		bp->b_dotp = lp;
		bp->b_doto = 0;
		bp->b_markp = NULL;
		bp->b_marko = 0;
		bp->b_flag = bflag;
		bp->b_mode = global_mode;
		bp->b_nwnd = 0;
		bp->b_linep = lp;
		bp->b_fstate.fs_what = FSTATE_UNKNOWN;
		strcpy(bp->b_fname, "");
		strcpy(bp->b_bname, bname);
		lp->l_fp = lp;
		lp->l_bp = lp;
	}
	return bp;
}

/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return TRUE if everything
 * looks good.
 */
int clear_buffer(struct buffer *bp)
{
	struct line *lp;
	int s;

	if ((bp->b_flag & BFINVS) == 0		/* Not scratch buffer.  */
	    && (bp->b_flag & BFCHG) != 0	/* Something changed    */
	    && (s = ask_yesno("Discard changes")) != TRUE)
		return s;
	bp->b_flag &= ~BFCHG;			/* Not changed          */
	while ((lp = line_next(bp->b_linep)) != bp->b_linep)
		line_free(lp);
	bp->b_dotp = bp->b_linep;		/* Fix "."              */
	bp->b_doto = 0;
	bp->b_markp = NULL;			/* Invalidate "mark"    */
	bp->b_marko = 0;
	return TRUE;
}

/*
 * unmark the current buffers change flag
 *
 * int f, n;		unused command arguments
 */
int cmd_unmark_buffer(int f, int n)
{
	curbp->b_flag &= ~BFCHG;
	curwp->w_flag |= WFMODE;
	return TRUE;
}
