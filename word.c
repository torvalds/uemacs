/*	word.c
 *
 *      The routines in this file implement commands that work word or a
 *      paragraph at a time.  There are all sorts of word mode commands.  If I
 *      do any sentence mode commands, they are likely to be put in this file.
 *
 *	Modified by Petri Kutvonen
 */

#include <stdio.h>

#include "estruct.h"
#include "globals.h"
#include "efunc.h"
#include "line.h"

/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.	Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 * Returns TRUE on success, FALSE on errors.
 *
 * @f: default flag.
 * @n: numeric argument.
 */
int cmd_wrap_word(int f, int n)
{
	int cnt;				/* size of word wrapped to next line */
	int c;					/* charector temporary */

	/* backup from the <NL> 1 char */
	if (!cmd_backward_character(0, 1))
		return FALSE;

	/* back up until we aren't in a word,
	   make sure there is a break in the line */
	cnt = 0;
	while (((c = lgetc(curwp->w_dotp, curwp->w_doto)) != ' ')
	       && (c != '\t')) {
		cnt++;
		if (!cmd_backward_character(0, 1))
			return FALSE;
		/* if we make it to the beginning, start a new line */
		if (curwp->w_doto == 0) {
			cmd_end_of_line(FALSE, 0);
			return insert_newline();
		}
	}

	/* delete the forward white space */
	if (!cmd_delete_next_character(0, 1))
		return FALSE;

	/* put in a end of line */
	if (!insert_newline())
		return FALSE;

	/* and past the first word */
	while (cnt-- > 0) {
		if (cmd_forward_character(FALSE, 1) == FALSE)
			return FALSE;
	}
	return TRUE;
}

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backchar" and "forwchar" routines. Error if you try to
 * move beyond the buffers.
 */
