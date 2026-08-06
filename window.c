/*	window.c
 *
 *      Window management. Some of the functions are internal, and some are
 *      attached to keys that the user actually types.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "estruct.h"
#include "globals.h"
#include "efunc.h"
#include "line.h"
#include "wrapper.h"

/*
 * A window is going away.  If it was the last one showing its buffer then
 * the buffer has to be told where the dot had got to, because a window is
 * the only other place that is written down.
 */
static void free_window(struct window *wp)
{
	struct buffer *bp = wp->w_bufp;

	if (--bp->b_nwnd == 0) {
		bp->b_dotp = wp->w_dotp;
		bp->b_doto = wp->w_doto;
		bp->b_markp = wp->w_markp;
		bp->b_marko = wp->w_marko;
	}
	free(wp);
}

/*
 * The window before this one on the list, or NULL if it is the first.
 * The list is singly linked and short, so this is a walk.
 */
static struct window *window_before(struct window *wp)
{
	struct window *prev = NULL;

	for (struct window *p = window_head; p && p != wp; p = p->w_wndp)
		prev = p;
	return prev;
}

/*
 * Make this the current window.  The mode lines of both the window being
 * left and the one being entered change, because they say which one you
 * are in.
 */
static int enter_window(struct window *wp)
{
	curwp = wp;
	curbp = wp->w_bufp;
	update_modeline();
	shown_buffer_changed();
	return TRUE;
}

/*
 * Split the current window in two.  A window of fewer than three rows
 * cannot be split, since each half needs a row of text and a mode line.
 * With no argument the cursor stays in whichever half the dot is already
 * in; an argument of 1 forces it to the upper window and 2 to the lower.
 * Bound to "C-x 2".
 */
int cmd_split_current_window(int f, int n)
{
	struct window *wp;
	struct line *lp;
	int upper, lower, rows;

	if (curwp->w_ntrows < 3) {
		msg_printf("Cannot split a %d line window", curwp->w_ntrows);
		return FALSE;
	}

	wp = xmalloc(sizeof(struct window));
	++curbp->b_nwnd;			/* Displayed twice. */
	wp->w_bufp = curbp;
	wp->w_dotp = curwp->w_dotp;
	wp->w_doto = curwp->w_doto;
	wp->w_markp = curwp->w_markp;
	wp->w_marko = curwp->w_marko;
	wp->w_flag = 0;
	wp->w_force = 0;

	/* One row goes to the new mode line, and the rest are shared. */
	upper = (curwp->w_ntrows - 1) / 2;
	lower = (curwp->w_ntrows - 1) - upper;

	/* how far down the window the dot is sitting */
	rows = 0;
	for (lp = curwp->w_linep; lp != curwp->w_dotp; lp = line_next(lp))
		++rows;

	lp = curwp->w_linep;
	if (f ? n == 1 : rows <= upper) {
		/* the window we were in becomes the upper one */
		if (rows == upper)		/* dot hit the mode line */
			lp = line_next(lp);
		curwp->w_ntrows = upper;
		wp->w_wndp = curwp->w_wndp;
		curwp->w_wndp = wp;
		wp->w_toprow = curwp->w_toprow + upper + 1;
		wp->w_ntrows = lower;
	} else {
		/* the window we were in becomes the lower one */
		struct window *prev = window_before(curwp);

		if (prev == NULL)
			window_head = wp;
		else
			prev->w_wndp = wp;
		wp->w_wndp = curwp;
		wp->w_toprow = curwp->w_toprow;
		wp->w_ntrows = upper;
		curwp->w_toprow += upper + 1;	/* and the mode line */
		curwp->w_ntrows = lower;
		for (int i = upper + 1; i--; )
			lp = line_next(lp);
	}

	curwp->w_linep = lp;			/* Adjust the top lines */
	wp->w_linep = lp;			/* if necessary.        */
	curwp->w_flag |= WFMODE | WFHARD;
	wp->w_flag |= WFMODE | WFHARD;
	return TRUE;
}

/*
 * Make the next window down the screen the current one, wrapping round to
 * the top.  With an argument it is that window counting from the top, or
 * from the bottom if the argument is negative.  Bound to "C-x O".
 */
