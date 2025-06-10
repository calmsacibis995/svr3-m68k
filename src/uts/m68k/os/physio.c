/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/physio.c	10.7"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/sysinfo.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/inode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/getpages.h"
#include "sys/swap.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"

#define	SLEEP	0		/* Sleep to get buffer header */
#define NOSLP	1		/* Return immediately if no bp's available */
#define	SWPBPSZ	25		/* throttle variable to restrict no. of bp */

buf_t*
getpbp(slpflg)
{
	register buf_t* bp = NULL;
	register ospl;

	ospl = spl6();
	while (pfreecnt == 0) {
		if (slpflg)
			goto out;
		pfreelist.b_flags |= B_WANTED;
		sleep(&pfreelist, PRIBIO);
	}
	ASSERT(pfreecnt);
	ASSERT(pfreelist.av_forw);
	pfreecnt--;
	bp = pfreelist.av_forw;
	pfreelist.av_forw = bp->av_forw;
out:
	splx(ospl);
	return(bp);
}

freepbp(bp)
register buf_t* bp;
{
	register ospl;

	ospl = spl6();
	bp->av_forw = pfreelist.av_forw;
	pfreelist.av_forw = bp;
	pfreecnt++;
	if (pfreelist.b_flags & B_WANTED)
		wakeup(&pfreelist);
	pfreelist.b_flags = 0;
	splx(ospl);
}

/*
 * swap I/O
 */

swap(pglptr, npage, rw)
register pglst_t	*pglptr;
{
	register buf_t	*bp;
	register dev_t	dev;
	register int	i;
	register dbd_t	*dbd;
	int		num, npbp,flag;
	buf_t*	swpbp[SWPBPSZ];

	ASSERT(syswait.swap >= 0);
	syswait.swap++;

	u.u_iosw++;
	if (rw) {
		sysinfo.swapin++;
		sysinfo.bswapin += npage;
	} else {
		sysinfo.swapout++;
		sysinfo.bswapout += npage;
	}

	/*	Minimally allocate one buffer header, but
	 *	attempt to get as many as we need within the
	 *	limits set by the SWPBPSZ throttle variable.
	 */

	flag = SLEEP;
	npbp = 0;
	do {	if ((swpbp[npbp] = getpbp(flag)) == NULL)
			break;
		flag = NOSLP;
	} while (++npbp < min(npage, SWPBPSZ));

	ASSERT((npbp > 0) && (npbp <= npage));

	/*	We may have to loop multiple times if we
	 *	did not get enough buffer headers
	 */

	while ((num = min(npbp, npage)) != 0) {
		ASSERT((num <= npbp) && (num <= npage));

		/*	Queue up a list of disk requests for
		 *	all the buffer headers that we have.
		 *	(Or for the remaining pages to be written.)
		 */

		for (i = 0;  i < num;  i++) {
			bp = swpbp[i];
			dbd = (dbd_t *)(pglptr->gp_ptptr + NPGPT);
			dev = swaptab[dbd->dbd_swpi].st_dev;
			bp->b_proc = u.u_procp;
			bp->b_flags = B_BUSY | B_PHYS | rw;
			bp->b_dev = dev;
			bp->b_blkno = dbd->dbd_blkno;
			bp->b_un.b_addr = 
				(caddr_t)ctob(pg_getpfn(pglptr++->gp_ptptr));
			bp->b_bcount = NBPP;
			bp->b_flags &= ~(B_DONE | B_ERROR);

			(*bdevsw[bmajor(dev)].d_strategy)(bp);
		}

		/*	Wait for all of the I/O to occur.
		 *	Check for errors and panic if any are found.
		 */

		for (i = 0;  i < num;  i++) {
			iowait(swpbp[i]);
			if (swpbp[i]->b_flags & B_ERROR) {
			   cmn_err(CE_WARN,"swap: proc = 0x%x, bp = 0x%x\n",
				swpbp[i]->b_proc, swpbp[i]);
			   cmn_err(CE_WARN,"swap: dev  = 0x%x, blkno = 0x%x\n",
				swpbp[i]->b_dev, swpbp[i]->b_blkno);
			   cmn_err(CE_WARN,"swap: addr - 0x%x, flags = 0x%x\n",
				swpbp[i]->b_un.b_addr, swpbp[i]->b_flags);
			   cmn_err(CE_PANIC, "swap - i/o error in swap");
			}
		}
		npage -= num;
	}

	/*	Free up the buffer headers
	 */

	for (i = 0; i < npbp; i++)
		freepbp(swpbp[i]);

	ASSERT(syswait.swap);
	syswait.swap--;
}

/*
 * Raw I/O. The arguments are
 * The strategy routine for the device
 * A buffer, which is usually NULL, or special buffer
 *   header owned exclusively by the device for this purpose
 * The device number
 * Read/write flag
 */

physio(strat, bp, dev, rw)
register struct buf *bp;
int (*strat)();
{	register int		count;
	register caddr_t	base;
	register caddr_t	lbase;
	register int		hpf;
	register int		cc;
	register char		oldsegflg;

	count = u.u_count;
	base = u.u_base;
	if (server()) {
		if ((lbase = (caddr_t)sptalloc(btoc(count), PG_VALID, 0)) == 0)
			return;
		if (rw == B_WRITE) {
			if (remio(base, lbase, count)) {
				sptfree(lbase, btoc(count), 1);
				return;
			}
		}
		u.u_base = lbase;
		oldsegflg = u.u_segflg;
		u.u_segflg = 1;	/*you're now dealing with kernel space*/
	} else {
		if (userdma(base, count, rw) == NULL) {
			if (u.u_error == 0)
				u.u_error = EFAULT;
			return;
		}
	}

	ASSERT(syswait.physio >= 0);
	syswait.physio++;
	if (rw)
		sysinfo.phread++;
	else
		sysinfo.phwrite++;

	/*	If we need a buffer header,
	 *	get a buffer header off of the free list.
	 */

	hpf = (bp == NULL);
	if (hpf)
		bp = getpbp(SLEEP);

	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	bp->b_dev = dev;
	bp->b_un.b_addr = u.u_base;
	bp->b_blkno = btod(u.u_offset);
	bp->b_bcount = u.u_count;

	(*strat)(bp);

	iowait(bp);

	if (server()) {
		if (rw == B_READ) {
			u.u_segflg = oldsegflg;
			if (unremio(lbase, base, count))
				printf("unremio failed: err=%d\n",u.u_error);
		}
		sptfree(lbase, btoc(count), 1);
	}
	else
		undma(base, count, rw);

	bp->b_flags &= ~(B_BUSY|B_PHYS);

	if (hpf)
		freepbp(bp);

	u.u_count = bp->b_resid;
	ASSERT(syswait.physio);
	syswait.physio--;
}


physck(nblocks, rw)
daddr_t nblocks;
{
	register unsigned over;
	register off_t upper, limit;
	struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	limit = nblocks << SCTRSHFT;
	if (u.u_offset >= limit) {
		if (u.u_offset > limit || rw == B_WRITE)
			u.u_error = ENXIO;
		return(0);
	}
	upper = u.u_offset + u.u_count;
	if (upper > limit) {
		over = upper - limit;
		u.u_count -= over;
		uap = (struct a *)u.u_ap;
		uap->count -= over;
	}
	return(1);
}

