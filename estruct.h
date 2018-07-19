/*      ESTRUCT.H
 *
 *      Structure and preprocessor defines
 *
 *	written by Dave G. Conroy
 *	modified by Steve Wilhite, George Jones
 *      substantially modified by Daniel Lawrence
 *	modified by Petri Kutvonen
 */

#define MAXCOL	500
#define MAXROW	500

#ifdef	MSDOS
#undef	MSDOS
#endif
#ifdef	EGA
#undef	EGA
#endif
#ifdef	CTRLZ
#undef	CTRLZ
#endif

/* Machine/OS definitions. */

#if defined(AUTOCONF) || defined(MSDOS) || defined(BSD) || defined(SYSV) || defined(VMS)

/* Make an intelligent guess about the target system. */

#if defined(__TURBOC__)
#define MSDOS 1 /* MS/PC DOS 3.1-4.0 with Turbo C 2.0 */
#else
#define	MSDOS 0
#endif

#if defined(BSD) || defined(sun) || defined(ultrix) || (defined(vax) && defined(unix)) || defined(ultrix) || defined(__osf__)
#ifndef BSD
#define BSD 1 /* Berkeley UNIX */
#endif
#else
#define	BSD 0
#endif

#if defined(SVR4) || defined(__linux__)	/* ex. SunOS 5.3 */
#define SVR4 1
#define SYSV 1
#undef BSD
#endif

#if defined(SYSV) || defined(u3b2) || defined(_AIX) || (defined(i386) && defined(unix)) || defined(__hpux)
#define	USG 1 /* System V UNIX */
#else
#define	USG 0
#endif

#if defined(VMS) || (defined(vax) && ! defined(unix))
#define VMS 1 /* VAX/VMS */
#else
#define VMS 0
#endif

#define	V7 0 /* No more. */

#else

#define MSDOS   1		/* MS-DOS                       */
#define V7      0		/* V7 UNIX or Coherent or BSD4.2 */
#define	BSD	0		/* UNIX BSD 4.2 and ULTRIX      */
#define	USG	0		/* UNIX system V                */
#define VMS     0		/* VAX/VMS                      */

#endif				/*autoconf */

#ifndef	AUTOCONF

/*	Compiler definitions			*/
#define	UNIX	0		/* a random UNIX compiler */
#define	MSC	0		/* MicroSoft C compiler, versions 3 up */
#define	TURBO	1		/* Turbo C/MSDOS */

#else

#define	UNIX	(V7 | BSD | USG)
#define	MSC	0
#define	TURBO	MSDOS

#endif				/*autoconf */

/*	Debugging options	*/

#define	RAMSIZE	0		/* dynamic RAM memory usage tracking */
#define	RAMSHOW	0		/* auto dynamic RAM reporting */

#ifndef	AUTOCONF

/*   Special keyboard definitions            */

#define VT220	0		/* Use keypad escapes P.K.      */
#define VT100   0		/* Handle VT100 style keypad.   */

/*	Terminal Output definitions		*/

#define ANSI    0		/* ANSI escape sequences        */
#define	VMSVT	0		/* various VMS terminal entries */
#define VT52    0		/* VT52 terminal (Zenith).      */
#define TERMCAP 0		/* Use TERMCAP                  */
#define	IBMPC	1		/* IBM-PC CGA/MONO/EGA driver   */

#else

#define	VT220	(UNIX | VMS)
#define	VT100	0

#define	ANSI	0
#define	VMSVT	VMS
#define	VT52	0
#define	TERMCAP	UNIX
#define	IBMPC	MSDOS

#endif /* Autoconf. */

/*	Configuration options	*/

