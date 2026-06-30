/*	ibmpc.c
 *
 * The routines in this file provide support for the IBM-PC and other
 * compatible terminals. It goes directly to the graphics RAM to do
 * screen output. It compiles into nothing if not an IBM-PC driver
 * Supported monitor cards include CGA, MONO and EGA.
 *
 *	modified by Petri Kutvonen
 */

#define	termdef	1		/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if     IBMPC
#if	PKCODE
#define NROW	50
#else
#define NROW	43		/* Max Screen size.             */
#endif
#define NCOL    80		/* Edit if you want to.         */
#define	MARGIN	8		/* size of minimim margin and   */
#define	SCRSIZ	64		/* scroll size for extended lines */
#define	NPAUSE	200		/* # times thru update to pause */
#define BEL     0x07		/* BEL character.               */
#define ESC     0x1B		/* ESC character.               */
#define	SPACE	32		/* space character              */

#define	SCADC	0xb8000000L	/* CGA address of screen RAM    */
#define	SCADM	0xb0000000L	/* MONO address of screen RAM   */
#define SCADE	0xb8000000L	/* EGA address of screen RAM    */

#define MONOCRSR 0x0B0D		/* monochrome cursor        */
#define CGACRSR 0x0607		/* CGA cursor               */
#define EGACRSR 0x0709		/* EGA cursor               */

#define	CDCGA	0		/* color graphics card          */
#define	CDMONO	1		/* monochrome text card         */
#define	CDEGA	2		/* EGA color adapter            */
#if	PKCODE
#define	CDVGA	3
#endif
#define	CDSENSE	9		/* detect the card type         */

#if	PKCODE
#define NDRIVE	4
#else
#define NDRIVE	3		/* number of screen drivers     */
#endif

int dtype = -1;			/* current display type         */
char drvname[][8] = {		/* screen resolution names      */
	"CGA", "MONO", "EGA"
#if	PKCODE
	    , "VGA"
#endif
};
long scadd;			/* address of screen ram        */
int *scptr[NROW];		/* pointer to screen lines      */
unsigned int sline[NCOL];	/* screen line image            */
int egaexist = FALSE;		/* is an EGA card available?    */
extern union REGS rg;		/* cpu register for use of DOS calls */

extern int ttopen();		/* Forward references.          */
extern int ttgetc();
extern int ttputc();
extern int ttflush();
extern int ttclose();
extern int ibmmove();
extern int ibmeeol();
extern int ibmeeop();
extern int ibmbeep();
extern int ibmopen();
extern int ibmrev();
extern int ibmcres();
extern int ibmclose();
extern int ibmputc();
extern int ibmkopen();
extern int ibmkclose();

#if	COLOR
extern int ibmfcol();
extern int ibmbcol();
extern int ibmscroll_reg();

int cfcolor = -1;		/* current forground color */
int cbcolor = -1;		/* current background color */
int ctrans[] =			/* ansi to ibm color translation table */
#if	PKCODE
{ 0, 4, 2, 6, 1, 5, 3, 7, 15 };
#else
{ 0, 4, 2, 6, 1, 5, 3, 7 };
#endif
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
struct terminal term = {
	NROW - 1,
	NROW - 1,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	ibmopen,
	ibmclose,
	ibmkopen,
	ibmkclose,
	ttgetc,
	ibmputc,
	ttflush,
	ibmmove,
	ibmeeol,
	ibmeeop,
	ibmbeep,
	ibmrev,
	ibmcres
#if	COLOR
	    , ibmfcol,
	ibmbcol
#endif
#if     SCROLLCODE
	    , ibmscroll_reg
#endif
};

#if	COLOR
/* Set the current output color.
 *
 * @color: color to set.
 */
void ibmfcol(int color)
{
	cfcolor = ctrans[color];
}

/* Set the current background color.
 *
 * @color: color to set.
 */
void ibmbcol(int color)
{
	cbcolor = ctrans[color];
}
#endif

void ibmmove(int row, int col)
{
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col;
	rg.h.dh = row;
	rg.h.bh = 0;		/* set screen page number */
	int86(0x10, &rg, &rg);
}

