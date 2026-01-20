/*
 *	main.c

 *	uEmacs/PK 4.0
 *
 *	Based on:
 *
 *	MicroEMACS 3.9
 *	Written by Dave G. Conroy.
 *	Substantially modified by Daniel M. Lawrence
 *	Modified by Petri Kutvonen
 *
 *	MicroEMACS 3.9 (c) Copyright 1987 by Daniel M. Lawrence
 *
 *	Original statement of copying policy:
 *
 *	MicroEMACS 3.9 can be copied and distributed freely for any
 *	non-commercial purposes. MicroEMACS 3.9 can only be incorporated
 *	into commercial software with the permission of the current author.
 *
 *	No copyright claimed for modifications made by Petri Kutvonen.
 *
 *	This file contains the main driving routine, and some keyboard
 *	processing code.
 *
 * REVISION HISTORY:
 *
 * 1.0  Steve Wilhite, 30-Nov-85
 *
 * 2.0  George Jones, 12-Dec-85
 *
 * 3.0  Daniel Lawrence, 29-Dec-85
 *
 * 3.2-3.6 Daniel Lawrence, Feb...Apr-86
 *
 * 3.7	Daniel Lawrence, 14-May-86
 *
 * 3.8	Daniel Lawrence, 18-Jan-87
 *
 * 3.9	Daniel Lawrence, 16-Jul-87
 *
 * 3.9e	Daniel Lawrence, 16-Nov-87
 *
 * After that versions 3.X and Daniel Lawrence went their own ways.
 * A modified 3.9e/PK was heavily used at the University of Helsinki
 * for several years on different UNIX, VMS, and MSDOS platforms.
 *
 * This modified version is now called eEmacs/PK.
 *
 * 4.0	Petri Kutvonen, 1-Sep-91
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <hunspell.h>

/* Make global definitions not external. */
#define	maindef

#include "estruct.h"				/* Global structures and defines. */
#include "edef.h"				/* Global definitions. */
#include "efunc.h"				/* Function declarations and name table. */
#include "ebind.h"				/* Default key bindings. */
#include "version.h"

#include <signal.h>
static void emergencyexit(int);
extern void sizesignal(int);

void usage(int status)
{
	printf("Usage: %s filename\n", PROGRAM_NAME);
	printf("   or: %s [options]\n\n", PROGRAM_NAME);
	fputs("      +          start at the end of file\n", stdout);
	fputs("      +<n>       start at line <n>\n", stdout);
	fputs("      -g[G]<n>   go to line <n>\n", stdout);
	fputs("      --help     display this help and exit\n", stdout);
	fputs("      --version  output version information and exit\n", stdout);

	exit(status);
}

static Hunhandle *hunhandle;

int spellcheck(const char *word)
{
	if (!hunhandle)
		return 1;
	return Hunspell_spell(hunhandle, word);
}

static void local_dictionary(Hunhandle *handle, const char *filename)
{
	struct stat st;
	if (!stat(filename, &st) && S_ISREG(st.st_mode))
		Hunspell_add_dic(handle, filename);
}