#define CVMVAS  1  /* arguments to page forward/back in pages      */
#define	CLRMSG	0  /* space clears the message line with no insert */
#define	CFENCE	1  /* fench matching in CMODE                      */
#define	TYPEAH	1  /* type ahead causes update to be skipped       */
#define DEBUGM	1  /* $debug triggers macro debugging              */
#define	VISMAC	0  /* update display during keyboard macros        */
#define	CTRLZ	0  /* add a ^Z at end of files under MSDOS only    */
#define ADDCR	0  /* ajout d'un CR en fin de chaque ligne (ST520) */
#define	NBRACE	1  /* new style brace matching command             */
#define	REVSTA	1  /* Status line appears in reverse video         */

#ifndef	AUTOCONF

#define	COLOR	1  /* color commands and windows                   */
#define	FILOCK	0  /* file locking under unix BSD 4.2              */

#else

#define	COLOR	MSDOS
#ifdef  SVR4
#define FILOCK  1
#else
#define	FILOCK	BSD
#endif

#endif /* Autoconf. */

#define	ISRCH	1  /* Incremental searches like ITS EMACS          */
#define	WORDPRO	1  /* Advanced word processing features            */
#define	APROP	1  /* Add code for Apropos command                 */
#define	CRYPT	1  /* file encryption enabled?                     */
#define MAGIC	1  /* include regular expression matching?         */
#define	AEDIT	1  /* advanced editing options: en/detabbing       */
#define	PROC	1  /* named procedures                             */
#define	CLEAN	0  /* de-alloc memory on exit                      */

#define ASCII	1  /* always using ASCII char sequences for now    */
#define EBCDIC	0  /* later IBM mainfraim versions will use EBCDIC */

#ifndef	AUTOCONF

#define	XONXOFF	0  /* don't disable XON-XOFF flow control P.K.     */
#define	NATIONL	0  /* interprete [,],\,{,},| as characters P.K.    */

#else

#define	XONXOFF	(UNIX | VMS)
#define	NATIONL	(UNIX | VMS)

#endif /* Autoconf. */

#define	PKCODE	1      /* include my extensions P.K., define always    */
#define	IBMCHR	MSDOS  /* use IBM PC character set P.K.                */
#define SCROLLCODE 1   /* scrolling code P.K.                          */

/* System dependant library redefinitions, structures and includes. */

#if TURBO
#include <dos.h>
#include <mem.h>
#undef peek
#undef poke
#define       peek(a,b,c,d)   movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define       poke(a,b,c,d)   movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#endif

#if	VMS
#define	atoi	xatoi
#define	abs	xabs
#define	getname	xgetname
#endif

#if MSDOS & MSC
#include	<dos.h>
#include	<memory.h>
#define	peek(a,b,c,d)	movedata(a,b,FP_SEG(c),FP_OFF(c),d)
#define	poke(a,b,c,d)	movedata(FP_SEG(c),FP_OFF(c),a,b,d)
#define	movmem(a, b, c)		memcpy(b, a, c)
#endif

#if	VMS
#define	unlink(a)	delete(a)
#endif

/* Define some ability flags. */

#if	IBMPC
#define	MEMMAP	1
#else
#define	MEMMAP	0
#endif

#if	MSDOS | V7 | USG | BSD
#define	ENVFUNC	1
#else
#define	ENVFUNC	0
#endif

/* Emacs global flag bit definitions (for gflags). */

#define	GFREAD	1

/* Internal constants. */

#define	NBINDS	256		/* max # of bound keys          */
#define NFILEN  256		/* # of bytes, file name        */
#define NBUFN   16		/* # of bytes, buffer name      */
#define NLINE   256		/* # of bytes, input line       */
#define	NSTRING	128		/* # of bytes, string buffers   */
#define NKBDM   256		/* # of strokes, keyboard macro */
#define NPAT    128		/* # of bytes, pattern          */
#define HUGE    1000		/* Huge number                  */
#define	NLOCKS	100		/* max # of file locks active   */
#define	NCOLORS	8		/* number of supported colors   */
#define	KBLOCK	250		/* sizeof kill buffer chunks    */

#define CONTROL 0x10000000	/* Control flag, or'ed in       */
#define META    0x20000000	/* Meta flag, or'ed in          */
#define CTLX    0x40000000	/* ^X flag, or'ed in            */
#define	SPEC	0x80000000	/* special key (function keys)  */

