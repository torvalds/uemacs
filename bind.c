/*	bind.c
 *
 *	This file is for functions having to do with key bindings,
 *	descriptions, help commands and startup file.
 *
 *	Written 11-feb-86 by Daniel Lawrence
 *	Modified by Petri Kutvonen
 */

#include <stdio.h>

#include "estruct.h"
#include "globals.h"
#include "efunc.h"
#include "epath.h"
#include "line.h"
#include "util.h"

/*
 * Show the help file, in a window of its own and in view mode.  Bound to
 * "M-?".
 *
 * The lookup asks flook() to try $HOME and $HOME/lib, which is the whole
 * reason this works at all: the Makefile installs emacs.hlp into
 * $(HOME)/lib, and that is the one place the old FALSE here told flook
 * not to look.  Unless you happened to be sitting in a directory with a
 * copy of the file in it, help had nothing to show.
 */
int cmd_help(int f, int n)
{
	struct buffer *bp;
	char *fname = NULL;

	/* already read in once? */
	bp = bfind("emacs.hlp", FALSE, BFINVS);
	if (bp == NULL) {
		fname = flook(pathname[1], TRUE);
		if (fname == NULL) {
			msg_printf("(Help file is not online)");
			return FALSE;
		}
	}

	/* a window to put it in */
	if (cmd_split_current_window(FALSE, 1) == FALSE)
		return FALSE;

	if (bp == NULL) {
		if (getfile(fname, FALSE) == FALSE)
			return FALSE;
	} else
		swbuffer(bp);

	curwp->w_bufp->b_mode |= MDVIEW;
	curwp->w_bufp->b_flag |= BFINVS;
	update_modeline();
	return TRUE;
}

/*
 * Fill the *Binding list* buffer with every command name, each followed by
 * the keys bound to it, and switch to it.  With a match string only the
 * names containing it are listed, which is what apropos is.
 *
 * Like list-buffers, this builds a buffer and goes there rather than
 * popping up a window for it - see commit cfad0e9 ("Restore list-buffers,
 * without needing a window to put it in").  C-x b gets you back.
 */
static int build_binding_list(char *match)
{
	struct name_bind *nptr;
	struct key_tab *ktp;
	struct buffer *bp;
	char outseq[NSTRING];
	int cpos;

	bp = bfind("*Binding list*", TRUE, BFINVS);
	if (bp == NULL || bclear(bp) == FALSE) {
		msg_printf("Can not display binding list");
		return FALSE;
	}

	for (nptr = &names[0]; nptr->n_func != NULL; ++nptr) {
		if (match && strinc(nptr->n_name, match) == FALSE)
			continue;

		strcpy(outseq, nptr->n_name);
		cpos = strlen(outseq);

		/* every key bound to it, one line each */
		for (ktp = &keytab[0]; ktp->k_fp != NULL; ++ktp) {
			if (ktp->k_fp != nptr->n_func)
				continue;
			while (cpos < 28)
				outseq[cpos++] = ' ';
			cmdstr(ktp->k_code, &outseq[cpos]);
			if (addline(bp, outseq) == FALSE)
				return FALSE;
			cpos = 0;		/* the name is said already */
		}

		/* and the ones with no key at all still get a line */
		if (cpos > 0) {
			outseq[cpos] = 0;
			if (addline(bp, outseq) == FALSE)
				return FALSE;
		}
	}

	bp->b_mode |= MDVIEW;
	bp->b_flag &= ~BFCHG;

	if (curwp->w_bufp != bp && swbuffer(bp) != TRUE)
		return FALSE;
	curwp->w_linep = line_next(bp->b_linep);
	curwp->w_dotp = line_next(bp->b_linep);
	curwp->w_doto = 0;
	curwp->w_markp = NULL;
	curwp->w_marko = 0;
	curwp->w_flag |= WFMODE | WFHARD;
	return TRUE;
}

/*
 * Every key binding there is.  Bound to nothing by default; upstream
 * bound it to nothing either, and M-x is how you get at it.
 */
int cmd_describe_bindings(int f, int n)
{
	return build_binding_list(NULL);
}

/*
 * The same list, narrowed to the command names containing a string.
 * Bound to "M-A".
 */
