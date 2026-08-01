/*	LOCK.C
 *
 *	File locking command routines
 *
 *	written by Daniel Lawrence
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"

/*
 * A lock is an exclusive flock() on an open file descriptor, so the
 * descriptor has to stay open for as long as we hold the file.  Nothing
 * ever reads 'fd' again - it is here so that the descriptor is not lost,
 * and it is closed by exit() along with everything else.
 *
 * The table is keyed on device and inode rather than on the file name,
 * because the same file reached by two different names is the same file.
 * With name keying flock() would refuse our own second descriptor and
 * leave us reporting that somebody else had it, which is a silly way to
 * be told you can spell.
 */
static struct filelock {
	dev_t dev;
	ino_t ino;
	int fd;
} locks[NLOCKS];
static int numlocks;				/* # of current locks active */

/*
 * lockchk:
 *	check a file for locking and add it to the list
 *
 *	returns	TRUE = we hold it now, or nobody meaningfully does
 *		ABORT = somebody else holds it, don't read the file
 *
 * char *fname;			file to check for a lock
 */
int lockchk(char *fname)
{
	struct stat st;
	int fd, i;

	/*
	 * Nothing to lock: a new file, or one we have no business
	 * reading.  Either way readin() is about to report it properly,
	 * so don't report it first and worse.
	 *
	 * O_CLOEXEC matters here - spawn.c forks shells, and a
	 * descriptor inherited by one of them would hold the lock open
	 * long after we exited.
	 */
	fd = open(fname, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return TRUE;

	if (fstat(fd, &st) < 0) {
		close(fd);
		return TRUE;
	}

	/* already ours? */
	for (i = 0; i < numlocks; i++) {
		if (locks[i].dev == st.st_dev && locks[i].ino == st.st_ino) {
			close(fd);
			return TRUE;
		}
	}

	/* if we have a full locking table, bitch and leave */
	if (numlocks == NLOCKS) {
		mlwrite("LOCK ERROR: Lock table full");
		close(fd);
		return ABORT;
	}

	if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
		close(fd);

		/*
		 * Only EWOULDBLOCK means somebody else has it.  Any
		 * other error means this filesystem does not do locking,
		 * and refusing to edit over that would be worse than not
		 * locking at all.
		 */
		if (errno != EWOULDBLOCK)
			return TRUE;

		/* someone else has it....override? */
		if (mlyesno("File in use, override") == TRUE)
			return TRUE;
		return ABORT;
	}

	locks[numlocks].dev = st.st_dev;
	locks[numlocks].ino = st.st_ino;
	locks[numlocks].fd = fd;
	numlocks++;
	return TRUE;
}
