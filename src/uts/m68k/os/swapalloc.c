/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/swapalloc.c	10.8"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/sysinfo.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/conf.h"
#include "sys/inode.h"
#include "sys/file.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/systm.h"
#include "sys/debug.h"
#include "sys/open.h"
#include "sys/getpages.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/tuneable.h"

proc_t		*swapwold;	/* Head of "waiting for swap" list. */
proc_t		*swapwnew;	/* Tail of "waiting for swap" list. */

/*	Allocate swap file space.
 */

swalloc(pglist, size, waitflag)
register pglst_t	*pglist;/* Ptr to a list of pointers to page  */
				/* table entries for which swap is to */
				/* be allocated.		      */
register int	size;		/* Number of pages of swap needed.    */
register int	waitflag;	/* If required space is not available */
				/* then wait for it if this flag is   */
				/* set and return an error otherwise. */
{
	register int	smi;
	register use_t	*cntptr;
	register int	i;
	register int	swappg;
	register dbd_t	*dbd;
	register proc_t	*pp;

	swappg = 0;

	/*	Search all of the swap files, starting with the one
	 *	following the one which we allocated on last, looking
	 *	for a file with enough space to satisfy the current
	 *	request.
	 */
	for (;;) {

		smi = nextswap;

		/*	There can be holes in the swap file table
		 *	(swaptab) due to deletions.
		 */

		do {
			/*	If the current swaptab entry is not
			 *	in use or is in the process of being
			 *	deleted, go on to the next one.
			 */

			if ((swaptab[smi].st_ucnt == NULL)  ||
			   (swaptab[smi].st_flags & ST_INDEL))
				continue;
			swappg = swapfind(&swaptab[smi], size);
			if (swappg >= 0)
				break;
		} while ((smi = (smi + 1) % MSFILES) != nextswap);

		/*	If we got the swap space, then go set up the
		 *	disk block descriptors.
		 */

		if (swappg >= 0)
			break;

		/*	Since swapclup can sleep, test the waitflag first
		 *	and return if the caller can't wait for swap.
		 */

		if (waitflag == 0)
			return(-1);

		/*	Try to free up some swap space by removing
		 *	unused sticky text regions.  If this
		 *	suceeds, try to allocate again.  Otherwise,
		 *	either return an error or go to sleep
		 *	waiting for swap space depending on the
		 *	setting of the "waitflag" argument.
		 */

		if (swapclup())
			continue;

		cmn_err(CE_CONT, "DANGER: Out of swap space.\n");
		cmn_err(CE_CONT, "\tWaiting for %d pages.\n", size);

		pp = u.u_procp;
		pp->p_mpgneed = size;
		pp->p_mlink = NULL;
		if (swapwold) {
			swapwnew->p_mlink = pp;
			swapwnew = pp;
		} else {
			swapwold = swapwnew = pp;
		}
		sleep(&swapwold, PMEM);
	}


	/*	Set up for main processing loop.
	*/

	cntptr = &swaptab[smi].st_ucnt[swappg];
	swappg = swaptab[smi].st_swplo + (swappg << DPPSHFT);
	swaptab[smi].st_nfpgs -= size;
	nextswap = (smi + 1) % MSFILES;


	/*	Initialize the swap use counts for each page
	 *	and set up the disk block descriptors (dbd's).
	 */

	for (i = 0  ;  i < size  ;  i++, cntptr++, pglist++) {
		*cntptr = 1;
		dbd = (dbd_t *)(pglist->gp_ptptr + NPGPT);
		dbd->dbd_type = DBD_SWAP;
		dbd->dbd_swpi = smi;
		dbd->dbd_blkno = swappg + (i << DPPSHFT);
	}
	
	return(swappg);
}


/*	Free one page of swap and return the resulting use count.
 */

swfree1(dbd)
register dbd_t	*dbd;	/* Ptr to disk block descriptor for	*/
			/* block to be removed.			*/
{
	register use_t	*cntptr;
	register int	pgnbr;
	register swpt_t	*st;
	register int	retval;


	st = &swaptab[dbd->dbd_swpi];
	pgnbr = (dbd->dbd_blkno - st->st_swplo) >> DPPSHFT;
	cntptr = &st->st_ucnt[pgnbr];

	ASSERT(*cntptr != 0);

	/*	Decrement the use count for this page.  If it goes
	 *	to zero, then free the page.  If anyone is waiting
	 *	for swap space, wake them up.
	 */

	retval = (*cntptr -= 1);

	if (retval == 0) {
		st->st_nfpgs += 1;

		/*	Wake up the first process waiting for swap
		 *	if we have freed up enough space.  Since we
		 *	are only freeing one page, we cannot
		 *	satisfy more than one process's request.
		 */

		if (swapwold  &&  swapwold->p_mpgneed <= st->st_nfpgs) {
			setrun(swapwold);
			swapwold = swapwold->p_mlink;
		}
	}

	return(retval);
}


