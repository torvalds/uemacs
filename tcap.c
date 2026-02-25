/*	tcap.c
 *
 *	Unix V7 SysV and BS4 Termcap video driver
 *
 *	modified by Petri Kutvonen
 */

/*
 * Defining this to 1 breaks tcapopen() - it doesn't check if the
 * sceen size has changed.
 *	-lbt
 */
#define	termdef	1				/* Don't define "term" external. */

#include <curses.h>
#include <stdio.h>
#include <term.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#include <signal.h>

#define	MARGIN	8
#define	SCRSIZ	64
#define	NPAUSE	2				/* Pause in 0.1 seconds */
#define BEL     0x07
#define ESC     0x1B

static void tcapkopen(void);
static void tcapkclose(void);
static void tcapmove(int, int);
static void tcapeeol(void);
static void tcapeeop(void);
static void tcapbeep(void);
static void tcaprev(int);
static void putpad(char *str);

static void tcapopen(void);
static void tcapclose(void);

#define TCAPSLEN 315
static char tcapbuf[TCAPSLEN];
static char *UP, PC, *CM, *CE, *CL, *SO, *SE;

static char *TI, *TE;

struct terminal term = {
	0,					/* These four values are set dynamically at open time. */
	0,
	0,
	0,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	tcapopen,
	tcapclose,
	tcapkopen,
	tcapkclose,
	ttgetc,
	ttputc,
	ttflush,
	tcapmove,
	tcapeeol,
	tcapeeop,
	tcapbeep,
	tcaprev,
};

static void tcapopen(void)
{
	char *t, *p;
	char tcbuf[1024];
	char *tv_stype;
	char err_str[72];
	int int_col, int_row;

	if ((tv_stype = getenv("TERM")) == NULL) {
		puts("Environment variable TERM not defined!");
		exit(1);
	}

	if ((tgetent(tcbuf, tv_stype)) != 1) {
		sprintf(err_str, "Unknown terminal type %s!", tv_stype);
		puts(err_str);
		exit(1);
	}

	/* Get screen size from system, or else from termcap.  */
	getscreensize(&int_col, &int_row);
	term.t_nrow = int_row - 1;
	term.t_ncol = int_col;

	if ((term.t_nrow <= 0)
	    && (term.t_nrow = (short)tgetnum("li") - 1) == -1) {
		puts("termcap entry incomplete (lines)");
		exit(1);
	}

	if ((term.t_ncol <= 0)
	    && (term.t_ncol = (short)tgetnum("co")) == -1) {
		puts("Termcap entry incomplete (columns)");
		exit(1);
	}
	term.t_mrow = MAXROW;
	term.t_mcol = MAXCOL;
	p = tcapbuf;
	t = tgetstr("pc", &p);
	if (t)
		PC = *t;
	else
		PC = 0;

	CL = tgetstr("cl", &p);
	CM = tgetstr("cm", &p);
	CE = tgetstr("ce", &p);
	UP = tgetstr("up", &p);
	SE = tgetstr("se", &p);
	SO = tgetstr("so", &p);
	if (SO != NULL)
		revexist = TRUE;

	if (tgetnum("sg") > 0) {		/* can reverse be used? P.K. */
		revexist = FALSE;
		SE = NULL;
		SO = NULL;
	}
	TI = tgetstr("ti", &p);			/* terminal init and exit */
	TE = tgetstr("te", &p);

	if (CL == NULL || CM == NULL || UP == NULL) {
		puts("Incomplete termcap entry\n");
		exit(1);
	}

	if (CE == NULL)				/* will we be able to use clear to EOL? */
		eolexist = FALSE;

	if (p >= &tcapbuf[TCAPSLEN]) {
		puts("Terminal description too big!\n");
		exit(1);
	}
	ttopen();
}

static void tcapclose(void)
{
	putpad(tgoto(CM, 0, term.t_nrow));
	putpad(TE);
	ttflush();
	ttclose();
}

static void tcapkopen(void)
{
	putpad(TI);
	ttflush();
	ttrow = 999;
	ttcol = 999;
	sgarbf = TRUE;
}

static void tcapkclose(void)
{
	putpad(TE);
	ttflush();
}

static void tcapmove(int row, int col)
{
	putpad(tgoto(CM, col, row));
}

static void tcapeeol(void)
{
	putpad(CE);
}

static void tcapeeop(void)
{
	putpad(CL);
}

/*
 * Change reverse video status
 *
 * @state: FALSE = normal video, TRUE = reverse video.
 */
static void tcaprev(int state)
{
	if (state) {
		if (SO != NULL)
			putpad(SO);
	} else if (SE != NULL)
		putpad(SE);
}

static void tcapbeep(void)
{
	ttputc(BEL);
}

static void putpad(char *str)
{
	tputs(str, 1, ttputc);
}