#ifdef	FALSE
#undef	FALSE
#endif
#ifdef	TRUE
#undef	TRUE
#endif

#define FALSE   0		/* False, no, bad, etc.         */
#define TRUE    1		/* True, yes, good, etc.        */
#define ABORT   2		/* Death, ^G, abort, etc.       */
#define	FAILED	3		/* not-quite fatal false return */

#define	STOP	0		/* keyboard macro not in use    */
#define	PLAY	1		/*                playing       */
#define	RECORD	2		/*                recording     */

/*	Directive definitions	*/

#define	DIF		0
#define DELSE		1
#define DENDIF		2
#define DGOTO		3
#define DRETURN		4
#define DENDM		5
#define DWHILE		6
#define	DENDWHILE	7
#define	DBREAK		8
#define DFORCE		9

#define NUMDIRS		10

/*
 * PTBEG, PTEND, FORWARD, and REVERSE are all toggle-able values for
 * the scan routines.
 */
#define	PTBEG	0		/* Leave the point at the beginning on search   */
#define	PTEND	1		/* Leave the point at the end on search         */
#define	FORWARD	0		/* forward direction            */
#define REVERSE	1		/* backwards direction          */

#define FIOSUC  0		/* File I/O, success.           */
#define FIOFNF  1		/* File I/O, file not found.    */
#define FIOEOF  2		/* File I/O, end of file.       */
#define FIOERR  3		/* File I/O, error.             */
#define	FIOMEM	4		/* File I/O, out of memory      */
#define	FIOFUN	5		/* File I/O, eod of file/bad line */

#define CFCPCN  0x0001		/* Last command was C-P, C-N    */
#define CFKILL  0x0002		/* Last command was a kill      */

#define	BELL	0x07		/* a bell character             */
#define	TAB	0x09		/* a tab character              */

#if	V7 | USG | BSD
#define	PATHCHR	':'
#else
#define	PATHCHR	';'
#endif

#define	INTWIDTH	sizeof(int) * 3

/*	Macro argument token types					*/

#define	TKNUL	0		/* end-of-string                */
#define	TKARG	1		/* interactive argument         */
#define	TKBUF	2		/* buffer argument              */
#define	TKVAR	3		/* user variables               */
#define	TKENV	4		/* environment variables        */
#define	TKFUN	5		/* function....                 */
#define	TKDIR	6		/* directive                    */
#define	TKLBL	7		/* line label                   */
#define	TKLIT	8		/* numeric literal              */
#define	TKSTR	9		/* quoted string literal        */
#define	TKCMD	10		/* command name                 */

/*	Internal defined functions					*/

#define	nextab(a)	(a & ~tabmask) + (tabmask+1)
#ifdef	abs
#undef	abs
#endif

/* DIFCASE represents the integer difference between upper
   and lower case letters.  It is an xor-able value, which is
   fortunate, since the relative positions of upper to lower
   case letters is the opposite of ascii in ebcdic.
*/

#ifdef	islower
#undef	islower
#endif

#if	PKCODE
#ifdef	isupper
#undef	isupper
#endif
#endif

#if	ASCII

#define	DIFCASE		0x20

#if	NATIONL
#define LASTUL ']'
#define LASTLL '}'
#else
#define LASTUL 'Z'
#define LASTLL 'z'
#endif

#if	IBMCHR

#define isletter(c)	(('a' <= c && LASTLL >= c) || ('A' <= c && LASTUL >= c) || (128<=c && c<=167))
#define islower(c)	(('a' <= c && LASTLL >= c))
#define isupper(c)	(('A' <= c && LASTUL >= c))

#else

#define isletter(c)	isxletter((0xFF & (c)))
#define islower(c)	isxlower((0xFF & (c)))
#define isupper(c)	isxupper((0xFF & (c)))