int cmd_next_window(int f, int n)
{
	struct window *wp;

	if (!f) {
		wp = curwp->w_wndp;
		if (wp == NULL)
			wp = window_head;
		return enter_window(wp);
	}

	int count = 0;
	for (wp = window_head; wp != NULL; wp = wp->w_wndp)
		count++;
	if (n < 0)				/* from the bottom */
		n = count + n + 1;
	if (n < 1 || n > count) {
		msg_printf("Window number out of range");
		return FALSE;
	}
	wp = window_head;
	while (--n)
		wp = wp->w_wndp;
	return enter_window(wp);
}

/*
 * Make the previous window up the screen the current one.  Bound to
 * "C-x P".  With one window this does very little.
 */
int cmd_previous_window(int f, int n)
{
	struct window *wp;

	if (f)					/* nth from the bottom */
		return cmd_next_window(f, -n);

	wp = window_before(curwp);
	if (wp == NULL) {			/* already at the top */
		for (wp = window_head; wp->w_wndp != NULL; wp = wp->w_wndp)
			;
	}
	return enter_window(wp);
}

/*
 * Make the current window the only one, giving it the whole screen.
 * Bound to "C-x 1".
 */
int cmd_delete_other_windows(int f, int n)
{
	struct window *wp;
	struct line *lp;
	int i;

	while (window_head != curwp) {		/* the ones above */
		wp = window_head;
		window_head = wp->w_wndp;
		free_window(wp);
	}
	while (curwp->w_wndp != NULL) {		/* and the ones below */
		wp = curwp->w_wndp;
		curwp->w_wndp = wp->w_wndp;
		free_window(wp);
	}

	/*
	 * The window keeps the rows it had and gains the rest, so the text
	 * that was in it stays where it is on the screen: back up the top
	 * line by as far as the window is about to move up.
	 */
	lp = curwp->w_linep;
	i = curwp->w_toprow;
	while (i != 0 && line_prev(lp) != curbp->b_linep) {
		--i;
		lp = line_prev(lp);
	}
	curwp->w_toprow = 0;
	curwp->w_ntrows = term.t_nrow - 1;
	curwp->w_linep = lp;
	curwp->w_flag |= WFMODE | WFHARD;
	return TRUE;
}

/*
 * Delete the current window, giving its rows to the window above, or to
 * the one below if it was the top window.  Bound to "C-x 0".
 */
int cmd_delete_window(int f, int n)
{
	struct window *wp, *prev;
	int target;

	if (window_head->w_wndp == NULL) {
		msg_printf("Can not delete this window");
		return FALSE;
	}

	prev = window_before(curwp);

	/* find the neighbour that takes the space, and give it */
	if (curwp->w_toprow == 0) {
		target = curwp->w_ntrows + 1;	/* the one below */
		for (wp = window_head; wp != NULL; wp = wp->w_wndp)
			if (wp->w_toprow == target)
				break;
		if (wp == NULL)
			return FALSE;
		wp->w_toprow = 0;
		wp->w_ntrows += target;
	} else {
		target = curwp->w_toprow - 1;	/* the one above */
		for (wp = window_head; wp != NULL; wp = wp->w_wndp)
			if (wp->w_toprow + wp->w_ntrows == target)
				break;
		if (wp == NULL)
			return FALSE;
		wp->w_ntrows += 1 + curwp->w_ntrows;
	}

	if (prev == NULL)
		window_head = curwp->w_wndp;
	else
		prev->w_wndp = curwp->w_wndp;
	free_window(curwp);

	wp->w_flag |= WFMODE | WFHARD;
	return enter_window(wp);
}

/*
 * The window next to this one, which is the one that gives up rows when
 * this one grows and takes them when it shrinks: the one below, or the
 * one above if this is the bottom window.  NULL if there is only one.
 */
static struct window *adjacent_window(void)
{
	if (window_head->w_wndp == NULL)
		return NULL;
	if (curwp->w_wndp != NULL)
		return curwp->w_wndp;
	return window_before(curwp);
}

/*
 * Grow the current window by "n" rows, at the expense of its neighbour.
 * Bound to "C-x Z" and "C-x ^".
 */