/*	Find the use count for a block of swap.
 */

swpuse(dbd)
register dbd_t	*dbd;
{
	register swpt_t	*st;
	register int	pg;

	st = &swaptab[dbd->dbd_swpi];
	pg = (dbd->dbd_blkno - st->st_swplo) >> DPPSHFT;

	return(st->st_ucnt[pg]);
}


/*	Increment the use count for a block of swap.
 */

swpinc(dbd, nm)
register dbd_t	*dbd;
char		*nm;
{
	register swpt_t	*st;
	register int	pg;

	st = &swaptab[dbd->dbd_swpi];
	pg = (dbd->dbd_blkno - st->st_swplo) >> DPPSHFT;

	if (st->st_ucnt[pg] >= MAXSUSE) {
		cmn_err(CE_NOTE, "%s - swpuse count overflow.\n", nm);
		return(0);
	}
	st->st_ucnt[pg]++;
	return(1);
}


/*	Add a new swap file.
 */

swapadd(dev, lowblk, nblks)
dev_t		dev;		/* The device code.		*/
int		lowblk;		/* First block on device to use.*/
int		nblks;		/* Nbr of blocks to use.	*/
{
	register int	smi;
	register swpt_t	*st;
	register int	i, pgs;

	/*	Find a free entry in the swap file table.
	 *	Check to see if the new entry duplicates an
	 *	existing entry.  If so, this is an error unless
	 *	the existing entry is being deleted.  In this
	 *	case, just clear the INDEL flag and the swap
	 *	file will be used again.
	 */


	smi = -1;
	for (i = 0  ;  i < MSFILES  ;  i++) {
		st = &swaptab[i];
		if (st->st_ucnt == NULL) {
			if (smi == -1)
				smi = i;
		} else if (st->st_dev == dev  && st->st_swplo == lowblk) {
			if ((st->st_flags & ST_INDEL)  &&
			   (st->st_npgs == (nblks >> DPPSHFT))) {
				st->st_flags &= ~ST_INDEL;
				availsmem += st->st_npgs;
				return(smi);
			}
			u.u_error = EEXIST;
			return(-1);
		}
	}

	/*	If no free entry is available, give an error
	 *	return.
	 */

	if (smi < 0) {
		u.u_error = ENOSPC;
		return(-1);
	}
	st = &swaptab[smi];

	/*	Open the swap file.
	 */

	u.u_error = 0;
	(*bdevsw[major(dev)].d_open)(dev, FREAD|FWRITE, OTYP_SWP);
	if (u.u_error) {
		st->st_ucnt = NULL;
		st->st_flags = 0;
		return(-1);
	}

	/*	Initialize the new entry.
	 */

	st->st_dev = dev;
	st->st_swplo = lowblk;
	st->st_npgs = nblks >> DPPSHFT;
	st->st_nfpgs = st->st_npgs;


	/*	Allocate space for the use count array.  One counter
	 *	for each page of swap.
	 */

	i = st->st_npgs * sizeof(use_t);  /* Nbr of bytes for use   */
					  /* count.		    */
	pgs = (i + NBPP - 1) / NBPP;	  /* Nbr pages for use array*/
	st->st_ucnt = (use_t *)sptalloc(pgs, PG_VALID, 0);
	if (st->st_ucnt == NULL)
		return(-1);
	bzero(st->st_ucnt, i);		  /* Zero all use counts    */
	st->st_next = st->st_ucnt;
	availsmem += st->st_npgs;

	/*	Clearing the flags allows swalloc to find it
	 */
	st->st_flags = 0;
	while (swapwold) {
		setrun(swapwold);
		swapwold = swapwold->p_mlink;
	}
	return(smi);
}


/*	Delete a swap file.
 */

