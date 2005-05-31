		MicroEMACS 3.9 Release Notes	July 22, 1987

**********************************************************************

	(C)opyright 1987 by Daniel M. Lawrence
	MicroEMACS 3.9 can be copied and distributed freely for any
	non-commercial purposes. MicroEMACS 3.9 can only be incorporated
	into commercial software with the permission of the current author.

**********************************************************************

	MicroEMACS 3.9 is availible in a couple of different ways. 
First, it is availible via my bulletin board system..... 

	The Programmer's Room
	FIDO 201/2
	(317) 742-5533
	24 hours  300/1200 baud

	Also it should be online on the following BBS's:

	The Starship Trooper	Fido 201/1	(317) 423-2281	2400

	[These following two are open from 10pm till 5pm
				and only while Purdue is in session]
	The NightStaff		Fido 201/4	(317) 495-4270	1200
	The Access Violation	Fido 201/5	(317) 495-4270	9600

	There it is arranged as three MSDOS .ARC files, EMACSDOC.ARC
which contains the documentation and command files, EMACSSRC.ARC which
contains the sources and the UNIX Makefile, and EMACSEXE.EXE which
contains the MSDOS executables.  Also all the various executables are
available individually.

EMACSDOC.ARC includes the files:

	README		This file
	
	(These four files should be in your path for the standard setup)
	EMACS.RC	Standard startup file
	NEWPAGE.CMD	Shifted Function key Pager
	PPAGE.CMD	Programming page
	WPAGE.CMD	Word processing page
	BPAGE.CMD	Block and box manipulation page

	ME110.RC	HP110 startup file
	ME150.RC	HP150 startup file
	AMIGA.RC	AMIGA ".emacsrc" startup file
	ST520.RC	ATARI ST startup file

	EMACS.HLP	Online help file
	EMACS.MSS	MicroSCRIBE format of EMACS.TXT
	EMACS.TXT       EMACS BEGINNER'S/REFERENCE MANUAL

	AZMAP.CMD	Translate AZTEC .SYM files to .MAP
	BDATA.CMD	BASIC Data statement builder
	FINDCOM.CMD	Find mismatched C comments
	FUNC.CMD	Allow function keys on systems with non (like UNIX)
	MENU.CMD	Sample Menu system
	MENU1		   datafile for menu system
	SHELL.CMD	Sample interactive MSDOS shell
	TRAND.CMD	Generate random numbers and do statistics on them

EMACSSRC.ARC includes the files:

	ALINE.H		Atari ST graphic header file
	ANSI.C		ANSI screen driver
	BASIC.C		basic cursor movement
	BIND.C		key binding commands
	BUFFER.C	buffer manipulation commands
	CRYPT.C		encryption functions
	DOLOCK		file locking stub routines
	DG10.C		Data General 10 screen driver
	DISPLAY.C	main display driver
	EBIND.H		binding list
	EDEF.H		global variable declarations
	EFUNC.H		function name list
	EPATH.H		default path settings
	ESTRUCT.H	configuration and structure definitions
	EVAL.C		expression evaluator
	EVAR.H		EMACS macro variable declarations
	EXEC.C		macro execution functions
	FILE.C		user file functions
	FILEIO.C	low level file I/O driver
	HP110.C		HP110 screen driver
	HP150.C		HP150(A or C) screen driver
	IBMPC.C		IBM-PC CGA and MONOCHROME driver
	INPUT.C		low level user input driver
	ISEARCH.C	incremental search functions
	LINE.C		text line manipulation functions
	LOCK.C		file locking functions
	MAIN.C		argument parsing and command loop
	RANDOM.C	other random user functions
	REGION.C	region cut & paste functions
	SEARCH.C	search and replace functions
	SPAWN.C		OS interface driver
	ST520.C		ATARI ST1040 screen driver
	TCAP.C		Termcap screen driver
	TERMIO.C	low level I/O driver
	TIPC.C		TI-PC screen driver
	VMSVT.C		VMS screen driver
	VT52.C		VT52 screen driver
	WINDOW.C	window manipulation functions
	WORD.C		word manipulation functions
	Z309.C		Zenith 100 PC series terminal driver

