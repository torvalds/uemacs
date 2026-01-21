/*	window.c
 *
 *      Window management. Some of the functions are internal, and some are
 *      attached to keys that the user actually types.
 *
 */

#include <stdio.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"
#include "wrapper.h"

/*
 * Reposition dot in the current window to line "n". If the argument is
 * positive, it is that line. If it is negative it is that line from the
 * bottom. If it is 0 the window is centered (this is what the standard
 * redisplay code does). With no argument it defaults to 0. Bound to M-!.
 */
int reposition(int f, int n)
{
	if (f == FALSE)				/* default to 0 to center screen */
		n = 0;
	curwp->w_force = n;
	curwp->w_flag |= WFFORCE;
	return TRUE;
}

/*
 * Refresh the screen. With no argument, it just does the refresh. With an
 * argument it recenters "." in the current window. Bound to "C-L".
 */
int redraw(int f, int n)
{
	if (f == FALSE)
		sgarbf = TRUE;
	else {
		curwp->w_force = 0;		/* Center dot. */
		curwp->w_flag |= WFFORCE;
	}

	return TRUE;
}

/*
 * resize the screen, re-writing the screen
 *
 * int f;	default flag
 * int n;	numeric argument
 */
int newsize(int f, int n)
{
	struct window *wp;			/* current window being examined */

	/* if the command defaults, assume the largest */
	if (f == FALSE)
		n = term.t_mrow + 1;

	/* make sure it's in range */
	if (n < 3 || n > term.t_mrow + 1) {
		mlwrite("%%Screen size out of range");
		return FALSE;
	}

	if (term.t_nrow == n - 1)
		return TRUE;

	/* Update the current window as needed */
	wp = curwp;
	wp->w_ntrows = n - wp->w_toprow - 2;
	wp->w_flag |= WFHARD | WFMODE;

	/* screen is garbage */
	term.t_nrow = n - 1;
	sgarbf = TRUE;
	return TRUE;
}

/*
 * resize the screen, re-writing the screen
 *
 * int f;		default flag
 * int n;		numeric argument
 */
int newwidth(int f, int n)
{
	/* if the command defaults, assume the largest */
	if (f == FALSE)
		n = term.t_mcol;

	/* make sure it's in range */
	if (n < 10 || n > term.t_mcol) {
		mlwrite("%%Screen width out of range");
		return FALSE;
	}

	/* otherwise, just re-width it (no big deal) */
	term.t_ncol = n;
	term.t_margin = n / 10;
	term.t_scrsiz = n - (term.t_margin * 2);

	/* force window to redraw */
	curwp->w_flag |= WFHARD | WFMOVE | WFMODE;
	sgarbf = TRUE;

	return TRUE;
}
