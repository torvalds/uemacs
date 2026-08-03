/*	FILEIO.C
 *
 * The routines in this file read and write ASCII files from the disk. All of
 * the knowledge about files are here.
 *
 *	modified by Petri Kutvonen
 */

#include        <stdio.h>
#include	"estruct.h"
#include        "globals.h"
#include	"efunc.h"

static FILE *ffp;				/* File pointer, all functions. */
static int eofflag;				/* end-of-file flag */

/*
 * Open a file for reading.
 */
int file_open_read(char *fn)
{
	if ((ffp = fopen(fn, "r")) == NULL)
		return FIOFNF;
	eofflag = FALSE;
	return FIOSUC;
}

/*
 * Open a file for writing. Return TRUE if all is well, and FALSE on error
 * (cannot create).
 */
int file_open_write(char *fn)
{
	if ((ffp = fopen(fn, "w")) == NULL) {
		msg_printf("Cannot open file for writing");
		return FIOERR;
	}
	return FIOSUC;
}

/*
 * Close a file. Should look at the status in all systems.
 */
int file_close(void)
{
	/* free this since we do not need it anymore */
	if (file_line) {
		free(file_line);
		file_line = NULL;
	}
	eofflag = FALSE;

	if (fclose(ffp) != FALSE) {
		msg_printf("Error closing file");
		return FIOERR;
	}
	return FIOSUC;
}

/*
 * Write a line to the already opened file. The "buf" points to the buffer,
 * and the "nbuf" is its length, less the free newline. Return the status.
 * Check only at the newline.
 */
int file_put_line(char *buf, int nbuf)
{
	int i;

	for (i = 0; i < nbuf; ++i)
		fputc(buf[i] & 0xFF, ffp);

	fputc('\n', ffp);

	if (ferror(ffp)) {
		msg_printf("Write I/O error");
		return FIOERR;
	}

	return FIOSUC;
}

/*
 * Read a line from a file, and store the bytes in the supplied buffer. The
 * "nbuf" is the length of the buffer. Complain about long lines and lines
 * at the end of the file that don't have a newline present. Check for I/O
 * errors too. Return status.
 */
int file_get_line(void)
{
	int c;					/* current character read */
	int i;					/* current index into fline */
	char *tmpline;				/* temp storage for expanding line */

	/* if we are at the end...return it */
	if (eofflag)
		return FIOEOF;

	/* dump fline if it ended up too big */
	if (file_line_size > NSTRING) {
		free(file_line);
		file_line = NULL;
	}

	/* if we don't have an fline, allocate one */
	if (file_line == NULL)
		if ((file_line = malloc(file_line_size = NSTRING)) == NULL)
			return FIOMEM;

	/* read the line in */
	if (!nullflag) {
		if (fgets(file_line, NSTRING, ffp) == (char *)NULL) {	/* EOF ? */
			i = 0;
			c = EOF;
		} else {
			i = strlen(file_line);
			c = 0;
			if (i > 0) {
				c = file_line[i - 1];
				i--;
			}
		}
	} else {
		i = 0;
		c = fgetc(ffp);
	}
	while (c != EOF && c != '\n') {
		if (c) {
			file_line[i++] = c;
			/* if it's longer, get more room */
			if (i >= file_line_size) {
				if ((tmpline = malloc(file_line_size + NSTRING)) == NULL)
					return FIOMEM;
				strncpy(tmpline, file_line, file_line_size);
				file_line_size += NSTRING;
				free(file_line);
				file_line = tmpline;
			}
		}
		c = fgetc(ffp);
	}

	/* test for any errors that may have occured */
	if (c == EOF) {
		if (ferror(ffp)) {
			msg_printf("File read error");
			return FIOERR;
		}

		if (i != 0)
			eofflag = TRUE;
		else
			return FIOEOF;
	}

	/* terminate and decrypt the string */
	file_line[i] = 0;
	return FIOSUC;
}

/*
 * does <fname> exist on disk?
 *
 * char *fname;		file to check for existance
 */
int file_exists(char *fname)
{
	FILE *fp;

	/* try to open the file for reading */
	fp = fopen(fname, "r");

	/* if it fails, just return false! */
	if (fp == NULL)
		return FALSE;

	/* otherwise, close it and report true */
	fclose(fp);
	return TRUE;
}
