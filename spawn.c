/*	spaw.c
 *
 *	Various operating system access commands.
 *
 *	<odified by Petri Kutvonen
 */

#include <stdio.h>
#include <unistd.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#include        <signal.h>
extern int chg_width, chg_height;
extern void sizesignal(int);

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
int spawncli(int f, int n)
{
	char *cp;

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	movecursor(term.t_nrow, 0);		/* Seek to last line.   */
	TTflush();
	TTclose();				/* stty to old settings */
	TTkclose();				/* Close "keyboard" */
	if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
		system(cp);
	else
		system("exec /bin/sh");
	sgarbf = TRUE;
	sleep(2);
	TTopen();
	TTkopen();

/*
 * This fools the update routines to force a full
 * redraw with complete window size checking.
 *		-lbt
 */
	chg_width = term.t_ncol;
	chg_height = term.t_nrow + 1;
	term.t_nrow = term.t_ncol = 0;
	return TRUE;
}

int bktoshell(int f, int n)
{						/* suspend MicroEMACS and wait to wake up */
	vttidy();
/******************************
	int pid;

	pid = getpid();
	kill(pid,SIGTSTP);
******************************/
	kill(0, SIGTSTP);
	return TRUE;
}

void rtfrmshell(void)
{
	TTopen();
	curwp->w_flag = WFHARD;
	sgarbf = TRUE;
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
int spawn(int f, int n)
{
	int s;
	char line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	if ((s = mlreply("!", line, NLINE)) != TRUE)
		return s;
	TTflush();
	TTclose();				/* stty to old modes    */
	TTkclose();
	system(line);
	fflush(stdout);				/* to be sure P.K.      */
	TTopen();

	if (clexec == FALSE) {
		mlputs("(End)");		/* Pause.               */
		TTflush();
		while ((s = tgetc()) != '\r' && s != ' ') ;
		mlputs("\r\n");
	}
	TTkopen();
	sgarbf = TRUE;
	return TRUE;
}

/*
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */

int execprg(int f, int n)
{
	int s;
	char line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	if ((s = mlreply("!", line, NLINE)) != TRUE)
		return s;
	TTputc('\n');				/* Already have '\r'    */
	TTflush();
	TTclose();				/* stty to old modes    */
	TTkclose();
	system(line);
	fflush(stdout);				/* to be sure P.K.      */
	TTopen();
	mlputs("(End)");			/* Pause.               */
	TTflush();
	while ((s = tgetc()) != '\r' && s != ' ') ;
	sgarbf = TRUE;
	return TRUE;
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int pipecmd(int f, int n)
{
	int s;					/* return status from CLI */
	struct window *wp;			/* pointer to new window */
	struct buffer *bp;			/* pointer to buffer to zot */
	char line[NLINE];			/* command line send to shell */
	static char bname[] = "command";

	static char filnam[NSTRING] = "command";

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	/* get the command to pipe in */
	if ((s = mlreply("@", line, NLINE)) != TRUE)
		return s;

	/* get rid of the command output buffer if it exists */
	if ((bp = bfind(bname, FALSE, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == bp) {
				if (wp == curwp)
					delwind(FALSE, 1);
				else
					onlywind(FALSE, 1);
				break;
			}
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)
			return FALSE;
	}

	TTflush();
	TTclose();				/* stty to old modes    */
	TTkclose();
	strcat(line, ">");
	strcat(line, filnam);
	system(line);
	TTopen();
	TTkopen();
	TTflush();
	sgarbf = TRUE;
	s = TRUE;

	if (s != TRUE)
		return s;

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
		return FALSE;

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return FALSE;

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

	/* and get rid of the temporary file */
	unlink(filnam);
	return TRUE;
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
int filter_buffer(int f, int n)
{
	int s;					/* return status from CLI */
	struct buffer *bp;			/* pointer to buffer to zot */
	char line[NLINE];			/* command line send to shell */
	char tmpnam[NFILEN];			/* place to store real file name */
	static char bname1[] = "fltinp";

	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	if (curbp->b_mode & MDVIEW)		/* don't allow this command if      */
		return rdonly();		/* we are in read only mode     */

	/* get the filter name and its args */
	if ((s = mlreply("#", line, NLINE)) != TRUE)
		return s;

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);		/* save the original name */
	strcpy(bp->b_fname, bname1);		/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1) != TRUE) {
		mlwrite("(Cannot write filter file)");
		strcpy(bp->b_fname, tmpnam);
		return FALSE;
	}
	TTputc('\n');				/* Already have '\r'    */
	TTflush();
	TTclose();				/* stty to old modes    */
	TTkclose();
	strcat(line, " <fltinp >fltout");
	system(line);
	TTopen();
	TTkopen();
	TTflush();
	sgarbf = TRUE;
	s = TRUE;

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2, FALSE) == FALSE)) {
		mlwrite("(Execution failed)");
		strcpy(bp->b_fname, tmpnam);
		unlink(filnam1);
		unlink(filnam2);
		return s;
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);		/* restore name */
	bp->b_flag |= BFCHG;			/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return TRUE;
}
