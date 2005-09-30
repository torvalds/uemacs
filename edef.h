/*	EDEF.H
 *
 *	Global variable definitions
 *
 *	written by Dave G. Conroy
 *	modified by Steve Wilhite, George Jones
 *	greatly modified by Daniel Lawrence
 *	modified by Petri Kutvonen
 */

/* some global fuction declarations */

#include <stdlib.h>
#include <string.h>

char *flook();
char *getctext();
char *getfname();
char *getval();
char *gtenv();
char *gtfun();
char *gtusr();
char *itoa();
char *ltos();
char *mklower();
char *mkupper();
char *token();
char *transbind();
unsigned int getckey();
unsigned int stock();

#ifdef	maindef

/* for MAIN.C */

/* initialized global definitions */

int fillcol = 72;		/* Current fill column          */
short kbdm[NKBDM];		/* Macro                        */
char *execstr = NULL;		/* pointer to string to execute */
char golabel[NPAT] = "";	/* current line to go to        */
int execlevel = 0;		/* execution IF level           */
int eolexist = TRUE;		/* does clear to EOL exist      */
int revexist = FALSE;		/* does reverse video exist?    */
int flickcode = FALSE;		/* do flicker supression?       */
char *modename[] = {		/* name of modes                */
	"WRAP", "CMODE", "SPELL", "EXACT", "VIEW", "OVER",
	"MAGIC", "CRYPT", "ASAVE"
};
char *mode2name[] = {		/* name of modes                */
	"Wrap", "Cmode", "Spell", "Exact", "View", "Over",
	"Magic", "Crypt", "Asave"
};
char modecode[] = "WCSEVOMYA";	/* letters to represent modes   */
int gmode = 0;			/* global editor mode           */
int gflags = GFREAD;		/* global control flag          */
#if	PKCODE & IBMPC
int gfcolor = 8;		/* global forgrnd color (white) */
#else
int gfcolor = 7;		/* global forgrnd color (white) */
#endif
int gbcolor = 0;		/* global backgrnd color (black) */
int gasave = 256;		/* global ASAVE size            */
int gacount = 256;		/* count until next ASAVE       */
int sgarbf = TRUE;		/* TRUE if screen is garbage    */
int mpresf = FALSE;		/* TRUE if message in last line */
int clexec = FALSE;		/* command line execution flag  */
int mstore = FALSE;		/* storing text to macro flag   */
int discmd = TRUE;		/* display command flag         */
int disinp = TRUE;		/* display input characters     */
struct BUFFER *bstore = NULL;	/* buffer to store macro text to */
int vtrow = 0;			/* Row location of SW cursor */
int vtcol = 0;			/* Column location of SW cursor */
int ttrow = HUGE;		/* Row location of HW cursor */
int ttcol = HUGE;		/* Column location of HW cursor */
int lbound = 0;			/* leftmost column of current line
				   being displayed */
int taboff = 0;			/* tab offset for display       */
int metac = CONTROL | '[';	/* current meta character */
int ctlxc = CONTROL | 'X';	/* current control X prefix char */
int reptc = CONTROL | 'U';	/* current universal repeat char */
int abortc = CONTROL | 'G';	/* current abort command char   */

int quotec = 0x11;		/* quote char during mlreply() */
int tabmask = 0x07;		/* tabulator mask */
char *cname[] = {		/* names of colors              */
	"BLACK", "RED", "GREEN", "YELLOW", "BLUE",
	"MAGENTA", "CYAN", "WHITE"
#if	PKCODE & IBMPC
	    , "HIGH"
#endif
};
KILL *kbufp = NULL;		/* current kill buffer chunk pointer    */
KILL *kbufh = NULL;		/* kill buffer header pointer           */
int kused = KBLOCK;		/* # of bytes used in kill buffer       */
WINDOW *swindow = NULL;		/* saved window pointer                 */
int cryptflag = FALSE;		/* currently encrypting?                */
short *kbdptr;			/* current position in keyboard buf */
short *kbdend = &kbdm[0];	/* ptr to end of the keyboard */
int kbdmode = STOP;		/* current keyboard macro mode  */
int kbdrep = 0;			/* number of repetitions        */
int restflag = FALSE;		/* restricted use?              */
int lastkey = 0;		/* last keystoke                */
int seed = 0;			/* random number seed           */
long envram = 0l;		/* # of bytes current in use by malloc */
int macbug = FALSE;		/* macro debuging flag          */
char errorm[] = "ERROR";	/* error literal                */
char truem[] = "TRUE";		/* true literal                 */
char falsem[] = "FALSE";	/* false litereal               */
int cmdstatus = TRUE;		/* last command status          */
char palstr[49] = "";		/* palette string               */
int saveflag = 0;		/* Flags, saved with the $target var */
char *fline = NULL;		/* dynamic return line */
int flen = 0;			/* current length of fline */
int rval = 0;			/* return value of a subprocess */
#if	CALLED
int eexitflag = FALSE;		/* EMACS exit flag */
int eexitval = 0;		/* and the exit return value */
#endif
#if	PKCODE
int nullflag = FALSE;		/* accept null characters */
int justflag = FALSE;		/* justify, don't fill */
#endif
int overlap = 0;		/* line overlap in forw/back page */
int scrollcount = 1;		/* number of lines to scroll */

