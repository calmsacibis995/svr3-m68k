/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/getpages.c	10.8"
#include "sys/types.h"
#include "sys/tuneable.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/getpages.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"

int		getpgslim;	/* Current limit for getpages.  It is	*/
				/* either tune.t_gpgslo or		*/
				/* tune.t_gpgshi.			*/
int		pglstlock = 0;	/* Lock for the page lists below.	*/
				/* Needed because sched and vhand can	*/
				/* try to use at the same time.		*/
unsigned int	currentstamp = 0;	/* region time stamp used to 	*/
				/* determine the oldest region on the	*/
				/* active list.  			*/

/*	The following tables are described in getpages.h
 */

extern	pglst_t	spglst[];
extern	gprgl_t	gprglst[];
int	spglndx;
int	gprgndx;

pglstlk()
{
	while (pglstlock & PL_LOCK) {
		pglstlock |= PL_WANTED;
		sleep(&pglstlock, PZERO);
	}
	pglstlock |= PL_LOCK;
}

pglstunlk()
{
	ASSERT(pglstlock & PL_LOCK);
	if (pglstlock & PL_WANTED)
		wakeup(&pglstlock);
	pglstlock = 0;
}

tstpglstlk()
{
	return(pglstlock & PL_LOCK);
}

reg_t*
regoldest( cutoff )
{
	register reg_t *rp;
	register reg_t *srp;
	register int   msk;
	register int   tstamp;

	srp = NULL;
	msk = tune.t_gpgsmsk;
	tstamp = cutoff;

	/* 	Look for the region on the active list with the smallest
	 *	timestamp.  The timestamp is an indicator for when pages
	 *	were collected from a region.  The region with the smallest
	 *	timestamp has gone the longest without having its pages 
	 *	collected.
	 */

	for (rp = ractive.r_forw; rp != &ractive; rp = rp->r_forw) {

		if (reglocked(rp))	/* ignore locked regions */
			continue;

		/*
		**	Test for new un-aged regions.  Ignore these
		**	regions since the time stamp is either zero 
		**	or they have not aged enough to collect pages
		**	from them.
		*/

		if ((rp->r_flags & msk) != msk)
			continue;

		if (rp->r_timestamp < tstamp) {
			tstamp = rp->r_timestamp;
			srp = rp;
		}
	}
	return(srp);
}

/*	This process is awakened periodically by clock to update the
 *	system's idea of the working sets of all processes and to
 *	steal pages from processes if freemem is too low.
 */

vhand()
{
	register reg_t	*rp;
	register unsigned int cutoff;

	getpgslim = tune.t_gpgslo;
	tune.t_vhandl = max(maxmem/v.v_vhndfrac, tune.t_gpgshi);

	for (;;) {

		minfo.vhandwake++;
		pglstlk();

		/*
		 * If memory is tight, start collecting pages from the
		 * region with the oldest time stamp.  Continue until
		 * either enough memory is collected or all regions
		 * have been advanced greater than the cutoff time.
		 */

		if (freemem <= getpgslim) {

		       /*	If memory is not tight, then don't steal any
		 	*	pages.  Once we start to steal pages, then
		 	*	steal enough to get up to the high water mark.
		 	*	This is to avoid getting into repeated tight
		 	*	memory situations.
		 	*/

			cutoff = currentstamp;
			getpgslim = tune.t_gpgshi;

			while( rp = regoldest( cutoff ) ) {
				reglock(rp);
				minfo.vhandout++;
				getpages(rp, 0);

				/*	If the current region was not put
				**	into the list of pages to be
				**	swapped out, then unlock the region
				**	now.
				*/
	
				if (gprglst[gprgndx].gpr_count == 0  ||
	   				gprglst[gprgndx].gpr_rgptr != rp) {
					regrele(rp);
				}

				if (freemem > getpgslim) {
					getpgslim = tune.t_gpgslo;
					break;
				}
			}
		}

		for (rp = ractive.r_forw; rp != &ractive; rp = rp->r_forw) {
			if (!reglocked(rp))	/* ignore locked regions */
				ageregion(rp);
		}

		if (spglndx) {
			if (freemem <= getpgslim)
				swapchunk(0);
			else {
				freelist(spglst, spglndx, 0);
				spglndx = 0;
			}
		}

		ASSERT(spglndx == 0);
		ASSERT(gprglst[gprgndx].gpr_count == 0);

		pglstunlk();

		/*	Go to sleep until clock wakes us up again.
		*/

		sleep(vhand, PSWP);
	}
}

