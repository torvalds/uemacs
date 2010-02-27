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

typedef int (*fn_t)(int, int);

struct VIDEO;

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
extern struct buffer *bstore;	/* buffer to store macro text to */
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
extern window_t *swindow;	/* saved window pointer         */
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
extern int nullflag;
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
extern window_t *curwp;		/* Current window               */
extern struct buffer *curbp;		/* Current buffer               */
extern window_t *wheadp;	/* Head of list of windows      */
extern struct buffer *bheadp;		/* Head of list of buffers      */
extern struct buffer *blistp;		/* Buffer for C-X C-B           */

extern struct buffer *bfind(char *bname, int cflag, int bflag); /* Lookup a buffer by name      */
extern window_t *wpopup(void);	/* Pop up window creation       */
extern LINE *lalloc(int);	/* Allocate a line              */
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

/* terminal table defined only in TERM.C */
extern TERM term;		/* Terminal information.        */
