/*	input.c
 *
 *	Various input routines
 *
 *	written by Daniel Lawrence 5/9/86
 *	modified by Petri Kutvonen
 */

#include <stdio.h>
#include <unistd.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "wrapper.h"

#if	PKCODE
#if     MSDOS && TURBO
#include	<dir.h>
#endif
#endif

#if	PKCODE && (UNIX || (MSDOS && TURBO))
#define	COMPLC	1
#else
#define COMPLC	0
#endif

/*
 * Ask a yes or no question in the message line. Return either TRUE, FALSE, or
 * ABORT. The ABORT status is returned if the user bumps out of the question
 * with a ^G. Used any time a confirmation is required.
 */
int mlyesno(char *prompt)
{
	char c;			/* input character */
	char buf[NPAT];		/* prompt to user */

	for (;;) {
		/* build and prompt the user */
		strcpy(buf, prompt);
		strcat(buf, " (y/n)? ");
		mlwrite(buf);

		/* get the responce */
		c = tgetc();

		if (c == ectoc(abortc))	/* Bail out! */
			return ABORT;

		if (c == 'y' || c == 'Y')
			return TRUE;

		if (c == 'n' || c == 'N')
			return FALSE;
	}
}

/*
 * Write a prompt into the message line, then read back a response. Keep
 * track of the physical position of the cursor. If we are in a keyboard
 * macro throw the prompt away, and return the remembered response. This
 * lets macros run at full speed. The reply is always terminated by a carriage
 * return. Handle erase, kill, and abort keys.
 */

int mlreply(char *prompt, char *buf, int nbuf)
{
	return nextarg(prompt, buf, nbuf, ctoec('\n'));
}

int mlreplyt(char *prompt, char *buf, int nbuf, int eolchar)
{
	return nextarg(prompt, buf, nbuf, eolchar);
}

/*
 * ectoc:
 *	expanded character to character
 *	collapse the CONTROL and SPEC flags back into an ascii code
 */
int ectoc(int c)
{
	if (c & CONTROL)
		c = c & ~(CONTROL | 0x40);
	if (c & SPEC)
		c = c & 255;
	return c;
}

/*
 * ctoec:
 *	character to extended character
 *	pull out the CONTROL and SPEC prefixes (if possible)
 */
int ctoec(int c)
{
	if (c >= 0x00 && c <= 0x1F)
		c = CONTROL | (c + '@');
	return c;
}

/*
 * get a command name from the command line. Command completion means
 * that pressing a <SPACE> will attempt to complete an unfinished command
 * name if it is unique.
 */
fn_t getname(void)
{
	int cpos;	/* current column on screen output */
	int c;
	char *sp;	/* pointer to string for output */
	struct name_bind *ffp;	/* first ptr to entry in name binding table */
	struct name_bind *cffp;	/* current ptr to entry in name binding table */
	struct name_bind *lffp;	/* last ptr to entry in name binding table */
	char buf[NSTRING];	/* buffer to hold tentative command name */

	/* starting at the beginning of the string buffer */
	cpos = 0;

	/* if we are executing a command line get the next arg and match it */
	if (clexec) {
		if (macarg(buf) != TRUE)
			return NULL;
		return fncmatch(&buf[0]);
	}

	/* build a name string from the keyboard */
	while (TRUE) {
		c = tgetc();

		/* if we are at the end, just match it */
		if (c == 0x0d) {
			buf[cpos] = 0;

			/* and match it off */
			return fncmatch(&buf[0]);

		} else if (c == ectoc(abortc)) {	/* Bell, abort */
			ctrlg(FALSE, 0);
			TTflush();
			return NULL;

		} else if (c == 0x7F || c == 0x08) {	/* rubout/erase */
			if (cpos != 0) {
				TTputc('\b');
				TTputc(' ');
				TTputc('\b');
				--ttcol;
				--cpos;
				TTflush();
			}

		} else if (c == 0x15) {	/* C-U, kill */
			while (cpos != 0) {
				TTputc('\b');
				TTputc(' ');
				TTputc('\b');
				--cpos;
				--ttcol;
			}

			TTflush();

		} else if (c == ' ' || c == 0x1b || c == 0x09) {
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
			/* attempt a completion */
			buf[cpos] = 0;	/* terminate it for us */
			ffp = &names[0];	/* scan for matches */
			while (ffp->n_func != NULL) {
				if (strncmp(buf, ffp->n_name, strlen(buf))
				    == 0) {
					/* a possible match! More than one? */
					if ((ffp + 1)->n_func == NULL ||
					    (strncmp
					     (buf, (ffp + 1)->n_name,
					      strlen(buf)) != 0)) {
						/* no...we match, print it */
						sp = ffp->n_name + cpos;
						while (*sp)
							TTputc(*sp++);
						TTflush();
						return ffp->n_func;
					} else {
/* << << << << << << << << << << << << << << << << << */
						/* try for a partial match against the list */

						/* first scan down until we no longer match the current input */
						lffp = (ffp + 1);
						while ((lffp +
							1)->n_func !=
						       NULL) {
							if (strncmp
							    (buf,
							     (lffp +
							      1)->n_name,
							     strlen(buf))
							    != 0)
								break;
							++lffp;
						}

						/* and now, attempt to partial complete the string, char at a time */
						while (TRUE) {
							/* add the next char in */
							buf[cpos] =
							    ffp->
							    n_name[cpos];

							/* scan through the candidates */
							cffp = ffp + 1;
							while (cffp <=
							       lffp) {
								if (cffp->
								    n_name
								    [cpos]
								    !=
								    buf
								    [cpos])
									goto onward;
								++cffp;
							}

							/* add the character */
							TTputc(buf
							       [cpos++]);
						}
/* << << << << << << << << << << << << << << << << << */
					}
				}
				++ffp;
			}

			/* no match.....beep and onward */
			TTbeep();
		      onward:;
			TTflush();
/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
		} else {
			if (cpos < NSTRING - 1 && c > ' ') {
				buf[cpos++] = c;
				TTputc(c);
			}

			++ttcol;
			TTflush();
		}
	}
}