/* uninitialized global definitions */

int currow;			/* Cursor row                   */
int curcol;			/* Cursor column                */
int thisflag;			/* Flags, this command          */
int lastflag;			/* Flags, last command          */
int curgoal;			/* Goal for C-P, C-N            */
WINDOW *curwp;			/* Current window               */
BUFFER *curbp;			/* Current buffer               */
WINDOW *wheadp;			/* Head of list of windows      */
BUFFER *bheadp;			/* Head of list of buffers      */
BUFFER *blistp;			/* Buffer for C-X C-B           */

BUFFER *bfind();		/* Lookup a buffer by name      */
WINDOW *wpopup();		/* Pop up window creation       */
LINE *lalloc();			/* Allocate a line              */
char sres[NBUFN];		/* current screen resolution    */

char pat[NPAT];			/* Search pattern               */
char tap[NPAT];			/* Reversed pattern array.      */
char rpat[NPAT];		/* replacement pattern          */

/* The variable matchlen holds the length of the matched
 * string - used by the replace functions.
 * The variable patmatch holds the string that satisfies
 * the search command.
 * The variables matchline and matchoff hold the line and
 * offset position of the *start* of match.
 */
unsigned int matchlen = 0;
unsigned int mlenold = 0;
char *patmatch = NULL;
LINE *matchline = NULL;
int matchoff = 0;

#if	MAGIC
/*
 * The variables magical and rmagical determine if there
 * were actual metacharacters in the search and replace strings -
 * if not, then we don't have to use the slower MAGIC mode
 * search functions.
 */
short int magical = FALSE;
short int rmagical = FALSE;
MC mcpat[NPAT];			/* the magic pattern            */
MC tapcm[NPAT];			/* the reversed magic pattern   */
RMC rmcpat[NPAT];		/* the replacement magic array  */

#endif

/* directive name table:
	This holds the names of all the directives....	*/

char *dname[] = {
	"if", "else", "endif",
	"goto", "return", "endm",
	"while", "endwhile", "break",
	"force"
};

#if	DEBUGM
/*	vars needed for macro debugging output	*/
char outline[NSTRING];		/* global string to hold debug line text */
#endif

#else

/* for all the other .C files */

/* initialized global external declarations */

extern int fillcol;		/* Fill column                  */
extern short kbdm[];		/* Holds kayboard macro data    */
extern char pat[];		/* Search pattern               */
extern char rpat[];		/* Replacement pattern          */
extern char *execstr;		/* pointer to string to execute */
extern char golabel[];		/* current line to go to        */
extern int execlevel;		/* execution IF level           */
extern int eolexist;		/* does clear to EOL exist?     */
extern int revexist;		/* does reverse video exist?    */
extern int flickcode;		/* do flicker supression?       */
extern char *modename[];	/* text names of modes          */
extern char *mode2name[];	/* text names of modes          */
extern char modecode[];		/* letters to represent modes   */
extern KEYTAB keytab[];		/* key bind to functions table  */
extern NBIND names[];		/* name to function table       */
extern int gmode;		/* global editor mode           */
extern int gflags;		/* global control flag          */
extern int gfcolor;		/* global forgrnd color (white) */
extern int gbcolor;		/* global backgrnd color (black) */
extern int gasave;		/* global ASAVE size            */
extern int gacount;		/* count until next ASAVE       */
extern int sgarbf;		/* State of screen unknown      */
extern int mpresf;		/* Stuff in message line        */
extern int clexec;		/* command line execution flag  */
extern int mstore;		/* storing text to macro flag   */
extern int discmd;		/* display command flag         */
extern int disinp;		/* display input characters     */
extern struct BUFFER *bstore;	/* buffer to store macro text to */
extern int vtrow;		/* Row location of SW cursor */
extern int vtcol;		/* Column location of SW cursor */
extern int ttrow;		/* Row location of HW cursor */
extern int ttcol;		/* Column location of HW cursor */
extern int lbound;		/* leftmost column of current line
				   being displayed */