int cmd_grow_window(int f, int n)
{
	struct window *adjwp;
	struct line *lp;
	int i;

	if (n < 0)
		return cmd_shrink_window(f, -n);
	adjwp = adjacent_window();
	if (adjwp == NULL) {
		msg_printf("Only one window");
		return FALSE;
	}
	if (adjwp->w_ntrows <= n) {
		msg_printf("Impossible change");
		return FALSE;
	}

	if (curwp->w_wndp == adjwp) {		/* shrink the one below */
		lp = adjwp->w_linep;
		for (i = 0; i < n && lp != adjwp->w_bufp->b_linep; ++i)
			lp = line_next(lp);
		adjwp->w_linep = lp;
		adjwp->w_toprow += n;
	} else {				/* shrink the one above */
		lp = curwp->w_linep;
		for (i = 0; i < n && line_prev(lp) != curbp->b_linep; ++i)
			lp = line_prev(lp);
		curwp->w_linep = lp;
		curwp->w_toprow -= n;
	}
	curwp->w_ntrows += n;
	adjwp->w_ntrows -= n;
	curwp->w_flag |= WFMODE | WFHARD;
	adjwp->w_flag |= WFMODE | WFHARD;
	return TRUE;
}

/*
 * Shrink the current window by "n" rows, giving them to its neighbour.
 * Bound to "C-x C-Z".
 */
int cmd_shrink_window(int f, int n)
{
	struct window *adjwp;
	struct line *lp;
	int i;

	if (n < 0)
		return cmd_grow_window(f, -n);
	adjwp = adjacent_window();
	if (adjwp == NULL) {
		msg_printf("Only one window");
		return FALSE;
	}
	if (curwp->w_ntrows <= n) {
		msg_printf("Impossible change");
		return FALSE;
	}

	if (curwp->w_wndp == adjwp) {		/* grow the one below */
		lp = adjwp->w_linep;
		for (i = 0; i < n && line_prev(lp) != adjwp->w_bufp->b_linep; ++i)
			lp = line_prev(lp);
		adjwp->w_linep = lp;
		adjwp->w_toprow -= n;
	} else {				/* grow the one above */
		lp = curwp->w_linep;
		for (i = 0; i < n && lp != curbp->b_linep; ++i)
			lp = line_next(lp);
		curwp->w_linep = lp;
		curwp->w_toprow += n;
	}
	curwp->w_ntrows -= n;
	adjwp->w_ntrows += n;
	curwp->w_flag |= WFMODE | WFHARD;
	adjwp->w_flag |= WFMODE | WFHARD;
	return TRUE;
}

/*
 * Move the frame of the current window up by "n" lines, leaving the dot
 * where it is if it is still on the screen afterwards and centring it if
 * it is not.  This moves the window over the text, not the text through
 * the window.  Bound to "C-x C-P".
 */
int cmd_move_window_up(int f, int n)
{
	struct line *lp = curwp->w_linep;
	int i;

	if (n < 0) {
		while (n++ && lp != curbp->b_linep)
			lp = line_next(lp);
	} else {
		while (n-- && line_prev(lp) != curbp->b_linep)
			lp = line_prev(lp);
	}
	/*
	 * The mode line has to be redrawn too, whatever upstream's "Mode
	 * line is OK" said: the Top/Bot/percentage it ends with is worked
	 * out from w_linep, which is exactly what just moved.
	 */
	curwp->w_linep = lp;
	curwp->w_flag |= WFHARD | WFMODE;

	/* if the dot is still showing, leave it alone */
	for (i = 0; i < curwp->w_ntrows; ++i) {
		if (lp == curwp->w_dotp)
			return TRUE;
		if (lp == curbp->b_linep)
			break;
		lp = line_next(lp);
	}

	/* it is not, so put it in the middle of where we are looking */
	lp = curwp->w_linep;
	i = curwp->w_ntrows / 2;
	while (i-- && lp != curbp->b_linep)
		lp = line_next(lp);
	curwp->w_dotp = lp;
	curwp->w_doto = 0;
	return TRUE;
}

/*
 * The same thing the other way up.  Bound to "C-x C-N".
 */
int cmd_move_window_down(int f, int n)
{
	return cmd_move_window_up(f, -n);
}

/*
 * Make the current window "n" rows tall.  Without an argument there is no
 * size to change to, so nothing happens.  Bound to "C-x W".
 */
