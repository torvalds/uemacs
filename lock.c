/*	LOCK.C
 *
 *	File locking command routines
 *
 *	written by Daniel Lawrence
 */

#include <stdio.h>
#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#if	BSD | SVR4
#include <sys/errno.h>

static char *lname[NLOCKS];		/* names of all locked files */
static int numlocks;			/* # of current locks active */

/*
 * lockchk:
 *	check a file for locking and add it to the list
 *
 * char *fname;			file to check for a lock
 */
int lockchk(char *fname)
{
	int i;		/* loop indexes */
	int status;	/* return status */

	/* check to see if that file is already locked here */
	if (numlocks > 0)
		for (i = 0; i < numlocks; ++i)
			if (strcmp(fname, lname[i]) == 0)
				return TRUE;

	/* if we have a full locking table, bitch and leave */
	if (numlocks == NLOCKS) {
		mlwrite("LOCK ERROR: Lock table full");
		return ABORT;
	}

	/* next, try to lock it */
	status = lock(fname);
	if (status == ABORT)	/* file is locked, no override */
		return ABORT;
	if (status == FALSE)	/* locked, overriden, dont add to table */
		return TRUE;

	/* we have now locked it, add it to our table */
	lname[++numlocks - 1] = (char *) malloc(strlen(fname) + 1);
	if (lname[numlocks - 1] == NULL) {	/* malloc failure */
		undolock(fname);	/* free the lock */
		mlwrite("Cannot lock, out of memory");
		--numlocks;
		return ABORT;
	}

	/* everthing is cool, add it to the table */
	strcpy(lname[numlocks - 1], fname);
	return TRUE;
}

/*
 * lockrel:
 *	release all the file locks so others may edit
 */
int lockrel(void)
{
	int i;		/* loop index */
	int status;	/* status of locks */
	int s;		/* status of one unlock */

	status = TRUE;
	if (numlocks > 0)
		for (i = 0; i < numlocks; ++i) {
			if ((s = unlock(lname[i])) != TRUE)
				status = s;
			free(lname[i]);
		}
	numlocks = 0;
	return status;
}

/*
 * lock:
 *	Check and lock a file from access by others
 *	returns	TRUE = files was not locked and now is
 *		FALSE = file was locked and overridden
 *		ABORT = file was locked, abort command
 *
 * char *fname;		file name to lock
 */
int lock(char *fname)
{
	char *locker;	/* lock error message */
	int status;	/* return status */
	char msg[NSTRING];	/* message string */

	/* attempt to lock the file */
	locker = dolock(fname);
	if (locker == NULL)	/* we win */
		return TRUE;

	/* file failed...abort */
	if (strncmp(locker, "LOCK", 4) == 0) {
		lckerror(locker);
		return ABORT;
	}

	/* someone else has it....override? */
	strcpy(msg, "File in use by ");
	strcat(msg, locker);
	strcat(msg, ", override?");
	status = mlyesno(msg);	/* ask them */
	if (status == TRUE)
		return FALSE;
	else
		return ABORT;
}

/*
 * unlock:
 *	Unlock a file
 *	this only warns the user if it fails
 *
 * char *fname;		file to unlock
 */
int unlock(char *fname)
{
	char *locker;	/* undolock return string */

	/* unclock and return */
	locker = undolock(fname);
	if (locker == NULL)
		return TRUE;

	/* report the error and come back */
	lckerror(locker);
	return FALSE;
}

/*
 * report a lock error
 *
 * char *errstr;	lock error string to print out
 */
void lckerror(char *errstr)
{
	char obuf[NSTRING];	/* output buffer for error message */

	strcpy(obuf, errstr);
	strcat(obuf, " - ");
	strcat(obuf, strerror(errno));
	mlwrite(obuf);
}
#endif