EMACSEXE.ARC includes the files:

	MEIBM.EXE	IBM-PC CGA/MONO/EGA version
	MEANSI.EXE	MSDOS ANSI graphics version
	ME110.EXE	HP110 portable version
	ME150.EXE	HP150 version
	ME309.EXE	Zenith 100 PC series version
	ME520.PRG	Atari 520/1040ST version
	MEAMIGA.	Amiga 1000 version

	Recently, MicroSPELL 1.0 has been released.  This program allows
you to spell check text files and uses MicroEMACS to scan the file,
doing various corrections. 

	MicroSCRIBE, a fairly SCRIBE compatible text formatter to go
along with these programs will probably be available for beta testing
early spring 1988.  This program is turning out to be a lot more complex
than I thought it would be, and is taking more time to get out.

	I have in my possesion a port of MicroEMACS 3.8i to the
Macintosh, and I will be incorporating the needed changes for the current
version to support the Macintosh.

	As before, I will continue to support these programs, and
encourage everyone to spread them around as much as they can.  If you
make what you think are changes that are useful to many, send me the
updates, and as time permits, I will incorporate the ones I understand,
and agree with into the master sources.

	MicroEMACS is available on disk directly from my by sending me
$25 per order and a note specifying the disk format and the product that
you need.  I can fill orders for IBM-PC high/low density 5 1/4 and 3
1/5, ATARI ST single and double density, AMIGA disks and HP150 disks. 
(You do not need to send disks or mailers, I will provide these.) The
distribution set includes on disk all docs, executables and sources. 
Also I will register you and you will receive automatic notices of new
versions of all the programs I am releasing.

	Commercial lisences to allow MicroEMACS to be incorporated into
other software packages are also available at a reasonable per package
price. Also I am available to do customization of MicroEMACS at an
hourly rate. Send all requests to the address below:

	USmail:	Daniel Lawrence
		617 New York St
		Lafayette, IN 47901

	UUCP:	pur-ee!j.cc.purdue.edu!nwd
	ARPA:	nwd@j.cc.purdue.edu
	FIDO:	The Programmer's Room 201/2
		(317) 742-5533
	ATT:	(317) 742-5153

	
		New Features since version 3.8i
		===============================

**	New standard startup file

	The new emacs.rc file is segmented into more parts and loads much
faster than before. Separate "pages" of shifted function keys are
available. Users can write their own "pages".

***	New Variables (there are a lot...)

	$status		returns status of last command
	$palette	color palette settings
	$lastkey	returns last keystroke
	$curchar	returns and set the ascii number of the
			character under the point
	$progname	always returns "MicroEMACS"
	$version	always returns the current version ("3.9")
	$discmd		sets display of messages on the command
			line (except via the write-message command)
	$disinp		sets echoing of characters during input on the
			command line
	$wline		returns and sets # lines in current window
	$cwline		returns and set current line within window
	$target		returns/sets target for line moves
	$search		returns/sets default search string
	$replace	returns/sets default replace string
	$match		returns last matched string in magic search
	$cmode		returns/sets encoded mode of current buffer
	$gmode		returns/sets encoded global mode (see appendix E
			in emacs.mss to decode this)
	$tpause		returns/sets the pause for fence matching
			(this is in rather arbitrary units which
			WILL vary from machine to machine)
	$line		return/sets the contents of the current line
	$gflags		global operations flag (see emacs.txt Appendix G)
	$rval		child process return value

***	New computers supported

	Atari 1040ST	all three graphics modes and 50 line mode on a
			monochrome monitor. The mouse is bound to the 
			cursor keys for now.

