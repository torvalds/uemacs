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
	{CONTROL | 'A', gotobol}
	,
	{CONTROL | 'B', backchar}
	,
	{CONTROL | 'C', insspace}
	,
	{CONTROL | 'D', forwdel}
	,
	{CONTROL | 'E', gotoeol}
	,
	{CONTROL | 'F', forwchar}
	,
	{CONTROL | 'G', ctrlg}
	,
	{CONTROL | 'H', backdel}
	,
	{CONTROL | 'I', insert_tab}
	,
	{CONTROL | 'J', indent}
	,
	{CONTROL | 'K', killtext}
	,
	{CONTROL | 'L', redraw}
	,
	{CONTROL | 'M', insert_newline}
	,
	{CONTROL | 'N', forwline}
	,
	{CONTROL | 'O', openline}
	,
	{CONTROL | 'P', backline}
	,
	{CONTROL | 'Q', quote}
	,
	{CONTROL | 'R', backsearch}
	,
	{CONTROL | 'S', forwsearch}
	,
	{CONTROL | 'T', twiddle}
	,
	{CONTROL | 'U', unarg}
	,
	{CONTROL | 'V', forwpage}
	,
	{CONTROL | 'W', killregion}
	,
	{CONTROL | 'X', cex}
	,
	{CONTROL | 'Y', yank}
	,
	{CONTROL | 'Z', backpage}
	,
	{CONTROL | ']', metafn}
	,
	{CTLX | CONTROL | 'B', listbuffers}
	,
	{CTLX | CONTROL | 'C', quit}
	,			/* Hard quit.           */
#if	PKCODE & AEDIT
	{CTLX | CONTROL | 'A', detab}
	,
#endif
#if	PKCODE
	{CTLX | CONTROL | 'D', filesave}
	,			/* alternative          */
#else
#if	AEDIT
	{CTLX | CONTROL | 'D', detab}
	,
#endif
#endif
#if	AEDIT
	{CTLX | CONTROL | 'E', entab}
	,
#endif
	{CTLX | CONTROL | 'F', filefind}
	,
	{CTLX | CONTROL | 'I', insfile}
	,
	{CTLX | CONTROL | 'L', lowerregion}
	,
	{CTLX | CONTROL | 'M', delmode}
	,
	{CTLX | CONTROL | 'N', mvdnwind}
	,
	{CTLX | CONTROL | 'O', deblank}
	,
	{CTLX | CONTROL | 'P', mvupwind}
	,
	{CTLX | CONTROL | 'R', fileread}
	,
	{CTLX | CONTROL | 'S', filesave}
	,
#if	AEDIT
	{CTLX | CONTROL | 'T', trim}
	,
#endif
	{CTLX | CONTROL | 'U', upperregion}
	,
	{CTLX | CONTROL | 'V', viewfile}
	,
	{CTLX | CONTROL | 'W', filewrite}
	,
	{CTLX | CONTROL | 'X', swapmark}
	,
	{CTLX | CONTROL | 'Z', shrinkwind}
	,
	{CTLX | '?', deskey}
	,
	{CTLX | '!', spawn}
	,
	{CTLX | '@', pipecmd}
	,
	{CTLX | '#', filter_buffer}
	,
	{CTLX | '$', execprg}
	,
	{CTLX | '=', showcpos}
	,
	{CTLX | '(', ctlxlp}
	,
	{CTLX | ')', ctlxrp}
	,
	{CTLX | '^', enlargewind}
	,
	{CTLX | '0', delwind}
	,
	{CTLX | '1', onlywind}
	,
	{CTLX | '2', splitwind}
	,
	{CTLX | 'A', setvar}
	,
	{CTLX | 'B', usebuffer}
	,
	{CTLX | 'C', spawncli}
	,
#if	BSD | __hpux | SVR4
	{CTLX | 'D', bktoshell}
	,
#endif
	{CTLX | 'E', ctlxe}
	,
	{CTLX | 'F', setfillcol}
	,
	{CTLX | 'K', killbuffer}
	,
	{CTLX | 'M', setemode}
	,
	{CTLX | 'N', filename}
	,
	{CTLX | 'O', nextwind}
	,
	{CTLX | 'P', prevwind}
	,
#if	PKCODE
	{CTLX | 'Q', quote}
	,			/* alternative  */
#endif
#if	ISRCH
	{CTLX | 'R', risearch}
	,
	{CTLX | 'S', fisearch}
	,
#endif
	{CTLX | 'W', resize}
	,
	{CTLX | 'X', nextbuffer}
	,
	{CTLX | 'Z', enlargewind}
	,
#if	WORDPRO
	{META | CONTROL | 'C', wordcount}
	,
#endif
#if	PKCODE
	{META | CONTROL | 'D', newsize}
	,
#endif
#if	PROC
	{META | CONTROL | 'E', execproc}
	,
#endif
#if	CFENCE
	{META | CONTROL | 'F', getfence}
	,
