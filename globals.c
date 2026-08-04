#include "estruct.h"
#include "globals.h"

/* initialized global definitions */

int fill_column = 72;				/* fill column, the $fillcol setting */
int keyboard_macro[NKBDM];			/* the recorded keystrokes */
char *command_string = NULL;			/* the command line being executed, or NULL */
char goto_label[NPAT] = "";			/* the label a !goto is looking for */
int if_level = 0;				/* !if levels being skipped; 0 means executing */
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
int executing_command_line = FALSE;		/* running a command line, not reading keys */
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
char error_text[] = "ERROR";			/* what a macro expression evaluates to on an error */
char true_text[] = "TRUE";			/* and what TRUE and FALSE come back as */
char false_text[] = "FALSE";
int command_status = TRUE;			/* last command status, the $status setting */
int saveflag = 0;				/* Flags, saved with the $target var */
char *file_line = NULL;				/* buffer file_get_line() reads a line into */
int file_line_size = 0;				/* how big that allocation is, not the length in it */
int subprocess_status = 0;			/* what the last shell command exited with, the $rval setting */
int nullflag = FALSE;				/* accept null characters */
int justflag = FALSE;				/* justify, don't fill */
int overlap = 0;				/* line overlap in forw/back page */
int scroll_lines = 1;				/* lines a scroll moves, the $jump setting */

/* uninitialized global definitions */

int cursor_row;					/* where the cursor belongs, worked out from dot */
int cursor_col;					/* and its column, counting from the line start */
int thisflag;					/* Flags, this command          */
int lastflag;					/* Flags, last command          */
int goal_column;				/* column C-p and C-n try to stay in */
struct window *curwp;				/* Current window               */
struct buffer *curbp;				/* Current buffer               */
struct buffer *buffer_head;			/* first of the chain of all buffers */
struct buffer *list_buffer;			/* the *List* buffer, which shows the others */

char search_pattern[NPAT];			/* what to search for */
char reversed_pattern[NPAT];			/* the same thing backwards, for reverse search */
char replace_pattern[NPAT];			/* what to replace it with */

/*
 * Where the last search matched, for the replace functions and for
 * $match.  match_line and match_offset are the start of it, and
 * match_bytes is how long it is - in bytes, from strlen(), which is
 * not the same as how many characters it is.
 */
unsigned int match_bytes = 0;			/* length of the last match, in bytes */
unsigned int previous_match_bytes = 0;		/* and of the one before, for undoing a replace */
char *matched_text = NULL;			/* the text that matched, for $match */
struct line *match_line = NULL;			/* the line the match started on */
int match_offset = 0;				/* and the byte offset within it */

/* directive name table:
	This holds the names of all the directives....	*/

char *directive_names[] = {
	"if", "else", "endif",
	"goto", "return", "endm",
	"while", "endwhile", "break",
	"force"
};
