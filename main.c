/*
 *	uEmacs/PK 4.0
 *
 *		based on
 *
 *	MicroEMACS 3.9
 * 			written by Dave G. Conroy.
 *			substatially modified by Daniel M. Lawrence
 *			modified by Petri Kutvonen
 *	
 *	MicroEMACS 3.9 (C)opyright 1987 by Daniel M. Lawrence
 *
 *	Original statement of copying policy:
 *
 *	MicroEMACS 3.9 can be copied and distributed freely for any
 *	non-commercial purposes. MicroEMACS 3.9 can only be incorporated
 *	into commercial software with the permission of the current author.
 *
 *	No copyright claimed for modifications made by Petri Kutvonen.
 *
 *	MAIN.C
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

#include        <stdio.h>

/* make global definitions not external */
#define	maindef

#include        "estruct.h"	/* global structures and defines */
#include	"efunc.h"	/* function declarations and name table	*/
#include	"edef.h"	/* global definitions */
#include	"ebind.h"	/* default key bindings */

/* for MSDOS, increase the default stack space */

#if	MSDOS & TURBO
#if	PKCODE
extern unsigned _stklen = 20000;
#else
extern unsigned _stklen = 32766;
#endif
#endif

#if     VMS
#include        <ssdef.h>
#define GOOD    (SS$_NORMAL)
#endif

#ifndef GOOD
#define GOOD    0
#endif

#if	UNIX
#include <signal.h>
#endif

#if	CALLED
emacs(argc, argv)
#else
main(argc, argv)
#endif
int argc;	/* # of arguments */
char *argv[];	/* argument strings */

{
        register int    c;		/* command character */
        register int    f;		/* default flag */
        register int    n;		/* numeric repeat count */
        register int    mflag;		/* negative flag on repeat */
	register BUFFER *bp;		/* temp buffer pointer */
	register int	firstfile;	/* first file flag */
	register int	carg;		/* current arg to scan */
	register int	startflag;	/* startup executed flag */
	BUFFER *firstbp = NULL;		/* ptr to first buffer in cmd line */
	int basec;			/* c stripped of meta character */
	int viewflag;			/* are we starting in view mode? */
        int gotoflag;                   /* do we need to goto a line at start? */
        int gline;                      /* if so, what line? */
        int searchflag;                 /* Do we need to search at start? */
	int saveflag;			/* temp store for lastflag */
	int errflag;			/* C error processing? */
        char bname[NBUFN];		/* buffer name of file to read */
#if	CRYPT
	int cryptflag;			/* encrypting on the way in? */
	char ekey[NPAT];		/* startup encryption key */
#endif
	char *strncpy();
	extern *pathname[];		/* startup file path/name array */
	int newc;
#if	PKCODE
	int (*getbind())();
	int (*execfunc)();		/* ptr to function to execute */
#endif

#if UNIX
	static void emergencyexit();
#ifdef SIGWINCH
	extern void sizesignal();
#endif
#endif

#if	PKCODE & VMS
	(void) umask(-1);	/* use old protection (this is at wrong place) */
#endif

#if	PKCODE & BSD
	sleep(1);		/* time for window manager */
#endif

#if	UNIX
#ifdef SIGWINCH
	signal(SIGWINCH, sizesignal);
#endif
#endif

	/* initialize the editor */
        vtinit();		/* Display */
        edinit("main");		/* Buffers, windows */
	varinit();		/* user variables */

	viewflag = FALSE;	/* view mode defaults off in command line */
	gotoflag = FALSE;	/* set to off to begin with */
	searchflag = FALSE;	/* set to off to begin with */
	firstfile = TRUE;	/* no file to edit yet */
	startflag = FALSE;	/* startup file not executed yet */
	errflag = FALSE;	/* not doing C error parsing */
#if	CRYPT
	cryptflag = FALSE;	/* no encryption by default */
#endif
#if	CALLED
	eexitflag = FALSE;	/* not time to exit yet */
#endif

	/* Parse the command line */
	for (carg = 1; carg < argc; ++carg) {

		/* Process Switches */
#if	PKCODE
		if (argv[carg][0] == '+') {
			gotoflag = TRUE;
			gline = atoi(&argv[carg][1]);
		} else
#endif
		if (argv[carg][0] == '-') {
			switch (argv[carg][1]) {
				/* Process Startup macroes */
				case 'a':	/* process error file */
				case 'A':
					errflag = TRUE;
					break;
				case 'e':	/* -e for Edit file */
				case 'E':
					viewflag = FALSE;
					break;
				case 'g':	/* -g for initial goto */
				case 'G':
					gotoflag = TRUE;
					gline = atoi(&argv[carg][2]);
					break;
#if	CRYPT
				case 'k':	/* -k<key> for code key */
				case 'K':
					cryptflag = TRUE;
					strcpy(ekey, &argv[carg][2]);
					break;
#endif
#if	PKCODE
				case 'n':	/* -n accept null chars */
				case 'N':
					nullflag = TRUE;
					break;
#endif
				case 'r':	/* -r restrictive use */
				case 'R':
					restflag = TRUE;
					break;
				case 's':	/* -s for initial search string */
				case 'S':
					searchflag = TRUE;
					strncpy(pat,&argv[carg][2],NPAT);
					break;
				case 'v':	/* -v for View File */
				case 'V':
					viewflag = TRUE;
					break;
				default:	/* unknown switch */
					/* ignore this for now */
					break;
			}

		} else if (argv[carg][0]== '@') {

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
#if	CRYPT
			if (cryptflag) {
				bp->b_mode |= MDCRYPT;
				crypt((char *)NULL, 0);
				crypt(ekey, strlen(ekey));
				strncpy(bp->b_key, ekey, NPAT);
			}
#endif
		}
	}

#if	UNIX
	signal(SIGHUP,  emergencyexit);
	signal(SIGTERM, emergencyexit);
#endif

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
	discmd = TRUE;	/* P.K. */

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
	}
        else if (gotoflag) {
                if (gotoline(TRUE,gline) == FALSE) {
                	update(FALSE);
			mlwrite("(Bogus goto argument)");
		}
        } else if (searchflag) {
                if (forwhunt(FALSE, 0) == FALSE)
                	update(FALSE);
        }

	/* setup to process commands */
        lastflag = 0;                           /* Fake last flags.     */

