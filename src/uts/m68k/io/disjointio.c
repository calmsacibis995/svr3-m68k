/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/disjointio.c	1.0"
/* 
 *		VME 1000 DMA break-up routine for any physical I/O
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/region.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/disjointio.h"
#include "sys/debug.h"

disjointio(p, base, count, rw, djntptr, szdjnt)
proc_t      *p;		/* proc pointer to process doing I/O */
caddr_t	    base;	/* virtual address in process for I/O */
unsigned    count;	/* size of I/O block */
int         rw;		/* Read/Write flag */
register    struct djntio *djntptr; /* disjoint array for discontiguous pages */
unsigned    szdjnt;	/* size of disjoint array */
{
	preg_t	*prp;	/* I/O pregion */
	reg_t	*rp;	/* I/O region  */
	register pde_t	*pd;	/* I/O pde     */
	register djcount = 0;	/* disjoint count */
	register i, j, size;
	register caddr, ccount;
	int	tmp, seglim, iosize;
	preg_t *vtopreg();

	if (count == 0)				/* just because */
		return(0);
	if ((prp = vtopreg(p, base)) == NULL) {	/* find pregion */
		u.u_error = EFAULT;
		return(-1);
	}
	if ((prp->p_flags & PF_RDONLY) && (rw == B_READ)) {
		u.u_error = EFAULT;		/* check protection */
		return(-1);
	}

	rp = prp->p_reg;
	reglock(rp);
	caddr  = btoct(base - prp->p_regva);
	ccount = btoct(base + count - prp->p_regva - 1) - caddr + 1;

	if ((caddr + ccount -1) > (rp->r_pgoff + rp->r_pgsz)) {
		u.u_error = EFAULT;
		regrele(rp);
		return(-1);
	}

	iosize = caddr + ccount;
	i = ctost(caddr);
	seglim = ctos(iosize);
	ASSERT(i <= seglim);

	for ( ; i < seglim; ++i) {
		pd = rp->r_list[i];
		ASSERT(pd != 0);
		size = iosize - stoc(i);
		if (size > NPGPT)
			size = NPGPT;
		if (caddr > stoc(i)) {
			tmp = caddr - stoc(i);
			size -= tmp;
			pd += tmp;
		}
		for (j = 0; j < size; ++j, ++pd) {
			ASSERT(pg_tstvalid(pd));
			ASSERT(!pg_tstcw(pd) || (rw == B_WRITE));
			ASSERT(pg_tstlock(pd));

			if (djcount == 0) {
				djntptr->addr = ctob(pg_getpfn(pd));
				djntptr->addr += poff(base);
				djntptr->count = min(NBPP-poff(base), count);
				count -= djntptr->count;
				djcount++;
			} else {
				if ((djntptr->addr + djntptr->count) != 
				    ctob(pg_getpfn(pd))) {
					if (++djcount > szdjnt) {
						regrele(rp);
						return(szdjnt);
					}
					djntptr++;
					djntptr->addr = ctob(pg_getpfn(pd));
					djntptr->count = min(NBPP, count);
				} else
					djntptr->count += min(NBPP, count);
				count -= min(NBPP, count);
			}
			if (count == 0) {
				regrele(rp);
				return(djcount);
			}
		}
	}
	regrele(rp);
	return(djcount);
}

undisjointio(p, base, count, rw, djntptr, szdjnt)
proc_t      *p;		/* proc pointer to process doing I/O */
caddr_t	    base;	/* virtual address in process for I/O */
unsigned    count;	/* size of I/O block */
int         rw;		/* Read/Write flag */
struct djntio *djntptr;	/* disjoint array for discontiguous pages */
unsigned    szdjnt;	/* size of disjoint array */
{
}