int cmd_apropos(int f, int n)
{
	char match[NSTRING];
	int status;

	status = ask_string("Apropos string: ", match, NSTRING - 1);
	if (status != TRUE)
		return status;
	return build_binding_list(match);
}

int cmd_describe_key(int f, int n)
{						/* describe the command for a certain key */
	int c;					/* key to describe */
	char *ptr;				/* string pointer to scan output strings */
	char outseq[NSTRING];			/* output buffer for command sequence */

	/* prompt the user to type us a key to describe */
	msg_printf(": describe-key ");

	/* get the command sequence to describe
	   change it to something we can print as well */
	cmdstr(c = getckey(FALSE), &outseq[0]);

	/* and dump it out */
	ostring(outseq);
	ostring(" ");

	/* find the right ->function */
	if ((ptr = getfname(getbind(c))) == NULL)
		ptr = "Not Bound";

	/* output the command sequence */
	ostring(ptr);
	return TRUE;
}

/*
 * bindtokey:
 *	add a new key to the key binding table
 *
 * int f, n;		command arguments [IGNORED]
 */
int cmd_bind_to_key(int f, int n)
{
	unsigned int c;				/* command key to bind */
	fn_t kfunc;				/* ptr to the requested function to bind to */
	struct key_tab *ktp;			/* pointer into the command table */
	int found;				/* matched command flag */
	char outseq[80];			/* output buffer for keystroke sequence */

	/* prompt the user to type in a key to bind */
	msg_printf(": bind-to-key ");

	/* get the function name to bind it to */
	kfunc = getname();
	if (kfunc == NULL) {
		msg_printf("(No such function)");
		return FALSE;
	}
	ostring(" ");

	/* get the command sequence to bind */
	c = getckey((kfunc == cmd_meta_prefix) || (kfunc == cmd_ctlx_prefix) || (kfunc == cmd_universal_argument) || (kfunc == cmd_abort_command));

	/* change it to something we can print as well */
	cmdstr(c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);

	/* if the function is a prefix key */
	if (kfunc == cmd_meta_prefix || kfunc == cmd_ctlx_prefix || kfunc == cmd_universal_argument || kfunc == cmd_abort_command) {

		/* search for an existing binding for the prefix key */
		ktp = &keytab[0];
		found = FALSE;
		while (ktp->k_fp != NULL) {
			if (ktp->k_fp == kfunc)
				unbindchar(ktp->k_code);
			++ktp;
		}

		/* reset the appropriate global prefix variable */
		if (kfunc == cmd_meta_prefix)
			meta_char = c;
		if (kfunc == cmd_ctlx_prefix)
			ctlx_char = c;
		if (kfunc == cmd_universal_argument)
			repeat_key = c;
		if (kfunc == cmd_abort_command)
			abort_char = c;
	}

	/* search the table to see if it exists */
	ktp = &keytab[0];
	found = FALSE;
	while (ktp->k_fp != NULL) {
		if (ktp->k_code == c) {
			found = TRUE;
			break;
		}
		++ktp;
	}

	if (found) {				/* it exists, just change it then */
		ktp->k_fp = kfunc;
	} else {				/* otherwise we need to add it to the end */
		/* if we run out of binding room, bitch */
		if (ktp >= &keytab[NBINDS]) {
			msg_printf("Binding table FULL!");
			return FALSE;
		}

		ktp->k_code = c;		/* add keycode */
		ktp->k_fp = kfunc;		/* and the function pointer */
		++ktp;				/* and make sure the next is null */
		ktp->k_code = 0;
		ktp->k_fp = NULL;
	}
	return TRUE;
}

/*
 * unbindkey:
 *	delete a key from the key binding table
 *
 * int f, n;		command arguments [IGNORED]
 */
int cmd_unbind_key(int f, int n)
{
	int c;					/* command key to unbind */
	char outseq[80];			/* output buffer for keystroke sequence */

	/* prompt the user to type in a key to unbind */
	msg_printf(": unbind-key ");

	/* get the command sequence to unbind */
	c = getckey(FALSE);			/* get a command sequence */

	/* change it to something we can print as well */
	cmdstr(c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);

	/* if it isn't bound, bitch */
	if (unbindchar(c) == FALSE) {
		msg_printf("(Key not bound)");
		return FALSE;
	}
	return TRUE;
}