***	New Compilers supported

	Turbo C v1.0 under MSDOS is now a supported compiler.
	Mark Williams C on the Atari ST is also supported.

**	New directives

	!while <condition>	loops while <cond> is true

		!break		breaks out of the innermost !while

	!endwhile		delimits the end of a !while loop

	All !gotos are legal into and out of a !while loop.

***	Autosave mode

	This mode saves the file out to disk every time 256 have been
inserted. $asave controls the # of characters between saves, $acount
controls the # of chars to the next save.

***	New functions

	&and	<log> <log>	Logical AND
	&or	<log> <log>	Logical OR
	&len	<str>		returns length of <str>
	&lower	<str>		lowercase <str>
	&upper	<str>		uppercase <str>
	&rnd	<int>		generate a random integer between 1 and <int>
	&sindex <str1> <str2>	search for string 2 within string 1
				returning its position, or zero if it fails
	&env	<str>		return value of DOS environment
				variable <str>
	&bind	<str>		returns the function name bound to the
				key <str>
	&exist	<str>		Does file <str> exist?
	&find	<str>		find file <str> along the PATH
	&band	<num> <num>	bitwise and
	&bor	<num> <num>	bitwise or
	&bxor	<num> <num>	bitwise xor
	&bnot	<num>		bitwise not
	&xlate	<str1> <str2> <str3>
				scan <str1> replacing characters in
				<str2> with the coresponding characters
				in <str3>

***	Advanced word processing commands

	^X^T	trim-line	trim all trailing whitespace
	^X^E	entab-line	change all multiple char runs to tabs
	^X^D	detab-line	change all tabs to multiple spaces

***	Merged EGA driver

	The EGA driver is now part of the IBM-PC driver. This driver now
supports MONO, CGA and EGA cards/modes. (settable by using the $sres
variable)

***	8 bit characters fully supported

	Eight bit characters (including foreign language and line
drawing characters) are now supported on the various micro environments)

***	List Buffers expanded

	Given a numeric argument, ^X^B (list-buffers) will now also list
all the hidden internal buffers.

***	-k switch enhanced

	If you use the -k (encrypted file) switch on the command line
without a key immediatly following it, it will prompt you for the key to
use to decrypt with.

***	word delete enhanced

	with a zero (0) argument, M-D (delete-next-word) deletes the
next word and not any intervening whitespace or special characters.

***	New File read hook

	Whenever MicroEMACS reads a file from disk, right before it is
read, whatever function is bound to M-FNR (which is an illegal
keystroke) will execute. By default this would be (nop), but the
standard emacs.rc binds this to a file that examines the file name and
places the buffer int CMODE if the extension ends in a .c or .h. You can
of course redefine this macro to taste.

***	Search Path modified

	The order in which emacs looks for all .rc (startup) and
.cmd (command macros) is as follows:

	$HOME			(the HOME environment variable if it exists)
	the current directory
	$PATH			(executable PATH)
	default list contained in epath.h

***	Line length limits removed

	Lines of arbitrary length may be read, edited, and written.

***	Out of memory handling improved

	EMACS will announce "OUT OF MEMORY" when it runs out of dynamic
memory while reading files or inserting new text.  It should then be
safe to save buffers out IF THE CONTENTS OF THE BUFFER ARE COMPLETE at
that time.  When a buffer has been truncated while reading, a pound sign
"#" will appear in the first position of the mode line.  Also a # will
appear in a buffer listing.  If you attempt to save a truncated buffer,
EMACS will ask if you are certain before allowing the truncated file to
be written.  As before, still beware of killing blocks of text after you
have run out of memory. 

***	DENSE mode on the Atari ST

	On an Atari ST monochrome monitor, setting $sres to "DENSE" will
result in a 50 line display.

***	Execute command

	Execute-program (^X-$) will execute an external program without
calling up an intervening shell is possible.

***	Better close braces in CMODE

	The name says it all, try it.