void ibmeeol(void)
{				/* erase to the end of the line */
	unsigned int attr;	/* attribute byte mask to place in RAM */
	unsigned int *lnptr;	/* pointer to the destination line */
	int i;
	int ccol;		/* current column cursor lives */
	int crow;		/*         row  */

	/* find the current cursor position */
	rg.h.ah = 3;		/* read cursor position function code */
	rg.h.bh = 0;		/* current video page */
	int86(0x10, &rg, &rg);
	ccol = rg.h.dl;		/* record current column */
	crow = rg.h.dh;		/* and row */

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((cbcolor & 15) << 4) | (cfcolor & 15)) << 8;
	else
		attr = 0x0700;
#else
	attr = 0x0700;
#endif
	lnptr = &sline[0];
	for (i = 0; i < term.t_ncol; i++)
		*lnptr++ = SPACE | attr;

	if (flickcode && (dtype == CDCGA)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8));

		/* and to be back on */
		while ((inp(0x3da) & 8) == 0);
	}

	/* and send the string out */
	movmem(&sline[0], scptr[crow] + ccol, (term.t_ncol - ccol) * 2);

}

/* Put a character at the current position in the current colors */
void ibmputc(int ch)
{
	rg.h.ah = 14;		/* write char to screen with current attrs */
	rg.h.al = ch;
#if	COLOR
	if (dtype != CDMONO)
		rg.h.bl = cfcolor;
	else
		rg.h.bl = 0x07;
#else
	rg.h.bl = 0x07;
#endif
	int86(0x10, &rg, &rg);
}

void ibmeeop(void)
{
	int attr;		/* attribute to fill screen with */

	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x.cx = 0;		/* upper left corner of scroll */
	rg.x.dx = (term.t_nrow << 8) | (term.t_ncol - 1);
	/* lower right corner of scroll */
#if	COLOR
	if (dtype != CDMONO)
		attr =
		    ((ctrans[gbcolor] & 15) << 4) | (ctrans[gfcolor] & 15);
	else
		attr = 0;
#else
	attr = 0;
#endif
	rg.h.bh = attr;
	int86(0x10, &rg, &rg);
}

/* Change reverse video state.
 *
 * @state: TRUE = reverse, FALSE = normal.
 */
void ibmrev(int state)
{
	/* This never gets used under the IBM-PC driver */
}

/* Change screen resolution.
 *
 * @res: resolution to change to.
 */
void ibmcres(char *res)
{
	int i;
	for (i = 0; i < NDRIVE; i++) {
		if (strcmp(res, drvname[i]) == 0) {
			scinit(i);
			return TRUE;
		}
	}
	return FALSE;
}

#if SCROLLCODE

/* Move howmany lines starting at from to to. */
void ibmscroll_reg(from, to, howmany)
{
	int i;

	if (to < from) {
		for (i = 0; i < howmany; i++) {
			movmem(scptr[from + i], scptr[to + i],
			       term.t_ncol * 2);
		}
	}
	else if (to > from) {
		for (i = howmany - 1; i >= 0; i--) {
			movmem(scptr[from + i], scptr[to + i],
			       term.t_ncol * 2);
		}
	}
}

#endif

void ibmbeep(void)
{
	bdos(6, BEL, 0);
}

void ibmopen(void)
{
	scinit(CDSENSE);
	revexist = TRUE;
	ttopen();
}

void ibmclose(void)
{
#if	COLOR
	ibmfcol(7);
	ibmbcol(0);
#endif
	/* if we had the EGA open... close it */
	if (dtype == CDEGA)
		egaclose();
#if	PKCODE
	if (dtype == CDVGA)
		egaclose();
#endif

	ttclose();
}

/* Open the keyboard. */
void ibmkopen(void)
{
}

/* Close the keyboard. */
void ibmkclose(void)
{
}

/* Initialize the screen head pointers.
 *
 * @type: type of adapter to init for.
 */