swapdel(dev, lowblk)
register dev_t	dev;	/* Device to delete.			*/
register int	lowblk;	/* Low block number of area to delete.	*/
{
	register swpt_t	*st;
	register int	smi;
	register int	i;
	register int	ok;

	/*	Find the swap file table entry for the file to
	 *	be deleted.  Also, make sure that we don't
	 *	delete the last swap file.
	 */

	ok = 0;
	smi = -1;
	for (i = 0  ;  i < MSFILES  ;  i++) {
		st = &swaptab[i];
		if (st->st_ucnt == NULL)
			continue;
		if (st->st_dev == dev  && st->st_swplo == lowblk)
			smi = i;
		else if ((st->st_flags & ST_INDEL) == 0)
			ok++;
	};
	
	/*	If the file was not found, then give an error
	 *	return.
	 */

	if (smi < 0) {
		u.u_error = EINVAL;
		return(-1);
	}

	/*	If we are trying to delete the last swap file,
	 *	then give an error return.
	 */
	
	if (!ok) {
		u.u_error = ENOMEM;
		return(-1);
	}
	
	st = &swaptab[smi];

	/*	Set the delete flag.  Clean up its pages.
	 *	The file will be removed by swfree1 when
	 *	all of the pages are freed.
	 */

	if (!(st->st_flags & ST_INDEL)) {
		if (availsmem - st->st_npgs < tune.t_minasmem) {
			cmn_err(CE_NOTE, "swapdel - too few free pages");
			u.u_error = ENOMEM;
			return(-1);
		}
		availsmem -= st->st_npgs;
		st->st_flags |= ST_INDEL;
	}

	while ((st->st_nfpgs < st->st_npgs) && (st->st_flags & ST_INDEL)) {
		if (!getswap(smi))
			delay(100);
	}

	if ((st->st_nfpgs == st->st_npgs) && (st->st_flags & ST_INDEL))
		swaprem(st);

	return(smi);
}


/*	Remove a swap file from swaptab.
 *	Called with swaplock set.
 */

swaprem(st)
swpt_t	*st;
{
	register int	i;
	register dev_t	dev;

	/*	Release the space used by the use count array.
	 */

	dev = st->st_dev;
	i = st->st_npgs * sizeof(use_t);  /* Nbr of bytes for use   */
					  /* count.		    */
	i = (i + NBPP - 1) / NBPP;	  /* Nbr pages for use array*/
	sptfree(st->st_ucnt, i, 1);

	/*	Mark the swaptab entry as unused.
	 */

	st->st_ucnt = NULL;
	(*bdevsw[major(dev)].d_close)(dev, 1, OTYP_SWP);
}

/*	Try to free up swap space on the swap device being deleted.
 *	Look at every region for pages which are swapped to the
 *	device we want to delete.  Read in these pages and delete
 *	the swap.
 */

getswap(smi)
int	smi;
{
	register reg_t	*rp;
	reg_t		*nrp;
	register pde_t	*pt;
	register dbd_t	*dbd;
	register int	pglim;
	register int	i;
	register int	seglim;
	register int	j;
	register int	tmp;
	register int	size;
	register int	unswaped = 0;

	for (rp = ractive.r_forw; rp != &ractive; rp = nrp) {

		/*	If we can't lock the region, then
		 *	skip it for now.
		 */

		if (reglocked(rp)) {
			nrp = rp->r_forw;
			continue;
		}
		reglock(rp);

		/*	Loop through all the segments of the region.
		*/

		size = rp->r_pgoff + rp->r_pgsz;
		seglim = ctos(size);

		for (i = ctost(rp->r_pgoff)  ;  i < seglim  ;  i++) {

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
			dbd = (dbd_t *)pt + NPGPT;

			for (j = 0;  j < pglim;  j++, pt++, dbd++) {
				if (dbd->dbd_type == DBD_SWAP  &&
				   dbd->dbd_swpi ==  smi) {
					unswap(rp, pt, dbd);
					unswaped = 1;
				}
			}
		}
		nrp = rp->r_forw;
		regrele(rp);
	}
	mmuflush();
	return(unswaped);
}

/*	Free up the swap block being used by the indicated page.
 *	The region is locked when we are called.
 */

