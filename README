+---------------+
| uEmacs/PK 4.0 |
+---------------+

        Full screen editor based on MicroEMACS 3.9e

        MicroEMACS was written by Dave G. Conroy and
        greatly modified by Daniel M. Lawrence

        Copyright Notices:

        MicroEMACS 3.9 (c) Copyright 1987 Daniel M. Lawrence.
        Reference Manual Copyright 1987 by Brian Straight and
        Daniel M. Lawrence. No copyright claimed for modifications
        made by Petri H. Kutvonen.

        Original statement of copying policy:

        MicroEMACS 3.9 can be copied and distributed freely for any
        non-commercial purposes. MicroEMACS 3.9 can only be incorporated
        into commercial software with the permission of the current author
        [Daniel M. Lawrence].


WHAT IS uEmacs/PK?

uEmacs/PK 4.0 is an enhanced version of MicroEMACS 3.9e. Enhancements
have been incorporated by Petri H. Kutvonen, University of Helsinki,
Finland <kutvonen@cs.Helsinki.FI>.


WHY IS IT BASED ON AN OLD VERSION OF MicroEMACS?

In my opinion 3.9e was the best of all MicroEMACSes. Creeping
featurism, growing size, and reduced portability made versions 3.10 and
3.11 less attractive. MicroEMACS 3.9e was one of the few editors that
were truly portable between different flavours of UNIX, PC/MS-DOS, and
VAX/VMS. It was pretty robust - although not flawless. uEmacs/PK 4.0
includes numerous bug fixes, adds some new functionality and comfort but
does not sacrifice the best things (small size and portability).


WHAT IS NEW - COMPARED TO MicroEMACS 3.9e?

Enhachements:

o  advisory file locking on BSD-derived systems
o  new screen update algorithm, borrowed largely form "vile" by
   Paul G. Fox <pgf@cayman.com>, uEmacs can now be used on slow (1200 bps)
   connections because it uses terminal scrolling capabilities
o  new variables $scroll, $jump, and $overlap to control scrolling
o  uEmacs reacts to windows size change signals (UNIX), also from "vile"
o  automatic file name completion, works under UNIX and PC/MS-DOS
o  functions keys on VT200 style keyboards can be used
o  new command: justify-paragraph (Meta J)
o  something important for us Europeans: allow the use of 8 bit ISO Latin 1
   alphabet (UNIX and VMS), on an IBM-PC the accented characters are
   interpreted as letters too
o  the characters {|}[\] can be interpreted as letters, these are
   frequently used as "national replacement characters" especially in
   the Nordic countries
o  allow use of XON/XOFF flow control: alternative key sequences for
   ^S and ^Q, don't disable flow control
o  speed up reading of files (under VMS especially)
o  new variable $tab, hardware tab stop (4 or 8)
o  automatic configuration on some common systems
o  new style mode line, includes percentage indicator
o  new help file

Bug fixes - not very interesting:

o  use TI and TE termcap strings, uEmacs works now correctly under
   Sunview and OpenWindows
o  use old protection mask on existing files (VMS)
o  catch data overrun errors (VMS)
o  allow VMS file names with characters < and >, replacements for [ and ]
o  allow ANSI screens larger than 24 lines
o  fix add/delete(-global)-mode
o  display EOF as char = 0x0, not as garbage
o  allow upper case letters in answers
o  fix command interpreter spawning
o  don't use reverse video on some (TVI925 style) terminals
o  fix message line writing
o  fix replace/undo
o  fix &left and &mid functions
o  fix documentation
o  smaller bug fixes are too numerous to mention

Something is gone:

o  removed (obsolete and untested) support for Amiga, Atari, and Mac


WHERE HAS IT BEEN TESTED?

uEmacs/PK 4.0 has been tested by myself on the following platforms:

        IBM PC and compatibles, PC/MS-DOS 3.2 and up
        Sun 3, SunOS 4.1.1
        SPARC, SunOS 4.1.X and 5.{2|3|4|5} (Solaris 2)
        VAX 8800 and 6000-420, VMS 5.4
        DECstation 3100, Ultrix V4.1
        IBM RS/6000, AIX 3.1.5
        IBM PS/2, UNIX System V/386 Release 3.2
        uVAX II, Ultrix V2.0
        AT&T 3B2/400, UNIX System V Release 2.0
        Various Toshiba i486 laptops, Linux 0.99pl13 thru 2.0.21

I have no reason to believe that there would be any problems to port
uEmacs/PK 4.0 to any reasonable BSD-, OSF/1-, or SVR4-based UNIX system.


HOW CAN I GET IT?

uEmacs/PK is available by anonymous FTP from ftp.cs.Helsinki.FI (IP
address can change) in the directory pub/Software/Local/uEmacs-PK. You
cannot get it by email of uucp. Hopefully it will bee soon available
from other file servers too.


WHAT IS IN THE PACKAGE

o  em-4.0.<x>.tar.gz:   full source, make and command files to build the
                        editor, reference manual as a MS-Write document,
                        tarred and gzipped, for patchlevel <x>

o  there used to be a packages with binaries for PC/MS-DOS,
   SPARC/SunOS4, Sun 3, MIPS/Ultrix, 386/ix, IBM RS/6000, VAX/VMS,
   if you are really desperate you can try contacting me for an old
   version of these


HOW TO INSTALL uEmacs/PK?

o  PC/MS-DOS: Compile the package with using Turbo C 2.0 or MSC 6.0.
         (Pretty obsolete both of these.) There are some support files
         you might find useful in the full distribution.

o  UNIX: Copy makefile.unx to makefile, edit to taste, look at estruct.h,
         do a 'make', test the editor, 'make install'.

o  VMS:  To compile use '@VMSMAKE', install manually, uEmacs/PK uses a
         logical name EMACS_DIR to locate its initialization files.


CONTACT INFORMATION

There will probably not be many new versions of uEmacs/PK, maybe just
some bug fixes. I have no intention to develope the code any further.
However, if you have some comments or good suggestions, you may find
the email address below useful.

Petri H. Kutvonen
Department of Computer Science
P.O.Box 26 (Teollisuuskatu 23)
FIN-00014 UNIVERSTITY OF HELSINKI
Finland

email: kutvonen@cs.Helsinki.FI
fax:   +358 9 70844441


ACKNOWLEDGEMENTS AND STATUS

I would like to thank Linus Torvalds and Jyrki Havia for their work on
some bugs. uEmacs/PK 4.0.10 included fixes for a number of bugs and it
was assumed to be the final release for the UNIX platform. However,
there has been a couple of maintenance releases, so the final version is
4.0.13. On other platforms there has been no new releases since 4.0.3 in
1991.

April 23, 1995

And yet another release (thanks Linus)! This is most definitely the
really last (not latest) version, 4.0.14. Hmm ... 14 looks nicer than 13.

May 2, 1996

Still one more release - or actually a small patch - which closes a
potential security hole. Now we are at 4.0.15. This IS the FINAL release!

September 25, 1996
