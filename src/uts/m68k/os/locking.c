/*
 * $Header: /rl/sys/os/RCS/locking.c,v 1.3 86/09/08 09:55:32 dap Exp $
 * $Locker:  $
 */
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/locking.c	1.1"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/flock.h"
#include "sys/conf.h"
#include "sys/unistd.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/cmn_err.h"

/*
 *	Locking() implements the /usr/group record locking system call
 *	using the same primatives as the supported UNIX 5.3 fcntl
 *	system call.  Actually a mapping occurs here from one to the
 *	other.
 */

locking() {
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	size;
	} *uap = (struct a *) u.u_ap;
	struct file *getf();

	if ((fp = getf(uap->fdes)) == NULL)
		return;

	FS_LOCKING(fp->f_inode, uap->cmd, fp->f_flag, fp->f_offset, uap->size);
}

/*
 * FS_LOCKING	-	minor kludge 'till  locking() goes away
 */
FS_LOCKING(ip, cmd, flag, offset, size)
struct inode *ip;
off_t offset; {
	extern struct fstypsw fstypsw[];

	if (ip->i_fstypp == &fstypsw[1])
		s5locking(ip, cmd, flag, offset, size);

	else if (ip->i_fstypp == &fstypsw[2])
		dulocking(ip, cmd, flag, offset, size);

	else if (ip->i_fstypp == &fstypsw[3])
		u.u_error =  EINVAL;
	else
		cmn_err(CE_PANIC, "FS_LOCKING: bad fstype", ip, ip->i_fstypp);
}

/*
 * s5locking -	implement file and record locking a la /usr/group
 *
 * Input:
 *	ip		-	generic FS inode
 *	cmd		-	locking command
 *	flag	-	flags from users file descriptor
 *	offset	-	current file offset
 *	size	-	length of region requested locked
 */
s5locking(ip, cmd, flag, offset, size)
off_t offset;
struct inode *ip; {
	struct flock l;

	l.l_whence = 1;
	if (size < 0) {
		l.l_start =  size;
		l.l_len   = -size;
	} else {
		l.l_start =  0L;
		l.l_len   =  size;
	}

	/*
	 * flag this file as one requiring checking for locks during rd/wri
	 */
	ip->i_flag |= ICHKLCK;

	/*
	 * We map /usr/group locks to sysV write locks.
	 * If we are performing an operation that may lock a region,
	 * and we may not have the file open for write,
	 * check to see if we could have had it open for write if we wanted,
	 * and kludge `flag' appropriately
	 */
	if (cmd == F_LOCK || cmd == F_TLOCK) {
		if (s5access(ip, IWRITE))
			return;

		flag |= FWRITE;
	}

	switch (cmd) {
	case F_ULOCK:
		/*
		 * unlock record lock and return
		 */
		l.l_type = F_UNLCK;
		u.u_error = reclock(ip, &l, SETFLCK, flag, offset);
		break;

	case F_LOCK:
		/*
		 * set record lock and wait if blocked
		 */
		l.l_type = F_WRLCK;
		u.u_error = reclock(ip, &l, SETFLCK | SLPFLCK, flag, offset);
		break;

	case F_TLOCK:
		/*
		 * set record lock and return if blocked
		 */
		l.l_type = F_WRLCK;
		u.u_error = reclock(ip, &l, SETFLCK, flag, offset);
		break;

	case F_TEST:
		/*
		 * get record lock
		 */
		l.l_type = F_WRLCK;
		if ((u.u_error = reclock(ip, &l, 0, flag, offset)) == 0)
			if (l.l_type != F_UNLCK)
				u.u_error = EACCES;
		break;

	default:
		u.u_error = EINVAL;
		break;
	}

	switch (u.u_error)  {
	case EMFILE:
	case ENOSPC:
		/*
		 * a DEADLOCK error is given if we run out of resources,
		 * in compliance with locking() specs.
		 */
		u.u_error = EDEADLK;
		break;
	
	case EAGAIN:
		/*
		 * EAGAIN ==> region is locked, we would block on the request.
		 * The locking() spec requires EACCES be returned here.
		 */
		u.u_error = EACCES;
		break;

	default:
		break;
	}
}
