/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/mem.c	10.6"

/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/NULL
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/immu.h"

mmread(dev)
{
	register unsigned n;
	register pde_t	  *mempde;
	register caddr_t  memvad;
	register c;

	dev = minor(dev);
	if (dev == 2)		/* dev/eof */
		return;

	memvad = (caddr_t) sptalloc(1, 0, -1);
	mempde = (pde_t *) kvtokptbl(memvad);

	while ((u.u_error == 0) && (u.u_count != 0)) {
		n = min(u.u_count, ctob(1));
		switch (dev) {
		case (0):
			pg_setpfn(mempde, btoct(u.u_offset));
			pg_setvalid(mempde);
			mmuflush();	/* was: kflushaddr(memvad); */
			c = poff(u.u_offset);
			n = min(n, NBPP-c);
			if (copyout(&memvad[c], u.u_base, n)) {
				u.u_error = ENXIO;
				goto out;
			}
			break;
		case (1):
			if (copyout(u.u_offset, u.u_base, n)) {
				u.u_error = ENXIO;
				goto out;
			}
			break;
		case (2):
			goto out;
		default:
			u.u_error = ENODEV;
			goto out;
		}
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}

out:
	sptfree(memvad, 1, 0);
}

mmwrite(dev)
{
	register unsigned n;
	register c;
	register pde_t	  *mempde;
	register caddr_t  memvad;

	dev = minor(dev);
	if (dev == 2) {		/* dev/null */
		u.u_offset += u.u_count;
		u.u_base   += u.u_count;
		u.u_count  = 0;
		return;
	}

	memvad = (caddr_t) sptalloc(1, 0, -1);
	mempde = (pde_t *) kvtokptbl(memvad);

	while ((u.u_error == 0) && (u.u_count != 0)) {
		n = min(u.u_count, ctob(1));
		switch (dev) {
		case (0):
			pg_setpfn(mempde, btoct(u.u_offset));
			pg_setvalid(mempde);
			mmuflush();	/* was: kflushaddr(memvad); */
			c = poff(u.u_offset);
			n = min(n, NBPP-c);
			if (copyin(u.u_base, &memvad[c], n)) {
				u.u_error = ENXIO;
				goto out;
			}
			break;
		case (1):	/* beware of kernel read-only text */
			pg_setpfn(mempde, pg_getpfn(kvtopde(u.u_offset)));
			pg_setvalid(mempde);
			mmuflush();	/* was: kflushaddr(memvad); */
			c = poff(u.u_offset);
			n = min(n, NBPP-c);
			if (copyin(u.u_base, &memvad[c], n)) {
				u.u_error = ENXIO;
				goto out;
			}
			/* drop thru */
		case (2):
			break;
		default:
			u.u_error = ENODEV;
			goto out;
		}
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}

out:
	sptfree(memvad, 1, 0);
}
