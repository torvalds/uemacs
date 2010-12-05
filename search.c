/*	search.c
 *
 * The functions in this file implement commands that search in the forward
 * and backward directions.  There are no special characters in the search
 * strings.  Probably should have a regular expression search, or something
 * like that.
 *
 * Aug. 1986 John M. Gamble:
 *	Made forward and reverse search use the same scan routine.
 *
 *	Added a limited number of regular expressions - 'any',
 *	'character class', 'closure', 'beginning of line', and
 *	'end of line'.
 *
 *	Replacement metacharacters will have to wait for a re-write of
 *	the replaces function, and a new variation of ldelete().
 *
 *	For those curious as to my references, i made use of
 *	Kernighan & Plauger's "Software Tools."
 *	I deliberately did not look at any published grep or editor
 *	source (aside from this one) for inspiration.  I did make use of
 *	Allen Hollub's bitmap routines as published in Doctor Dobb's Journal,
 *	June, 1985 and modified them for the limited needs of character class
 *	matching.  Any inefficiences, bugs, stupid coding examples, etc.,
 *	are therefore my own responsibility.
 *
 * April 1987: John M. Gamble
 *	Deleted the "if (n == 0) n = 1;" statements in front of the
 *	search/hunt routines.  Since we now use a do loop, these
 *	checks are unnecessary.  Consolidated common code into the
 *	function delins().  Renamed global mclen matchlen,
 *	and added the globals matchline, matchoff, patmatch, and
 *	mlenold.
 *	This gave us the ability to unreplace regular expression searches,
 *	and to put the matched string into an evironment variable.
 *	SOON TO COME: Meta-replacement characters!
 *
 *	25-apr-87	DML
 *	- cleaned up an unneccessary if/else in forwsearch() and
 *	  backsearch()
 *	- savematch() failed to malloc room for the terminating byte
 *	  of the match string (stomp...stomp...). It does now. Also
 *	  it now returns gracefully if malloc fails
 *
 *	July 1987: John M. Gamble
 *	Set the variables matchlen and matchoff in the 'unreplace'
 *	section of replaces().  The function savematch() would
 *	get confused if you replaced, unreplaced, then replaced
 *	again (serves you right for being so wishy-washy...)
 *
 *	August 1987: John M. Gamble
 *	Put in new function rmcstr() to create the replacement
 *	meta-character array.  Modified delins() so that it knows
 *	whether or not to make use of the array.  And, put in the
 *	appropriate new structures and variables.
 *
 *	Modified by Petri Kutvonen
 */

#include <stdio.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

#if defined(MAGIC)
/*
 * The variables magical and rmagical determine if there
 * were actual metacharacters in the search and replace strings -
 * if not, then we don't have to use the slower MAGIC mode
 * search functions.
 */
static short int magical;
static short int rmagical;
static struct magic mcpat[NPAT]; /* The magic pattern. */
static struct magic tapcm[NPAT]; /* The reversed magic patterni. */
static struct magic_replacement rmcpat[NPAT]; /* The replacement magic array. */
#endif

static int amatch(struct magic *mcptr, int direct, struct line **pcwline, int *pcwoff);
static int readpattern(char *prompt, char *apat, int srch);
static int replaces(int kind, int f, int n);
static int nextch(struct line **pcurline, int *pcuroff, int dir);
static int mcstr(void);
static int rmcstr(void);
static int mceq(int bc, struct magic *mt);
static int cclmake(char **ppatptr, struct magic *mcptr);
static int biteq(int bc, char *cclmap);
static char *clearbits(void);
static void setbit(int bc, char *cclmap);

/*
 * forwsearch -- Search forward.  Get a search string from the user, and
 *	search for the string.  If found, reset the "." to be just after
 *	the match string, and (perhaps) repaint the display.
 *
 * int f, n;			default flag / numeric argument
 */
int forwsearch(int f, int n)
{
	int status = TRUE;

	/* If n is negative, search backwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return backsearch(f, -n);

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status = readpattern("Search", &pat[0], TRUE)) == TRUE) {
		do {
#if	MAGIC
			if ((magical
			     && curwp->w_bufp->b_mode & MDMAGIC) != 0)
				status =
				    mcscanner(&mcpat[0], FORWARD, PTEND);
			else
#endif
				status = scanner(&pat[0], FORWARD, PTEND);
		} while ((--n > 0) && status);

		/* Save away the match, or complain
		 * if not there.
		 */
		if (status == TRUE)
			savematch();
		else
			mlwrite("Not found");
	}
	return status;
}

/*
 * forwhunt -- Search forward for a previously acquired search string.
 *	If found, reset the "." to be just after the match string,
 *	and (perhaps) repaint the display.
 *
 * int f, n;		default flag / numeric argument
 */
