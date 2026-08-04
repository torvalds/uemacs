/*	spaw.c
 *
 *	Various operating system access commands.
 *
 *	<odified by Petri Kutvonen
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "estruct.h"
#include "globals.h"
#include "efunc.h"

#include        <signal.h>

/*
 * Remember what a subprocess exited with, for $rval.  system() hands
 * back a wait status rather than an exit code, so report it the way a
 * shell reports $? - the exit status, or 128 plus the signal that
 * killed it.
 */
static void record_status(int status)
{
	if (status < 0)				/* system() itself failed */
		subprocess_status = -1;
	else if (WIFEXITED(status))
		subprocess_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		subprocess_status = 128 + WTERMSIG(status);
}

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
int cmd_interactive_shell(int f, int n)
{
	char *cp;

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	movecursor(term.t_nrow, 0);		/* Seek to last line.   */
	ttflush();
	tcapclose();				/* stty to old settings */
	tcapkclose();				/* Close "keyboard" */
	if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
		record_status(system(cp));
	else
		record_status(system("exec /bin/sh"));
	screen_garbage = TRUE;
	sleep(2);
	tcapopen();
	tcapkopen();
	return TRUE;
}

int cmd_suspend_emacs(int f, int n)
{						/* suspend MicroEMACS and wait to wake up */
	display_close();
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
	tcapopen();
	curwp->w_flag = WFHARD;
	screen_garbage = TRUE;
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
int cmd_shell_command(int f, int n)
{
	int s;
	char line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	if ((s = ask_string("!", line, NLINE)) != TRUE)
		return s;
	ttflush();
	tcapclose();				/* stty to old modes    */
	tcapkclose();
	record_status(system(line));
	fflush(stdout);				/* to be sure P.K.      */
	tcapopen();

	if (executing_command_line == FALSE) {
		msg_append("(End)");		/* Pause.               */
		ttflush();
		while ((s = tgetc()) != '\r' && s != ' ') ;
		msg_append("\r\n");
	}
	tcapkopen();
	screen_garbage = TRUE;
	return TRUE;
}

/*
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */

int cmd_execute_program(int f, int n)
{
	int s;
	char line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	if ((s = ask_string("!", line, NLINE)) != TRUE)
		return s;
	ttputc('\n');				/* Already have '\r'    */
	ttflush();
	tcapclose();				/* stty to old modes    */
	tcapkclose();
	record_status(system(line));
	fflush(stdout);				/* to be sure P.K.      */
	tcapopen();
	msg_append("(End)");			/* Pause.               */
	ttflush();
	while ((s = tgetc()) != '\r' && s != ' ') ;
	screen_garbage = TRUE;
	return TRUE;
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
/*
 * Run a command and read what it printed into a window of its own, in
 * view mode.  Bound to "C-x @".
 *
 * The output goes through a file called "command" in the current
 * directory, which is how filter-buffer's fltinp and fltout work two
 * functions down; all three share the same weakness about where they put
 * it and what happens if something is there already.
 */
int cmd_pipe_command(int f, int n)
{
	struct window *wp;
	struct buffer *bp;
	char line[NLINE];
	int s;
	static char bname[] = "command";
	static char filnam[] = "command";

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	if ((s = ask_string("@", line, NLINE)) != TRUE)
		return s;

	/* if the last one is still around, get it off the screen and go */
	bp = find_buffer(bname, FALSE, 0);
	if (bp != NULL) {
		for (wp = window_head; wp != NULL; wp = wp->w_wndp) {
			if (wp->w_bufp != bp)
				continue;
			if (wp == curwp)
				cmd_delete_window(FALSE, 1);
			else
				cmd_delete_other_windows(FALSE, 1);
			break;
		}
		if (zotbuf(bp) != TRUE)
			return FALSE;
	}

	ttflush();
	tcapclose();				/* stty to old modes    */
	tcapkclose();
	/*
	 * The space before the '>' matters: without it a command ending in
	 * a digit has that digit read as a file descriptor number, so "seq
	 * 4" becomes "seq" with fd 4 redirected and nothing comes back.
	 * filter-buffer below has always had the space.
	 */
	strcat(line, " >");
	strcat(line, filnam);
	record_status(system(line));
	tcapopen();
	tcapkopen();
	ttflush();
	screen_garbage = TRUE;

	if (cmd_split_current_window(FALSE, 1) == FALSE)
		return FALSE;
	if (getfile(filnam, FALSE) == FALSE)
		return FALSE;

	curwp->w_bufp->b_mode |= MDVIEW;
	update_modeline();
	unlink(filnam);
	return TRUE;
}

int cmd_filter_buffer(int f, int n)
{
	int s;					/* return status from CLI */
	struct buffer *bp;			/* pointer to buffer to zot */
	char line[NLINE];			/* command line send to shell */
	char tmpnam[NFILEN];			/* place to store real file name */
	struct filestate tmpstate;		/* and the state that goes with it */
	static char bname1[] = "fltinp";

	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";

	/* don't allow this command if restricted */
	if (restflag)
		return resterr();

	if (curbp->b_mode & MDVIEW)		/* don't allow this command if      */
		return rdonly();		/* we are in read only mode     */

	/* get the filter name and its args */
	if ((s = ask_string("#", line, NLINE)) != TRUE)
		return s;

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);		/* save the original name */
	tmpstate = bp->b_fstate;		/* and what we know about it */
	strcpy(bp->b_fname, bname1);		/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1) != TRUE) {
		msg_printf("(Cannot write filter file)");
		strcpy(bp->b_fname, tmpnam);
		bp->b_fstate = tmpstate;
		return FALSE;
	}
	ttputc('\n');				/* Already have '\r'    */
	ttflush();
	tcapclose();				/* stty to old modes    */
	tcapkclose();
	strcat(line, " <fltinp >fltout");
	record_status(system(line));
	tcapopen();
	tcapkopen();
	ttflush();
	screen_garbage = TRUE;
	s = TRUE;

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2, FALSE) == FALSE)) {
		msg_printf("(Execution failed)");
		strcpy(bp->b_fname, tmpnam);
		bp->b_fstate = tmpstate;
		unlink(filnam1);
		unlink(filnam2);
		return s;
	}

	/*
	 * Reset file name.  The state goes back with it: readin() has just
	 * recorded what the filter's output file looked like, which says
	 * nothing at all about the file the buffer is really for - and we
	 * have not touched that one, so what we knew about it still holds.
	 */
	strcpy(bp->b_fname, tmpnam);		/* restore name */
	bp->b_fstate = tmpstate;
	bp->b_flag |= BFCHG;			/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return TRUE;
}