unswap(rp, pt, dbd)
register reg_t	*rp;
register pde_t	*pt;
register dbd_t	*dbd;
{
	register pfd_t	*pfd;
	pglst_t		pglist;
	pde_t		pde_save;

	ASSERT(reglocked(rp));

	/*	If the page is in use, then just release the copy on swap.
	 *	Otherwise, check to see if there is a copy in memory.
	 */

	if (pg_tstvalid(pt)) {

		pfd = pdetopfdat(pt);

	} else {
		struct	region rrp;
		rrp.r_flags = 0;

	/*	Allocate a page here. We must do the pfind/pinsert w/o sleeping.
	**	We can not allocate physical memory for the page in region rp
	**	(i.e. ptmemall), because ptmemall might free the region
	**	in order to get memory, during that time the process could
	**	exit and the region might be cleaned up and freed.
	**	Allocation of all pages must be done through another region 
	**	while the original rp remains locked.  This also applies while
	**	I/O is occuring to the pages.
	*/

		reglock(&rrp);
		if (ptmemall(&rrp, pt, 1, 0)) 
			cmn_err(CE_PANIC,"unswap - ptmemall");

		if (pfd = pfind(rp, dbd)) {

			/*	A copy is in the hash table.
			 *	We don't need the page allocated above.
			 *	Check to see if the page is P_DONE.
		 	 */

			pde_save = *pt;
			pg_setvalid(&pde_save);
			pfree(&rrp, &pde_save, NULL, 1);

			if (!(pfd->pf_flags & P_DONE))
				return;

			/*	Page is in cache.
 			 *	If it is also on the free list,
			 *	remove it.
			 */

			if (pfd->pf_flags & P_QUEUE) {
				ASSERT(pfd->pf_use == 0);
				ASSERT(freemem > 0);
				freemem--;
				pfd->pf_flags &= ~P_QUEUE;
				pfd->pf_prev->pf_next = pfd->pf_next;
				pfd->pf_next->pf_prev = pfd->pf_prev;
				pfd->pf_next = NULL;
				pfd->pf_prev = NULL;
			}

			rp->r_nvalid++;
			pfd->pf_use++;
			pg_setpfn(pt, pfdattopfn(pfd));

		} else {

			/*	Read in the page from swap.
		 	*/

			rp->r_nvalid++;
			pfd = pdetopfdat(pt);
			pinsert(rp, dbd, pfd);
			pglist.gp_ptptr = pt;
			swap(&pglist, 1, B_READ);

			/*	Mark the I/O done in the pfdat and
			 *	awaken anyone who is waiting for it.
			 */

			pfd->pf_flags |= P_DONE;
			if (pfd->pf_flags & P_WAITING) {
				pfd->pf_flags &= ~P_WAITING;
				wakeup(pfd);
			}
		}
	}

	/* 	Finally attempt to clean up the swap space.
	*/

	if (swfree1(dbd) == 0)
		if (!premove(pfd))
			cmn_err(CE_PANIC,"unswap - premove");

	dbd->dbd_type = DBD_NONE;
	pg_setvalid(pt);
	pg_setref(pt);
	pg_setndref(pt);
	pg_clrmod(pt);
}

/*	Search swap use counters looking for size contiguous free pages.
 *	Returns the page number found + 1 on sucess, 0 on failure.
 */

swapfind(st, size)
register swpt_t	*st;
register int size;
{
	register use_t *p, *e;
	register int i;
	use_t *b;

	e = &st->st_ucnt[st->st_npgs - size];
	for (p = st->st_next; p <= e; p++) {
		if (*p == 0) {
			b = p;
			p++;
			for (i = 1; i < size; i++, p++)
				if (*p != 0) goto Cont;
			st->st_next = p;
			return(b - st->st_ucnt);
		}
	  Cont:;
	}
	e = st->st_next - size;
	for (p = st->st_ucnt; p <= e; p++) {
		if (*p == 0) {
			b = p;
			p++;
			for (i = 1; i < size; i++, p++)
				if (*p != 0) goto Cont2;
			st->st_next = p;
			return(b - st->st_ucnt);
		}
	  Cont2:;
	}

	st->st_next = st->st_ucnt;
	return(-1);
}

/*
 *	manipulate swap files.
 */

swapfunc(si)
register swpi_t *si;
{
	register int		i;
	register inode_t	*ip;

	switch (si->si_cmd) {
		case SI_LIST:
			i = sizeof(swpt_t) * MSFILES;
			if (copyout(swaptab, si->si_buf, i) < 0)
				u.u_error = EFAULT;
			break;

		case SI_ADD:
		case SI_DEL:
			if (!suser())
				break;
			u.u_dirp = si->si_buf;
			if ((ip = namei(upath, 0)) == NULL)
				break;
			if (ip->i_ftype != IFBLK) {
				u.u_error = ENOTBLK;
				break;
			}

			if (si->si_cmd == SI_DEL)
				swapdel(ip->i_rdev, si->si_swplo);
			else
				swapadd(ip->i_rdev, si->si_swplo,
					si->si_nblks);
			iput(ip);
			break;
		default:
			u.u_error = EINVAL;
	}
}