/*	Do page use bit ageing for the pte's in
 *	the passed region
 */
ageregion(rp)
register reg_t	*rp;
{
	register pde_t	*pt;
	register int	i;
	register int	j;
	register int	seglim;
	register int	pglim;
	register int	size;
	register int 	tmp;

	if ((rp->r_flags & tune.t_gpgsmsk) != tune.t_gpgsmsk) { /* age region */
		if (rp->r_flags & RG_REF)
			rp->r_flags |= RG_NDREF;
		rp->r_flags |= RG_REF;
	}

	if (rp->r_nvalid == 0)
		return;				/* if nothing to do return  */

	minfo.vhandage++;

	/*	Look at all of the segments of the region.
	 */

	size = rp->r_pgoff + rp->r_pgsz;
	seglim = ctos(size);

	for (i = ctost(rp->r_pgoff);  i < seglim  ;  i++) {

		/*	Look at all of the pages of the segment.
		 */

		pt = rp->r_list[i];
		ASSERT(pt != 0);
		pglim = size - stoc(i);
		if (pglim > NPGPT)
			pglim = NPGPT;
		if (rp->r_pgoff > stoc(i)) {
			tmp = rp->r_pgoff - stoc(i);
			pt += tmp;
			pglim -= tmp;
		}

		for (j = 0  ;  j < pglim  ;  j++, pt++) {

			/*	Check to see if this page is part of
			 *	the working set.  If not, it does
			 *	not have to be aged.
			 */

			if (!pg_tstvalid(pt))
				continue;

			/*	We have an active page.  Age it
			 *	unless it is already as old as 
			 *	it can get.
			 */

			if (pg_tstref(pt)) {
				pg_clrref(pt);
				pg_setndref(pt);
			} else {
				pg_clrndref(pt);
			}
		}
	}
}

/*	Swap out pages from region rp which is locked by
 *	our caller.  If hard is set, take all valid pages,
 *	othersize take only unreferenced pages
 */