int main(int argc, char **argv)
{
	int c = -1;				/* command character */
	int f;					/* default flag */
	int n;					/* numeric repeat count */
	int mflag;				/* negative flag on repeat */
	struct buffer *bp;			/* temp buffer pointer */
	int firstfile;				/* first file flag */
	int carg;				/* current arg to scan */
	int startflag;				/* startup executed flag */
	struct buffer *firstbp = NULL;		/* ptr to first buffer in cmd line */
	int basec;				/* c stripped of meta character */
	int viewflag;				/* are we starting in view mode? */
	int gotoflag;				/* do we need to goto a line at start? */
	int gline = 0;				/* if so, what line? */
	int searchflag;				/* Do we need to search at start? */
	int saveflag;				/* temp store for lastflag */
	int errflag;				/* C error processing? */
	char bname[NBUFN];			/* buffer name of file to read */
	int newc;

	const char *aff_path = "/usr/share/hunspell/en_US.aff";
	const char *dic_path = "/usr/share/hunspell/en_US.dic";
	hunhandle = Hunspell_create(aff_path, dic_path);
	if (hunhandle) {
		local_dictionary(hunhandle, ".dictionary");
		const char *home = getenv("HOME");
		if (home) {
			char buf[1024];
			snprintf(buf, sizeof(buf), "%s/.dictionary", home);
			local_dictionary(hunhandle, buf);
		}
	}

	signal(SIGWINCH, sizesignal);
	if (argc == 2) {
		if (strcmp(argv[1], "--help") == 0) {
			usage(EXIT_FAILURE);
		}
		if (strcmp(argv[1], "--version") == 0) {
			version();
			exit(EXIT_SUCCESS);
		}
	}

	/* Initialize the editor. */
	vtinit();				/* Display */
	edinit("main");				/* Buffers, windows */
	varinit();				/* user variables */

	viewflag = FALSE;			/* view mode defaults off in command line */
	gotoflag = FALSE;			/* set to off to begin with */
	searchflag = FALSE;			/* set to off to begin with */
	firstfile = TRUE;			/* no file to edit yet */
	startflag = FALSE;			/* startup file not executed yet */
	errflag = FALSE;			/* not doing C error parsing */

	/* Parse the command line */
	for (carg = 1; carg < argc; ++carg) {
		/* Process Switches */
		if (argv[carg][0] == '+') {
			gotoflag = TRUE;
			gline = atoi(&argv[carg][1]);
		} else if (argv[carg][0] == '-') {
			switch (argv[carg][1]) {
				/* Process Startup macroes */
			case 'a':		/* process error file */
			case 'A':
				errflag = TRUE;
				break;
			case 'e':		/* -e for Edit file */
			case 'E':
				viewflag = FALSE;
				break;
			case 'g':		/* -g for initial goto */
			case 'G':
				gotoflag = TRUE;
				gline = atoi(&argv[carg][2]);
				break;
			case 'n':		/* -n accept null chars */
			case 'N':
				nullflag = TRUE;
				break;
			case 'r':		/* -r restrictive use */
			case 'R':
				restflag = TRUE;
				break;
			case 's':		/* -s for initial search string */
			case 'S':
				searchflag = TRUE;
				strncpy(pat, &argv[carg][2], NPAT);
				break;
			case 'v':		/* -v for View File */
			case 'V':
				viewflag = TRUE;
				break;
			default:		/* unknown switch */
				/* ignore this for now */
				break;
			}

		} else if (argv[carg][0] == '@') {

			/* Process Startup macroes */
			if (startup(&argv[carg][1]) == TRUE)
				/* don't execute emacs.rc */
				startflag = TRUE;

		} else {

			/* Process an input file */

			/* set up a buffer for this file */
			makename(bname, argv[carg]);
			unqname(bname);

			/* set this to inactive */
			bp = bfind(bname, TRUE, 0);
			strcpy(bp->b_fname, argv[carg]);
			bp->b_active = FALSE;
			if (firstfile) {
				firstbp = bp;
				firstfile = FALSE;
			}

			/* set the modes appropriatly */
			if (viewflag)
				bp->b_mode |= MDVIEW;
		}
	}

	signal(SIGHUP, emergencyexit);
	signal(SIGTERM, emergencyexit);

	/* if we are C error parsing... run it! */
	if (errflag) {
		if (startup("error.cmd") == TRUE)
			startflag = TRUE;
	}

	/* if invoked with no other startup files,
	   run the system startup file here */
	if (startflag == FALSE) {
		startup("");
		startflag = TRUE;
	}
	discmd = TRUE;				/* P.K. */

	/* if there are any files to read, read the first one! */
	bp = bfind("main", FALSE, 0);
	if (firstfile == FALSE && (gflags & GFREAD)) {
		swbuffer(firstbp);
		zotbuf(bp);
	} else
		bp->b_mode |= gmode;

	/* Deal with startup gotos and searches */
	if (gotoflag && searchflag) {
		update(FALSE);
		mlwrite("(Can not search and goto at the same time!)");
	} else if (gotoflag) {
		if (gotoline(TRUE, gline) == FALSE) {
			update(FALSE);
			mlwrite("(Bogus goto argument)");
		}
	} else if (searchflag) {
		if (forwhunt(FALSE, 0) == FALSE)
			update(FALSE);
	}

	/* Setup to process commands. */
	lastflag = 0;				/* Fake last flags. */

 loop:
	/* Execute the "command" macro...normally null. */
	saveflag = lastflag;			/* Preserve lastflag through this. */
	execute(META | SPEC | 'C', FALSE, 1);
	lastflag = saveflag;

	if (typahead()) {
		newc = getcmd();
		update(FALSE);
		do {
			fn_t execfunc;

			if (c == newc && (execfunc = getbind(c)) != NULL
			    && execfunc != insert_newline && execfunc != insert_tab)
				newc = getcmd();
			else
				break;
		} while (typahead());
		c = newc;
	} else {
		update(FALSE);
		c = getcmd();
	}
	/* if there is something on the command line, clear it */
	if (mpresf != FALSE) {
		mlerase();
		update(FALSE);
	}
	f = FALSE;
	n = 1;

	/* do META-# processing if needed */

	basec = c & ~META;			/* strip meta char off if there */
	if ((c & META) && ((basec >= '0' && basec <= '9') || basec == '-')) {
		f = TRUE;			/* there is a # arg */
		n = 0;				/* start with a zero default */
		mflag = 1;			/* current minus flag */
		c = basec;			/* strip the META */
		while ((c >= '0' && c <= '9') || (c == '-')) {
			if (c == '-') {
				/* already hit a minus or digit? */
				if ((mflag == -1) || (n != 0))
					break;
				mflag = -1;
			} else {
				n = n * 10 + (c - '0');
			}
			if ((n == 0) && (mflag == -1))	/* lonely - */
				mlwrite("Arg:");
			else
				mlwrite("Arg: %d", n * mflag);

			c = getcmd();		/* get the next key */
		}
		n = n * mflag;			/* figure in the sign */
	}

	/* do ^U repeat argument processing */

	if (c == reptc) {			/* ^U, start argument   */
		f = TRUE;
		n = 4;				/* with argument of 4 */
		mflag = 0;			/* that can be discarded. */
		mlwrite("Arg: 4");
		while (((c = getcmd()) >= '0' && c <= '9') || c == reptc || c == '-') {
			if (c == reptc)
				if ((n > 0) == ((n * 4) > 0))
					n = n * 4;
				else
					n = 1;
			/*
			 * If dash, and start of argument string, set arg.
			 * to -1.  Otherwise, insert it.
			 */
			else if (c == '-') {
				if (mflag)
					break;
				n = 0;
				mflag = -1;
			}
			/*
			 * If first digit entered, replace previous argument
			 * with digit and set sign.  Otherwise, append to arg.
			 */
			else {
				if (!mflag) {
					n = 0;
					mflag = 1;
				}
				n = 10 * n + c - '0';
			}
			mlwrite("Arg: %d", (mflag >= 0) ? n : (n ? -n : -1));
		}
		/*
		 * Make arguments preceded by a minus sign negative and change
		 * the special argument "^U -" to an effective "^U -1".
		 */
		if (mflag == -1) {
			if (n == 0)
				n++;
			n = -n;
		}
	}

	/* and execute the command */
	execute(c, f, n);
	goto loop;
}