static int scinit(int type)
{
	union {
		long laddr;	/* long form of address */
		int *paddr;	/* pointer form of address */
	} addr;
	int i;

	/* if asked...find out what display is connected */
	if (type == CDSENSE)
		type = getboard();

	/* if we have nothing to do....don't do it */
	if (dtype == type)
		return TRUE;

	/* if we try to switch to EGA and there is none, don't */
	if (type == CDEGA && egaexist != TRUE)
		return FALSE;

	/* if we had the EGA open... close it */
	if (dtype == CDEGA)
		egaclose();
#if	PKCODE
	if (dtype == CDVGA)
		egaclose();
#endif

	/* and set up the various parameters as needed */
	switch (type) {
	case CDMONO:		/* Monochrome adapter */
		scadd = SCADM;
		newsize(TRUE, 25);
		break;

	case CDCGA:		/* Color graphics adapter */
		scadd = SCADC;
		newsize(TRUE, 25);
		break;

	case CDEGA:		/* Enhanced graphics adapter */
		scadd = SCADE;
		egaopen();
		newsize(TRUE, 43);
		break;
	case CDVGA:		/* Enhanced graphics adapter */
		scadd = SCADE;
		egaopen();
		newsize(TRUE, 50);
		break;
	}

	/* reset the $sres environment variable */
	strcpy(sres, drvname[type]);
	dtype = type;

	/* initialize the screen pointer array */
	for (i = 0; i < NROW; i++) {
		addr.laddr = scadd + (long) (NCOL * i * 2);
		scptr[i] = addr.paddr;
	}
	return TRUE;
}

/* getboard:	Determine which type of display board is attached.
		Current known types include:

		CDMONO	Monochrome graphics adapter
		CDCGA	Color Graphics Adapter
		CDEGA	Extended graphics Adapter
*/

/* getboard:	Detect the current display adapter
		if MONO		set to MONO
		   CGA		set to CGA	EGAexist = FALSE
		   EGA		set to CGA	EGAexist = TRUE
*/
int getboard(void)
{
	int type;		/* board type to return */

	type = CDCGA;
	int86(0x11, &rg, &rg);
	if ((((rg.x.ax >> 4) & 3) == 3))
		type = CDMONO;

	/* test if EGA present */
	rg.x.ax = 0x1200;
	rg.x.bx = 0xff10;
	int86(0x10, &rg, &rg);	/* If EGA, bh=0-1 and bl=0-3 */
	egaexist = !(rg.x.bx & 0xfefc);	/* Yes, it's EGA */
	return type;
}

/* init the computer to work with the EGA */
void egaopen(void)
{
	/* put the beast into EGA 43 row mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);

	rg.h.ah = 17;		/* set char. generator function code */
	rg.h.al = 18;		/*  to 8 by 8 double dot ROM         */
	rg.h.bl = 0;		/* block 0                           */
	int86(16, &rg, &rg);

	rg.h.ah = 18;		/* alternate select function code    */
	rg.h.al = 0;		/* clear AL for no good reason       */
	rg.h.bl = 32;		/* alt. print screen routine         */
	int86(16, &rg, &rg);

	rg.h.ah = 1;		/* set cursor size function code */
	rg.x.cx = 0x0607;	/* turn cursor on code */
	int86(0x10, &rg, &rg);

	outp(0x3d4, 10);	/* video bios bug patch */
	outp(0x3d5, 6);
}

void egaclose(void)
{
	/* put the beast into 80 column mode */
	rg.x.ax = 3;
	int86(16, &rg, &rg);
}

/* Write a line out.
 *
 * @row:    row of screen to place outstr on.
 * @outstr: string to write out (must be term.t_ncol long).
 * @forg:   forground color of string to write.
 * @bacg:   background color.
 */
void scwrite(int row, char *outstr, int forg, int bacg)
{
	unsigned int attr;	/* attribute byte mask to place in RAM */
	unsigned int *lnptr;	/* pointer to the destination line */
	int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	if (dtype != CDMONO)
		attr = (((ctrans[bacg] & 15) << 4) | (ctrans[forg] & 15)) << 8;
	else
		attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#else
	attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#endif
	lnptr = &sline[0];
	for (i = 0; i < term.t_ncol; i++)
		*lnptr++ = (outstr[i] & 255) | attr;

	if (flickcode && (dtype == CDCGA)) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8));

		/* and to be back on */
		while ((inp(0x3da) & 8) == 0);
	}

	/* and send the string out */
	movmem(&sline[0], scptr[row], term.t_ncol * 2);
}

#endif
