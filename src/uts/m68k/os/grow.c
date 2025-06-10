/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/grow.c	10.8"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/tuneable.h"

/* brk and sbrk system calls */

sbreak()
{
	struct a {
		uint nva;
	};
	register preg_t	*prp;
	register reg_t	*rp;
	register uint	nva;
	register int	change;

	/*	Find the processes data region.
	 */

	prp = findpreg(u.u_procp, PT_DATA);
	if (prp == NULL)
		goto sbrk_err;
	rp = prp->p_reg;
	reglock(rp);

	nva = ((struct a *)u.u_ap)->nva;

	if (nva < u.u_exdata.ux_datorg)
		nva = u.u_exdata.ux_datorg;

	change = btoc(nva) - btoct(prp->p_regva) - (rp->r_pgoff + rp->r_pgsz);
	if (change > 0  &&  chkpgrowth(change) < 0) {
		regrele(rp);
		goto sbrk_err;
	}

	if (growreg(prp, change, DBD_DZERO) < 0) {
		regrele(rp);
		return;
	}

	regrele(rp);
	return;

sbrk_err:
	u.u_error = ENOMEM;
	return;
}

/* grow the stack to include the SP
* true return if successful.
*/

grow(sp)
unsigned sp;
{
	register preg_t	*prp;
	register reg_t	*rp;
	register	si;

	/*
	 * Since the faulting address could be off by SPILLSIZE bytes
	 * from the stack pointer, decrement the sp to allow for this
	 * discrepency.
	 */

	sp -= SPILLSIZE;

	/*	Find the processes stack region.
	 */

	prp = findpreg(u.u_procp, PT_STACK);
	if (prp == NULL)
		return(-1);
	rp = prp->p_reg;
	reglock(rp);

	si = btoc(USRSTACK - sp) - rp->r_pgsz + SINCR;
	if (si <= 0) {
		regrele(rp);
		return(0);
	}

	if (chkpgrowth(si) < 0) {
		regrele(rp);
		return(-1);
	}

	if (growreg(prp, si, DBD_DZERO) < 0) {
		regrele(rp);
		return(-1);
	}

	u.u_ssize = rp->r_pgsz;

	regrele(rp);
	return(1);
}

/*	Check that a process is not trying to expand
**	beyond the maximum allowed virtual address
**	space size.
*/

chkpgrowth(size)
register int	size;	/* Increment being added (in pages).	*/
{
	register preg_t	*prp;
	register reg_t	*rp;

	prp = u.u_procp->p_region;

	while (rp = prp->p_reg) {
		size += rp->r_pgsz;
		prp++;
	}

	if (size > tune.t_maxumem)
		return(-1);
	
	return(0);
}