/*
 * Initialize all of the buffers and windows. The buffer name is passed down
 * as an argument, because the main routine may have been told to read in a
 * file by default, and we want the buffer name to be right.
 */
void edinit(char *bname)
{
	struct buffer *bp;
	struct window *wp;

	bp = bfind(bname, TRUE, 0);		/* First buffer         */
	blistp = bfind("*List*", TRUE, BFINVS);	/* Buffer list buffer   */
	wp = (struct window *)malloc(sizeof(struct window));	/* First window         */
	if (bp == NULL || wp == NULL || blistp == NULL)
		exit(1);
	curbp = bp;				/* Make this current    */
	wheadp = wp;
	curwp = wp;
	wp->w_wndp = NULL;			/* Initialize window    */
	wp->w_bufp = bp;
	bp->b_nwnd = 1;				/* Displayed.           */
	wp->w_linep = bp->b_linep;
	wp->w_dotp = bp->b_linep;
	wp->w_doto = 0;
	wp->w_markp = NULL;
	wp->w_marko = 0;
	wp->w_toprow = 0;
	wp->w_ntrows = term.t_nrow - 1;		/* "-1" for mode line.  */
	wp->w_force = 0;
	wp->w_flag = WFMODE | WFHARD;		/* Full.                */
}

/*
 * This is the general command execution routine. It handles the fake binding
 * of all the keys to "self-insert". It also clears out the "thisflag" word,
 * and arranges to move it to the "lastflag", so that the next command can
 * look at it. Return the status of command.
 */
