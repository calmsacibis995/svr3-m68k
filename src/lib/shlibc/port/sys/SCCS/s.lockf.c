h38991
s 00072/00000/00000
d D 1.1 86/07/31 11:47:13 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:sys/lockf.c	1.12"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

lockf(fildes, function, size)
long size;
int fildes, function;
{
	struct flock l;
	int rv;

	l.l_whence = 1;
	if (size < 0) {
		l.l_start = size;
		l.l_len = -size;
	} else {
		l.l_start = 0L;
		l.l_len = size;
	}
	switch (function) {
	case F_ULOCK:
		l.l_type = F_UNLCK;
		rv = fcntl(fildes, F_SETLK, &l);
		break;
	case F_LOCK:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_SETLKW, &l);
		break;
	case F_TLOCK:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_SETLK, &l);
		break;
	case F_TEST:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_GETLK, &l);
		if (rv != -1) {
			if (l.l_type == F_UNLCK)
				return (0);
			else {
				errno = EACCES;
				return (-1);
			}
		}
		break;
	default:
		errno = EINVAL;
		return (-1);
	}
	if (rv < 0) {
		switch(errno) {
		case EMFILE:
		case ENOSPC:
		case ENOLCK:
			/* A deadlock error is given if we run out of resources,
			 * in compliance with /usr/group standards.
			 */
			errno = EDEADLK;
			break;
		default:
			break;
		}
	}
	return (rv);
}
E 1
