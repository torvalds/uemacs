/*	VMSVT.C
 *
 *	Advanced VMS terminal driver
 *
 *	Knows about any terminal defined in SMGTERMS.TXT and TERMTABLE.TXT
 *	located in SYS$SYSTEM.
 *
 *	Author:  Curtis Smith
 *	modified by Petri Kutvonen
 */

#include	<stdio.h>	/* Standard I/O package         */
#include	"estruct.h"	/* Emacs' structures            */
#include	"edef.h"	/* Emacs' definitions           */

#if	VMSVT

#include	 <descrip.h>	/* Descriptor definitions       */

/*  These would normally come from iodef.h and ttdef.h  */
#define IO$_SENSEMODE	0x27	/* Sense mode of terminal       */
#define TT$_UNKNOWN	0x00	/* Unknown terminal             */
#define TT$_VT100	96

/** Forward references **/
int vmsopen(), ttclose(), vmskopen(), vmskclose(), ttgetc(), ttputc();
int ttflush(), vmsmove(), vmseeol(), vmseeop(), vmsbeep(), vmsrev();
int vmscres();
extern int eolexist, revexist;
extern char sres[];

#if COLOR
int vmsfcol(), vmsbcol();
#endif

/** SMG stuff **/
static char *begin_reverse, *end_reverse, *erase_to_end_line;
static char *erase_whole_display;
static int termtype;

#define SMG$K_BEGIN_REVERSE		0x1bf
#define SMG$K_END_REVERSE		0x1d6
#define SMG$K_SET_CURSOR_ABS		0x23a
#define SMG$K_ERASE_WHOLE_DISPLAY	0x1da
#define SMG$K_ERASE_TO_END_LINE		0x1d9

#if SCROLLCODE

#define SMG$K_SCROLL_FORWARD    561	/* from sys$library:smgtrmptr.h */
#define SMG$K_SCROLL_REVERSE	562
#define SMG$K_SET_SCROLL_REGION	572

static char *scroll_forward, *scroll_reverse;

#endif

/* Dispatch table. All hard fields just point into the terminal I/O code. */
struct terminal term = {
#if	PKCODE
	MAXROW,
#else
	24 - 1,			/* Max number of rows allowable */
#endif
				/* Filled in */ -1,
				/* Current number of rows used  */
	MAXCOL,			/* Max number of columns        */
				/* Filled in */ 0,
				/* Current number of columns    */
	64,			/* Min margin for extended lines */
	8,			/* Size of scroll region        */
	100,			/* # times thru update to pause */
	vmsopen,		/* Open terminal at the start   */
	ttclose,		/* Close terminal at end        */
	vmskopen,		/* Open keyboard                */
	vmskclose,		/* Close keyboard               */
	ttgetc,			/* Get character from keyboard  */
	ttputc,			/* Put character to display     */
	ttflush,		/* Flush output buffers         */
	vmsmove,		/* Move cursor, origin 0        */
	vmseeol,		/* Erase to end of line         */
	vmseeop,		/* Erase to end of page         */
	vmsbeep,		/* Beep                         */
	vmsrev,			/* Set reverse video state      */
	vmscres			/* Change screen resolution     */
#if	COLOR
	    , vmsfcol,		/* Set forground color          */
	vmsbcol			/* Set background color         */
#endif
#if	SCROLLCODE
	    , NULL
#endif
};

/***
 *  ttputs  -  Send a string to ttputc
 *
 *  Nothing returned
 ***/
ttputs(string)
char *string;			/* String to write              */
{
	if (string)
		while (*string != '\0')
			ttputc(*string++);
}


/***
 *  vmsmove  -  Move the cursor (0 origin)
 *
 *  Nothing returned
 ***/
vmsmove(row, col)
int row;			/* Row position                 */
int col;			/* Column position              */
{
	char buffer[32];
	int ret_length;
	static int request_code = SMG$K_SET_CURSOR_ABS;
	static int max_buffer_length = sizeof(buffer);
	static int arg_list[3] = { 2 };
	char *cp;

	int i;

	/* Set the arguments into the arg_list array
	 * SMG assumes the row/column positions are 1 based (boo!)
	 */
	arg_list[1] = row + 1;
	arg_list[2] = col + 1;

	if ((smg$get_term_data(	/* Get terminal data            */
				      &termtype,	/* Terminal table address       */
				      &request_code,	/* Request code                 */
				      &max_buffer_length,	/* Maximum buffer length        */
				      &ret_length,	/* Return length                */
				      buffer,	/* Capability data buffer       */
				      arg_list)

	     /* Argument list array              */
	     /* We'll know soon enough if this doesn't work         */
	     &1) == 0) {
		ttputs("OOPS");
		return;
	}

	/* Send out resulting sequence                          */
	i = ret_length;
	cp = buffer;
	while (i-- > 0)
		ttputc(*cp++);
}

