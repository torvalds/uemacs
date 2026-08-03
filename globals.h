/*	globals.h
 *
 *	Declarations for the editor's global variables; globals.c has
 *	the definitions.  It was edef.h, and before commit 8967dfc
 *	("Fix ugly \"definitions in header file\" crud") it really was
 *	the definitions too, behind a #ifdef maindef.
 *
 *	written by Dave G. Conroy
 *	modified by Steve Wilhite, George Jones
 *	greatly modified by Daniel Lawrence
 *	modified by Petri Kutvonen
 */
#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <signal.h>
#include <stdlib.h>
#include <string.h>

/* Some global fuction declarations. */
typedef int (*fn_t)(int, int);

/* Initialized global external declarations. */

extern int fill_column;				/* fill column, the $fillcol setting */
extern int keyboard_macro[];			/* the recorded keystrokes */
extern char pat[];				/* Search pattern               */
extern char rpat[];				/* Replacement pattern          */
extern char *execstr;				/* pointer to string to execute */
extern char golabel[];				/* current line to go to        */
extern int execlevel;				/* execution IF level           */
extern int eolexist;				/* does clear to EOL exist?     */
extern int revexist;				/* does reverse video exist?    */
extern char *modename[];			/* text names of modes          */
extern char *mode2name[];			/* text names of modes          */
extern char modecode[];				/* letters to represent modes   */
extern struct key_tab keytab[];			/* key bind to functions table  */
extern struct name_bind names[];		/* name to function table */
extern int global_mode;				/* default modes for new buffers, the $gmode setting */
extern int global_flags;			/* global control flags, the $gflags setting */
extern int autosave_interval;			/* changes between autosaves, the $asave setting */
extern int autosave_countdown;			/* changes left before one, the $acount setting */
extern int sgarbf;				/* State of screen unknown      */
extern int mpresf;				/* Stuff in message line        */
extern int clexec;				/* command line execution flag  */
extern int storing_macro;			/* capturing command lines into a buffer */
extern int display_commands;			/* echo commands, the $discmd setting */
extern int display_input;			/* echo input, the $disinp setting */
extern struct buffer *store_buffer;		/* the buffer they are being captured into */
/* set by the SIGWINCH handler, acted on by checkwinsize() */
extern volatile sig_atomic_t chg_width, chg_height;
extern int shown_row;				/* where the terminal is actually showing it */
extern int shown_col;				/* and its column, so a move can be skipped */
extern int left_column;				/* leftmost column shown of a sideways-scrolled line */
extern int meta_char;				/* current meta character */
extern int ctlx_char;				/* current control-X prefix character */
extern int repeat_key;				/* current universal repeat key */
extern int abort_char;				/* current abort character */

extern int quote_char;				/* quote the next character, during ask_string() */
extern int tabmask;
extern struct kill *kill_last;			/* the chunk being filled, at the end of the list */
extern struct kill *kill_head;			/* first chunk of the kill buffer, or NULL */
extern int kill_used;				/* bytes used in that last chunk */
extern int *keyboard_macro_pos;			/* where replay or recording has got to */
extern int *keyboard_macro_end;			/* one past the last recorded keystroke */
extern int keyboard_macro_mode;			/* STOP, PLAY or RECORD */
extern int keyboard_macro_repeat;		/* how many times left to replay */
extern int restflag;				/* restricted use?              */
extern int last_key;				/* last keystroke, the $lastkey setting */
extern int random_seed;				/* random number seed, the $seed setting */
extern int macro_debug;				/* trace macros, the $debug setting */
extern char errorm[];				/* error literal                */
extern char truem[];				/* true literal                 */
extern char falsem[];				/* false litereal               */
extern int command_status;			/* last command status, the $status setting */
extern int saveflag;				/* Flags, saved with the $target var */
extern char *file_line;				/* buffer file_get_line() reads a line into */
extern int file_line_size;			/* how big that allocation is, not the length in it */
extern int rval;				/* return value of a subprocess */
extern int nullflag;
extern int justflag;				/* justify, don't fill */
extern int overlap;				/* line overlap in forw/back page */
extern int scroll_lines;			/* lines a scroll moves, the $jump setting */

/* Uninitialized global external declarations. */

extern int cursor_row;				/* where the cursor belongs, worked out from dot */
extern int cursor_col;				/* and its column, counting from the line start */
extern int thisflag;				/* Flags, this command          */
extern int lastflag;				/* Flags, last command          */
extern int curgoal;				/* Goal for C-P, C-N            */
extern struct window *curwp;			/* Current window               */
extern struct buffer *curbp;			/* Current buffer               */
extern struct buffer *bheadp;			/* Head of list of buffers      */
extern struct buffer *blistp;			/* Buffer for C-X C-B           */

extern char pat[];				/* Search pattern.              */
extern char tap[];				/* Reversed pattern array.      */
extern char rpat[];				/* Replacement pattern.         */

extern unsigned int matchlen;
extern unsigned int mlenold;
extern char *patmatch;
extern struct line *matchline;
extern int matchoff;

extern char *dname[];				/* Directive name table.        */

/* Terminal table defined only in term.c */
extern struct terminal term;

#endif				/* GLOBALS_H_ */