loop:

#if	CALLED
	/* if we were called as a subroutine and want to leave, do so */
	if (eexitflag)
		return(eexitval);
#endif

	/* execute the "command" macro...normally null */
	saveflag = lastflag;	/* preserve lastflag through this */
	execute(META|SPEC|'C', FALSE, 1);
	lastflag = saveflag;

#if 	TYPEAH && PKCODE
	if (typahead())
	{
		newc = getcmd();
		update(FALSE);
		do
		{
			if (c == newc && (execfunc = getbind(c)) != NULL
				      &&  execfunc != newline 
				      &&  execfunc != tab)
				newc = getcmd();
			else
				break;
		} while (typahead());
		c = newc;
	}
	else
	{
		update(FALSE);
		c = getcmd();
	}
#else
	/* Fix up the screen    */
        update(FALSE);

	/* get the next command from the keyboard */
	c = getcmd();
#endif
	/* if there is something on the command line, clear it */
        if (mpresf != FALSE) {
                mlerase();
                update(FALSE);
#if	CLRMSG
                if (c == ' ')                   /* ITS EMACS does this  */
                        goto loop;
#endif
        }
        f = FALSE;
        n = 1;

	/* do META-# processing if needed */

	basec = c & ~META;		/* strip meta char off if there */
	if ((c & META) && ((basec >= '0' && basec <= '9') || basec == '-')) {
		f = TRUE;		/* there is a # arg */
		n = 0;			/* start with a zero default */
		mflag = 1;		/* current minus flag */
		c = basec;		/* strip the META */
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
				mlwrite("Arg: %d",n * mflag);

			c = getcmd();	/* get the next key */
		}
		n = n * mflag;	/* figure in the sign */
	}

	/* do ^U repeat argument processing */

        if (c == reptc) {                  /* ^U, start argument   */
                f = TRUE;
                n = 4;                          /* with argument of 4 */
                mflag = 0;                      /* that can be discarded. */
                mlwrite("Arg: 4");
                while ((c=getcmd()) >='0' && c<='9' || c==reptc || c=='-'){
                        if (c == reptc)
				if ((n > 0) == ((n*4) > 0))
	                                n = n*4;
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
                                n = 10*n + c - '0';
                        }
                        mlwrite("Arg: %d", (mflag >=0) ? n : (n ? -n : -1));
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
edinit(bname)
char    bname[];
{
        register BUFFER *bp;
        register WINDOW *wp;
	char *malloc();

        bp = bfind(bname, TRUE, 0);             /* First buffer         */
        blistp = bfind("*List*", TRUE, BFINVS); /* Buffer list buffer   */
        wp = (WINDOW *) malloc(sizeof(WINDOW)); /* First window         */
        if (bp==NULL || wp==NULL || blistp==NULL)
                exit(1);
        curbp  = bp;                            /* Make this current    */
        wheadp = wp;
        curwp  = wp;
        wp->w_wndp  = NULL;                     /* Initialize window    */
        wp->w_bufp  = bp;
        bp->b_nwnd  = 1;                        /* Displayed.           */
        wp->w_linep = bp->b_linep;
        wp->w_dotp  = bp->b_linep;
        wp->w_doto  = 0;
        wp->w_markp = NULL;
        wp->w_marko = 0;
        wp->w_toprow = 0;
#if	COLOR
	/* initalize colors to global defaults */
	wp->w_fcolor = gfcolor;
	wp->w_bcolor = gbcolor;
#endif
        wp->w_ntrows = term.t_nrow-1;           /* "-1" for mode line.  */
        wp->w_force = 0;
        wp->w_flag  = WFMODE|WFHARD;            /* Full.                */
}

/*
 * This is the general command execution routine. It handles the fake binding
 * of all the keys to "self-insert". It also clears out the "thisflag" word,
 * and arranges to move it to the "lastflag", so that the next command can
 * look at it. Return the status of command.
 */
execute(c, f, n)
{
        register int status;
	int (*execfunc)();		/* ptr to function to execute */
	int (*getbind())();

	/* if the keystroke is a bound function...do it */
	execfunc = getbind(c);
        if (execfunc != NULL) {
		thisflag = 0;
		status	 = (*execfunc)(f, n);
		lastflag = thisflag;
		return (status);
        }

        /*
         * If a space was typed, fill column is defined, the argument is non-
         * negative, wrap mode is enabled, and we are now past fill column,
	 * and we are not read-only, perform word wrap.
         */
        if (c == ' ' && (curwp->w_bufp->b_mode & MDWRAP) && fillcol > 0 &&
	    n >= 0 && getccol(FALSE) > fillcol &&
	    (curwp->w_bufp->b_mode & MDVIEW) == FALSE)
		execute(META|SPEC|'W', FALSE, 1);

#if	PKCODE
        if ((c>=0x20 && c<=0x7E)                /* Self inserting.      */
#if	IBMPC
        ||  (c>=0x80 && c<=0xFE)) {
#else
#if	VMS || BSD || USG				/* 8BIT P.K. */
        ||  (c>=0xA0 && c<=0xFE)) {
#else
	) {
#endif
#endif
#else
        if ((c>=0x20 && c<=0xFF)) {	/* Self inserting.      */
#endif
                if (n <= 0) {                   /* Fenceposts.          */
                        lastflag = 0;
                        return (n<0 ? FALSE : TRUE);
                }
                thisflag = 0;                   /* For the future.      */

		/* if we are in overwrite mode, not at eol,
		   and next char is not a tab or we are at a tab stop,
		   delete a char forword			*/
		if (curwp->w_bufp->b_mode & MDOVER &&
		    curwp->w_doto < curwp->w_dotp->l_used &&
			(lgetc(curwp->w_dotp, curwp->w_doto) != '\t' ||
			 (curwp->w_doto) % 8 == 7))
				ldelete(1L, FALSE);

		/* do the appropriate insertion */
		if (c == '}' && (curbp->b_mode & MDCMOD) != 0)
	        	status = insbrace(n, c);
	        else if (c == '#' && (curbp->b_mode & MDCMOD) != 0)
	        	status = inspound();
	        else
	                status = linsert(n, c);

#if	CFENCE
		/* check for CMODE fence matching */
		if ((c == '}' || c == ')' || c == ']') &&
				(curbp->b_mode & MDCMOD) != 0)
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
                return (status);
        }
	TTbeep();
	mlwrite("(Key not bound)");		/* complain		*/
        lastflag = 0;                           /* Fake last flags.     */
        return (FALSE);
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit emacs, otherwise simply exit.
 */
quickexit(f, n)
{
	register BUFFER *bp;	/* scanning pointer to buffers */
        register BUFFER *oldcb; /* original current buffer */
	register int status;

        oldcb = curbp;                          /* save in case we fail */

	bp = bheadp;
	while (bp != NULL) {
		if ((bp->b_flag&BFCHG) != 0	/* Changed.             */
		&& (bp->b_flag&BFTRUNC) == 0	/* Not truncated P.K.	*/
        	&& (bp->b_flag&BFINVS) == 0) {	/* Real.                */
			curbp = bp;		/* make that buffer cur	*/
			mlwrite("(Saving %s)",bp->b_fname);
#if	PKCODE
#else
			mlwrite("\n");
#endif
                	if ((status = filesave(f, n)) != TRUE) {
                		curbp = oldcb;	/* restore curbp */
                		return(status);
                	}
		}
	bp = bp->b_bufp;			/* on to the next buffer */
	}
        quit(f, n);                             /* conditionally quit   */
	return(TRUE);
}

static void emergencyexit(signr)
int signr;
{
	quickexit(FALSE, 0);
	quit(TRUE, 0);		
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out. Normally bound to "C-X C-C".
 */
quit(f, n)
{
        register int    s;

        if (f != FALSE                          /* Argument forces it.  */
        || anycb() == FALSE                     /* All buffers clean.   */
						/* User says it's OK.   */
        || (s=mlyesno("Modified buffers exist. Leave anyway")) == TRUE) {
#if	(FILOCK && BSD) || SVR4
		if (lockrel() != TRUE) {
			TTputc('\n');
			TTputc('\r');
			TTclose();
			TTkclose();
			exit(1);
		}
#endif
                vttidy();
		if (f)
			exit(n);
		else
	                exit(GOOD);
        }
	mlwrite("");
        return(s);
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
ctlxlp(f, n)
{
        if (kbdmode != STOP) {
                mlwrite("%%Macro already active");
                return(FALSE);
        }
        mlwrite("(Start macro)");
	kbdptr = &kbdm[0];
	kbdend = kbdptr;
        kbdmode = RECORD;
        return (TRUE);
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
ctlxrp(f, n)
{
        if (kbdmode == STOP) {
                mlwrite("%%Macro not active");
                return(FALSE);
        }
	if (kbdmode == RECORD) {
	        mlwrite("(End macro)");
	        kbdmode = STOP;
	}
        return(TRUE);
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
ctlxe(f, n)
{
        if (kbdmode != STOP) {
                mlwrite("%%Macro already active");
                return(FALSE);
        }
        if (n <= 0)
                return (TRUE);
	kbdrep = n;		/* remember how many times to execute */
	kbdmode = PLAY;		/* start us in play mode */
	kbdptr = &kbdm[0];	/*    at the beginning */
	return(TRUE);
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
ctrlg(f, n)
{
        TTbeep();
	kbdmode = STOP;
	mlwrite("(Aborted)");
        return(ABORT);
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode				*/

rdonly()

{
	TTbeep();
	mlwrite("(Key illegal in VIEW mode)");
	return(FALSE);
}

resterr()

{
	TTbeep();
	mlwrite("(That command is RESTRICTED)");
	return(FALSE);
}

nullproc()	/* user function that does NOTHING */

{
}

meta()	/* dummy function for binding to meta prefix */
{
}

cex()	/* dummy function for binding to control-x prefix */
{
}

unarg()	/* dummy function for binding to universal-argument */
{
}

/*****		Compiler specific Library functions	****/

#if	MSDOS || PKCODE
/*	strncpy:	copy a string...with length restrictions
			ALWAYS null terminate
*/

char *strncpy(dst, src, maxlen)

char *dst;	/* destination of copied string */
char *src;	/* source */
int maxlen;	/* maximum length */

{
	char *dptr;	/* ptr into dst */

	dptr = dst;
/*	while (*src && (maxlen-- > 0))
		*dptr++ = *src++;	*/
	while ((maxlen > 0) && *src) {
		*dptr++ = *src++;
		maxlen--;
	}
	
	*dptr = 0;		/* orig	*/
/*	if (maxlen > 0)
		*dptr = 0;		*/
	return(dst);
}
#endif

#if	RAMSIZE
/*	These routines will allow me to track memory usage by placing
	a layer on top of the standard system malloc() and free() calls.
	with this code defined, the environment variable, $RAM, will
	report on the number of bytes allocated via malloc.

	with SHOWRAM defined, the number is also posted on the
	end of the bottom mode line and is updated whenever it is changed.
*/

#undef	malloc
#undef	free

char *allocate(nbytes)	/* allocate nbytes and track */

unsigned nbytes;	/* # of bytes to allocate */

{
	char *mp;	/* ptr returned from malloc */
	char *malloc();

	mp = malloc(nbytes);
	if (mp) {
		envram += nbytes;
#if	RAMSHOW
		dspram();
#endif
	}

	return(mp);
}

release(mp)	/* release malloced memory and track */

char *mp;	/* chunk of RAM to release */

{
	unsigned *lp;	/* ptr to the long containing the block size */

	if (mp) {
		/* update amount of ram currently malloced */
		lp = ((unsigned *)mp) - 1;
		envram -= (long)*lp - 2;
		free(mp);
#if	RAMSHOW
		dspram();
#endif
	}
}

#if	RAMSHOW
dspram()	/* display the amount of RAM currently malloced */

{
	char mbuf[20];
	char *sp;

	TTmove(term.t_nrow - 1, 70);
#if	COLOR
	TTforg(7);
	TTbacg(0);
#endif
	sprintf(mbuf, "[%lu]", envram);
	sp = &mbuf[0];
	while (*sp)
		TTputc(*sp++);
	TTmove(term.t_nrow, 0);
	movecursor(term.t_nrow, 0);
}
#endif
#endif

/*	On some primitave operation systems, and when emacs is used as
	a subprogram to a larger project, emacs needs to de-alloc its
	own used memory
*/

#if	CLEAN
cexit(status)

int status;	/* return status of emacs */

{
	register BUFFER *bp;	/* buffer list pointer */
	register WINDOW *wp;	/* window list pointer */
	register WINDOW *tp;	/* temporary window pointer */

	/* first clean up the windows */
	wp = wheadp;
	while (wp) {
		tp = wp->w_wndp;
		free(wp);
		wp = tp;
	}
	wheadp = NULL;

	/* then the buffers */
	bp = bheadp;
	while (bp) {
		bp->b_nwnd = 0;
		bp->b_flag = 0;	/* don't say anything about a changed buffer! */
		zotbuf(bp);
		bp = bheadp;
	}

	/* and the kill buffer */
	kdelete();

	/* and the video buffers */
	vtfree();

	/* and now.. we leave [pick the return if we are a subprogram] */
#if	CALLED
	eexitflag = TRUE;	/* flag a program exit */
	eexitval = status;
	return(status);
#else
#undef	exit
	exit(status);
#endif
}
#endif
