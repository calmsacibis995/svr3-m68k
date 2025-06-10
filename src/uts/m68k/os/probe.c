/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/probe.c	10.6"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/pfdat.h"
#include "sys/debug.h"
#include "sys/tuneable.h"
#include "sys/cmn_err.h"

/*
* Calculate number of pages transfer touchs
*/
#define len(base, count)	\
btoc(base + count) - btoct(base)

/*
* Calulate starting user PTE address for transfer
*/
#define upt(base)	\
(pde_t *) uvtopde(base)

/*
* Check user read/write access
* If rw has B_PHYS set, the user PTEs will be faulted
* in and locked down.  Returns 0 on failure, 1 on success
*/

useracc(base, count, rw)
register base, count;
{
register int	i;
register int	npgs;
register int	x;
register pde_t	*pt;
register reg_t  *rp;


/*
 * Check page permissions and existence
 */

npgs = len(base, count);

rp = findreg(u.u_procp, base);
if (rp == NULL)
	return(0);
regrele(rp);

for (i = npgs; --i >= 0; base += ctob(1)) {
	x = fubyte(base);
	if (x == -1)
		goto out;
	
	/*	In the following test we check for a copy-
	**	on-write page and if we find one, we break
	**	the copy-on-write even if we are not going
	**	to write to the page.  This is necessary
	**	in order to make the lock accounting
	**	work correctly.  The problem is that the
	**	lock bit is in the pde but the count of
	**	the number of lockers is in the pfdat.
	**	Therefore, if more than one pde refers
	**	to the same pfdat, the accounting gets
	**	wrong and we could fail to unlock a
	**	page when we should.  Note that this is
	**	not very likely to happen since it means
	**	we did a fork, no exec, and then started
	**	doing raw I/O.  Still, it could happen.
	*/

	if (rw & B_READ)
		if (subyte(base, x) == -1)
			goto out;
	if ((rw & B_PHYS)  &&  ! rp->r_noswapcnt) {
		pt = upt(base);
		if (pg_tstcw(pt)  &&  !(rw & B_READ))
			if (subyte(base, x) == -1)
				goto out;
		pg_setlock(pt);
		if (pdetopfdat(pt)->pf_rawcnt++ == 0) {
			if (availrmem - 1 < tune.t_minarmem) {
				cmn_err(CE_NOTE,
					"useracc - couldn't lock page");
				pg_clrlock(pt);
				--pdetopfdat(pt)->pf_rawcnt;
				u.u_error = EAGAIN;
				goto out;
			}
			availrmem -= 1;
		}
	}
}
return(1);
out:

if ((rw & B_PHYS)  &&  ! rp->r_noswapcnt) {
	for (i++, base -= ctob(1)  ;  i < npgs  ;
	    i++, base -= ctob(1)) {
		pt = upt(base);
		if (--pdetopfdat(pt)->pf_rawcnt == 0) {
			pg_clrlock(pt);
			availrmem += 1;
		}
	}
}
return(0);
}

/*
* Setup user physio
*/

userdma(base, count, rw)
{
return(useracc(base, count, rw | B_PHYS));
}

/*
* Terminate user physio
*/

undma(base, count, rw)
{
register pde_t	*pt;
register int	npgs;
register reg_t	*rp;
register pfd_t	*pfd;


/*
 * Unlock PTEs, set the reference bit.
 * Set the modify bit if B_READ
 */

rp = findreg(u.u_procp, base);
ASSERT(rp != NULL);

	for (npgs = len(base, count) ; --npgs >= 0 ; base += ctob(1)) {
		pt = upt(base);
		pfd = pdetopfdat(pt);
		ASSERT(rp->r_noswapcnt  ||  pg_tstlock(pt));
		ASSERT(rp->r_noswapcnt  ||  pfd->pf_rawcnt > 0);
		if (! rp->r_noswapcnt  &&  --pfd->pf_rawcnt == 0) {
			pg_clrlock(pt);
			availrmem += 1;
		}
		pg_setref(pt);
		if (rw == B_READ) 
			pg_setmod(pt);
	}
	regrele(rp);
}

