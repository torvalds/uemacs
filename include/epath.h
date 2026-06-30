/*	EPATH.H
 *
 *	This file contains certain info needed to locate the
 *	initialization (etc) files on a system dependent basis
 *
 *	modified by Petri Kutvonen
 */
#ifndef EPATH_H_
#define EPATH_H_

/*	possible names and paths of help files under different OSs	*/
static char *pathname[] =
#if	MSDOS
{
	"emacs.rc",
	"emacs.hlp",
	"\\sys\\public\\",
	"\\usr\\bin\\",
	"\\bin\\",
	"\\",
	""
};
#endif

#if	V7 | BSD | USG
{
	".emacsrc", "emacs.hlp",
#if	PKCODE
	    "/usr/global/lib/", "/usr/local/bin/", "/usr/local/lib/",
#endif
"/usr/local/", "/usr/lib/", ""};
#endif

#if	VMS
{
	"emacs.rc", "emacs.hlp", "",
#if	PKCODE
	    "sys$login:", "emacs_dir:",
#endif
"sys$sysdevice:[vmstools]"};
#endif

#endif  /* EPATH_H_ */