int cmd_previous_word(int f, int n)
{
	if (n < 0)
		return cmd_next_word(f, -n);
	if (cmd_backward_character(FALSE, 1) == FALSE)
		return FALSE;
	while (n--) {
		while (inword() == FALSE) {
			if (cmd_backward_character(FALSE, 1) == FALSE)
				return FALSE;
		}
		while (inword() != FALSE) {
			if (cmd_backward_character(FALSE, 1) == FALSE)
				return FALSE;
		}
	}
	return cmd_forward_character(FALSE, 1);
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
int cmd_next_word(int f, int n)
{
	if (n < 0)
		return cmd_previous_word(f, -n);
	while (n--) {
		while (inword() == TRUE) {
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
		}

		while (inword() == FALSE) {
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

/*
 * Move the cursor forward by the specified number of words. As you move,
 * convert any characters to upper case. Error if you try and move beyond the
 * end of the buffer. Bound to "M-U".
 */
int cmd_case_word_upper(int f, int n)
{
	int c;

	if (curbp->b_mode & MDVIEW)		/* don't allow this command if      */
		return readonly_error();		/* we are in read only mode     */
	if (n < 0)
		return FALSE;
	while (n--) {
		while (inword() == FALSE) {
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (islower(c)) {
				c -= 'a' - 'A';
				lputc(curwp->w_dotp, curwp->w_doto, c);
				buffer_changed(WFHARD);
			}
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert characters to lower case. Error if you try and move over the end of
 * the buffer. Bound to "M-L".
 */
int cmd_case_word_lower(int f, int n)
{
	int c;

	if (curbp->b_mode & MDVIEW)		/* don't allow this command if      */
		return readonly_error();		/* we are in read only mode     */
	if (n < 0)
		return FALSE;
	while (n--) {
		while (inword() == FALSE) {
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (isupper(c)) {
				c += 'a' - 'A';
				lputc(curwp->w_dotp, curwp->w_doto, c);
				buffer_changed(WFHARD);
			}
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert the first character of the word to upper case, and subsequent
 * characters to lower case. Error if you try and move past the end of the
 * buffer. Bound to "M-C".
 */
int cmd_case_word_capitalize(int f, int n)
{
	int c;

	if (curbp->b_mode & MDVIEW)		/* don't allow this command if      */
		return readonly_error();		/* we are in read only mode     */
	if (n < 0)
		return FALSE;
	while (n--) {
		while (inword() == FALSE) {
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
		}
		if (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (islower(c)) {
				c -= 'a' - 'A';
				lputc(curwp->w_dotp, curwp->w_doto, c);
				buffer_changed(WFHARD);
			}
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
			while (inword() != FALSE) {
				c = lgetc(curwp->w_dotp, curwp->w_doto);
				if (isupper(c)) {
					c += 'a' - 'A';
					lputc(curwp->w_dotp, curwp->w_doto, c);
					buffer_changed(WFHARD);
				}
				if (cmd_forward_character(FALSE, 1) == FALSE)
					return FALSE;
			}
		}
	}
	return TRUE;
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. With a zero argument, just
 * kill one word and no whitespace. Bound to "M-D".
 */
int cmd_delete_next_word(int f, int n)
{
	struct line *dotp;			/* original cursor line */
	int doto;				/*      and row */
	int c;					/* temp char */
	long size;				/* # of chars to delete */

	/* don't allow this command if we are in read only mode */
	if (curbp->b_mode & MDVIEW)
		return readonly_error();

	/* ignore the command if there is a negative argument */
	if (n < 0)
		return FALSE;

	/* Clear the kill buffer if last command wasn't a kill */
	if ((lastflag & CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;			/* this command is a kill */

	/* save the current cursor position */
	dotp = curwp->w_dotp;
	doto = curwp->w_doto;

	/* figure out how many characters to give the axe */
	size = 0;

	/* get us into a word.... */
	while (inword() == FALSE) {
		if (cmd_forward_character(FALSE, 1) == FALSE)
			return FALSE;
		++size;
	}

	if (n == 0) {
		/* skip one word, no whitespace! */
		while (inword() == TRUE) {
			if (cmd_forward_character(FALSE, 1) == FALSE)
				return FALSE;
			++size;
		}
	} else {
		/* skip n words.... */
		while (n--) {

			/* if we are at EOL; skip to the beginning of the next */
			while (curwp->w_doto == line_length(curwp->w_dotp)) {
				if (cmd_forward_character(FALSE, 1) == FALSE)
					return FALSE;
				++size;
			}

			/* move forward till we are at the end of the word */
			while (inword() == TRUE) {
				if (cmd_forward_character(FALSE, 1) == FALSE)
					return FALSE;
				++size;
			}

			/* if there are more words, skip the interword stuff */
			if (n != 0)
				while (inword() == FALSE) {
					if (cmd_forward_character(FALSE, 1) == FALSE)
						return FALSE;
					++size;
				}
		}

		/* skip whitespace and newlines */
		while ((curwp->w_doto == line_length(curwp->w_dotp)) ||
		       ((c = lgetc(curwp->w_dotp, curwp->w_doto)) == ' ')
		       || (c == '\t')) {
			if (cmd_forward_character(FALSE, 1) == FALSE)
				break;
			++size;
		}
	}

	/* restore the original position and delete the words */
	curwp->w_dotp = dotp;
	curwp->w_doto = doto;
	return delete_bytes(size, TRUE);
}

/*
 * Kill backwards by "n" words. Move backwards by the desired number of words,
 * counting the characters. When dot is finally moved to its resting place,
 * fire off the kill command. Bound to "M-Rubout" and to "M-Backspace".
 */
int cmd_delete_previous_word(int f, int n)
{
	long size;

	/* don't allow this command if we are in read only mode */
	if (curbp->b_mode & MDVIEW)
		return readonly_error();

	/* ignore the command if there is a nonpositive argument */
	if (n <= 0)
		return FALSE;

	/* Clear the kill buffer if last command wasn't a kill */
	if ((lastflag & CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;			/* this command is a kill */

	if (cmd_backward_character(FALSE, 1) == FALSE)
		return FALSE;
	size = 0;
	while (n--) {
		while (inword() == FALSE) {
			if (cmd_backward_character(FALSE, 1) == FALSE)
				return FALSE;
			++size;
		}
		while (inword() != FALSE) {
			++size;
			if (cmd_backward_character(FALSE, 1) == FALSE)
				goto bckdel;
		}
	}
	if (cmd_forward_character(FALSE, 1) == FALSE)
		return FALSE;
 bckdel:return delete_characters(size, TRUE);
}

/*
 * Return TRUE if the character at dot is a character that is considered to be
 * part of a word. The word character list is hard coded. Should be setable.
 */
int inword(void)
{
	int c;

	if (curwp->w_doto == line_length(curwp->w_dotp))
		return FALSE;
	c = lgetc(curwp->w_dotp, curwp->w_doto);
	if (isletter(c))
		return TRUE;
	if (c >= '0' && c <= '9')
		return TRUE;
	return FALSE;
}

/*
 * Fill the current paragraph according to the current
 * fill column
 *
 * f and n - deFault flag and Numeric argument
 */
int cmd_fill_paragraph(int f, int n)
{
	unicode_t c;				/* current char during scan    */
	unicode_t wbuf[NSTRING];		/* buffer for current word      */
	int wordlen;				/* length of current word       */
	int clength;				/* position on line during fill */
	int i;					/* index during word copy       */
	int newlength;				/* tentative new line length    */
	int eopflag;				/* Are we at the End-Of-Paragraph? */
	int firstflag;				/* first word? (needs no space) */
	struct line *eopline;			/* pointer to line just past EOP */
	int dotflag;				/* was the last char a period?  */

	if (curbp->b_mode & MDVIEW)		/* don't allow this command if      */
		return readonly_error();		/* we are in read only mode     */
	if (fill_column == 0) {			/* no fill column set */
		msg_printf("No fill column set");
		return FALSE;
	}
	justflag = FALSE;

	/* record the pointer to the line just past the EOP */
	cmd_next_paragraph(FALSE, 1);
	eopline = line_next(curwp->w_dotp);

	/* and back top the beginning of the paragraph */
	cmd_previous_paragraph(FALSE, 1);

	/* initialize various info */
	clength = curwp->w_doto;
	if (clength && curwp->w_dotp->l_text[0] == TAB)
		clength = 8;
	wordlen = 0;
	dotflag = FALSE;

	/* scan through lines, filling words */
	firstflag = TRUE;
	eopflag = FALSE;
	while (!eopflag) {
		int bytes = 1;

		/* get the next character in the paragraph */
		if (curwp->w_doto == line_length(curwp->w_dotp)) {
			c = ' ';
			if (line_next(curwp->w_dotp) == eopline)
				eopflag = TRUE;
		} else
			bytes = char_at_dot(&c);

		/* and then delete it */
		delete_bytes(bytes, FALSE);

		/* if not a separator, just add it in */
		if (c != ' ' && c != '\t') {
			dotflag = (c == '.');	/* was it a dot */
			if (wordlen < NSTRING - 1)
				wbuf[wordlen++] = c;
		} else if (wordlen) {
			/* at a word break with a word waiting */
			/* calculate tentitive new length with word added */
			newlength = clength + 1 + wordlen;
			if (newlength <= fill_column) {
				/* add word to current line */
				if (!firstflag) {
					insert_char(1, ' ');	/* the space */
					++clength;
				}
				firstflag = FALSE;
			} else {
				/* start a new line */
				insert_newline();
				clength = 0;
			}

			/* and add the word in in either case */
			for (i = 0; i < wordlen; i++) {
				insert_char(1, wbuf[i]);
				++clength;
			}
			if (dotflag) {
				insert_char(1, ' ');
				++clength;
			}
			wordlen = 0;
		}
	}
	/* and add a last newline for the end of our new paragraph */
	insert_newline();
	return TRUE;
}

/* Fill the current paragraph according to the current
 * fill column and cursor position
 *
 * int f, n;		deFault flag and Numeric argument
 */
int cmd_justify_paragraph(int f, int n)
{
	unicode_t c;				/* current char durring scan    */
	unicode_t wbuf[NSTRING];		/* buffer for current word      */
	int wordlen;				/* length of current word       */
	int clength;				/* position on line during fill */
	int i;					/* index during word copy       */
	int newlength;				/* tentative new line length    */
	int eopflag;				/* Are we at the End-Of-Paragraph? */
	int firstflag;				/* first word? (needs no space) */
	struct line *eopline;			/* pointer to line just past EOP */
	int leftmarg;				/* left marginal */

	if (curbp->b_mode & MDVIEW)		/* don't allow this command if      */
		return readonly_error();		/* we are in read only mode     */
	if (fill_column == 0) {			/* no fill column set */
		msg_printf("No fill column set");
		return FALSE;
	}
	justflag = TRUE;
	leftmarg = curwp->w_doto;
	if (leftmarg + 10 > fill_column) {
		leftmarg = 0;
		msg_printf("Column too narrow");
		return FALSE;
	}

	/* record the pointer to the line just past the EOP */
	cmd_next_paragraph(FALSE, 1);
	eopline = line_next(curwp->w_dotp);

	/* and back top the beginning of the paragraph */
	cmd_previous_paragraph(FALSE, 1);

	/* initialize various info */
	if (leftmarg < line_length(curwp->w_dotp))
		curwp->w_doto = leftmarg;

	clength = curwp->w_doto;
	if (clength && curwp->w_dotp->l_text[0] == TAB)
		clength = 8;

	wordlen = 0;

	/* scan through lines, filling words */
	firstflag = TRUE;
	eopflag = FALSE;
	while (!eopflag) {
		int bytes = 1;

		/* get the next character in the paragraph */
		if (curwp->w_doto == line_length(curwp->w_dotp)) {
			c = ' ';
			if (line_next(curwp->w_dotp) == eopline)
				eopflag = TRUE;
		} else
			bytes = char_at_dot(&c);

		/* and then delete it */
		delete_bytes(bytes, FALSE);

		/* if not a separator, just add it in */
		if (c != ' ' && c != '\t') {
			if (wordlen < NSTRING - 1)
				wbuf[wordlen++] = c;
		} else if (wordlen) {
			/* at a word break with a word waiting */
			/* calculate tentitive new length with word added */
			newlength = clength + 1 + wordlen;
			if (newlength <= fill_column) {
				/* add word to current line */
				if (!firstflag) {
					insert_char(1, ' ');	/* the space */
					++clength;
				}
				firstflag = FALSE;
			} else {
				/* start a new line */
				insert_newline();
				for (i = 0; i < leftmarg; i++)
					insert_char(1, ' ');
				clength = leftmarg;
			}

			/* and add the word in in either case */
			for (i = 0; i < wordlen; i++) {
				insert_char(1, wbuf[i]);
				++clength;
			}
			wordlen = 0;
		}
	}
	/* and add a last newline for the end of our new paragraph */
	insert_newline();

	cmd_next_word(FALSE, 1);
	if (line_length(curwp->w_dotp) > leftmarg)
		curwp->w_doto = leftmarg;
	else
		curwp->w_doto = line_length(curwp->w_dotp);

	justflag = FALSE;
	return TRUE;
}

/*
 * delete n paragraphs starting with the current one
 *
 * int f	default flag
 * int n	# of paras to delete
 */
int cmd_kill_paragraph(int f, int n)
{
	int status;				/* returned status of functions */

	while (n--) {				/* for each paragraph to delete */

		/* mark out the end and beginning of the para to delete */
		cmd_next_paragraph(FALSE, 1);

		/* set the mark here */
		curwp->w_markp = curwp->w_dotp;
		curwp->w_marko = curwp->w_doto;

		/* go to the beginning of the paragraph */
		cmd_previous_paragraph(FALSE, 1);
		curwp->w_doto = 0;		/* force us to the beginning of line */

		/* and delete it */
		if ((status = cmd_kill_region(FALSE, 1)) != TRUE)
			return status;

		/* and clean up the 2 extra lines */
		delete_bytes(2L, TRUE);
	}
	return TRUE;
}

/*
 *	wordcount:	count the # of words in the marked region,
 *			along with average word sizes, # of chars, etc,
 *			and report on them.
 *
 * int f, n;		ignored numeric arguments
 */
int cmd_count_words(int f, int n)
{
	struct line *lp;			/* current line to scan */
	int offset;				/* current char to scan */
	long size;				/* size of region left to count */
	int ch;					/* current character to scan */
	int wordflag;				/* are we in a word now? */
	int lastword;				/* were we just in a word? */
	long nwords;				/* total # of words */
	long nchars;				/* total number of chars */
	int nlines;				/* total number of lines in region */
	int avgch;				/* average number of chars/word */
	int status;				/* status return code */
	struct region region;			/* region to look at */

	/* make sure we have a region to count */
	if ((status = getregion(&region)) != TRUE)
		return status;
	lp = region.r_linep;
	offset = region.r_offset;
	size = region.r_size;

	/* count up things */
	lastword = FALSE;
	nchars = 0L;
	nwords = 0L;
	nlines = 0;
	while (size--) {

		/* get the current character */
		if (offset == line_length(lp)) {	/* end of line */
			ch = '\n';
			lp = line_next(lp);
			offset = 0;
			++nlines;
		} else {
			ch = lgetc(lp, offset);
			++offset;
		}

		/* and tabulate it */
		wordflag = ((isletter(ch)) || (ch >= '0' && ch <= '9'));
		if (wordflag == TRUE && lastword == FALSE)
			++nwords;
		lastword = wordflag;
		++nchars;
	}

	/* and report on the info */
	if (nwords > 0L)
		avgch = (int)((100L * nchars) / nwords);
	else
		avgch = 0;

	msg_printf("Words %ld Chars %ld Lines %d Avg chars/word %d.%02d",
		nwords, nchars, nlines + 1, avgch / 100, avgch % 100);
	return TRUE;
}