int forwhunt(int f, int n)
{
	int status = TRUE;

	if (n < 0)		/* search backwards */
		return backhunt(f, -n);

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode until after we entered the pattern.
	 */
	if (pat[0] == '\0') {
		mlwrite("No pattern set");
		return FALSE;
	}
#if	MAGIC
	if ((curwp->w_bufp->b_mode & MDMAGIC) != 0 &&
	    mcpat[0].mc_type == MCNIL) {
		if (!mcstr())
			return FALSE;
	}
#endif

	/* Search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	do {
#if	MAGIC
		if ((magical && curwp->w_bufp->b_mode & MDMAGIC) != 0)
			status = mcscanner(&mcpat[0], FORWARD, PTEND);
		else
#endif
			status = scanner(&pat[0], FORWARD, PTEND);
	} while ((--n > 0) && status);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status == TRUE)
		savematch();
	else
		mlwrite("Not found");

	return status;
}

/*
 * backsearch -- Reverse search.  Get a search string from the user, and
 *	search, starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 *
 * int f, n;		default flag / numeric argument
 */
int backsearch(int f, int n)
{
	int status = TRUE;

	/* If n is negative, search forwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return forwsearch(f, -n);

	/* Ask the user for the text of a pattern.  If the
	 * response is TRUE (responses other than FALSE are
	 * possible), search for the pattern for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	if ((status =
	     readpattern("Reverse search", &pat[0], TRUE)) == TRUE) {
		do {
#if	MAGIC
			if ((magical
			     && curwp->w_bufp->b_mode & MDMAGIC) != 0)
				status =
				    mcscanner(&tapcm[0], REVERSE, PTBEG);
			else
#endif
				status = scanner(&tap[0], REVERSE, PTBEG);
		} while ((--n > 0) && status);

		/* Save away the match, or complain
		 * if not there.
		 */
		if (status == TRUE)
			savematch();
		else
			mlwrite("Not found");
	}
	return status;
}

/*
 * backhunt -- Reverse search for a previously acquired search string,
 *	starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 *
 * int f, n;		default flag / numeric argument
 */
int backhunt(int f, int n)
{
	int status = TRUE;

	if (n < 0)
		return forwhunt(f, -n);

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode until after we entered the pattern.
	 */
	if (tap[0] == '\0') {
		mlwrite("No pattern set");
		return FALSE;
	}
#if	MAGIC
	if ((curwp->w_bufp->b_mode & MDMAGIC) != 0 &&
	    tapcm[0].mc_type == MCNIL) {
		if (!mcstr())
			return FALSE;
	}
#endif

	/* Go search for it for as long as
	 * n is positive (n == 0 will go through once, which
	 * is just fine).
	 */
	do {
#if	MAGIC
		if ((magical && curwp->w_bufp->b_mode & MDMAGIC) != 0)
			status = mcscanner(&tapcm[0], REVERSE, PTBEG);
		else
#endif
			status = scanner(&tap[0], REVERSE, PTBEG);
	} while ((--n > 0) && status);

	/* Save away the match, or complain
	 * if not there.
	 */
	if (status == TRUE)
		savematch();
	else
		mlwrite("Not found");

	return status;
}

#if	MAGIC
/*
 * mcscanner -- Search for a meta-pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 *
 * struct magic *mcpatrn;			pointer into pattern
 * int direct;			which way to go.
 * int beg_or_end;		put point at beginning or end of pattern.
 */