#define isxletter(c)	(('a' <= c && LASTLL >= c) || ('A' <= c && LASTUL >= c) || (192<=c && c<=255))
#define isxlower(c)	(('a' <= c && LASTLL >= c) || (224 <= c && 252 >= c))
#define isxupper(c)	(('A' <= c && LASTUL >= c) || (192 <= c && 220 >= c))

#endif

#endif

#if	EBCDIC

#define	DIFCASE		0x40
#define isletter(c)	(('a' <= c && 'i' >= c) || ('j' <= c && 'r' >= c) || ('s' <= c && 'z' >= c) || ('A' <= c && 'I' >= c) || ('J' <= c && 'R' >= c) || ('S' <= c && 'Z' >= c))
#define islower(c)	(('a' <= c && 'i' >= c) || ('j' <= c && 'r' >= c) || ('s' <= c && 'z' >= c))
#if	PKCODE
#define isupper(c)	(('A' <= c && 'I' >= c) || ('J' <= c && 'R' >= c) || ('S' <= c && 'Z' >= c))
#endif

#endif

/*	Dynamic RAM tracking and reporting redefinitions	*/

#if	RAMSIZE
#define	malloc	allocate
#define	free	release
#endif

/*	De-allocate memory always on exit (if the operating system or
	main program can not
*/

#if	CLEAN
#define	exit(a)	cexit(a)
#endif

/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay. Although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 */
struct window {
	struct window *w_wndp;	/* Next window                  */
	struct buffer *w_bufp;	/* Buffer displayed in window   */
	struct line *w_linep;	/* Top line in the window       */
	struct line *w_dotp;	/* Line containing "."          */
	struct line *w_markp;	/* Line containing "mark"       */
	int w_doto;		/* Byte offset for "."          */
	int w_marko;		/* Byte offset for "mark"       */
	int w_toprow;		/* Origin 0 top row of window   */
	int w_ntrows;		/* # of rows of text in window  */
	char w_force;		/* If NZ, forcing row.          */
	char w_flag;		/* Flags.                       */
#if	COLOR
	char w_fcolor;		/* current forground color      */
	char w_bcolor;		/* current background color     */
#endif
};

#define WFFORCE 0x01		/* Window needs forced reframe  */
#define WFMOVE  0x02		/* Movement from line to line   */
#define WFEDIT  0x04		/* Editing within a line        */
#define WFHARD  0x08		/* Better to a full display     */
#define WFMODE  0x10		/* Update mode line.            */
#define	WFCOLR	0x20		/* Needs a color change         */

#if SCROLLCODE
#define WFKILLS 0x40		/* something was deleted        */
#define WFINS   0x80		/* something was inserted       */
#endif


/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_linep".
 * 	Buffers may be "Inactive" which means the files associated with them
 * have not been read in yet. These get read in at "use buffer" time.
 */
struct buffer {
        struct buffer *b_bufp;	/* Link to next struct buffer   */
	struct line *b_dotp;	/* Link to "." struct line structure   */
	struct line *b_markp;	/* The same as the above two,   */
	struct line *b_linep;	/* Link to the header struct line      */
	int b_doto;		/* Offset of "." in above struct line  */
	int b_marko;		/* but for the "mark"           */
	int b_mode;		/* editor mode of this buffer   */
	char b_active;		/* window activated flag        */
	char b_nwnd;		/* Count of windows on buffer   */
	char b_flag;		/* Flags                        */
	char b_fname[NFILEN];	/* File name                    */
	char b_bname[NBUFN];	/* Buffer name                  */
#if	CRYPT
	char b_key[NPAT];	/* current encrypted key        */
#endif
};

#define BFINVS  0x01		/* Internal invisable buffer    */
#define BFCHG   0x02		/* Changed since last write     */
#define	BFTRUNC	0x04		/* buffer was truncated when read */

/*	mode flags	*/
#define	NUMMODES	10	/* # of defined modes           */