#if SCROLLCODE

vmsscroll_reg(from, to, howmany)
{
	int i;
	if (to == from)
		return;
	if (to < from) {
		vmsscrollregion(to, from + howmany - 1);
		vmsmove(from + howmany - 1, 0);
		for (i = from - to; i > 0; i--)
			ttputs(scroll_forward);
	} else {		/* from < to */
		vmsscrollregion(from, to + howmany - 1);
		vmsmove(from, 0);
		for (i = to - from; i > 0; i--)
			ttputs(scroll_reverse);
	}
	vmsscrollregion(-1, -1);
}

vmsscrollregion(top, bot)
int top;			/* Top position                 */
int bot;			/* Bottom position              */
{
	char buffer[32];
	int ret_length;
	static int request_code = SMG$K_SET_SCROLL_REGION;
	static int max_buffer_length = sizeof(buffer);
	static int arg_list[3] = { 2 };
	char *cp;

	int i;

	/* Set the arguments into the arg_list array
	 * SMG assumes the row/column positions are 1 based (boo!)
	 */
	arg_list[1] = top + 1;
	arg_list[2] = bot + 1;

	if ((smg$get_term_data(	/* Get terminal data            */
				      &termtype,	/* Terminal table address       */
				      &request_code,	/* Request code                 */
				      &max_buffer_length,	/* Maximum buffer length        */
				      &ret_length,	/* Return length                */
				      buffer,	/* Capability data buffer       */
				      arg_list)

	     /* Argument list array              */
	     /* We'll know soon enough if this doesn't work         */
	     &1) == 0) {
		ttputs("OOPS");
		return;
	}

	ttputc(0);
	/* Send out resulting sequence                          */
	i = ret_length;
	cp = buffer;
	while (i-- > 0)
		ttputc(*cp++);
}
#endif

/***
 *  vmsrev  -  Set the reverse video status
 *
 *  Nothing returned
 ***/
vmsrev(status)
int status;			/* TRUE if setting reverse      */
{
	if (status)
		ttputs(begin_reverse);
	else
		ttputs(end_reverse);
}

/***
 *  vmscres  -  Change screen resolution (which it doesn't)
 *
 *  Nothing returned
 ***/
vmscres()
{
	/* But it could.  For vt100/vt200s, one could switch from
	   80 and 132 columns modes */
}


#if	COLOR
/***
 *  vmsfcol  -  Set the forground color (not implimented)
 *
 *  Nothing returned
 ***/
vmsfcol()
{
}

/***
 *  vmsbcol  -  Set the background color (not implimented)
 *
 *  Nothing returned
 ***/
vmsbcol()
{
}
#endif

/***
 *  vmseeol  -  Erase to end of line
 *
 *  Nothing returned
 ***/
vmseeol()
{
	ttputs(erase_to_end_line);
}


/***
 *  vmseeop  -  Erase to end of page (clear screen)
 *
 *  Nothing returned
 ***/
vmseeop()
{
	ttputs(erase_whole_display);
}


/***
 *  vmsbeep  -  Ring the bell
 *
 *  Nothing returned
 ***/
vmsbeep()
{
	ttputc('\007');
}


/***
 *  vmsgetstr  -  Get an SMG string capability by name
 *
 *  Returns:	Escape sequence
 *		NULL	No escape sequence available
 ***/
char *vmsgetstr(request_code)
int request_code;		/* Request code                 */
{
	char *result;
	static char seq_storage[1024];
	static char *buffer = seq_storage;
	static int arg_list[2] = { 1, 1 };
	int max_buffer_length, ret_length;

	/*  Precompute buffer length */

	max_buffer_length = (seq_storage + sizeof(seq_storage)) - buffer;

	/* Get terminal commands sequence from master table */

	if ((smg$get_term_data(	/* Get terminal data            */
				      &termtype,	/* Terminal table address       */
				      &request_code,	/* Request code                 */
				      &max_buffer_length,	/* Maximum buffer length     */
				      &ret_length,	/* Return length                */
				      buffer,	/* Capability data buffer       */
				      arg_list)


	     /* Argument list array              */
	     /* If this doesn't work, try again with no arguments */
	     &1) == 0 && (smg$get_term_data(	/* Get terminal data            */
						   &termtype,	/* Terminal table address       */
						   &request_code,	/* Request code                 */
						   &max_buffer_length,	/* Maximum buffer length     */
						   &ret_length,	/* Return length                */
						   buffer)


			  /* Capability data buffer   */
			  /* Return NULL pointer if capability is not available */
			  &1) == 0)
		return NULL;

	/* Check for empty result */
	if (ret_length == 0)
		return NULL;

	/* Save current position so we can return it to caller */

	result = buffer;

	/* NIL terminate the sequence for return */

	buffer[ret_length] = 0;

	/* Advance buffer */

	buffer += ret_length + 1;

	/* Return capability to user */
	return result;
}


