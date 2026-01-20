/*	PKLOCK.C
 *
 *	locking routines as modified by Petri Kutvonen
 */

#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

#define MAXLOCK 512
#define MAXNAME 128

/**********************
 *
 * if successful, returns NULL  
 * if file locked, returns username of person locking the file
 * if other error, returns "LOCK ERROR: explanation"
 *
 *********************/
char *dolock(char *fname)
{
	int fd, n;
	static char lname[MAXLOCK], locker[MAXNAME + 1];
	int mask;
	struct stat sbuf;

	strcat(strcpy(lname, fname), ".lock~");

	/* check that we are not being cheated, qname must point to     */
	/* a regular file - even this code leaves a small window of     */
	/* vulnerability but it is rather hard to exploit it            */

	if (lstat(lname, &sbuf) == 0)
		if (!S_ISREG(sbuf.st_mode))
			return "LOCK ERROR: not a regular file";

	mask = umask(0);
	fd = open(lname, O_RDWR | O_CREAT, 0666);
	umask(mask);
	if (fd < 0) {
		if (errno == EACCES)
			return NULL;
		if (errno == EROFS)
			return NULL;
		return "LOCK ERROR: cannot access lock file";
	}
	if ((n = read(fd, locker, MAXNAME)) < 1) {
		struct passwd *pw;
		lseek(fd, 0, SEEK_SET);
		pw = getpwuid(getuid());
		if (pw && pw->pw_name)
			strncpy(locker, pw->pw_name, MAXNAME - 1);
		else
			strcpy(locker, "unknown");
		strcat(locker + strlen(locker), "@");
		gethostname(locker + strlen(locker), 64);
		write(fd, locker, strlen(locker));
		close(fd);
		return NULL;
	}
	locker[n > MAXNAME ? MAXNAME : n] = 0;
	return locker;
}

/*********************
 *
 * undolock -- unlock the file fname
 *
 * if successful, returns NULL
 * if other error, returns "LOCK ERROR: explanation"
 *
 *********************/

char *undolock(char *fname)
{
	static char lname[MAXLOCK];

	strcat(strcpy(lname, fname), ".lock~");
	if (unlink(lname) != 0) {
		if (errno == EACCES || errno == ENOENT)
			return NULL;
		if (errno == EROFS)
			return NULL;
		return "LOCK ERROR: cannot remove lock file";
	}
	return NULL;
}
