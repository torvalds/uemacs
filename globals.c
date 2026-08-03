#include "estruct.h"
#include "globals.h"

/* initialized global definitions */

int fill_column = 72;				/* fill column, the $fillcol setting */
int keyboard_macro[NKBDM];			/* the recorded keystrokes */
char *execstr = NULL;				/* pointer to string to execute */
char golabel[NPAT] = "";			/* current line to go to        */
int execlevel = 0;				/* execution IF level           */
int can_erase_to_eol = TRUE;			/* terminal has an erase-to-end-of-line */
int can_reverse_video = FALSE;			/* terminal has reverse video */
char *modename[] = {				/* name of modes                */
	"WRAP", "CMODE", "SPELL", "EXACT", "VIEW", "OVER",
	"MAGIC", "ASAVE", "UTF-8"
};

char *mode2name[] = {				/* name of modes                */
	"Wrap", "Cmode", "Spell", "Exact", "View", "Over",
	"Magic", "Asave", "utf-8"
};

char modecode[] = "WCSEVOMYAU";			/* letters to represent modes   */
int global_mode = 0;				/* default modes for new buffers, the $gmode setting */
int global_flags = GFREAD;			/* global control flags, the $gflags setting */
int autosave_interval = 256;			/* changes between autosaves, the $asave setting */
int autosave_countdown = 256;			/* changes left before one, the $acount setting */
int screen_garbage = TRUE;			/* the screen is not what we think it is */
int message_present = FALSE;			/* something is showing on the message line */
int clexec = FALSE;				/* command line execution flag  */
int storing_macro = FALSE;			/* capturing command lines into a buffer */
int display_commands = TRUE;			/* echo commands, the $discmd setting */
int display_input = TRUE;			/* echo input, the $disinp setting */
struct buffer *store_buffer = NULL;		/* the buffer they are being captured into */
int shown_row = HUGE;				/* where the terminal is actually showing it */
int shown_col = HUGE;				/* and its column, so a move can be skipped */
int left_column = 0;				/* leftmost column shown of a sideways-scrolled line */
int meta_char = CONTROL | '[';			/* current meta character */
int ctlx_char = CONTROL | 'X';			/* current control-X prefix character */
int repeat_key = CONTROL | 'U';		/* current universal repeat key */
int abort_char = CONTROL | 'G';			/* current abort character */

int quote_char = 0x11;				/* quote the next character, during ask_string() */
int tabmask = 0x07;				/* tabulator mask */
struct kill *kill_last = NULL;			/* the chunk being filled, at the end of the list */
struct kill *kill_head = NULL;			/* first chunk of the kill buffer, or NULL */
int kill_used = KBLOCK;				/* bytes used in that last chunk */
int *keyboard_macro_pos;			/* where replay or recording has got to */
int *keyboard_macro_end = &keyboard_macro[0];	/* one past the last recorded keystroke */
int keyboard_macro_mode = STOP;			/* STOP, PLAY or RECORD */
int keyboard_macro_repeat = 0;			/* how many times left to replay */
int restflag = FALSE;				/* restricted use?              */
int last_key = 0;				/* last keystroke, the $lastkey setting */
int random_seed = 0;				/* random number seed, the $seed setting */
int macro_debug = FALSE;			/* trace macros, the $debug setting */
char errorm[] = "ERROR";			/* error literal                */
char truem[] = "TRUE";				/* true literal                 */
char falsem[] = "FALSE";			/* false litereal               */
int command_status = TRUE;			/* last command status, the $status setting */
int saveflag = 0;				/* Flags, saved with the $target var */
char *file_line = NULL;				/* buffer file_get_line() reads a line into */
int file_line_size = 0;				/* how big that allocation is, not the length in it */
int rval = 0;					/* return value of a subprocess */
int nullflag = FALSE;				/* accept null characters */
int justflag = FALSE;				/* justify, don't fill */
int overlap = 0;				/* line overlap in forw/back page */
int scroll_lines = 1;				/* lines a scroll moves, the $jump setting */

/* uninitialized global definitions */

int cursor_row;					/* where the cursor belongs, worked out from dot */
int cursor_col;					/* and its column, counting from the line start */
int thisflag;					/* Flags, this command          */
int lastflag;					/* Flags, last command          */
int curgoal;					/* Goal for C-P, C-N            */
struct window *curwp;				/* Current window               */
struct buffer *curbp;				/* Current buffer               */
struct buffer *bheadp;				/* Head of list of buffers      */
struct buffer *blistp;				/* Buffer for C-X C-B           */

char pat[NPAT];					/* Search pattern               */
char tap[NPAT];					/* Reversed pattern array.      */
char rpat[NPAT];				/* replacement pattern          */

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
struct line *matchline = NULL;
int matchoff = 0;

/* directive name table:
	This holds the names of all the directives....	*/

char *dname[] = {
	"if", "else", "endif",
	"goto", "return", "endm",
	"while", "endwhile", "break",
	"force"
};