int mcscanner(struct magic *mcpatrn, int direct, int beg_or_end)
{
	struct line *curline;		/* current line during scan */
	int curoff;		/* position within current line */

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/*
	 * Save the old matchlen length, in case it is
	 * very different (closure) from the old length.
	 * This is important for query-replace undo
	 * command.
	 */
	mlenold = matchlen;

	/* Setup local scan pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff = curwp->w_doto;

	/* Scan each character until we hit the head link record.
	 */
	while (!boundry(curline, curoff, direct)) {
		/* Save the current position in case we need to
		 * restore it on a match, and initialize matchlen to
		 * zero in case we are doing a search for replacement.
		 */
		matchline = curline;
		matchoff = curoff;
		matchlen = 0;

		if (amatch(mcpatrn, direct, &curline, &curoff)) {
			/* A SUCCESSFULL MATCH!!!
			 * reset the global "." pointers.
			 */
			if (beg_or_end == PTEND) {	/* at end of string */
				curwp->w_dotp = curline;
				curwp->w_doto = curoff;
			} else {	/* at beginning of string */

				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			curwp->w_flag |= WFMOVE;	/* flag that we have moved */
			return TRUE;
		}

		/* Advance the cursor.
		 */
		nextch(&curline, &curoff, direct);
	}

	return FALSE;		/* We could not find a match. */
}

/*
 * amatch -- Search for a meta-pattern in either direction.  Based on the
 *	recursive routine amatch() (for "anchored match") in
 *	Kernighan & Plauger's "Software Tools".
 *
 * struct magic *mcptr;		string to scan for
 * int direct;		which way to go.
 * struct line **pcwline;	current line during scan
 * int *pcwoff;		position within current line
 */
static int amatch(struct magic *mcptr, int direct, struct line **pcwline, int *pcwoff)
{
	int c;		/* character at current position */
	struct line *curline;		/* current line during scan */
	int curoff;		/* position within current line */
	int nchars;

	/* Set up local scan pointers to ".", and get
	 * the current character.  Then loop around
	 * the pattern pointer until success or failure.
	 */
	curline = *pcwline;
	curoff = *pcwoff;

	/* The beginning-of-line and end-of-line metacharacters
	 * do not compare against characters, they compare
	 * against positions.
	 * BOL is guaranteed to be at the start of the pattern
	 * for forward searches, and at the end of the pattern
	 * for reverse searches.  The reverse is true for EOL.
	 * So, for a start, we check for them on entry.
	 */
	if (mcptr->mc_type == BOL) {
		if (curoff != 0)
			return FALSE;
		mcptr++;
	}

	if (mcptr->mc_type == EOL) {
		if (curoff != llength(curline))
			return FALSE;
		mcptr++;
	}

	while (mcptr->mc_type != MCNIL) {
		c = nextch(&curline, &curoff, direct);

		if (mcptr->mc_type & CLOSURE) {
			/* Try to match as many characters as possible
			 * against the current meta-character.  A
			 * newline never matches a closure.
			 */
			nchars = 0;
			while (c != '\n' && mceq(c, mcptr)) {
				c = nextch(&curline, &curoff, direct);
				nchars++;
			}

			/* We are now at the character that made us
			 * fail.  Try to match the rest of the pattern.
			 * Shrink the closure by one for each failure.
			 * Since closure matches *zero* or more occurences
			 * of a pattern, a match may start even if the
			 * previous loop matched no characters.
			 */
			mcptr++;

			for (;;) {
				c = nextch(&curline, &curoff,
					   direct ^ REVERSE);

				if (amatch
				    (mcptr, direct, &curline, &curoff)) {
					matchlen += nchars;
					goto success;
				}

				if (nchars-- == 0)
					return FALSE;
			}
		} else {	/* Not closure. */

			/* The only way we'd get a BOL metacharacter
			 * at this point is at the end of the reversed pattern.
			 * The only way we'd get an EOL metacharacter
			 * here is at the end of a regular pattern.
			 * So if we match one or the other, and are at
			 * the appropriate position, we are guaranteed success
			 * (since the next pattern character has to be MCNIL).
			 * Before we report success, however, we back up by
			 * one character, so as to leave the cursor in the
			 * correct position.  For example, a search for ")$"
			 * will leave the cursor at the end of the line, while
			 * a search for ")<NL>" will leave the cursor at the
			 * beginning of the next line.  This follows the
			 * notion that the meta-character '$' (and likewise
			 * '^') match positions, not characters.
			 */
			if (mcptr->mc_type == BOL) {
				if (curoff == llength(curline)) {
					c = nextch(&curline, &curoff,
						   direct ^ REVERSE);
					goto success;
				} else
					return FALSE;
			}

			if (mcptr->mc_type == EOL) {
				if (curoff == 0) {
					c = nextch(&curline, &curoff,
						   direct ^ REVERSE);
					goto success;
				} else
					return FALSE;
			}

			/* Neither BOL nor EOL, so go through
			 * the meta-character equal function.
			 */
			if (!mceq(c, mcptr))
				return FALSE;
		}

		/* Increment the length counter and
		 * advance the pattern pointer.
		 */
		matchlen++;
		mcptr++;
	}			/* End of mcptr loop. */

	/* A SUCCESSFULL MATCH!!!
	 * Reset the "." pointers.
	 */
      success:
	*pcwline = curline;
	*pcwoff = curoff;

	return TRUE;
}
#endif

/*
 * scanner -- Search for a pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 *
 * unsigned char *patrn;	string to scan for
 * int direct;			which way to go.
 * int beg_or_end;		put point at beginning or end of pattern.
 */
int scanner(const char *patrn, int direct, int beg_or_end)
{
	int c;		/* character at current position */
	const char *patptr;	/* pointer into pattern */
	struct line *curline;		/* current line during scan */
	int curoff;		/* position within current line */
	struct line *scanline;		/* current line during scanning */
	int scanoff;		/* position in scanned line */

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/* Set up local pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff = curwp->w_doto;

	/* Scan each character until we hit the head link record.
	 */
	while (!boundry(curline, curoff, direct)) {
		/* Save the current position in case we match
		 * the search string at this point.
		 */
		matchline = curline;
		matchoff = curoff;

		/* Get the character resolving newlines, and
		 * test it against first char in pattern.
		 */
		c = nextch(&curline, &curoff, direct);

		if (eq(c, patrn[0])) {	/* if we find it.. */
			/* Setup scanning pointers.
			 */
			scanline = curline;
			scanoff = curoff;
			patptr = &patrn[0];

			/* Scan through the pattern for a match.
			 */
			while (*++patptr != '\0') {
				c = nextch(&scanline, &scanoff, direct);

				if (!eq(c, *patptr))
					goto fail;
			}

			/* A SUCCESSFULL MATCH!!!
			 * reset the global "." pointers
			 */
			if (beg_or_end == PTEND) {	/* at end of string */
				curwp->w_dotp = scanline;
				curwp->w_doto = scanoff;
			} else {	/* at beginning of string */

				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			curwp->w_flag |= WFMOVE;	/* Flag that we have moved. */
			return TRUE;

		}
	      fail:;		/* continue to search */
	}

	return FALSE;		/* We could not find a match */
}

/*
 * eq -- Compare two characters.  The "bc" comes from the buffer, "pc"
 *	from the pattern.  If we are not in EXACT mode, fold out the case.
 */
int eq(unsigned char bc, unsigned char pc)
{
	if ((curwp->w_bufp->b_mode & MDEXACT) == 0) {
		if (islower(bc))
			bc ^= DIFCASE;

		if (islower(pc))
			pc ^= DIFCASE;
	}

	return bc == pc;
}

/*
 * readpattern -- Read a pattern.  Stash it in apat.  If it is the
 *	search string, create the reverse pattern and the magic
 *	pattern, assuming we are in MAGIC mode (and defined that way).
 *	Apat is not updated if the user types in an empty line.  If
 *	the user typed an empty line, and there is no old pattern, it is
 *	an error.  Display the old pattern, in the style of Jeff Lomicka.
 *	There is some do-it-yourself control expansion.  Change to using
 *	<META> to delimit the end-of-pattern to allow <NL>s in the search
 *	string. 
 */
static int readpattern(char *prompt, char *apat, int srch)
{
	int status;
	char tpat[NPAT + 20];

	strcpy(tpat, prompt);	/* copy prompt to output string */
	strcat(tpat, " (");	/* build new prompt string */
	expandp(&apat[0], &tpat[strlen(tpat)], NPAT / 2);	/* add old pattern */
	strcat(tpat, ")<Meta>: ");

	/* Read a pattern.  Either we get one,
	 * or we just get the META charater, and use the previous pattern.
	 * Then, if it's the search string, make a reversed pattern.
	 * *Then*, make the meta-pattern, if we are defined that way.
	 */
	if ((status = mlreplyt(tpat, tpat, NPAT, metac)) == TRUE) {
		strcpy(apat, tpat);
		if (srch) {	/* If we are doing the search string. */
			/* Reverse string copy, and remember
			 * the length for substitution purposes.
			 */
			rvstrcpy(tap, apat);
			mlenold = matchlen = strlen(apat);
		}
#if	MAGIC
		/* Only make the meta-pattern if in magic mode,
		 * since the pattern in question might have an
		 * invalid meta combination.
		 */
		if ((curwp->w_bufp->b_mode & MDMAGIC) == 0) {
			mcclear();
			rmcclear();
		} else
			status = srch ? mcstr() : rmcstr();
#endif
	} else if (status == FALSE && apat[0] != 0)	/* Old one */
		status = TRUE;

	return status;
}

/*
 * savematch -- We found the pattern?  Let's save it away.
 */
void savematch(void)
{
	char *ptr;	/* pointer to last match string */
	int j;
	struct line *curline;		/* line of last match */
	int curoff;		/* offset "      "    */

	/* Free any existing match string, then
	 * attempt to allocate a new one.
	 */
	if (patmatch != NULL)
		free(patmatch);

	ptr = patmatch = malloc(matchlen + 1);

	if (ptr != NULL) {
		curoff = matchoff;
		curline = matchline;

		for (j = 0; j < matchlen; j++)
			*ptr++ = nextch(&curline, &curoff, FORWARD);

		*ptr = '\0';
	}
}

/*
 * rvstrcpy -- Reverse string copy.
 */
void rvstrcpy(char *rvstr, char *str)
{
	int i;

	str += (i = strlen(str));

	while (i-- > 0)
		*rvstr++ = *--str;

	*rvstr = '\0';
}

/*
 * sreplace -- Search and replace.
 *
 * int f;		default flag
 * int n;		# of repetitions wanted
 */
int sreplace(int f, int n)
{
	return replaces(FALSE, f, n);
}

/*
 * qreplace -- search and replace with query.
 *
 * int f;		default flag
 * int n;		# of repetitions wanted
 */
int qreplace(int f, int n)
{
	return replaces(TRUE, f, n);
}

/*
 * replaces -- Search for a string and replace it with another
 *	string.  Query might be enabled (according to kind).
 *
 * int kind;		Query enabled flag
 * int f;		default flag
 * int n;		# of repetitions wanted
 */
static int replaces(int kind, int f, int n)
{
	int status;	/* success flag on pattern inputs */
	int rlength;	/* length of replacement string */
	int numsub;	/* number of substitutions */
	int nummatch;	/* number of found matches */
	int nlflag;		/* last char of search string a <NL>? */
	int nlrepl;		/* was a replace done on the last line? */
	char c;			/* input char for query */
	char tpat[NPAT];	/* temporary to hold search pattern */
	struct line *origline;		/* original "." position */
	int origoff;		/* and offset (for . query option) */
	struct line *lastline;		/* position of last replace and */
	int lastoff;		/* offset (for 'u' query option) */

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if      */
		return rdonly();	/* we are in read only mode     */

	/* Check for negative repetitions.
	 */
	if (f && n < 0)
		return FALSE;

	/* Ask the user for the text of a pattern.
	 */
	if ((status = readpattern((kind ==
				   FALSE ? "Replace" : "Query replace"),
				  &pat[0], TRUE))
	    != TRUE)
		return status;

	/* Ask for the replacement string.
	 */
	if ((status = readpattern("with", &rpat[0], FALSE)) == ABORT)
		return status;

	/* Find the length of the replacement string.
	 */
	rlength = strlen(&rpat[0]);

	/* Set up flags so we can make sure not to do a recursive
	 * replace on the last line.
	 */
	nlflag = (pat[matchlen - 1] == '\n');
	nlrepl = FALSE;

	if (kind) {
		/* Build query replace question string.
		 */
		strcpy(tpat, "Replace '");
		expandp(&pat[0], &tpat[strlen(tpat)], NPAT / 3);
		strcat(tpat, "' with '");
		expandp(&rpat[0], &tpat[strlen(tpat)], NPAT / 3);
		strcat(tpat, "'? ");

		/* Initialize last replaced pointers.
		 */
		lastline = NULL;
		lastoff = 0;
	}

	/* Save original . position, init the number of matches and
	 * substitutions, and scan through the file.
	 */
	origline = curwp->w_dotp;
	origoff = curwp->w_doto;
	numsub = 0;
	nummatch = 0;

	while ((f == FALSE || n > nummatch) &&
	       (nlflag == FALSE || nlrepl == FALSE)) {
		/* Search for the pattern.
		 * If we search with a regular expression,
		 * matchlen is reset to the true length of
		 * the matched string.
		 */
#if	MAGIC
		if ((magical && curwp->w_bufp->b_mode & MDMAGIC) != 0) {
			if (!mcscanner(&mcpat[0], FORWARD, PTBEG))
				break;
		} else
#endif
		if (!scanner(&pat[0], FORWARD, PTBEG))
			break;	/* all done */

		++nummatch;	/* Increment # of matches */

		/* Check if we are on the last line.
		 */
		nlrepl = (lforw(curwp->w_dotp) == curwp->w_bufp->b_linep);

		/* Check for query.
		 */
		if (kind) {
			/* Get the query.
			 */
		      pprompt:mlwrite(&tpat[0], &pat[0],
				&rpat[0]);
		      qprompt:
			update(TRUE);	/* show the proposed place to change */
			c = tgetc();	/* and input */
			mlwrite("");	/* and clear it */

			/* And respond appropriately.
			 */
			switch (c) {
#if	PKCODE
			case 'Y':
#endif
			case 'y':	/* yes, substitute */
			case ' ':
				savematch();
				break;

#if	PKCODE
			case 'N':
#endif
			case 'n':	/* no, onword */
				forwchar(FALSE, 1);
				continue;

			case '!':	/* yes/stop asking */
				kind = FALSE;
				break;

#if	PKCODE
			case 'U':
#endif
			case 'u':	/* undo last and re-prompt */

				/* Restore old position.
				 */
				if (lastline == NULL) {
					/* There is nothing to undo.
					 */
					TTbeep();
					goto pprompt;
				}
				curwp->w_dotp = lastline;
				curwp->w_doto = lastoff;
				lastline = NULL;
				lastoff = 0;

				/* Delete the new string.
				 */
				backchar(FALSE, rlength);
#if	PKCODE
				matchline = curwp->w_dotp;
				matchoff = curwp->w_doto;
#endif
				status = delins(rlength, patmatch, FALSE);
				if (status != TRUE)
					return status;

				/* Record one less substitution,
				 * backup, save our place, and
				 * reprompt.
				 */
				--numsub;
				backchar(FALSE, mlenold);
				matchline = curwp->w_dotp;
				matchoff = curwp->w_doto;
				goto pprompt;

			case '.':	/* abort! and return */
				/* restore old position */
				curwp->w_dotp = origline;
				curwp->w_doto = origoff;
				curwp->w_flag |= WFMOVE;

			case BELL:	/* abort! and stay */
				mlwrite("Aborted!");
				return FALSE;

			default:	/* bitch and beep */
				TTbeep();

			case '?':	/* help me */
				mlwrite
				    ("(Y)es, (N)o, (!)Do rest, (U)ndo last, (^G)Abort, (.)Abort back, (?)Help: ");
				goto qprompt;

			}	/* end of switch */
		}

		/* end of "if kind" */
		/*
		 * Delete the sucker, and insert its
		 * replacement.
		 */
		status = delins(matchlen, &rpat[0], TRUE);
		if (status != TRUE)
			return status;

		/* Save our position, since we may
		 * undo this.
		 */
		if (kind) {
			lastline = curwp->w_dotp;
			lastoff = curwp->w_doto;
		}

		numsub++;	/* increment # of substitutions */
	}

	/* And report the results.
	 */
	mlwrite("%d substitutions", numsub);
	return TRUE;
}

/*
 * delins -- Delete a specified length from the current point
 *	then either insert the string directly, or make use of
 *	replacement meta-array.
 */
int delins(int dlength, char *instr, int use_meta)
{
	int status;
#if	MAGIC
	struct magic_replacement *rmcptr;
#endif

	/* Zap what we gotta,
	 * and insert its replacement.
	 */
	if ((status = ldelete((long) dlength, FALSE)) != TRUE)
		mlwrite("%%ERROR while deleting");
	else
#if	MAGIC
	if ((rmagical && use_meta) &&
		    (curwp->w_bufp->b_mode & MDMAGIC) != 0) {
		rmcptr = &rmcpat[0];
		while (rmcptr->mc_type != MCNIL && status == TRUE) {
			if (rmcptr->mc_type == LITCHAR)
				status = linstr(rmcptr->rstr);
			else
				status = linstr(patmatch);
			rmcptr++;
		}
	} else
#endif
		status = linstr(instr);

	return status;
}

/*
 * expandp -- Expand control key sequences for output.
 *
 * char *srcstr;		string to expand
 * char *deststr;		destination of expanded string
 * int maxlength;		maximum chars in destination
 */
int expandp(char *srcstr, char *deststr, int maxlength)
{
	unsigned char c;	/* current char to translate */

	/* Scan through the string.
	 */
	while ((c = *srcstr++) != 0) {
		if (c == '\n') {	/* it's a newline */
			*deststr++ = '<';
			*deststr++ = 'N';
			*deststr++ = 'L';
			*deststr++ = '>';
			maxlength -= 4;
		}
#if	PKCODE
		else if ((c > 0 && c < 0x20) || c == 0x7f)	/* control character */
#else
		else if (c < 0x20 || c == 0x7f)	/* control character */
#endif
		{
			*deststr++ = '^';
			*deststr++ = c ^ 0x40;
			maxlength -= 2;
		} else if (c == '%') {
			*deststr++ = '%';
			*deststr++ = '%';
			maxlength -= 2;
		} else {	/* any other character */

			*deststr++ = c;
			maxlength--;
		}

		/* check for maxlength */
		if (maxlength < 4) {
			*deststr++ = '$';
			*deststr = '\0';
			return FALSE;
		}
	}
	*deststr = '\0';
	return TRUE;
}

/*
 * boundry -- Return information depending on whether we may search no
 *	further.  Beginning of file and end of file are the obvious
 *	cases, but we may want to add further optional boundry restrictions
 *	in future, a' la VMS EDT.  At the moment, just return TRUE or
 *	FALSE depending on if a boundry is hit (ouch).
 */
int boundry(struct line *curline, int curoff, int dir)
{
	int border;

	if (dir == FORWARD) {
		border = (curoff == llength(curline)) &&
		    (lforw(curline) == curbp->b_linep);
	} else {
		border = (curoff == 0) &&
		    (lback(curline) == curbp->b_linep);
	}
	return border;
}

/*
 * nextch -- retrieve the next/previous character in the buffer,
 *	and advance/retreat the point.
 *	The order in which this is done is significant, and depends
 *	upon the direction of the search.  Forward searches look at
 *	the current character and move, reverse searches move and
 *	look at the character.
 */
static int nextch(struct line **pcurline, int *pcuroff, int dir)
{
	struct line *curline;
	int curoff;
	int c;

	curline = *pcurline;
	curoff = *pcuroff;

	if (dir == FORWARD) {
		if (curoff == llength(curline)) {	/* if at EOL */
			curline = lforw(curline);	/* skip to next line */
			curoff = 0;
			c = '\n';	/* and return a <NL> */
		} else
			c = lgetc(curline, curoff++);	/* get the char */
	} else {		/* Reverse. */

		if (curoff == 0) {
			curline = lback(curline);
			curoff = llength(curline);
			c = '\n';
		} else
			c = lgetc(curline, --curoff);

	}
	*pcurline = curline;
	*pcuroff = curoff;

	return c;
}

#if	MAGIC
/*
 * mcstr -- Set up the 'magic' array.  The closure symbol is taken as
 *	a literal character when (1) it is the first character in the
 *	pattern, and (2) when preceded by a symbol that does not allow
 *	closure, such as a newline, beginning of line symbol, or another
 *	closure symbol.
 *
 *	Coding comment (jmg):  yes, i know i have gotos that are, strictly
 *	speaking, unnecessary.  But right now we are so cramped for
 *	code space that i will grab what i can in order to remain
 *	within the 64K limit.  C compilers actually do very little
 *	in the way of optimizing - they expect you to do that.
 */
static int mcstr(void)
{
	struct magic *mcptr, *rtpcm;
	char *patptr;
	int mj;
	int pchr;
	int status = TRUE;
	int does_closure = FALSE;

	/* If we had metacharacters in the struct magic array previously,
	 * free up any bitmaps that may have been allocated.
	 */
	if (magical)
		mcclear();

	magical = FALSE;
	mj = 0;
	mcptr = &mcpat[0];
	patptr = &pat[0];

	while ((pchr = *patptr) && status) {
		switch (pchr) {
		case MC_CCL:
			status = cclmake(&patptr, mcptr);
			magical = TRUE;
			does_closure = TRUE;
			break;
		case MC_BOL:
			if (mj != 0)
				goto litcase;

			mcptr->mc_type = BOL;
			magical = TRUE;
			does_closure = FALSE;
			break;
		case MC_EOL:
			if (*(patptr + 1) != '\0')
				goto litcase;

			mcptr->mc_type = EOL;
			magical = TRUE;
			does_closure = FALSE;
			break;
		case MC_ANY:
			mcptr->mc_type = ANY;
			magical = TRUE;
			does_closure = TRUE;
			break;
		case MC_CLOSURE:
			/* Does the closure symbol mean closure here?
			 * If so, back up to the previous element
			 * and indicate it is enclosed.
			 */
			if (!does_closure)
				goto litcase;
			mj--;
			mcptr--;
			mcptr->mc_type |= CLOSURE;
			magical = TRUE;
			does_closure = FALSE;
			break;

			/* Note: no break between MC_ESC case and the default.
			 */
		case MC_ESC:
			if (*(patptr + 1) != '\0') {
				pchr = *++patptr;
				magical = TRUE;
			}
		default:
		      litcase:mcptr->mc_type =
			    LITCHAR;
			mcptr->u.lchar = pchr;
			does_closure = (pchr != '\n');
			break;
		}		/* End of switch. */
		mcptr++;
		patptr++;
		mj++;
	}			/* End of while. */

	/* Close off the meta-string.
	 */
	mcptr->mc_type = MCNIL;

	/* Set up the reverse array, if the status is good.  Please note the
	 * structure assignment - your compiler may not like that.
	 * If the status is not good, nil out the meta-pattern.
	 * The only way the status would be bad is from the cclmake()
	 * routine, and the bitmap for that member is guarenteed to be
	 * freed.  So we stomp a MCNIL value there, and call mcclear()
	 * to free any other bitmaps.
	 */
	if (status) {
		rtpcm = &tapcm[0];
		while (--mj >= 0) {
#if	MSC | TURBO | VMS | USG | BSD | V7
			*rtpcm++ = *--mcptr;
#endif
		}
		rtpcm->mc_type = MCNIL;
	} else {
		(--mcptr)->mc_type = MCNIL;
		mcclear();
	}

	return status;
}

/*
 * rmcstr -- Set up the replacement 'magic' array.  Note that if there
 *	are no meta-characters encountered in the replacement string,
 *	the array is never actually created - we will just use the
 *	character array rpat[] as the replacement string.
 */
static int rmcstr(void)
{
	struct magic_replacement *rmcptr;
	char *patptr;
	int status = TRUE;
	int mj;

	patptr = &rpat[0];
	rmcptr = &rmcpat[0];
	mj = 0;
	rmagical = FALSE;

	while (*patptr && status == TRUE) {
		switch (*patptr) {
		case MC_DITTO:

			/* If there were non-magical characters
			 * in the string before reaching this
			 * character, plunk it in the replacement
			 * array before processing the current
			 * meta-character.
			 */
			if (mj != 0) {
				rmcptr->mc_type = LITCHAR;
				if ((rmcptr->rstr =
				     malloc(mj + 1)) == NULL) {
					mlwrite("%%Out of memory");
					status = FALSE;
					break;
				}
				strncpy(rmcptr->rstr, patptr - mj, mj);
				rmcptr++;
				mj = 0;
			}
			rmcptr->mc_type = DITTO;
			rmcptr++;
			rmagical = TRUE;
			break;

		case MC_ESC:
			rmcptr->mc_type = LITCHAR;

			/* We malloc mj plus two here, instead
			 * of one, because we have to count the
			 * current character.
			 */
			if ((rmcptr->rstr = malloc(mj + 2)) == NULL) {
				mlwrite("%%Out of memory");
				status = FALSE;
				break;
			}

			strncpy(rmcptr->rstr, patptr - mj, mj + 1);

			/* If MC_ESC is not the last character
			 * in the string, find out what it is
			 * escaping, and overwrite the last
			 * character with it.
			 */
			if (*(patptr + 1) != '\0')
				*((rmcptr->rstr) + mj) = *++patptr;

			rmcptr++;
			mj = 0;
			rmagical = TRUE;
			break;

		default:
			mj++;
		}
		patptr++;
	}

	if (rmagical && mj > 0) {
		rmcptr->mc_type = LITCHAR;
		if ((rmcptr->rstr = malloc(mj + 1)) == NULL) {
			mlwrite("%%Out of memory.");
			status = FALSE;
		}
		strncpy(rmcptr->rstr, patptr - mj, mj);
		rmcptr++;
	}

	rmcptr->mc_type = MCNIL;
	return status;
}

/*
 * mcclear -- Free up any CCL bitmaps, and MCNIL the struct magic search arrays.
 */
void mcclear(void)
{
	struct magic *mcptr;

	mcptr = &mcpat[0];

	while (mcptr->mc_type != MCNIL) {
		if ((mcptr->mc_type & MASKCL) == CCL ||
		    (mcptr->mc_type & MASKCL) == NCCL)
			free(mcptr->u.cclmap);
		mcptr++;
	}
	mcpat[0].mc_type = tapcm[0].mc_type = MCNIL;
}

/*
 * rmcclear -- Free up any strings, and MCNIL the struct magic_replacement array.
 */
void rmcclear(void)
{
	struct magic_replacement *rmcptr;

	rmcptr = &rmcpat[0];

	while (rmcptr->mc_type != MCNIL) {
		if (rmcptr->mc_type == LITCHAR)
			free(rmcptr->rstr);
		rmcptr++;
	}

	rmcpat[0].mc_type = MCNIL;
}

/*
 * mceq -- meta-character equality with a character.  In Kernighan & Plauger's
 *	Software Tools, this is the function omatch(), but i felt there
 *	were too many functions with the 'match' name already.
 */
static int mceq(int bc, struct magic *mt)
{
	int result;

#if	PKCODE
	bc = bc & 0xFF;
#endif
	switch (mt->mc_type & MASKCL) {
	case LITCHAR:
		result = eq(bc, mt->u.lchar);
		break;

	case ANY:
		result = (bc != '\n');
		break;

	case CCL:
		if (!(result = biteq(bc, mt->u.cclmap))) {
			if ((curwp->w_bufp->b_mode & MDEXACT) == 0 &&
			    (isletter(bc))) {
				result = biteq(CHCASE(bc), mt->u.cclmap);
			}
		}
		break;

	case NCCL:
		result = !biteq(bc, mt->u.cclmap);

		if ((curwp->w_bufp->b_mode & MDEXACT) == 0 &&
		    (isletter(bc))) {
			result &= !biteq(CHCASE(bc), mt->u.cclmap);
		}
		break;

	default:
		mlwrite("mceq: what is %d?", mt->mc_type);
		result = FALSE;
		break;

	}			/* End of switch. */

	return result;
}

extern char *clearbits(void);

/*
 * cclmake -- create the bitmap for the character class.
 *	ppatptr is left pointing to the end-of-character-class character,
 *	so that a loop may automatically increment with safety.
 */
static int cclmake(char **ppatptr, struct magic *mcptr)
{
	char *bmap;
	char *patptr;
	int pchr, ochr;

	if ((bmap = clearbits()) == NULL) {
		mlwrite("%%Out of memory");
		return FALSE;
	}

	mcptr->u.cclmap = bmap;
	patptr = *ppatptr;

	/*
	 * Test the initial character(s) in ccl for
	 * special cases - negate ccl, or an end ccl
	 * character as a first character.  Anything
	 * else gets set in the bitmap.
	 */
	if (*++patptr == MC_NCCL) {
		patptr++;
		mcptr->mc_type = NCCL;
	} else
		mcptr->mc_type = CCL;

	if ((ochr = *patptr) == MC_ECCL) {
		mlwrite("%%No characters in character class");
		return FALSE;
	} else {
		if (ochr == MC_ESC)
			ochr = *++patptr;

		setbit(ochr, bmap);
		patptr++;
	}

	while (ochr != '\0' && (pchr = *patptr) != MC_ECCL) {
		switch (pchr) {
			/* Range character loses its meaning
			 * if it is the last character in
			 * the class.
			 */
		case MC_RCCL:
			if (*(patptr + 1) == MC_ECCL)
				setbit(pchr, bmap);
			else {
				pchr = *++patptr;
				while (++ochr <= pchr)
					setbit(ochr, bmap);
			}
			break;

			/* Note: no break between case MC_ESC and the default.
			 */
		case MC_ESC:
			pchr = *++patptr;
		default:
			setbit(pchr, bmap);
			break;
		}
		patptr++;
		ochr = pchr;
	}

	*ppatptr = patptr;

	if (ochr == '\0') {
		mlwrite("%%Character class not ended");
		free(bmap);
		return FALSE;
	}
	return TRUE;
}

/*
 * biteq -- is the character in the bitmap?
 */
static int biteq(int bc, char *cclmap)
{
#if	PKCODE
	bc = bc & 0xFF;
#endif
	if (bc >= HICHAR)
		return FALSE;

	return (*(cclmap + (bc >> 3)) & BIT(bc & 7)) ? TRUE : FALSE;
}

/*
 * clearbits -- Allocate and zero out a CCL bitmap.
 */
static char *clearbits(void)
{
	char *cclstart;
        char *cclmap;
	int i;

	if ((cclmap = cclstart = (char *)malloc(HIBYTE)) != NULL) {
		for (i = 0; i < HIBYTE; i++)
			*cclmap++ = 0;
	}
	return cclstart;
}

/*
 * setbit -- Set a bit (ON only) in the bitmap.
 */
static void setbit(int bc, char *cclmap)
{
#if	PKCODE
	bc = bc & 0xFF;
#endif
	if (bc < HICHAR)
		*(cclmap + (bc >> 3)) |= BIT(bc & 7);
}
#endif