int cmd_resize_window(int f, int n)
{
	if (f == FALSE)
		return TRUE;
	if (curwp->w_ntrows == n)
		return TRUE;
	return cmd_grow_window(TRUE, n - curwp->w_ntrows);
}

/*
 * Page the *other* window, without leaving this one - which is the point
 * of having two windows open on different things.  Bound to "M-C-Z" and
 * "M-C-V".
 */
int cmd_scroll_next_up(int f, int n)
{
	cmd_next_window(FALSE, 1);
	cmd_previous_page(f, n);
	cmd_previous_window(FALSE, 1);
	return TRUE;
}

int cmd_scroll_next_down(int f, int n)
{
	cmd_next_window(FALSE, 1);
	cmd_next_page(f, n);
	cmd_previous_window(FALSE, 1);
	return TRUE;
}

/*
 * Remember the current window, and come back to it later.  Not bound to
 * anything: these are for macros, which is why the saved window is checked
 * against the list rather than trusted - the window it names may have been
 * closed in between.
 */
int cmd_save_window(int f, int n)
{
	saved_window = curwp;
	return TRUE;
}

int cmd_restore_window(int f, int n)
{
	struct window *wp;

	for (wp = window_head; wp != NULL; wp = wp->w_wndp) {
		if (wp == saved_window)
			return enter_window(wp);
	}
	msg_printf("(No such window exists)");
	return FALSE;
}

/*
 * Reposition dot in the current window to line "n". If the argument is
 * positive, it is that line. If it is negative it is that line from the
 * bottom. If it is 0 the window is centered (this is what the standard
 * redisplay code does). With no argument it defaults to 0. Bound to M-!.
 */
int cmd_redraw_display(int f, int n)
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
int cmd_clear_and_redraw(int f, int n)
{
	if (f == FALSE)
		screen_garbage = TRUE;
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
int cmd_change_screen_size(int f, int n)
{
	struct window *wp;			/* current window being examined */

	/* if the command defaults, assume the largest */
	if (f == FALSE)
		n = term.t_mrow + 1;

	/* make sure it's in range */
	if (n < 3 || n > term.t_mrow + 1) {
		msg_printf("%%Screen size out of range");
		return FALSE;
	}

	if (term.t_nrow == n - 1)
		return TRUE;

	if (term.t_nrow < n - 1) {
		/* getting taller: the bottom window takes the new rows */
		for (wp = window_head; wp->w_wndp != NULL; wp = wp->w_wndp)
			;
		wp->w_ntrows = n - wp->w_toprow - 2;
		wp->w_flag |= WFHARD | WFMODE;
	} else {
		/*
		 * Getting shorter, so windows that no longer fit have to
		 * go, and the last one that does fit gets trimmed.  The
		 * current window can be one of the ones that goes, so it
		 * may have to be moved before its memory is freed.
		 */
		struct window *next, *last = NULL;

		for (wp = window_head; wp != NULL; wp = next) {
			next = wp->w_wndp;
			if (wp->w_toprow > n - 2) {
				if (wp == curwp)
					curwp = window_head;
				if (last != NULL)
					last->w_wndp = NULL;
				free_window(wp);
				continue;
			}
			if (wp->w_toprow + wp->w_ntrows - 1 >= n - 2) {
				wp->w_ntrows = n - wp->w_toprow - 2;
				wp->w_flag |= WFHARD | WFMODE;
			}
			last = wp;
		}
		curbp = curwp->w_bufp;
	}

	/* screen is garbage */
	term.t_nrow = n - 1;
	screen_garbage = TRUE;
	return TRUE;
}

/*
 * resize the screen, re-writing the screen
 *
 * int f;		default flag
 * int n;		numeric argument
 */
int cmd_change_screen_width(int f, int n)
{
	/* if the command defaults, assume the largest */
	if (f == FALSE)
		n = term.t_mcol;

	/* make sure it's in range */
	if (n < 10 || n > term.t_mcol) {
		msg_printf("%%Screen width out of range");
		return FALSE;
	}

	/* otherwise, just re-width it (no big deal) */
	term.t_ncol = n;
	term.t_margin = n / 10;
	term.t_scrsiz = n - (term.t_margin * 2);

	/* force window to redraw */
	curwp->w_flag |= WFHARD | WFMOVE | WFMODE;
	screen_garbage = TRUE;

	return TRUE;
}
