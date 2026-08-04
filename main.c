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
#include "globals.h"				/* Global definitions. */
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

static void local_dictionary(Hunhandle *handle, const char *cmd_change_file_name)
{
	struct stat st;
	if (!stat(cmd_change_file_name, &st) && S_ISREG(st.st_mode))
		Hunspell_add_dic(handle, cmd_change_file_name);
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

	/*
	 * Deliberately not SA_RESTART: a resize has to interrupt the
	 * read() the editor spends its life in, or nothing acts on it
	 * until the next keystroke.
	 */
	struct sigaction winch = { .sa_handler = sizesignal };
	sigemptyset(&winch.sa_mask);
	sigaction(SIGWINCH, &winch, NULL);
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
	display_open();				/* Display */
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
				accept_nulls = TRUE;
				break;
			case 'r':		/* -r restrictive use */
			case 'R':
				restflag = TRUE;
				break;
			case 's':		/* -s for initial search string */
			case 'S':
				searchflag = TRUE;
				strncpy(search_pattern, &argv[carg][2], NPAT);
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
	display_commands = TRUE;				/* P.K. */

	/* if there are any files to read, read the first one! */
	bp = bfind("main", FALSE, 0);
	if (firstfile == FALSE && (global_flags & GFREAD)) {
		swbuffer(firstbp);
		zotbuf(bp);
	} else
		bp->b_mode |= global_mode;

	/* Deal with startup gotos and searches */
	if (gotoflag && searchflag) {
		update();
		msg_printf("(Can not search and goto at the same time!)");
	} else if (gotoflag) {
		if (cmd_goto_line(TRUE, gline) == FALSE) {
			update();
			msg_printf("(Bogus goto argument)");
		}
	} else if (searchflag) {
		if (cmd_hunt_forward(FALSE, 0) == FALSE)
			update();
	}

	/* Setup to process commands. */
	lastflag = 0;				/* Fake last flags. */

 loop:
	/* Execute the "command" macro...normally null. */
	saveflag = lastflag;			/* Preserve lastflag through this. */
	execute(META | SPEC | 'C', FALSE, 1);
	lastflag = saveflag;

	if (curwp->w_flag || !typahead())
		update();
	c = getcmd();

	/* if there is something on the command line, clear it */
	if (message_present != FALSE) {
		msg_erase();
		update();
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
				msg_printf("Arg:");
			else
				msg_printf("Arg: %d", n * mflag);

			c = getcmd();		/* get the next key */
		}
		n = n * mflag;			/* figure in the sign */
	}

	/* do ^U repeat argument processing */

	if (c == repeat_key) {			/* ^U, start argument   */
		f = TRUE;
		n = 4;				/* with argument of 4 */
		mflag = 0;			/* that can be discarded. */
		msg_printf("Arg: 4");
		while (((c = getcmd()) >= '0' && c <= '9') || c == repeat_key || c == '-') {
			if (c == repeat_key)
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
			msg_printf("Arg: %d", (mflag >= 0) ? n : (n ? -n : -1));
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
	list_buffer = bfind("*List*", TRUE, BFINVS);	/* Buffer list buffer   */
	wp = (struct window *)malloc(sizeof(struct window));	/* First window         */
	if (bp == NULL || wp == NULL || list_buffer == NULL)
		exit(1);
	curbp = bp;				/* Make this current    */
	curwp = wp;
	wp->w_bufp = bp;			/* Initialize window    */
	bp->b_nwnd = 1;				/* Displayed.           */
	wp->w_linep = bp->b_linep;
	wp->w_dotp = bp->b_linep;
	wp->w_doto = 0;
	wp->w_markp = NULL;
	wp->w_marko = 0;
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
	if (c == ' ' && (curwp->w_bufp->b_mode & MDWRAP) && fill_column > 0 &&
	    n >= 0 && getccol(FALSE) > fill_column && (curwp->w_bufp->b_mode & MDVIEW) == FALSE)
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
			delete_characters(1, FALSE);

		/* do the appropriate insertion */
		if (c == '}' && (curbp->b_mode & MDCMOD) != 0)
			status = insbrace(n, c);
		else if (c == '#' && (curbp->b_mode & MDCMOD) != 0)
			status = inspound();
		else
			status = insert_char(n, c);

		/* check for CMODE fence matching */
		if ((c == '}' || c == ')' || c == ']') && (curbp->b_mode & MDCMOD) != 0)
			fmatch(c);

		/* check auto-save mode */
		if (curbp->b_mode & MDASAVE)
			if (--autosave_countdown == 0) {
				/* and save the file if needed */
				cmd_update_screen(FALSE, 0);
				cmd_save_file(FALSE, 0);
				autosave_countdown = autosave_interval;
			}

		lastflag = thisflag;
		return status;
	}
	tcapbeep();
	msg_printf("(Key not bound)");		/* complain             */
	lastflag = 0;				/* Fake last flags.     */
	return FALSE;
}