getpages(rp, hard)
register reg_t	*rp;
{
	register pde_t	*pt;
	register int	i;
	register int	j;
	register int	seglim;
	register int	pglim;
	register dbd_t	*dbd;
	register int	size;
	register int	tmp;

	ASSERT(reglocked(rp));
	ASSERT(tstpglstlk());

	rp->r_timestamp = currentstamp++;	/* set the age time stamp   */

	if (rp->r_nvalid == 0)		/* if nothing to do return */
		return;

	/*	If the region is marked "don't swap", then don't
	 *	steal any pages from it.
	 */

	ASSERT(rp->r_noswapcnt >= 0);

	if (rp->r_noswapcnt)
		return;

	minfo.getpages++;

	/*	Look through all of the segments of the region.
	 */

	size = rp->r_pgoff + rp->r_pgsz;
	seglim = ctos(size);

	for (i = ctost(rp->r_pgoff);  i < seglim;  i++) {

		/* Look through segment's page table for valid
		 * pages to dump.
		 */

		pt = rp->r_list[i];
		ASSERT(pt != 0);
		pglim = size - stoc(i);
		if (pglim > NPGPT)
			pglim = NPGPT;
		if (rp->r_pgoff > stoc(i)) {
			tmp = rp->r_pgoff - stoc(i);
			pt += tmp;
			pglim -= tmp;
		}

		for (j = 0  ;  j < pglim  ;  j++, pt++) {

			/* If we have gotten enough pages, then don't
			 * steal any more.
			 */

			if (!hard && (freemem > getpgslim)) {
				getpgslim = tune.t_gpgslo;
				break;
			}

			/* Check to see if there is a page assigned
			 * and if it is eligible to be stolen.
			 */

			if (!pg_tstvalid(pt)  ||  pg_tstlock(pt))
				continue;

			/* We have a valid page assigned.
			 * See if we want to steal it.  
			 */

			/* 	Don't steal it, if the page has
			 * 	been referenced recently
			 */
			if (!hard && (pg_getpde(pt) & tune.t_gpgsmsk)) {
					minfo.recent++;
					continue;
			}

			/* See if this page must be written to swap.
			 */

			dbd = (dbd_t *)pt + NPGPT;

			switch (dbd->dbd_type) {
			case DBD_NONE: {
				register pfd_t	*pfd;

				/* Check to see if the page is already
				 * associated with swap.  If so, just
				 * use the same swap block unless the 
				 * swap use count overflows.
				 */

				pfd = pdetopfdat(pt);
				if (pfd->pf_flags & P_HASH) {
					ASSERT(pg_tstcw(pt));
					dbd->dbd_type = DBD_SWAP;
					dbd->dbd_swpi = pfd->pf_swpi;
					dbd->dbd_blkno = pfd->pf_blkno;
					if (swpinc(dbd, "getpages")) {
						pg_clrmod(pt);
						minfo.cwsw++;
						addfpg(rp, pt);
						break;
					}
					premove(pfd);
					dbd->dbd_type = DBD_NONE;
				}
				minfo.none++;
				addspg(rp, pt);
				break;
			}

			case DBD_SWAP:

				/* See if this page has been modified
				 * since it was read in from swap.
				 * If not, then just use the copy
				 * on the swap file.
				 */

				if (!pg_tstmod(pt)) {
					minfo.unmodsw++;
					addfpg(rp, pt);
					break;
				}

				/*	The page has been modified.
				 *	Release the current swap
				 *	block and add it to the list
				 *	of pages to be swapped out
				 *	later.
				 */

				minfo.modsw++;
				if (swfree1(dbd) == 0)
				    if (!pbremove(rp, dbd))
					cmn_err(CE_PANIC,"getpages - pbremove");
				dbd->dbd_type = DBD_NONE;
				addspg(rp, pt);
				break;

			case DBD_FILE:
			case DBD_LSTFILE:

				/* This page cannot have been modified
				 * since if it had been, then it would
				 * be marked DBD_NONE, not DBD_FILE.
				 * Either the page is text and so the
				 * segment table entry is RO or it is
				 * data in which case it is copy-on-
				 * write and also RO.
				 */

				ASSERT(!pg_tstmod(pt));
				addfpg(rp, pt);
				minfo.unmodfl++;
				continue;
			}
		}

		/*	If we have gotten enough pages, then 
		 *	don't steal any more.
		 */

		if (!hard && (freemem > getpgslim)) {
			getpgslim = tune.t_gpgslo;
			break;
		}
	}

	ASSERT(reglocked(rp));
	return;
}

/*	Add an entry to the spglst table.  If the page is already on
**	the list, ignore it.  Should the list become full, call swapchunk
**	to write out the entire list.
*/

addspg(rp, pt)
register reg_t	*rp;
register pde_t	*pt;
{
	register int	i;

	minfo.swappage++;

	ASSERT(spglndx <= v.v_maxswaplist);
	ASSERT(reglocked(rp));
	ASSERT(tstpglstlk());
	ASSERT(((dbd_t *)(pt + NPGPT))->dbd_type == DBD_NONE);

	/*
	 *	If the page is already on the list to be swapped out,
	 *	don't add it to the list.
	 */

	for (i = 0; i < spglndx; ++i) {
		if (pg_getpfn(pt) == pg_getpfn(spglst[i].gp_ptptr))
			return;
	}

	/*	Increment the count of pages from this region
	**	which are on one of the lists.
	*/

	bumprcnt(rp);

	/*	Add the page to the swap list.
	*/

	ASSERT(spglndx >= 0  &&  spglndx < v.v_maxswaplist);
	spglst[spglndx].gp_ptptr = pt;
	spglst[spglndx++].gp_rlptr = &gprglst[gprgndx];

	/*	If the swap table is full, then process it.
	*/

	if (spglndx == v.v_maxswaplist)
		swapchunk(rp);	/* Swap out the pages on the swap list.  */
}