extern int taboff;		/* tab offset for display       */
extern int metac;		/* current meta character */
extern int ctlxc;		/* current control X prefix char */
extern int reptc;		/* current universal repeat char */
extern int abortc;		/* current abort command char   */

extern int quotec;		/* quote char during mlreply() */
extern int tabmask;
extern char *cname[];		/* names of colors              */
extern KILL *kbufp;		/* current kill buffer chunk pointer */
extern KILL *kbufh;		/* kill buffer header pointer   */
extern int kused;		/* # of bytes used in KB        */
extern WINDOW *swindow;		/* saved window pointer         */
extern int cryptflag;		/* currently encrypting?        */
extern short *kbdptr;		/* current position in keyboard buf */
extern short *kbdend;		/* ptr to end of the keyboard */
extern int kbdmode;		/* current keyboard macro mode  */
extern int kbdrep;		/* number of repetitions        */
extern int restflag;		/* restricted use?              */
extern int lastkey;		/* last keystoke                */
extern int seed;		/* random number seed           */
extern long envram;		/* # of bytes current in use by malloc */
extern int macbug;		/* macro debuging flag          */
extern char errorm[];		/* error literal                */
extern char truem[];		/* true literal                 */
extern char falsem[];		/* false litereal               */
extern int cmdstatus;		/* last command status          */
extern char palstr[];		/* palette string               */
extern int saveflag;		/* Flags, saved with the $target var */
extern char *fline;		/* dynamic return line */
extern int flen;		/* current length of fline */
extern int rval;		/* return value of a subprocess */
#if	CALLED
extern int eexitflag;		/* EMACS exit flag */
extern int eexitval;		/* and the exit return value */
#endif
#if	PKCODE
extern int justflag;		/* justify, don't fill */
#endif
extern int overlap;		/* line overlap in forw/back page */
extern int scrollcount;		/* number of lines to scroll */

/* uninitialized global external declarations */

extern int currow;		/* Cursor row                   */
extern int curcol;		/* Cursor column                */
extern int thisflag;		/* Flags, this command          */
extern int lastflag;		/* Flags, last command          */
extern int curgoal;		/* Goal for C-P, C-N            */
extern WINDOW *curwp;		/* Current window               */
extern BUFFER *curbp;		/* Current buffer               */
extern WINDOW *wheadp;		/* Head of list of windows      */
extern BUFFER *bheadp;		/* Head of list of buffers      */
extern BUFFER *blistp;		/* Buffer for C-X C-B           */

extern BUFFER *bfind();		/* Lookup a buffer by name      */
extern WINDOW *wpopup();	/* Pop up window creation       */
extern LINE *lalloc();		/* Allocate a line              */
extern char sres[NBUFN];	/* current screen resolution    */
extern char pat[];		/* Search pattern               */
extern char tap[];		/* Reversed pattern array.      */
extern char rpat[];		/* replacement pattern          */

extern unsigned int matchlen;
extern unsigned int mlenold;
extern char *patmatch;
extern LINE *matchline;
extern int matchoff;

#if	MAGIC
extern short int magical;
extern short int rmagical;
extern MC mcpat[NPAT];		/* the magic pattern            */
extern MC tapcm[NPAT];		/* the reversed magic pattern   */
extern RMC rmcpat[NPAT];	/* the replacement magic array  */
#endif

extern char *dname[];		/* directive name table         */

#if	DEBUGM
/*	vars needed for macro debugging output	*/
extern char outline[];		/* global string to hold debug line text */
#endif

#endif

/* terminal table defined only in TERM.C */

#ifndef	termdef
extern TERM term;		/* Terminal information.        */
#endif