/*	tgetc:	Get a key from the terminal driver, resolve any keyboard
		macro action					*/

int tgetc(void)
{
	int c;			/* fetched character */

	/* if we are playing a keyboard macro back, */
	if (kbdmode == PLAY) {

		/* if there is some left... */
		if (kbdptr < kbdend)
			return (int) *kbdptr++;

		/* at the end of last repitition? */
		if (--kbdrep < 1) {
			kbdmode = STOP;
#if	VISMAC == 0
			/* force a screen update after all is done */
			update(FALSE);
#endif
		} else {

			/* reset the macro to the begining for the next rep */
			kbdptr = &kbdm[0];
			return (int) *kbdptr++;
		}
	}

	/* fetch a character from the terminal driver */
	c = TTgetc();

	/* record it for $lastkey */
	lastkey = c;

	/* save it if we need to */
	if (kbdmode == RECORD) {
		*kbdptr++ = c;
		kbdend = kbdptr;

		/* don't overrun the buffer */
		if (kbdptr == &kbdm[NKBDM - 1]) {
			kbdmode = STOP;
			TTbeep();
		}
	}

	/* and finally give the char back */
	return c;
}

/*	GET1KEY:	Get one keystroke. The only prefixs legal here
			are the SPEC and CONTROL prefixes.
								*/

int get1key(void)
{
	int c;

	/* get a keystroke */
	c = tgetc();

#if	MSDOS
	if (c == 0) {		/* Apply SPEC prefix    */
		c = tgetc();
		if (c >= 0x00 && c <= 0x1F)	/* control key? */
			c = CONTROL | (c + '@');
		return SPEC | c;
	}
#endif

	if (c >= 0x00 && c <= 0x1F)	/* C0 control -> C-     */
		c = CONTROL | (c + '@');
	return c;
}

/*	GETCMD:	Get a command from the keyboard. Process all applicable
		prefix keys
							*/