#endif
	{META | CONTROL | 'H', delbword}
	,
	{META | CONTROL | 'K', unbindkey}
	,
	{META | CONTROL | 'L', reposition}
	,
	{META | CONTROL | 'M', delgmode}
	,
	{META | CONTROL | 'N', namebuffer}
	,
	{META | CONTROL | 'R', qreplace}
	,
	{META | CONTROL | 'S', newsize}
	,
	{META | CONTROL | 'T', newwidth}
	,
	{META | CONTROL | 'V', scrnextdw}
	,
#if	WORDPRO
	{META | CONTROL | 'W', killpara}
	,
#endif
	{META | CONTROL | 'Z', scrnextup}
	,
	{META | ' ', setmark}
	,
	{META | '?', help}
	,
	{META | '!', reposition}
	,
	{META | '.', setmark}
	,
	{META | '>', gotoeob}
	,
	{META | '<', gotobob}
	,
	{META | '~', unmark}
	,
#if	APROP
	{META | 'A', apro}
	,
#endif
	{META | 'B', backword}
	,
	{META | 'C', capword}
	,
	{META | 'D', delfword}
	,
#if	CRYPT
	{META | 'E', set_encryption_key}
	,
#endif
	{META | 'F', forwword}
	,
	{META | 'G', gotoline}
	,
#if	PKCODE
#if	WORDPRO
	{META | 'J', justpara}
	,
#endif
#endif
	{META | 'K', bindtokey}
	,
	{META | 'L', lowerword}
	,
	{META | 'M', setgmode}
	,
#if	WORDPRO
	{META | 'N', gotoeop}
	,
	{META | 'P', gotobop}
	,
	{META | 'Q', fillpara}
	,
#endif
	{META | 'R', sreplace}
	,
#if	PKCODE
	{META | 'S', forwsearch}
	,			/* alternative P.K.     */
#else
#if	BSD
	{META | 'S', bktoshell}
	,
#endif
#endif
	{META | 'U', upperword}
	,
	{META | 'V', backpage}
	,
	{META | 'W', copyregion}
	,
	{META | 'X', namedcmd}
	,
	{META | 'Z', quickexit}
	,
	{META | 0x7F, delbword}
	,

#if	MSDOS
	{SPEC | CONTROL | '_', forwhunt}
	,
	{SPEC | CONTROL | 'S', backhunt}
	,
	{SPEC | 71, gotobol}
	,
	{SPEC | 72, backline}
	,
	{SPEC | 73, backpage}
	,
	{SPEC | 75, backchar}
	,
	{SPEC | 77, forwchar}
	,
	{SPEC | 79, gotoeol}
	,
	{SPEC | 80, forwline}
	,
	{SPEC | 81, forwpage}
	,
	{SPEC | 82, insspace}
	,
	{SPEC | 83, forwdel}
	,
	{SPEC | 115, backword}
	,
	{SPEC | 116, forwword}
	,
#if	WORDPRO
	{SPEC | 132, gotobop}
	,
	{SPEC | 118, gotoeop}
	,
#endif
	{SPEC | 84, cbuf1}
	,
	{SPEC | 85, cbuf2}
	,
	{SPEC | 86, cbuf3}
	,
	{SPEC | 87, cbuf4}
	,
	{SPEC | 88, cbuf5}
	,
	{SPEC | 89, cbuf6}
	,
	{SPEC | 90, cbuf7}
	,
	{SPEC | 91, cbuf8}
	,
	{SPEC | 92, cbuf9}
	,
	{SPEC | 93, cbuf10}
	,
#if PKCODE
	{SPEC | 117, gotoeob}
	,
	{SPEC | 119, gotobob}
	,
	{SPEC | 141, gotobop}
	,
	{SPEC | 145, gotoeop}
	,
	{SPEC | 146, yank}
	,
	{SPEC | 147, killregion}
	,
#endif
#endif

#if	VT220
	{SPEC | '1', fisearch}
	,			/* VT220 keys   */
	{SPEC | '2', yank}
	,
	{SPEC | '3', killregion}
	,
	{SPEC | '4', setmark}
	,
	{SPEC | '5', backpage}
	,
	{SPEC | '6', forwpage}
	,
	{SPEC | 'A', backline}
	,
	{SPEC | 'B', forwline}
	,
	{SPEC | 'C', forwchar}
	,
	{SPEC | 'D', backchar}
	,
	{SPEC | 'c', metafn}
	,
	{SPEC | 'd', backchar}
	,
	{SPEC | 'e', forwline}
	,
	{SPEC | 'f', gotobob}
	,
	{SPEC | 'h', help}
	,
	{SPEC | 'i', cex}
	,
#endif

	{0x7F, backdel}
	,

	/* special internal bindings */
	{ SPEC | META | 'W', wrapword },	/* called on word wrap */
	{ SPEC | META | 'C', nullproc },	/*  every command input */
	{ SPEC | META | 'R', nullproc },	/*  on file read */
	{ SPEC | META | 'X', nullproc },	/*  on window change P.K. */

	{0, NULL}
};

#endif  /* EBIND_H_ */