/*
 * unbindchar()
 *
 * int c;		command key to unbind
 */
int unbindchar(int c)
{
	struct key_tab *ktp;			/* pointer into the command table */
	struct key_tab *sktp;			/* saved pointer into the command table */
	int found;				/* matched command flag */

	/* search the table to see if the key exists */
	ktp = &keytab[0];
	found = FALSE;
	while (ktp->k_fp != NULL) {
		if (ktp->k_code == c) {
			found = TRUE;
			break;
		}
		++ktp;
	}

	/* if it isn't bound, bitch */
	if (!found)
		return FALSE;

	/* save the pointer and scan to the end of the table */
	sktp = ktp;
	while (ktp->k_fp != NULL)
		++ktp;
	--ktp;					/* backup to the last legit entry */

	/* copy the last entry to the current one */
	sktp->k_code = ktp->k_code;
	sktp->k_fp = ktp->k_fp;

	/* null out the last one */
	ktp->k_code = 0;
	ktp->k_fp = NULL;
	return TRUE;
}

/*
 * does source include sub?
 *
 * char *source;	string to search in
 * char *sub;		substring to look for
 */
int strinc(char *source, char *sub)
{
	char *sp;				/* ptr into source */
	char *nxtsp;				/* next ptr into source */
	char *tp;				/* ptr into substring */

	/* for each character in the source string */
	sp = source;
	while (*sp) {
		tp = sub;
		nxtsp = sp;

		/* is the substring here? */
		while (*tp) {
			if (*nxtsp++ != *tp)
				break;
			else
				tp++;
		}

		/* yes, return a success */
		if (*tp == 0)
			return TRUE;

		/* no, onward */
		sp++;
	}
	return FALSE;
}

/*
 * get a command key sequence from the keyboard
 *
 * int mflag;		going for a meta sequence?
 */
unsigned int getckey(int mflag)
{
	unsigned int c;				/* character fetched */
	char tok[NSTRING];			/* command incoming */

	/* check to see if we are executing a command line */
	if (executing_command_line) {
		macarg(tok);			/* get the next token */
		return stock(tok);
	}

	/* or the normal way */
	if (mflag)
		c = get1key();
	else
		c = getcmd();
	return c;
}

/*
 * execute the startup file
 *
 * char *sfname;	name of startup file (null if default)
 */
int startup(char *sfname)
{
	char *fname;				/* resulting file name to execute */

	/* look up the startup file */
	if (*sfname != 0)
		fname = flook(sfname, TRUE);
	else
		fname = flook(pathname[0], TRUE);

	/* if it isn't around, don't sweat it */
	if (fname == NULL)
		return TRUE;

	/* otherwise, execute the sucker */
	return dofile(fname);
}

/*
 * Look up the existance of a file along the normal or PATH
 * environment variable. Look first in the HOME directory if
 * asked and possible
 *
 * char *fname;		base file name to search for
 * int hflag;		Look in the HOME environment variable first?
 */
char *flook(char *fname, int hflag)
{
	char *home;				/* path to home directory */
	char *path;				/* environmental PATH variable */
	char *sp;				/* pointer into path spec */
	int i;					/* index */
	static char fspec[NSTRING];		/* full path spec to search */

	if (hflag) {
		home = getenv("HOME");
		if (home != NULL) {
			snprintf(fspec, sizeof(fspec), "%s/%s", home, fname);

			/* and try it out */
			if (file_open_read(fspec) == FIOSUC) {
				file_close();
				return fspec;
			}

			snprintf(fspec, sizeof(fspec), "%s/lib/%s", home, fname);
			if (file_open_read(fspec) == FIOSUC) {
				file_close();
				return fspec;
			}
		}
	}

	/* always try the current directory first */
	if (file_open_read(fname) == FIOSUC) {
		file_close();
		return fname;
	}

	/* get the PATH variable */
	path = getenv("PATH");
	if (path != NULL)
		while (*path) {

			/* build next possible file spec */
			sp = fspec;
			while (*path && (*path != PATHCHR))
				*sp++ = *path++;

			/* add a terminating dir separator if we need it */
			if (sp != fspec)
				*sp++ = '/';
			*sp = 0;
			strcat(fspec, fname);

			/* and try it out */
			if (file_open_read(fspec) == FIOSUC) {
				file_close();
				return fspec;
			}

			if (*path == PATHCHR)
				++path;
		}

	/* look it up via the old table method */
	for (i = 2; i < ARRAY_SIZE(pathname); i++) {
		strcpy(fspec, pathname[i]);
		strcat(fspec, fname);

		/* and try it out */
		if (file_open_read(fspec) == FIOSUC) {
			file_close();
			return fspec;
		}
	}

	return NULL;				/* no such luck */
}