/*	Free this page immediately.  This page does not require us 
**	to perform any I/O because it is already found on secondary
**	storage, either in the filesystem or on swap.
*/

addfpg(rp, pt)
register reg_t	*rp;
register pde_t	*pt;
{
	register pfd_t	*pfd;

	minfo.freepage++;

	ASSERT(tstpglstlk());
	ASSERT(reglocked(rp));
	ASSERT(!(pg_tstmod(pt)));
	ASSERT(((dbd_t *)(pt + NPGPT))->dbd_type == DBD_SWAP  ||
	       ((dbd_t *)(pt + NPGPT))->dbd_type == DBD_FILE  ||
	       ((dbd_t *)(pt + NPGPT))->dbd_type == DBD_LSTFILE);

	pfd = pdetopfdat(pt);
	ASSERT(pfd->pf_flags & P_HASH); /* Some problems here - K. Rusnock */
	ASSERT(pg_tstvalid(pt));
	pg_clrvalid(pt);

	/*	Now free up the actual page if unused.
	*/

	if (--pfd->pf_use == 0) {

		/*	Put pages at end of queue since they
		**	represent disk blocks and we hope they
		**	will be used again soon.
		*/
		
		pfd->pf_prev = phead.pf_prev;
		pfd->pf_next = &phead;
		phead.pf_prev = pfd;
		pfd->pf_prev->pf_next = pfd;
		pfd->pf_flags |= P_QUEUE;
		freemem++;
		minfo.freedpgs++;
	}

	rp->r_nvalid--;
}

/*	Swap out a chunk of user pages.
**	Swap all pages even if we are swaping more than the high
**	water mark.  We can not determine if swapping a page will actually
**	result in free'ing the memory space.  Therefore just swap out
**	the entire list.
*/

swapchunk(rp)
register reg_t	*rp;
{
	register int	i;
	register pde_t	*pd;
	register pfd_t	*pfd;

	ASSERT(tstpglstlk());
	ASSERT(spglndx > 0  &&  spglndx <= v.v_maxswaplist);

	minfo.swapchunk++;		/* Statistical info: number of swaps */
	minfo.chunksize += spglndx;	/*             average size of swaps */

	if (swalloc(spglst, spglndx, 0) < 0) {

		/*	We could not get a contiguous chunk of
		**	swap space of the required size so do
		**	the swaps one page at a time.  Hope
		**	this doesn't happen very often.  Note
		**	that we get a "low on swap" printout
		**	on the console if this happens.
		*/

		cmn_err(CE_WARN, "!Swap space running out.");
		cmn_err(CE_CONT, "!\t Needed %d pages.\n", spglndx);

		for (i = 0  ;  i < spglndx  ;  i++) {

			/*	Allocate one page of swap and quit
			**	if none is available.  Don't wait.
			*/

			if (spglndx == 1  || swalloc(&spglst[i], 1, 0) < 0) {
				cmn_err(CE_CONT,"DANGER: Out of swap space.\n");
				freelist(&spglst[i], spglndx - i, rp);
				spglndx = i;
				break;
			}
		}
	}

	if (spglndx) {
		/*	Invalidate all of the pages at once.
		**	Insert each page in the hash list.
		*/

		for (i = 0  ;  i < spglndx  ;  i++) {
			pd = spglst[i].gp_ptptr;
			pfd = pdetopfdat(pd);

			ASSERT(!(pfd->pf_flags & P_HASH));
			pinsert(spglst[i].gp_rlptr->gpr_rgptr, pd + NPGPT, pfd);

			ASSERT(pg_tstvalid(pd));
			pg_clrvalid(pd);
			pg_clrmod(pd);
		}

		/*	Write out all of the pages at once.
		**	Notice we do not free the regions during the I/O.
		**	We do not want the process to exit or manipulate
		**	its regions.  However the process can still run
		**	(execute) during this time.
		*/

		swap(spglst, spglndx, B_WRITE);

		/*	Free up the memory we just swapped out and
		**	reset the page list index.
		*/

		memfree(spglst, spglndx, rp);
	}

	spglndx = 0;
	return;
}

