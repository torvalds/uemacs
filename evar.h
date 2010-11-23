/*	EVAR.H
 *
 *	Environment and user variable definitions
 *
 *	written 1986 by Daniel Lawrence
 *	modified by Petri Kutvonen
 */
#ifndef EVAR_H_
#define EVAR_H_

/* Max #chars in a var name. */
#define	NVSIZE	10

/* Structure to hold user variables and their definitions. */
struct user_variable {
	char u_name[NVSIZE + 1]; /* name of user variable */
	char *u_value;		 /* value (string) */
};

/* List of recognized environment variables. */

static char *envars[] = {
	"fillcol",		/* current fill column */
	"pagelen",		/* number of lines used by editor */
	"curcol",		/* current column pos of cursor */
	"curline",		/* current line in file */
	"ram",			/* ram in use by malloc */
	"flicker",		/* flicker supression */
	"curwidth",		/* current screen width */
	"cbufname",		/* current buffer name */
	"cfname",		/* current file name */
	"sres",			/* current screen resolution */
	"debug",		/* macro debugging */
	"status",		/* returns the status of the last command */
	"palette",		/* current palette string */
	"asave",		/* # of chars between auto-saves */
	"acount",		/* # of chars until next auto-save */
	"lastkey",		/* last keyboard char struck */
	"curchar",		/* current character under the cursor */
	"discmd",		/* display commands on command line */
	"version",		/* current version number */
	"progname",		/* returns current prog name - "MicroEMACS" */
	"seed",			/* current random number seed */
	"disinp",		/* display command line input characters */
	"wline",		/* # of lines in current window */
	"cwline",		/* current screen line in window */
	"target",		/* target for line moves */
	"search",		/* search pattern */
	"replace",		/* replacement pattern */
	"match",		/* last matched magic pattern */
	"kill",			/* kill buffer (read only) */
	"cmode",		/* mode of current buffer */
	"gmode",		/* global modes */
	"tpause",		/* length to pause for paren matching */
	"pending",		/* type ahead pending flag */
	"lwidth",		/* width of current line */
	"line",			/* text of current line */
	"gflags",		/* global internal emacs flags */
	"rval",			/* child process return value */
	"tab",			/* tab 4 or 8 */
	"overlap",
	"jump",
#if SCROLLCODE
	"scroll",		/* scroll enabled */
#endif
};

/* And its preprocesor definitions. */

#define	EVFILLCOL	0
#define	EVPAGELEN	1
#define	EVCURCOL	2
#define	EVCURLINE	3
#define	EVRAM		4
#define	EVFLICKER	5
#define	EVCURWIDTH	6
#define	EVCBUFNAME	7
#define	EVCFNAME	8
#define	EVSRES		9
#define	EVDEBUG		10
#define	EVSTATUS	11
#define	EVPALETTE	12
#define	EVASAVE		13
#define	EVACOUNT	14
#define	EVLASTKEY	15
#define	EVCURCHAR	16
#define	EVDISCMD	17
#define	EVVERSION	18
#define	EVPROGNAME	19
#define	EVSEED		20
#define	EVDISINP	21
#define	EVWLINE		22
#define EVCWLINE	23
#define	EVTARGET	24
#define	EVSEARCH	25
#define	EVREPLACE	26
#define	EVMATCH		27
#define	EVKILL		28
#define	EVCMODE		29
#define	EVGMODE		30
#define	EVTPAUSE	31
#define	EVPENDING	32
#define	EVLWIDTH	33
#define	EVLINE		34
#define	EVGFLAGS	35
#define	EVRVAL		36
#define EVTAB		37
#define EVOVERLAP	38
#define EVSCROLLCOUNT	39
#define EVSCROLL	40

enum function_type {
	NILNAMIC = 0,
	MONAMIC,
	DYNAMIC,
	TRINAMIC,
};

/* List of recognized user functions. */
struct user_function {
	char *f_name;
	enum function_type f_type;
};

static struct user_function funcs[] = {
	{ "add", DYNAMIC },	/* add two numbers together */
	{ "sub", DYNAMIC },	/* subtraction */
	{ "tim", DYNAMIC },	/* multiplication */
	{ "div", DYNAMIC },	/* division */
	{ "mod", DYNAMIC },	/* mod */
	{ "neg", MONAMIC },	/* negate */
	{ "cat", DYNAMIC },	/* concatinate string */
	{ "lef", DYNAMIC },	/* left string(string, len) */
	{ "rig", DYNAMIC },	/* right string(string, pos) */
	{ "mid", TRINAMIC },	/* mid string(string, pos, len) */
	{ "not", MONAMIC },	/* logical not */
	{ "equ", DYNAMIC },	/* logical equality check */
	{ "les", DYNAMIC },	/* logical less than */
	{ "gre", DYNAMIC },	/* logical greater than */
	{ "seq", DYNAMIC },	/* string logical equality check */
	{ "sle", DYNAMIC },	/* string logical less than */
	{ "sgr", DYNAMIC },	/* string logical greater than */
	{ "ind", MONAMIC },	/* evaluate indirect value */
	{ "and", DYNAMIC },	/* logical and */
	{ "or", DYNAMIC },	/* logical or */
	{ "len", MONAMIC },	/* string length */
	{ "upp", MONAMIC },	/* uppercase string */
	{ "low", MONAMIC },	/* lower case string */
	{ "tru", MONAMIC },	/* Truth of the universe logical test */
	{ "asc", MONAMIC },	/* char to integer conversion */
	{ "chr", MONAMIC },	/* integer to char conversion */
	{ "gtk", NILNAMIC },	/* get 1 charater */
	{ "rnd", MONAMIC },	/* get a random number */
	{ "abs", MONAMIC },	/* absolute value of a number */
	{ "sin", DYNAMIC },	/* find the index of one string in another */
	{ "env", MONAMIC },	/* retrieve a system environment var */
	{ "bin", MONAMIC },	/* loopup what function name is bound to a key */
	{ "exi", MONAMIC },	/* check if a file exists */
	{ "fin", MONAMIC },	/* look for a file on the path... */
	{ "ban", DYNAMIC },	/* bitwise and   9-10-87  jwm */
	{ "bor", DYNAMIC },	/* bitwise or    9-10-87  jwm */
	{ "bxo", DYNAMIC },	/* bitwise xor   9-10-87  jwm */
	{ "bno", MONAMIC },	/* bitwise not */
	{ "xla", TRINAMIC },	/* XLATE character string translation */
};

/* And its preprocesor definitions. */

#define	UFADD		0
#define	UFSUB		1
#define	UFTIMES		2
#define	UFDIV		3
#define	UFMOD		4
#define	UFNEG		5
#define	UFCAT		6
#define	UFLEFT		7
#define	UFRIGHT		8
#define	UFMID		9
#define	UFNOT		10
#define	UFEQUAL		11
#define	UFLESS		12
#define	UFGREATER	13
#define	UFSEQUAL	14
#define	UFSLESS		15
#define	UFSGREAT	16
#define	UFIND		17
#define	UFAND		18
#define	UFOR		19
#define	UFLENGTH	20
#define	UFUPPER		21
#define	UFLOWER		22
#define	UFTRUTH		23
#define	UFASCII		24
#define	UFCHR		25
#define	UFGTKEY		26
#define	UFRND		27
#define	UFABS		28
#define	UFSINDEX	29
#define	UFENV		30
#define	UFBIND		31
#define	UFEXIST		32
#define	UFFIND		33
#define UFBAND		34
#define UFBOR		35
#define UFBXOR		36
#define	UFBNOT		37
#define	UFXLATE		38

#endif  /* EVAR_H_ */
