/*
 * $Header: /rl/sys/fs/du/RCS/durdwri.c,v 1.3 86/09/08 09:56:56 dap Exp $
 * $Locker:  $
 */
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/du/durdwri.c	10.4"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/file.h"
#include "sys/systm.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/message.h"
#include "sys/sysinfo.h"

dureadi(ip)
register struct inode *ip;
{
	int mult = 60;

	DUPRINT3 (DB_FSS,"dureadi: ip = %x   sndd = %x \n",ip,ip->i_fsptr);
	if (u.u_syscall != DUREAD) 
		u.u_syscall = DUREADI;
	if(ip->i_ftype & IFIFO)
		prele(ip);

	do {
		int waslocked;

		u.u_error = 0;
		remfileop (ip,NULL,NULL);
		if (u.u_error == EBUSY) {
			/*
			 * funny return code from remote, indicating that
			 * the region is locked, and we should retry again.
			 * This is necessary because we have the inode locked
			 * locally, and another local process may be deadlocked
			 * trying to close the file, which would unlock the
			 * region.  SIGH.
			 */
			if (waslocked = (ip->i_flag & ILOCK))
				prele(ip);

			delay((mult/60) * HZ);	/* delay one sec: needs to be interruptable	*/

			if (waslocked)
				plock(ip);
		}
		/*
		 * never go over 60 second delay
		 */
		if (++mult > (60*60))
			--mult;
	} while (u.u_error == EBUSY);

	if(ip->i_ftype & IFIFO)
		plock(ip);
}

duwritei(ip)
register struct inode *ip;
{
	int mult = 60;

	DUPRINT3 (DB_FSS,"duwritei: ip = %x   sndd = %x \n",ip,ip->i_fsptr);
	if (u.u_syscall != DUWRITE) 
		u.u_syscall = DUWRITEI;
	if(ip->i_ftype & IFIFO)
		prele(ip);
	do {
		int waslocked;

		u.u_error = 0;
		remfileop (ip,NULL,NULL);

		if (u.u_error == EBUSY) {
			/*
			 * funny return code from remote, indicating that
			 * the region is locked, and we should retry again.
			 * This is necessary because we have the inode locked
			 * locally, and another local process may be deadlocked
			 * trying to close the file, which would unlock the
			 * region.  SIGH.
			 */
			if (waslocked = (ip->i_flag & ILOCK))
				prele(ip);

			delay((mult/60) * HZ);	/* delay one second	*/

			if (waslocked)
				plock(ip);
		}
		/*
		 * never go over 60 second delay
		 */
		if (++mult > (60*60))
			--mult;
	} while (u.u_error == EBUSY);

	if(ip->i_ftype & IFIFO)
		plock(ip);
}