#define	MDWRAP	0x0001		/* word wrap                    */
#define	MDCMOD	0x0002		/* C indentation and fence match */
#define	MDSPELL	0x0004		/* spell error parcing          */
#define	MDEXACT	0x0008		/* Exact matching for searches  */
#define	MDVIEW	0x0010		/* read-only buffer             */
#define MDOVER	0x0020		/* overwrite mode               */
#define MDMAGIC	0x0040		/* regular expresions in search */
#define	MDCRYPT	0x0080		/* encrytion mode active        */
#define	MDASAVE	0x0100		/* auto-save mode               */

/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
struct region {
	struct line *r_linep;	/* Origin struct line address.         */
	int r_offset;		/* Origin struct line offset.          */
	long r_size;		/* Length in characters.        */
};

/*
 * The editor communicates with the display using a high level interface. A
 * "TERM" structure holds useful variables, and indirect pointers to routines
 * that do useful operations. The low level get and put routines are here too.
 * This lets a terminal, in addition to having non standard commands, have
 * funny get and put character code too. The calls might get changed to
 * "termp->t_field" style in the future, to make it possible to run more than
 * one terminal type.
 */
struct terminal {
	short t_mrow;		/* max number of rows allowable */
	short t_nrow;		/* current number of rows used  */
	short t_mcol;		/* max Number of columns.       */
	short t_ncol;		/* current Number of columns.   */
	short t_margin;		/* min margin for extended lines */
	short t_scrsiz;		/* size of scroll region "      */
	int t_pause;		/* # times thru update to pause */
	void (*t_open)(void);	/* Open terminal at the start.  */
	void (*t_close)(void);	/* Close terminal at end.       */
	void (*t_kopen)(void);	/* Open keyboard                */
	void (*t_kclose)(void);	/* close keyboard               */
	int (*t_getchar)(void);	/* Get character from keyboard. */
	int (*t_putchar)(int);	/* Put character to display.    */
	void (*t_flush) (void);	/* Flush output buffers.        */
	void (*t_move)(int, int);/* Move the cursor, origin 0.   */
	void (*t_eeol)(void);	/* Erase to end of line.        */
	void (*t_eeop)(void);	/* Erase to end of page.        */
	void (*t_beep)(void);	/* Beep.                        */
	void (*t_rev)(int);	/* set reverse video state      */
	int (*t_rez)(char *);	/* change screen resolution     */
#if	COLOR
	int (*t_setfor) ();	/* set forground color          */
	int (*t_setback) ();	/* set background color         */
#endif
#if     SCROLLCODE
	void (*t_scroll)(int, int,int);	/* scroll a region of the screen */
#endif
};

/*	TEMPORARY macros for terminal I/O  (to be placed in a machine
					    dependant place later)	*/

#define	TTopen		(*term.t_open)
#define	TTclose		(*term.t_close)
#define	TTkopen		(*term.t_kopen)
#define	TTkclose	(*term.t_kclose)
#define	TTgetc		(*term.t_getchar)
#define	TTputc		(*term.t_putchar)
#define	TTflush		(*term.t_flush)
#define	TTmove		(*term.t_move)
#define	TTeeol		(*term.t_eeol)
#define	TTeeop		(*term.t_eeop)
#define	TTbeep		(*term.t_beep)
#define	TTrev		(*term.t_rev)
#define	TTrez		(*term.t_rez)
#if	COLOR
#define	TTforg		(*term.t_setfor)
#define	TTbacg		(*term.t_setback)
#endif

/* Structure for the table of initial key bindings. */
struct key_tab {
	int k_code;		 /* Key code */
	int (*k_fp)(int, int);	 /* Routine to handle it */
};

/* Structure for the name binding table. */
struct name_bind {
	char *n_name;		 /* name of function key */
	int (*n_func)(int, int); /* function name is bound to */
};

/* The editor holds deleted text chunks in the struct kill buffer. The
 * kill buffer is logically a stream of ascii characters, however
 * due to its unpredicatable size, it gets implemented as a linked
 * list of chunks. (The d_ prefix is for "deleted" text, as k_
 * was taken up by the keycode structure).
 */