int getcmd(void)
{
	int c;			/* fetched keystroke */
#if VT220
	int d;			/* second character P.K. */
	int cmask = 0;
#endif
	/* get initial character */
	c = get1key();

#if VT220
      proc_metac:
#endif
	if (c == 128+27)		/* CSI */
		goto handle_CSI;
	/* process META prefix */
	if (c == (CONTROL | '[')) {
		c = get1key();
#if VT220
		if (c == '[' || c == 'O') {	/* CSI P.K. */
handle_CSI:
			c = get1key();
			if (c >= 'A' && c <= 'D')
				return SPEC | c | cmask;
			if (c >= 'E' && c <= 'z' && c != 'i' && c != 'c')
				return SPEC | c | cmask;
			d = get1key();
			if (d == '~')	/* ESC [ n ~   P.K. */
				return SPEC | c | cmask;
			switch (c) {	/* ESC [ n n ~ P.K. */
			case '1':
				c = d + 32;
				break;
			case '2':
				c = d + 48;
				break;
			case '3':
				c = d + 64;
				break;
			default:
				c = '?';
				break;
			}
			if (d != '~')	/* eat tilde P.K. */
				get1key();
			if (c == 'i') {	/* DO key    P.K. */
				c = ctlxc;
				goto proc_ctlxc;
			} else if (c == 'c')	/* ESC key   P.K. */
				c = get1key();
			else
				return SPEC | c | cmask;
		}
#endif
#if VT220
		if (c == (CONTROL | '[')) {
			cmask = META;
			goto proc_metac;
		}
#endif
		if (islower(c))	/* Force to upper */
			c ^= DIFCASE;
		if (c >= 0x00 && c <= 0x1F)	/* control key */
			c = CONTROL | (c + '@');
		return META | c;
	}
#if	PKCODE
	else if (c == metac) {
		c = get1key();
#if VT220
		if (c == (CONTROL | '[')) {
			cmask = META;
			goto proc_metac;
		}
#endif
		if (islower(c))	/* Force to upper */
			c ^= DIFCASE;
		if (c >= 0x00 && c <= 0x1F)	/* control key */
			c = CONTROL | (c + '@');
		return META | c;
	}
#endif


#if	VT220
      proc_ctlxc:
#endif
	/* process CTLX prefix */
	if (c == ctlxc) {
		c = get1key();
#if VT220
		if (c == (CONTROL | '[')) {
			cmask = CTLX;
			goto proc_metac;
		}
#endif
		if (c >= 'a' && c <= 'z')	/* Force to upper */
			c -= 0x20;
		if (c >= 0x00 && c <= 0x1F)	/* control key */
			c = CONTROL | (c + '@');
		return CTLX | c;
	}

	/* otherwise, just return it */
	return c;
}

/*	A more generalized prompt/reply function allowing the caller
	to specify the proper terminator. If the terminator is not
	a return ('\n') it will echo as "<NL>"
							*/