/* 	Free memory.
 */

memfree(pglptr, size, lockedreg)
register pglst_t	*pglptr;
int			size;
reg_t			*lockedreg;
{
	register pfd_t		*pfd;
	register int		j;
	register gprgl_t	*rlptr;
	pde_t			*ptptr;
	reg_t			*rgptr;

	ASSERT((size > 0) && (size <= v.v_maxswaplist));

	for (j = 0  ;  j < size  ;  j++, pglptr++) {

		ptptr = pglptr->gp_ptptr;
		rlptr = pglptr->gp_rlptr;
		rgptr = rlptr->gpr_rgptr;

		ASSERT(reglocked(rgptr));

		/*	Disassociate pte from physical page.
		*/

		pfd = pdetopfdat(ptptr);
		ASSERT(pfd->pf_use > 0);

		pfd->pf_flags |= P_DONE;

		/*	free unused pages.
		*/

		if (--pfd->pf_use == 0) {

			/*	Put pages at end of queue since they
			**	represent disk blocks and we hope they
			**	will be used again soon.
			*/
			
			pfd->pf_prev = phead.pf_prev;
			pfd->pf_next = &phead;
			phead.pf_prev = pfd;
			pfd->pf_prev->pf_next = pfd;
			pfd->pf_flags |= P_QUEUE;
			freemem++;
			minfo.freedpgs++;
		}

		rgptr->r_nvalid--;

		if (--rlptr->gpr_count == 0  &&  rgptr != lockedreg) 
			regrele(rgptr);
	}
}

/*	Increment the count of pages for a region.  Either
**	it is the current region or the region is not yet
**	in the list because we process each region only
**	once during each pass of vhand.
*/

bumprcnt(rp)
register reg_t	*rp;
{
	register int	i;

	/*	If this region is not in the region list,
	**	then add it now.  Otherwise, just increment
	**	the count of pages being stolen from this
	**	region.
	*/

	ASSERT(tstpglstlk());
	ASSERT(reglocked(rp));

	if (gprglst[gprgndx].gpr_count == 0  ||
	   gprglst[gprgndx].gpr_rgptr != rp) {

		/*	The region count list can get fragmented
		**	because we can process the free page list
		**	many time before we process the swap page
		**	list.  Therefore, we must search for a
		**	free slot in the region count list.  Note
		**	that the list is cleaned up and starts
		**	fresh at the start of a new pass in vhand.
		**	In addition, there should always be a
		**	free slot because we make the region count
		**	list big enough to hold a separate region
		**	for each entry on both page lists.
		*/

		if (gprglst[gprgndx].gpr_count != 0) {
			i = gprgndx + 1;

			while (i != gprgndx) {
				if (i >= v.v_maxswaplist)
					i = 0;
				if (gprglst[i].gpr_count == 0)
					break;
				i++;
			}

			if (i == gprgndx)
				cmn_err(CE_PANIC,
				  "bumprcnt - region count list overflow.");

			gprgndx = i;
		}
		gprglst[gprgndx].gpr_rgptr = rp;
		gprglst[gprgndx].gpr_count = 1;
	} else {
		gprglst[gprgndx].gpr_count++;
	}
}

/*	This routine is called to unlock the regions of a list
**	when the list does not really need to be processed.
*/

freelist(lp, count, lockedreg)
register pglst_t	*lp;
register int		count;
register reg_t		*lockedreg;
{
	register gprgl_t	*rl;
	register reg_t		*rp;
	register int		i;

	ASSERT(tstpglstlk());
	ASSERT(count > 0);

	for (i = 0  ;  i < count  ;  i++, lp++) {
		rl = lp->gp_rlptr;
		rp = rl->gpr_rgptr;
		if (--rl->gpr_count == 0  &&  rp != lockedreg)
			regrele(rp);
	}
}