/** I/O information block definitions **/
struct iosb {			/* I/O status block                     */
	short i_cond;		/* Condition value                      */
	short i_xfer;		/* Transfer count                       */
	long i_info;		/* Device information                   */
};
struct termchar {		/* Terminal characteristics             */
	char t_class;		/* Terminal class                       */
	char t_type;		/* Terminal type                        */
	short t_width;		/* Terminal width in characters         */
	long t_mandl;		/* Terminal's mode and length           */
	long t_extend;		/* Extended terminal characteristics    */
};
static struct termchar tc;	/* Terminal characteristics             */

/***
 *  vmsgtty - Get terminal type from system control block
 *
 *  Nothing returned
 ***/
vmsgtty()
{
	short fd;
	int status;
	struct iosb iostatus;
	$DESCRIPTOR(devnam, "SYS$INPUT");

	/* Assign input to a channel */
	status = sys$assign(&devnam, &fd, 0, 0);
	if ((status & 1) == 0)
		exit(status);

	/* Get terminal characteristics */
	status = sys$qiow(	/* Queue and wait               */
				 0,	/* Wait on event flag zero      */
				 fd,	/* Channel to input terminal    */
				 IO$_SENSEMODE,	/* Get current characteristic   */
				 &iostatus,	/* Status after operation       */
				 0, 0,	/* No AST service               */
				 &tc,	/* Terminal characteristics buf */
				 sizeof(tc),	/* Size of the buffer           */
				 0, 0, 0, 0);	/* P3-P6 unused                 */

	/* De-assign the input device */
	if ((sys$dassgn(fd) & 1) == 0)
		exit(status);

	/* Jump out if bad status */
	if ((status & 1) == 0)
		exit(status);
	if ((iostatus.i_cond & 1) == 0)
		exit(iostatus.i_cond);
}


/***
 *  vmsopen  -  Get terminal type and open terminal
 *
 *  Nothing returned
 ***/
vmsopen()
{
	/* Get terminal type */
	vmsgtty();
	if (tc.t_type == TT$_UNKNOWN) {
		printf("Terminal type is unknown!\n");
		printf
		    ("Try set your terminal type with SET TERMINAL/INQUIRE\n");
		printf("Or get help on SET TERMINAL/DEVICE_TYPE\n");
		exit(3);
	}

	/* Access the system terminal definition table for the          */
	/* information of the terminal type returned by IO$_SENSEMODE   */
	if ((smg$init_term_table_by_type(&tc.t_type, &termtype) & 1) == 0)
		return -1;

	/* Set sizes */
	term.t_nrow = ((unsigned int) tc.t_mandl >> 24) - 1;
	term.t_ncol = tc.t_width;

	/* Get some capabilities */
	begin_reverse = vmsgetstr(SMG$K_BEGIN_REVERSE);
	end_reverse = vmsgetstr(SMG$K_END_REVERSE);
	revexist = begin_reverse != NULL && end_reverse != NULL;
	erase_to_end_line = vmsgetstr(SMG$K_ERASE_TO_END_LINE);
	eolexist = erase_to_end_line != NULL;
	erase_whole_display = vmsgetstr(SMG$K_ERASE_WHOLE_DISPLAY);

#if SCROLLCODE
	scroll_forward = vmsgetstr(SMG$K_SCROLL_FORWARD);
	scroll_reverse = vmsgetstr(SMG$K_SCROLL_REVERSE);
	if (tc.t_type < TT$_VT100 || scroll_reverse == NULL ||
	    scroll_forward == NULL)
		term.t_scroll = NULL;
	else
		term.t_scroll = vmsscroll_reg;
#endif

	/* Set resolution */
	strcpy(sres, "NORMAL");

	/* Open terminal I/O drivers */
	ttopen();
}


/***
 *  vmskopen  -  Open keyboard (not used)
 *
 *  Nothing returned
 ***/
vmskopen()
{
}


/***
 *  vmskclose  -  Close keyboard (not used)
 *
 *  Nothing returned
 ***/
vmskclose()
{
}

#endif
