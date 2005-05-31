/*	TCAP.C
 *
 *	Unix V7 SysV and BS4 Termcap video driver
 *
 *	modified by Petri Kutvonen
 */

/*
 * defining this to 1 breaks tcapopen() - it doesn't check if the
 * sceen size has changed.
 *	-lbt
 */
#define USE_BROKEN_OPTIMIZATION 0
#define	termdef	1			/* don't define "term" external */

#include <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if TERMCAP

#if UNIX
#include <signal.h>
#endif

#define	MARGIN	8
#define	SCRSIZ	64
#define	NPAUSE	10			/* # times thru update to pause */
#define BEL     0x07
#define ESC     0x1B

extern int      ttopen();
extern int      ttgetc();
extern int      ttputc();
extern int	tgetnum();
extern int      ttflush();
extern int      ttclose();
extern int	tcapkopen();
extern int	tcapkclose();
extern int      tcapmove();
extern int      tcapeeol();
extern int      tcapeeop();
extern int      tcapbeep();
extern int	tcaprev();
extern int	tcapcres();
extern int      tcapopen();
#if	PKCODE
extern int	tcapclose();
#endif
extern int      tput();
extern char     *tgoto();
#if	COLOR
extern	int	tcapfcol();
extern	int	tcapbcol();
#endif
#if     SCROLLCODE
extern  int     tcapscroll_reg();
extern  int     tcapscroll_delins();
#endif


#define TCAPSLEN 315
char tcapbuf[TCAPSLEN];
char *UP, PC, *CM, *CE, *CL, *SO, *SE;

#if	PKCODE
char *TI, *TE;
int term_init_ok = 0;
#endif

#if     SCROLLCODE
char *CS, *DL, *AL, *SF, *SR;
#endif

TERM term = {
	0,	/* these four values are set dynamically at open time */
	0,
	0,
	0,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        tcapopen,
#if	PKCODE
	tcapclose,
#else
        ttclose,
#endif
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
        tcapcres
#if	COLOR
	, tcapfcol,
	tcapbcol
#endif
#if     SCROLLCODE 
        , NULL          /* set dynamically at open time */
#endif
};

tcapopen()

{
        char *getenv();
        char *t, *p, *tgetstr();
        char tcbuf[1024];
        char *tv_stype;
        char err_str[72];
        int int_col, int_row;

#if PKCODE && USE_BROKEN_OPTIMIZATION
	if (! term_init_ok)
	{
#endif
        if ((tv_stype = getenv("TERM")) == NULL)
        {
                puts("Environment variable TERM not defined!");
                exit(1);
        }

        if ((tgetent(tcbuf, tv_stype)) != 1)
        {
                sprintf(err_str, "Unknown terminal type %s!", tv_stype);
                puts(err_str);
                exit(1);
        }

	/* Get screen size from system, or else from termcap.  */
	getscreensize(&int_col, &int_row);
	term.t_nrow = int_row-1;
	term.t_ncol = int_col;
 
       if ((term.t_nrow <= 0) && (term.t_nrow=(short)tgetnum("li")-1) == -1){
               puts("termcap entry incomplete (lines)");
               exit(1);
       }

       if ((term.t_ncol <= 0) && (term.t_ncol=(short)tgetnum("co")) == -1){
               puts("Termcap entry incomplete (columns)");
               exit(1);
       }

#ifdef SIGWINCH
        term.t_mrow =  MAXROW;   
        term.t_mcol = MAXCOL;
#else
        term.t_mrow =  term.t_nrow > MAXROW ? MAXROW : term.t_nrow;
        term.t_mcol =  term.t_ncol > MAXCOL ? MAXCOL : term.t_ncol;
#endif
        p = tcapbuf;
        t = tgetstr("pc", &p);
        if(t)
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
#if	PKCODE
	if (tgetnum("sg") > 0) {		/* can reverse be used? P.K. */
		revexist = FALSE;
		SE = NULL;
		SO = NULL;
	}
	TI = tgetstr("ti", &p);		/* terminal init and exit */
	TE = tgetstr("te", &p);
#endif

        if(CL == NULL || CM == NULL || UP == NULL)
        {
                puts("Incomplete termcap entry\n");
                exit(1);
        }

	if (CE == NULL)		/* will we be able to use clear to EOL? */
		eolexist = FALSE;
#if SCROLLCODE 
	CS = tgetstr("cs", &p);
	SF = tgetstr("sf", &p);
	SR = tgetstr("sr", &p);
	DL = tgetstr("dl", &p);
	AL = tgetstr("al", &p);
     
	if (CS && SR) {
        	if (SF == NULL) /* assume '\n' scrolls forward */
			SF = "\n";
                term.t_scroll = tcapscroll_reg;
	} else if (DL && AL) {
		term.t_scroll = tcapscroll_delins;
	} else {
		term.t_scroll = NULL;
	}
#endif
		
        if (p >= &tcapbuf[TCAPSLEN])
        {
                puts("Terminal description too big!\n");
                exit(1);
        }
#if PKCODE && USE_BROKEN_OPTIMIZATION
	term_init_ok = 1;
	}
#endif
        ttopen();
}

