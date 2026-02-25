/*	ebind.h
 *
 *	Initial default key to function bindings
 *
 *	Modified by Petri Kutvonen
 */

#ifndef EBIND_H_
#define EBIND_H_

#include "line.h"

/*
 * Command table.
 * This table  is *roughly* in ASCII order, left to right across the
 * characters of the command. This explains the funny location of the
 * control-X commands.
 */
struct key_tab keytab[NBINDS] = {
	{ CONTROL | 'A', gotobol },
	{ CONTROL | 'B', backchar },
	{ CONTROL | 'C', insspace },
	{ CONTROL | 'D', forwdel },
	{ CONTROL | 'E', gotoeol },
	{ CONTROL | 'F', forwchar },
	{ CONTROL | 'G', ctrlg },
	{ CONTROL | 'H', backdel },
	{ CONTROL | 'I', insert_tab },
	{ CONTROL | 'J', indent },
	{ CONTROL | 'K', killtext },
	{ CONTROL | 'L', redraw },
	{ CONTROL | 'M', insert_newline },
	{ CONTROL | 'N', forwline },
	{ CONTROL | 'O', openline },
	{ CONTROL | 'P', backline },
	{ CONTROL | 'Q', quote },
	{ CONTROL | 'R', backsearch },
	{ CONTROL | 'S', forwsearch },
	{ CONTROL | 'T', twiddle },
	{ CONTROL | 'U', unarg },
	{ CONTROL | 'V', forwpage },
	{ CONTROL | 'W', killregion },
	{ CONTROL | 'X', cex },
	{ CONTROL | 'Y', yank },
	{ CONTROL | 'Z', backpage },
	{ CONTROL | ']', metafn },
	{ CTLX | CONTROL | 'C', quit },		/* Hard quit.           */
	{ CTLX | CONTROL | 'A', detab },
	{ CTLX | CONTROL | 'D', filesave },	/* alternative          */
	{ CTLX | CONTROL | 'E', entab },
	{ CTLX | CONTROL | 'F', filefind },
	{ CTLX | CONTROL | 'I', insfile },
	{ CTLX | CONTROL | 'L', lowerregion },
	{ CTLX | CONTROL | 'M', delmode },
	{ CTLX | CONTROL | 'O', deblank },
	{ CTLX | CONTROL | 'R', fileread },
	{ CTLX | CONTROL | 'S', filesave },
	{ CTLX | CONTROL | 'T', trim },
	{ CTLX | CONTROL | 'U', upperregion },
	{ CTLX | CONTROL | 'V', viewfile },
	{ CTLX | CONTROL | 'W', filewrite },
	{ CTLX | CONTROL | 'X', swapmark },
	{ CTLX | '?', deskey },
	{ CTLX | '!', spawn },
	{ CTLX | '#', filter_buffer },
	{ CTLX | '$', execprg },
	{ CTLX | '=', showcpos },
	{ CTLX | '(', ctlxlp },
	{ CTLX | ')', ctlxrp },
	{ CTLX | 'A', setvar },
	{ CTLX | 'B', usebuffer },
	{ CTLX | 'C', spawncli },
	{ CTLX | 'D', bktoshell },
	{ CTLX | 'E', ctlxe },
	{ CTLX | 'F', setfillcol },
	{ CTLX | 'K', killbuffer },
	{ CTLX | 'M', setemode },
	{ CTLX | 'N', filename },
	{ CTLX | 'Q', quote },			/* alternative  */
	{ CTLX | 'R', risearch },
	{ CTLX | 'S', fisearch },
	{ CTLX | 'X', nextbuffer },
	{ META | CONTROL | 'C', wordcount },
	{ META | CONTROL | 'D', newsize },
	{ META | CONTROL | 'E', execproc },
	{ META | CONTROL | 'F', getfence },
	{ META | CONTROL | 'H', delbword },
	{ META | CONTROL | 'K', unbindkey },
	{ META | CONTROL | 'L', reposition },
	{ META | CONTROL | 'M', delgmode },
	{ META | CONTROL | 'N', namebuffer },
	{ META | CONTROL | 'R', qreplace },
	{ META | CONTROL | 'S', newsize },
	{ META | CONTROL | 'T', newwidth },
	{ META | CONTROL | 'W', killpara },
	{ META | ' ', setmark },
	{ META | '!', reposition },
	{ META | '.', setmark },
	{ META | '>', gotoeob },
	{ META | '<', gotobob },
	{ META | '~', unmark },
	{ META | 'B', backword },
	{ META | 'C', capword },
	{ META | 'D', delfword },
	{ META | 'F', forwword },
	{ META | 'G', gotoline },
	{ META | 'J', justpara },
	{ META | 'K', bindtokey },
	{ META | 'L', lowerword },
	{ META | 'M', setgmode },
	{ META | 'N', gotoeop },
	{ META | 'P', gotobop },
	{ META | 'Q', fillpara },
	{ META | 'R', sreplace },
	{ META | 'S', forwsearch },		/* alternative P.K.     */
	{ META | 'U', upperword },
	{ META | 'V', backpage },
	{ META | 'W', copyregion },
	{ META | 'X', namedcmd },
	{ META | 'Z', quickexit },
	{ META | 0x7F, delbword },
	{ SPEC | '1', fisearch },		/* VT220 keys   */
	{ SPEC | '2', yank },
	{ SPEC | '3', killregion },
	{ SPEC | '4', setmark },
	{ SPEC | '5', backpage },
	{ SPEC | '6', forwpage },
	{ SPEC | 'A', backline },
	{ SPEC | 'B', forwline },
	{ SPEC | 'C', forwchar },
	{ SPEC | 'D', backchar },
	{ SPEC | 'c', metafn },
	{ SPEC | 'd', backchar },
	{ SPEC | 'e', forwline },
	{ SPEC | 'f', gotobob },
	{ SPEC | 'i', cex },

	{ 0x7F, backdel },

	/* special internal bindings */
	{ SPEC | META | 'W', wrapword },	/* called on word wrap */
	{ SPEC | META | 'C', nullproc },	/*  every command input */
	{ SPEC | META | 'R', nullproc },	/*  on file read */
	{ SPEC | META | 'X', nullproc },	/*  on window change P.K. */

	{ 0, NULL }
};

#endif				/* EBIND_H_ */