/*
 * Fancy quit command, as implemented by Norm. If the any buffer has
 * changed do a write on that buffer and exit emacs, otherwise simply exit.
 */
int cmd_quick_exit(int f, int n)
{
	struct buffer *bp;			/* scanning pointer to buffers */
	struct buffer *oldcb;			/* original current buffer */
	int status;

	oldcb = curbp;				/* save in case we fail */

	bp = buffer_head;
	while (bp != NULL) {
		if ((bp->b_flag & BFCHG) != 0	/* Changed.             */
		    && (bp->b_flag & BFTRUNC) == 0	/* Not truncated P.K.   */
		    && (bp->b_flag & BFINVS) == 0) {	/* Real.                */
			curbp = bp;		/* make that buffer cur */
			msg_printf("(Saving %s)", bp->b_fname);
			if ((status = cmd_save_file(f, n)) != TRUE) {
				curbp = oldcb;	/* restore curbp */
				return status;
			}
		}
		bp = bp->b_bufp;		/* on to the next buffer */
	}
	cmd_exit_emacs(f, n);				/* conditionally quit   */
	return TRUE;
}

static void emergencyexit(int signr)
{
	cmd_quick_exit(FALSE, 0);
	cmd_exit_emacs(TRUE, 0);
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out. Normally bound to "C-X C-C".
 */
int cmd_exit_emacs(int f, int n)
{
	int s;

	if (f != FALSE				/* Argument forces it.  */
	    || anycb() == FALSE			/* All buffers clean.   */
	    /* User says it's OK.   */
	    || (s = ask_yesno("Modified buffers exist. Leave anyway")) == TRUE) {
		display_close();
		if (f)
			exit(n);
		else
			exit(0);
	}
	msg_printf("");
	return s;
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level in keyboard processing. Set up variables and
 * return.
 */
int cmd_begin_macro(int f, int n)
{
	if (keyboard_macro_mode != STOP) {
		msg_printf("%%Macro already active");
		return FALSE;
	}
	msg_printf("(Start macro)");
	keyboard_macro_pos = &keyboard_macro[0];
	keyboard_macro_end = keyboard_macro_pos;
	keyboard_macro_mode = RECORD;
	return TRUE;
}

/*
 * End keyboard macro. Check for the same limit conditions as the above
 * routine. Set up the variables and return to the caller.
 */
int cmd_end_macro(int f, int n)
{
	if (keyboard_macro_mode == STOP) {
		msg_printf("%%Macro not active");
		return FALSE;
	}
	if (keyboard_macro_mode == RECORD) {
		msg_printf("(End macro)");
		keyboard_macro_mode = STOP;
	}
	return TRUE;
}

/*
 * Execute a macro.
 * The command argument is the number of times to loop. Quit as soon as a
 * command gets an error. Return TRUE if all ok, else FALSE.
 */
int cmd_execute_macro(int f, int n)
{
	if (keyboard_macro_mode != STOP) {
		msg_printf("%%Macro already active");
		return FALSE;
	}
	if (n <= 0)
		return TRUE;
	keyboard_macro_repeat = n;				/* remember how many times to execute */
	keyboard_macro_mode = PLAY;				/* start us in play mode */
	keyboard_macro_pos = &keyboard_macro[0];			/*    at the beginning */
	return TRUE;
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
int cmd_abort_command(int f, int n)
{
	tcapbeep();
	keyboard_macro_mode = STOP;
	msg_printf("(Aborted)");
	return ABORT;
}

/*
 * tell the user that this command is illegal while we are in
 * VIEW (read-only) mode
 */
int rdonly(void)
{
	tcapbeep();
	msg_printf("(Key illegal in VIEW mode)");
	return FALSE;
}

int resterr(void)
{
	tcapbeep();
	msg_printf("(That command is RESTRICTED)");
	return FALSE;
}

/* user function that does NOTHING */
int cmd_nop(int f, int n)
{
	return TRUE;
}

/* dummy function for binding to meta prefix */
int cmd_meta_prefix(int f, int n)
{
	return TRUE;
}

/* dummy function for binding to control-x prefix */
int cmd_ctlx_prefix(int f, int n)
{
	return TRUE;
}

/* dummy function for binding to universal-argument */
int cmd_universal_argument(int f, int n)
{
	return TRUE;
}