/*
 * change a key command to a string we can print out
 *
 * int c;		sequence to translate
 * char *seq;		destination string for sequence
 */
void cmdstr(int c, char *seq)
{
	char *ptr;				/* pointer into current position in sequence */

	ptr = seq;

	/* apply meta sequence if needed */
	if (c & META) {
		*ptr++ = 'M';
		*ptr++ = '-';
	}

	/* apply ^X sequence if needed */
	if (c & CTLX) {
		*ptr++ = '^';
		*ptr++ = 'X';
	}

	/* apply SPEC sequence if needed */
	if (c & SPEC) {
		*ptr++ = 'F';
		*ptr++ = 'N';
	}

	/* apply control sequence if needed */
	if (c & CONTROL) {
		*ptr++ = '^';
	}

	/* and output the final sequence */

	*ptr++ = c & 255;			/* strip the prefixes */

	*ptr = 0;				/* terminate the string */
}

/*
 * This function looks a key binding up in the binding table
 *
 * int c;		key to find what is bound to it
 */
int (*getbind(int c))(int, int) {
	struct key_tab *ktp;

	ktp = &keytab[0];			/* Look in key table. */
	while (ktp->k_fp != NULL) {
		if (ktp->k_code == c)
			return ktp->k_fp;
		++ktp;
	}

	/* no such binding */
	return NULL;
}

/*
 * getfname:
 *	This function takes a ptr to function and gets the name
 *	associated with it.
 */
char *getfname(fn_t func)
{
	struct name_bind *nptr;			/* pointer into the name binding table */

	/* skim through the table, looking for a match */
	nptr = &names[0];
	while (nptr->n_func != NULL) {
		if (nptr->n_func == func)
			return nptr->n_name;
		++nptr;
	}
	return NULL;
}

/*
 * match fname to a function in the names table
 * and return any match or NULL if none
 *
 * char *fname;		name to attempt to match
 */
int (*fncmatch(char *fname))(int, int) {
	struct name_bind *ffp;			/* pointer to entry in name binding table */

	/* scan through the table, returning any match */
	ffp = &names[0];
	while (ffp->n_func != NULL) {
		if (strcmp(fname, ffp->n_name) == 0)
			return ffp->n_func;
		++ffp;
	}
	return NULL;
}

/*
 * stock:
 *	String key name TO Command Key
 *
 * char *keyname;	name of key to translate to Command key form
 */
unsigned int stock(char *keyname)
{
	unsigned int c;				/* key sequence to return */

	/* parse it up */
	c = 0;

	/* first, the META prefix */
	if (*keyname == 'M' && *(keyname + 1) == '-') {
		c = META;
		keyname += 2;
	}

	/* next the function prefix */
	if (*keyname == 'F' && *(keyname + 1) == 'N') {
		c |= SPEC;
		keyname += 2;
	}

	/* control-x as well... (but not with FN) */
	if (*keyname == '^' && *(keyname + 1) == 'X' && !(c & SPEC)) {
		c |= CTLX;
		keyname += 2;
	}

	/* a control char? */
	if (*keyname == '^' && *(keyname + 1) != 0) {
		c |= CONTROL;
		++keyname;
	}
	if (*keyname < 32) {
		c |= CONTROL;
		*keyname += 'A';
	}

	/* make sure we are not lower case (not with function keys) */
	if (*keyname >= 'a' && *keyname <= 'z' && !(c & SPEC))
		*keyname -= 32;

	/* the final sequence... */
	c |= *keyname;
	return c;
}

/*
 * string key name to binding name....
 *
 * char *skey;		name of keey to get binding for
 */
char *transbind(char *skey)
{
	char *bindname;

	bindname = getfname(getbind(stock(skey)));
	if (bindname == NULL)
		bindname = "ERROR";

	return bindname;
}