#if	PKCODE

tcapclose()

{
	putpad(tgoto(CM, 0, term.t_nrow));
	putpad(TE);
	ttflush();
	ttclose();
}
#endif

tcapkopen()

{
#if	PKCODE
	putpad(TI);
	ttflush();
#endif
	strcpy(sres, "NORMAL");
}

tcapkclose()

{
}

tcapmove(row, col)
register int row, col;
{
        putpad(tgoto(CM, col, row));
}

tcapeeol()
{
        putpad(CE);
}

tcapeeop()
{
        putpad(CL);
}

tcaprev(state)		/* change reverse video status */

int state;		/* FALSE = normal video, TRUE = reverse video */

{
	static int revstate = FALSE;
	if (state) {
		if (SO != NULL)
			putpad(SO);
	} else
		if (SE != NULL)
			putpad(SE);
}

tcapcres()	/* change screen resolution */

{
	return(TRUE);
}

#if SCROLLCODE 
 
/* move howmanylines lines starting at from to to */
tcapscroll_reg(from,to,howmanylines)
{ 
        int i; 
        if (to == from) return;
        if (to < from) {
                tcapscrollregion(to, from + howmanylines - 1);
                tcapmove(from + howmanylines - 1,0);
                for (i = from - to; i > 0; i--)
                        putpad(SF);
        } else { /* from < to */
                tcapscrollregion(from, to + howmanylines - 1);
                tcapmove(from,0);
                for (i = to - from; i > 0; i--)
                        putpad(SR);
        } 
	tcapscrollregion(0, term.t_nrow);
}
        
/* move howmanylines lines starting at from to to */
tcapscroll_delins(from,to,howmanylines)
{
        int i;
        if (to == from) return;
        if (to < from) {
                tcapmove(to,0);
                for (i = from - to; i > 0; i--)
                        putpad(DL);
                tcapmove(to+howmanylines,0);
                for (i = from - to; i > 0; i--)
                        putpad(AL);
        } else {
                tcapmove(from+howmanylines,0);
                for (i = to - from; i > 0; i--)
                        putpad(DL);
                tcapmove(from,0);
                for (i = to - from; i > 0; i--)
                        putpad(AL);
        }
}
                
/* cs is set up just like cm, so we use tgoto... */
tcapscrollregion(top,bot)
{
	ttputc(PC);
        putpad(tgoto(CS, bot, top));
}
                
#endif

spal(dummy)	/* change palette string */

{
	/*	Does nothing here	*/
}

#if	COLOR
tcapfcol()	/* no colors here, ignore this */
{
}

tcapbcol()	/* no colors here, ignore this */
{
}
#endif

tcapbeep()
{
	ttputc(BEL);
}

putpad(str)
char    *str;
{
	tputs(str, 1, ttputc);
}

putnpad(str, n)
char    *str;
{
	tputs(str, n, ttputc);
}


#if	FNLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else

hello()
{
}

#endif