int getstring(char *prompt, char *buf, int nbuf, int eolchar)
{
	int cpos;	/* current character position in string */
	int c;
	int quotef;	/* are we quoting the next char? */
#if	COMPLC
	int ffile, ocpos, nskip = 0, didtry = 0;
#if     MSDOS
	struct ffblk ffblk;
	char *fcp;
#endif
#if	UNIX
	static char tmp[] = "/tmp/meXXXXXX";
	FILE *tmpf = NULL;
#endif
	ffile = (strcmp(prompt, "Find file: ") == 0
		 || strcmp(prompt, "View file: ") == 0
		 || strcmp(prompt, "Insert file: ") == 0
		 || strcmp(prompt, "Write file: ") == 0
		 || strcmp(prompt, "Read file: ") == 0
		 || strcmp(prompt, "File to execute: ") == 0);
#endif

	cpos = 0;
	quotef = FALSE;

	/* prompt the user for the input string */
	mlwrite(prompt);

	for (;;) {
#if	COMPLC
		if (!didtry)
			nskip = -1;
		didtry = 0;
#endif
		/* get a character from the user */
		c = get1key();

		/* If it is a <ret>, change it to a <NL> */
#if	PKCODE
		if (c == (CONTROL | 0x4d) && !quotef)
#else
		if (c == (CONTROL | 0x4d))
#endif
			c = CONTROL | 0x40 | '\n';

		/* if they hit the line terminate, wrap it up */
		if (c == eolchar && quotef == FALSE) {
			buf[cpos++] = 0;

			/* clear the message line */
			mlwrite("");
			TTflush();

			/* if we default the buffer, return FALSE */
			if (buf[0] == 0)
				return FALSE;

			return TRUE;
		}

		/* change from command form back to character form */
		c = ectoc(c);

		if (c == ectoc(abortc) && quotef == FALSE) {
			/* Abort the input? */
			ctrlg(FALSE, 0);
			TTflush();
			return ABORT;
		} else if ((c == 0x7F || c == 0x08) && quotef == FALSE) {
			/* rubout/erase */
			if (cpos != 0) {
				outstring("\b \b");
				--ttcol;

				if (buf[--cpos] < 0x20) {
					outstring("\b \b");
					--ttcol;
				}
				if (buf[cpos] == '\n') {
					outstring("\b\b  \b\b");
					ttcol -= 2;
				}

				TTflush();
			}

		} else if (c == 0x15 && quotef == FALSE) {
			/* C-U, kill */
			while (cpos != 0) {
				outstring("\b \b");
				--ttcol;

				if (buf[--cpos] < 0x20) {
					outstring("\b \b");
					--ttcol;
				}
				if (buf[cpos] == '\n') {
					outstring("\b\b  \b\b");
					ttcol -= 2;
				}
			}
			TTflush();

#if	COMPLC
		} else if ((c == 0x09 || c == ' ') && quotef == FALSE
			   && ffile) {
			/* TAB, complete file name */
			char ffbuf[255];
#if	MSDOS
			char sffbuf[128];
			int lsav = -1;
#endif
			int n, iswild = 0;

			didtry = 1;
			ocpos = cpos;
			while (cpos != 0) {
				outstring("\b \b");
				--ttcol;

				if (buf[--cpos] < 0x20) {
					outstring("\b \b");
					--ttcol;
				}
				if (buf[cpos] == '\n') {
					outstring("\b\b  \b\b");
					ttcol -= 2;
				}
				if (buf[cpos] == '*' || buf[cpos] == '?')
					iswild = 1;
#if	MSDOS
				if (lsav < 0 && (buf[cpos] == '\\' ||
						 buf[cpos] == '/' ||
						 buf[cpos] == ':'
						 && cpos == 1))
					lsav = cpos;
#endif
			}
			TTflush();
			if (nskip < 0) {
				buf[ocpos] = 0;
#if	UNIX
				if (tmpf != NULL)
					fclose(tmpf);
				strcpy(tmp, "/tmp/meXXXXXX");
				strcpy(ffbuf, "echo ");
				strcat(ffbuf, buf);
				if (!iswild)
					strcat(ffbuf, "*");
				strcat(ffbuf, " >");
				xmkstemp(tmp);
				strcat(ffbuf, tmp);
				strcat(ffbuf, " 2>&1");
				system(ffbuf);
				tmpf = fopen(tmp, "r");
#endif
#if	MSDOS
				strcpy(sffbuf, buf);
				if (!iswild)
					strcat(sffbuf, "*.*");
#endif
				nskip = 0;
			}
#if	UNIX
			c = ' ';
			for (n = nskip; n > 0; n--)
				while ((c = getc(tmpf)) != EOF
				       && c != ' ');
#endif
#if	MSDOS
			if (nskip == 0) {
				strcpy(ffbuf, sffbuf);
				c = findfirst(ffbuf, &ffblk,
					      FA_DIREC) ? '*' : ' ';
			} else if (nskip > 0)
				c = findnext(&ffblk) ? 0 : ' ';
#endif
			nskip++;

			if (c != ' ') {
				TTbeep();
				nskip = 0;
			}
#if	UNIX
			while ((c = getc(tmpf)) != EOF && c != '\n'
			       && c != ' ' && c != '*')
#endif
#if	MSDOS
				if (c == '*')
					fcp = sffbuf;
				else {
					strncpy(buf, sffbuf, lsav + 1);
					cpos = lsav + 1;
					fcp = ffblk.ff_name;
				}
			while (c != 0 && (c = *fcp++) != 0 && c != '*')
#endif
			{
				if (cpos < nbuf - 1)
					buf[cpos++] = c;
			}
#if	UNIX
			if (c == '*')
				TTbeep();
#endif

			for (n = 0; n < cpos; n++) {
				c = buf[n];
				if ((c < ' ') && (c != '\n')) {
					outstring("^");
					++ttcol;
					c ^= 0x40;
				}

				if (c != '\n') {
					if (disinp)
						TTputc(c);
				} else {	/* put out <NL> for <ret> */
					outstring("<NL>");
					ttcol += 3;
				}
				++ttcol;
			}
			TTflush();
#if	UNIX
			rewind(tmpf);
			unlink(tmp);
#endif
#endif

		} else if ((c == quotec || c == 0x16) && quotef == FALSE) {
			quotef = TRUE;
		} else {
			quotef = FALSE;
			if (cpos < nbuf - 1) {
				buf[cpos++] = c;

				if ((c < ' ') && (c != '\n')) {
					outstring("^");
					++ttcol;
					c ^= 0x40;
				}

				if (c != '\n') {
					if (disinp)
						TTputc(c);
				} else {	/* put out <NL> for <ret> */
					outstring("<NL>");
					ttcol += 3;
				}
				++ttcol;
				TTflush();
			}
		}
	}
}

/*
 * output a string of characters
 *
 * char *s;		string to output
 */
void outstring(char *s)
{
	if (disinp)
		while (*s)
			TTputc(*s++);
}

/*
 * output a string of output characters
 *
 * char *s;		string to output
 */
void ostring(char *s)
{
	if (discmd)
		while (*s)
			TTputc(*s++);
}
