/*	PKLOCK.C
 *
 *	locking routines as modified by Petri Kutvonen
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

#include "estruct.h"
#include "edef.h"
#include "efunc.h"

#define MAXLOCK 512
#define MAXNAME 128

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static const char LOCK_ERR_PATH_TOO_LONG[] =
	"LOCK ERROR: lock file path too long";
static const char LOCK_ERR_NOT_REGULAR[] =
	"LOCK ERROR: not a regular file";
static const char LOCK_ERR_CANNOT_ACCESS[] =
	"LOCK ERROR: cannot access lock file";
static const char LOCK_ERR_CANNOT_WRITE[] =
	"LOCK ERROR: cannot write lock file";
static const char LOCK_ERR_CANNOT_CLOSE[] =
	"LOCK ERROR: cannot close lock file";
static const char LOCK_ERR_MALFORMED[] =
	"LOCK ERROR: malformed lock file";
static const char LOCK_ERR_CANNOT_REMOVE[] =
	"LOCK ERROR: cannot remove lock file";

static int lock_err_equal(const char *msg, const char *err)
{
	return msg == err || (msg != NULL && strcmp(msg, err) == 0);
}

static int open_existing_lock(const char *lname, struct stat *sbuf)
{
	int fd;
	int saved_errno;

#if O_NOFOLLOW == 0
	struct stat lbuf;

	if (lstat(lname, &lbuf) != 0)
		return -1;
	if (!S_ISREG(lbuf.st_mode)) {
		errno = ELOOP;
		return -1;
	}
#endif

	fd = open(lname, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
	if (fd < 0)
		return -1;

	if (fstat(fd, sbuf) != 0) {
		saved_errno = errno;
		close(fd);
		errno = saved_errno;
		return -1;
	}
	if (!S_ISREG(sbuf->st_mode)) {
		close(fd);
		errno = ELOOP;
		return -1;
	}

#if O_NOFOLLOW == 0
	if (lstat(lname, &lbuf) != 0) {
		saved_errno = errno;
		close(fd);
		errno = saved_errno;
		return -1;
	}
	if (lbuf.st_dev != sbuf->st_dev || lbuf.st_ino != sbuf->st_ino) {
		close(fd);
		errno = ENOENT;
		return -1;
	}
#endif

	return fd;
}

int is_lock_error(const char *msg)
{
	return lock_err_equal(msg, LOCK_ERR_PATH_TOO_LONG)
	    || lock_err_equal(msg, LOCK_ERR_NOT_REGULAR)
	    || lock_err_equal(msg, LOCK_ERR_CANNOT_ACCESS)
	    || lock_err_equal(msg, LOCK_ERR_CANNOT_WRITE)
	    || lock_err_equal(msg, LOCK_ERR_CANNOT_CLOSE)
	    || lock_err_equal(msg, LOCK_ERR_MALFORMED)
	    || lock_err_equal(msg, LOCK_ERR_CANNOT_REMOVE);
}

/**********************
 *
 * if successful, returns NULL
 * if file locked, returns username of person locking the file
 * if other error, returns "LOCK ERROR: explanation"
 *
 *********************/
const char *dolock(char *fname)
{
	int fd;
	static char lname[MAXLOCK], locker[MAXNAME + 1];
	int mask;
	struct stat sbuf;
	ssize_t nread;
	ssize_t nwritten;
	size_t owner_len;

	if (snprintf(lname, sizeof(lname), "%s.lock~", fname)
	    >= (int)sizeof(lname)) {
		errno = ENAMETOOLONG;
		return LOCK_ERR_PATH_TOO_LONG;
	}

retry_create:
	mask = umask(0);
	fd = open(lname,
		  O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW,
		  0666);
	umask(mask);

	/* We created the lock file atomically and now own the lock. */
	if (fd >= 0) {
		if (fstat(fd, &sbuf) != 0) {
			close(fd);
			unlink(lname);
			return LOCK_ERR_CANNOT_ACCESS;
		}
		if (!S_ISREG(sbuf.st_mode)) {
			close(fd);
			unlink(lname);
			errno = ELOOP;
			return LOCK_ERR_NOT_REGULAR;
		}

		/* Generate the owner tag (user@host) for the lock file */
		const char *user = getlogin();
		if (!user) {
			/* No controlling terminal; try the passwd entry */
			struct passwd *pw = getpwuid(geteuid());
			if (pw)
				user = pw->pw_name;
		}

		if (!user) {
			/* Still no username; fall back to numeric UID */
			snprintf(locker, sizeof(locker), "uid%d", (int)geteuid());
		} else {
			snprintf(locker, sizeof(locker), "%s", user);
		}
		{
			size_t used = strlen(locker);

			/* Append host safely without overflowing locker. */
			if (used < sizeof(locker) - 1) {
				locker[used++] = '@';
				if (gethostname(locker + used,
						sizeof(locker) - used) != 0)
					locker[used] = '\0';
				locker[sizeof(locker) - 1] = '\0';
			}
		}

		/* Write the owner tag to the lock file */
		owner_len = strlen(locker);
		if (lseek(fd, 0, SEEK_SET) < 0) {
			close(fd);
			unlink(lname);
			return LOCK_ERR_CANNOT_WRITE;
		}
		nwritten = write(fd, locker, owner_len);
		if (nwritten < 0 || (size_t)nwritten != owner_len) {
			close(fd);
			unlink(lname);
			return LOCK_ERR_CANNOT_WRITE;
		}
		if (close(fd) != 0) {
			unlink(lname);
			return LOCK_ERR_CANNOT_CLOSE;
		}
		return NULL;
	}

	if (errno == EACCES || errno == EROFS)
		return NULL;

	/* A lock file exists: read and report its owner. */
	if (errno == EEXIST) {
		fd = open_existing_lock(lname, &sbuf);
		if (fd < 0) {
			if (errno == ENOENT)
				goto retry_create;
			if (errno == EACCES || errno == EROFS)
				return NULL;
			if (errno == ELOOP)
				return LOCK_ERR_NOT_REGULAR;
			return LOCK_ERR_CANNOT_ACCESS;
		}

		nread = read(fd, locker, MAXNAME);
		if (close(fd) != 0 && nread >= 0)
			return LOCK_ERR_CANNOT_ACCESS;
		if (nread < 0)
			return LOCK_ERR_CANNOT_ACCESS;
		if (nread == 0) {
			errno = EINVAL;
			return LOCK_ERR_MALFORMED;
		}

		locker[nread > MAXNAME ? MAXNAME : nread] = '\0';
		return locker;
	}

	if (errno == ELOOP)
		return LOCK_ERR_NOT_REGULAR;
	return LOCK_ERR_CANNOT_ACCESS;
}

/*********************
 *
 * undolock -- unlock the file fname
 *
 * if successful, returns NULL
 * if other error, returns "LOCK ERROR: explanation"
 *
 *********************/

const char *undolock(char *fname)
{
	static char lname[MAXLOCK];

	if (snprintf(lname, sizeof(lname), "%s.lock~", fname)
	    >= (int)sizeof(lname)) {
		errno = ENAMETOOLONG;
		return LOCK_ERR_PATH_TOO_LONG;
	}
	if (unlink(lname) != 0) {
		if (errno == EACCES || errno == ENOENT)
			return NULL;
		if (errno == EROFS)
			return NULL;
		return LOCK_ERR_CANNOT_REMOVE;
	}
	return NULL;
}