int execute(int c, int f, int n)
{
	int status;
	fn_t execfunc;

	/* if the keystroke is a bound function...do it */
	execfunc = getbind(c);
	if (execfunc != NULL) {
		thisflag = 0;
		status = (*execfunc) (f, n);
		lastflag = thisflag;
		return status;
	}

	/*
	 * If a space was typed, fill column is defined, the argument is non-
	 * negative, wrap mode is enabled, and we are now past fill column,
	 * and we are not read-only, perform word wrap.
	 */
	if (c == ' ' && (curwp->w_bufp->b_mode & MDWRAP) && fillcol > 0 &&
	    n >= 0 && getccol(FALSE) > fillcol && (curwp->w_bufp->b_mode & MDVIEW) == FALSE)
		execute(META | SPEC | 'W', FALSE, 1);

	if ((c >= 0x20 && c <= 0x7E)		/* Self inserting.      */
	    ||(c >= 0xA0 && c <= 0x10FFFF)) {
		if (n <= 0) {			/* Fenceposts.          */
			lastflag = 0;
			return n < 0 ? FALSE : TRUE;
		}
		thisflag = 0;			/* For the future.      */

		/* if we are in overwrite mode, not at eol,
		   and next char is not a tab or we are at a tab stop,
		   delete a char forword                        */
		if (curwp->w_bufp->b_mode & MDOVER &&
		    curwp->w_doto < curwp->w_dotp->l_used &&
		    (lgetc(curwp->w_dotp, curwp->w_doto) != '\t' || (curwp->w_doto) % 8 == 7))
			ldelchar(1, FALSE);

		/* do the appropriate insertion */
		if (c == '}' && (curbp->b_mode & MDCMOD) != 0)
			status = insbrace(n, c);
		else if (c == '#' && (curbp->b_mode & MDCMOD) != 0)
			status = inspound();
		else
			status = linsert(n, c);

#if	CFENCE
		/* check for CMODE fence matching */
		if ((c == '}' || c == ')' || c == ']') && (curbp->b_mode & MDCMOD) != 0)
			fmatch(c);
#endif

		/* check auto-save mode */
		if (curbp->b_mode & MDASAVE)
			if (--gacount == 0) {
				/* and save the file if needed */
				upscreen(FALSE, 0);
				filesave(FALSE, 0);
				gacount = gasave;
			}

		lastflag = thisflag;
		return status;
	}
	TTbeep();
	mlwrite("(Key not bound)");		/* complain             */
	lastflag = 0;				/* Fake last flags.     */
	return FALSE;
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit emacs, otherwise simply exit.
 */
int quickexit(int f, int n)
{
	struct buffer *bp;			/* scanning pointer to buffers */
	struct buffer *oldcb;			/* original current buffer */
	int status;

	oldcb = curbp;				/* save in case we fail */

	bp = bheadp;
	while (bp != NULL) {
		if ((bp->b_flag & BFCHG) != 0	/* Changed.             */
		    && (bp->b_flag & BFTRUNC) == 0	/* Not truncated P.K.   */
		    && (bp->b_flag & BFINVS) == 0) {	/* Real.                */
			curbp = bp;		/* make that buffer cur */
			mlwrite("(Saving %s)", bp->b_fname);
			if ((status = filesave(f, n)) != TRUE) {
				curbp = oldcb;	/* restore curbp */
				return status;
			}
		}
		bp = bp->b_bufp;		/* on to the next buffer */
	}
	quit(f, n);				/* conditionally quit   */
	return TRUE;
}

static void emergencyexit(int signr)
{
	quickexit(FALSE, 0);
	quit(TRUE, 0);
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out. Normally bound to "C-X C-C".
 */
int quit(int f, int n)
{
	int s;

	if (f != FALSE				/* Argument forces it.  */
	    || anycb() == FALSE			/* All buffers clean.   */
	    /* User says it's OK.   */
	    || (s = mlyesno("Modified buffers exist. Leave anyway")) == TRUE) {
		if (lockrel() != TRUE) {
			TTputc('\n');
			TTputc('\r');
			TTclose();
			TTkclose();
			exit(1);
		}
		vttidy();
		if (f)
			exit(n);
		else
			exit(0);
	}
	mlwrite("");
	return s;
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
int ctlxlp(int f, int n)
{
	if (kbdmode != STOP) {
		mlwrite("%%Macro already active");
		return FALSE;
	}
	mlwrite("(Start macro)");
	kbdptr = &kbdm[0];
	kbdend = kbdptr;
	kbdmode = RECORD;
	return TRUE;
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
int ctlxrp(int f, int n)
{
	if (kbdmode == STOP) {
		mlwrite("%%Macro not active");
		return FALSE;
	}
	if (kbdmode == RECORD) {
		mlwrite("(End macro)");
		kbdmode = STOP;
	}
	return TRUE;
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
int ctlxe(int f, int n)
{
	if (kbdmode != STOP) {
		mlwrite("%%Macro already active");
		return FALSE;
	}
	if (n <= 0)
		return TRUE;
	kbdrep = n;				/* remember how many times to execute */
	kbdmode = PLAY;				/* start us in play mode */
	kbdptr = &kbdm[0];			/*    at the beginning */
	return TRUE;
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
int ctrlg(int f, int n)
{
	TTbeep();
	kbdmode = STOP;
	mlwrite("(Aborted)");
	return ABORT;
}

/*
 * tell the user that this command is illegal while we are in
 * VIEW (read-only) mode
 */
int rdonly(void)
{
	TTbeep();
	mlwrite("(Key illegal in VIEW mode)");
	return FALSE;
}

int resterr(void)
{
	TTbeep();
	mlwrite("(That command is RESTRICTED)");
	return FALSE;
}

/* user function that does NOTHING */
int nullproc(int f, int n)
{
	return TRUE;
}

/* dummy function for binding to meta prefix */
int metafn(int f, int n)
{
	return TRUE;
}

/* dummy function for binding to control-x prefix */
int cex(int f, int n)
{
	return TRUE;
}

/* dummy function for binding to universal-argument */
int unarg(int f, int n)
{
	return TRUE;
}