struct kill {
	struct kill *d_next;   /* Link to next chunk, NULL if last. */
	char d_chunk[KBLOCK];  /* Deleted text. */
};

/* When emacs' command interpetor needs to get a variable's name,
 * rather than it's value, it is passed back as a variable description
 * structure. The v_num field is a index into the appropriate variable table.
 */
struct variable_description {
	int v_type;  /* Type of variable. */
	int v_num;   /* Ordinal pointer to variable in list. */
};

/* The !WHILE directive in the execution language needs to
 * stack references to pending whiles. These are stored linked
 * to each currently open procedure via a linked list of
 * the following structure.
*/
struct while_block {
	struct line *w_begin;        /* ptr to !while statement */
	struct line *w_end;          /* ptr to the !endwhile statement */
	int w_type;		     /* block type */
	struct while_block *w_next;  /* next while */
};

#define	BTWHILE		1
#define	BTBREAK		2

/*
 * Incremental search defines.
 */
#if	ISRCH

#define	CMDBUFLEN	256	/* Length of our command buffer */

#define	IS_ABORT	0x07	/* Abort the isearch */
#define IS_BACKSP	0x08	/* Delete previous char */
#define	IS_TAB		0x09	/* Tab character (allowed search char) */
#define IS_NEWLINE	0x0D	/* New line from keyboard (Carriage return) */
#define	IS_QUOTE	0x11	/* Quote next character */
#define IS_REVERSE	0x12	/* Search backward */
#define	IS_FORWARD	0x13	/* Search forward */
#define	IS_VMSQUOTE	0x16	/* VMS quote character */
#define	IS_VMSFORW	0x18	/* Search forward for VMS */
#define	IS_QUIT		0x1B	/* Exit the search */
#define	IS_RUBOUT	0x7F	/* Delete previous character */

/* IS_QUIT is no longer used, the variable metac is used instead */

#endif

#if defined(MAGIC)
/*
 * Defines for the metacharacters in the regular expression
 * search routines.
 */
#define	MCNIL		0	/* Like the '\0' for strings. */
#define	LITCHAR		1	/* Literal character, or string. */
#define	ANY		2
#define	CCL		3
#define	NCCL		4
#define	BOL		5
#define	EOL		6
#define	DITTO		7
#define	CLOSURE		256	/* An or-able value. */
#define	MASKCL		(CLOSURE - 1)

#define	MC_ANY		'.'	/* 'Any' character (except newline). */
#define	MC_CCL		'['	/* Character class. */
#define	MC_NCCL		'^'	/* Negate character class. */
#define	MC_RCCL		'-'	/* Range in character class. */
#define	MC_ECCL		']'	/* End of character class. */
#define	MC_BOL		'^'	/* Beginning of line. */
#define	MC_EOL		'$'	/* End of line. */
#define	MC_CLOSURE	'*'	/* Closure - does not extend past newline. */
#define	MC_DITTO	'&'	/* Use matched string in replacement. */
#define	MC_ESC		'\\'	/* Escape - suppress meta-meaning. */

#define	BIT(n)		(1 << (n))	/* An integer with one bit set. */
#define	CHCASE(c)	((c) ^ DIFCASE)	/* Toggle the case of a letter. */

/* HICHAR - 1 is the largest character we will deal with.
 * HIBYTE represents the number of bytes in the bitmap.
 */
#define	HICHAR		256
#define	HIBYTE		HICHAR >> 3

/* Typedefs that define the meta-character structure for MAGIC mode searching
 * (struct magic), and the meta-character structure for MAGIC mode replacement
 * (struct magic_replacement).
 */
struct magic {
	short int mc_type;
	union {
		int lchar;
		char *cclmap;
	} u;
};

struct magic_replacement {
	short int mc_type;
	char *rstr;
};

#endif  /* MAGIC */
