/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/page.c	10.8"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/mount.h"
#include "sys/map.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/tuneable.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"

/*	The following are used by the S68DELMEM and S68ADDMEM functions.
*/

pfd_t	Delmem;		/* Linked list of deleted pages.	*/
int	Delmem_cnt;	/* Count of number of deleted pages.	*/

int			firstfree, maxfree, freemem;
int			availrmem;
int			availsmem;
extern unsigned int	sxbrkcnt;

/*
 *	getcpages(npgs,nosleep) - gets physically continguous pages, 
 *		the kernel virtual address of the pages allocated.
 *		is returned.
 *	npgs = number of pages to be allocated.
 *	wait for pages only if nosleep = 0.
 *	will attempt to swap processes and retry 5 times if allowed to sleep
 *
 *	Remark: pages allocated are not mapped to sysreg
 */

caddr_t
getcpages(npgs, nosleep)
{
	register int	pfn;
	register int	i;

	for (i=0; ; ) {
		pfn = contmemall(npgs);
		if (pfn != NULL)
			return((caddr_t)phystokv(ctob(pfn)));

		if (nosleep || ++i >= 5)
			return(NULL);

		cmn_err(CE_NOTE, "!getcpages - waiting for %d contiguous pages",
			npgs);

		u.u_procp->p_stat = SXBRK;
		++sxbrkcnt;

		if (runout) {
			runout = 0;
			wakeup(&runout);
		}
		swtch();
	}
}

/*
 * freecpages(paddr, npgs)
 */

freecpages(paddr, npgs)
{
	pde_t base;
	int   pfn;

	availrmem += npgs;
	availsmem += npgs;

	pfn = btoct(paddr);
	pg_setpde(&base,0);

	while (npgs--) {
		pg_setpfn(&base, pfn++);
		pg_setvalid(&base);
		pfree(NULL,&base,NULL,1);
	}
}

/*
 * contmemall(npgs)
 *
 * allocate phycally contiguous pages
 */

contmemall(npgs)
{
	register struct pfdat	*pfd, *pfd1,*top;
	register int numpages;

	/*
	 * Need contiguous memory
	 */

	if (freemem < npgs)
		return(NULL);

	if (availsmem - npgs < tune.t_minasmem) {
		nomemmsg("contmemall", npgs, 0, 0);
		return(NULL);
	}

	if (availrmem - npgs < tune.t_minarmem) {
		nomemmsg("contmemall", npgs, 0, 1);
		return(NULL);
	}

	pfd = pfntopfdat(kpbase);
	top = pfntopfdat(maxclick);

	for (; pfd < top; pfd++) {
		if (pfd->pf_flags & P_QUEUE) {
			for (pfd1 = pfd + 1, numpages = npgs ;
			    pfd1 <= top && --numpages > 0 ;
			    pfd1++)
				if (!(pfd1->pf_flags & P_QUEUE))
					break;
			if (numpages == 0)
				break;
			pfd = pfd1;
		}
	}

	if (pfd >= top)
		return(NULL);

	ASSERT(pfd1 <= top);
	ASSERT(pfd1 - pfd == npgs);

	/*
	 * Take pages *pfd .. *(--pfd1)
	 */

	for (; --pfd1 >= pfd;) {
		if (pfd1->pf_flags & P_HASH)
			premove(pfd1);
		(pfd1->pf_prev)->pf_next = pfd1->pf_next;
		(pfd1->pf_next)->pf_prev = pfd1->pf_prev;
		pfd1->pf_next = NULL;
		pfd1->pf_prev = NULL;
		pfd1->pf_blkno = BLKNULL;
		pfd1->pf_use = 1;
		pfd1->pf_flags = 0;
		pfd1->pf_rawcnt = 0;
	}
	freemem -= npgs;
	availrmem -= npgs;
	availsmem -= npgs;
	return(pfdattopfn(pfd));
}

/*
 * Allocate pages for system and fill in page table
 *	base		-> address of page table
 *	size		-> # of pages needed
 *	validate	-> Mark pages valid if set.
 *	nosleep		-> wait for pages only if nosleep is 0.
 * returns:
 *	 0	Memory allocated.
 *	-1	Memory below system minimums.
 */

sptmemall(base, size, validate)
register pde_t	*base;
register int	validate;
{
	int retval;

	reglock(&sysreg);

	if (availsmem - size < tune.t_minasmem) {
		regrele(&sysreg);
		nomemmsg("sptmemall", size, 0, 0);
		return(-1);
	}

	if (availrmem - size < tune.t_minarmem) {
		regrele(&sysreg);
		nomemmsg("sptmemall", size, 0, 1);
		return(-1);
	}

	availrmem -= size;
	availsmem -= size;

	retval = ptmemall(&sysreg, base, size, validate);
	ASSERT(retval == 0);

	regrele(&sysreg);
	return(retval);
}

/*
 * Allocate pages and fill in page table
 *	rp		-> region pages are being added to.
 *	base		-> address of page table
 *	size		-> # of pages needed
 *	validate	-> Mark pages valid if set.
 * returns:
 *	0	Memory allocated, may have slept for it.
 *	1	Had to unlock region and go to sleep before
 *		memory was obtained.  After awakening, the
 *		page was valid or pfree'd so no page was
 *		allocated.
 */

ptmemall(rp, base, size, validate)
reg_t		*rp;
register pde_t	*base;
register int	validate;
{
	register pfd_t	*pfd;
	register pfd_t	*pfd_next;
	register int	i;

	/*	Check for illegal size.
	 */

	ASSERT(size > 0);
	ASSERT(reglocked(rp));

	if (memreserve(rp, size)) {
		if (pg_tstvalid(base)) {
			 freemem += size;
			 return(1);
		}
	}

	/*
	 * Take pages from head of queue
	 */

	pfd = phead.pf_next;
	i = 0;
	while (i < size) {
		ASSERT(pfd != &phead);
		ASSERT(pfd->pf_flags&P_QUEUE);
		ASSERT(pfd->pf_use == 0);
		ASSERT(!pg_tstvalid(base));

		/* Delink page from free queue and set up pfd
		 */

		if (pfd->pf_flags & P_HASH)
			premove(pfd);
		pfd_next = pfd->pf_next;
		pfd->pf_prev->pf_next = pfd_next;
		pfd_next->pf_prev = pfd->pf_prev;
		pfd->pf_next = NULL;
		pfd->pf_prev = NULL;
		pfd->pf_blkno = BLKNULL;
		pfd->pf_use = 1;
		pfd->pf_flags = 0;
		pfd->pf_rawcnt = 0;
		rp->r_nvalid++;

		/*
		 * Insert in page table
		 */

		pg_setpfn(base, pfdattopfn(pfd));
		pg_clrmod(base);

		if (validate)
			pg_setvalid(base);
		
		i++;
		base++;
		pfd = pfd_next;
	}
	return(0);
}

/*
 * Shred system page table and update accounting for swapped
 * and resident pages.
 *	pt	-> ptr to the first pte to free.
 *	size	-> nbr of pages to free.
 */

spfree(pt, size)
register pde_t	*pt;
int		size;
{
	pfree(NULL, pt, NULL, size);
	availrmem += size;
	availsmem += size;
	mmuflush();
}

/*
 * Shred page table and update accounting for swapped
 * and resident pages.
 *	rp	-> ptr to the region structure.
 *	pt	-> ptr to the first pte to free.
 *	dbd	-> ptr to disk block descriptor.
 *	size	-> nbr of pages to free.
 */

pfree(rp, pt, dbd, size)
reg_t		*rp;
register pde_t	*pt;
register dbd_t	*dbd;
int		size;
{
	register struct pfdat	*pfd;
	register int		k;

	/* 
	 * Zap page table entries
	 */

	for (k = 0; k < size; k++, pt++) {

		if (rp && dbd  &&  dbd->dbd_type == DBD_SWAP)
			if (swfree1(dbd) == 0)
				pbremove(rp, dbd);

		if (pg_tstvalid(pt)) {
			pfd = pdetopfdat(pt);
			ASSERT(pfd->pf_use > 0);
			ASSERT((pfd->pf_flags & P_QUEUE) == 0);
			ASSERT(pfd->pf_next == NULL);
			ASSERT(pfd->pf_prev == NULL);
			/* Free pages that aren't being used
			 * by anyone else
			 */
			if (--pfd->pf_use == 0) {

				/* Pages that are associated with disk
				 * go to end of queue in hopes that they
				 * will be reused.  All others go to
				 * head of queue so they will be reused
				 * quickly.  (I think this version is
				 * much better.    K. Rusnock)
				 */

				if (pfd->pf_flags & P_HASH) {
					/*
					 * put at tail 
					 */
					ASSERT(pg_tstcw(pt));
					ASSERT(!pg_tstmod(pt));
					pfd->pf_prev = phead.pf_prev;
					pfd->pf_next = &phead;
					phead.pf_prev = pfd;
					pfd->pf_prev->pf_next = pfd;
				} else {
					/*
					 * put at head 
					 */
					pfd->pf_next = phead.pf_next;
					pfd->pf_prev = &phead;
					phead.pf_next = pfd;
					pfd->pf_next->pf_prev = pfd;
				}
				pfd->pf_flags |= P_QUEUE;
				freemem++;
			}

			if (rp != NULL)
				rp->r_nvalid--;
		}

		/*
		 * Change to zero pte's.
		 */

		pg_setpde(pt, 0);
		if (dbd)
			dbd++->dbd_type = DBD_NONE;
	}
}

/*
 * Device number
 *	ip	-> inode pointer
 * returns:
 *	dev	-> device number
 */

effdev(ip)
register struct inode *ip;
{
	register int type;

	type = ip->i_ftype;
	if (type == IFREG || type == IFDIR) 
		return(ip->i_dev);
	return(ip->i_rdev);
}

/*
 * Find page by looking on hash chain
 *	dbd	-> Ptr to disk block descriptor being sought.
 * returns:
 *	0	-> can't find it
 *	pfd	-> ptr to pfdat entry
 */

struct pfdat *
pfind(rp, dbd)
register reg_t	*rp;
register dbd_t	*dbd;
{
	register dev_t		dev;
	register daddr_t	blkno;
	register pfd_t		*pfd;
	register pfd_t		**phashslot;
	register inode_t	*ip;
	register long		inumber;

	ASSERT(reglocked(rp));

	/*	Hash on block and look for match.
	 */

	if (dbd->dbd_type == DBD_SWAP) {
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
		inumber = 0;
	} else {
		ip = rp->r_iptr;
		ASSERT(ip != NULL);
		dev = effdev(ip);
		inumber = ip->i_number;
		blkno = dbd->dbd_blkno;
	}

	phashslot = &phash[(blkno >> DPPSHFT) & phashmask];
	pfd = *phashslot;
	for (; pfd != (pfd_t*)phashslot; pfd = pfd->pf_hchain) {
		if ((pfd->pf_blkno == blkno) && (pfd->pf_dev == dev)
			&& (pfd->pf_inumber == inumber)) {
			if (pfd->pf_flags & P_BAD)
				continue;
			return(pfd);
		}
	}
	return(NULL);
}

/*
 * Insert page on hash chain
 *	dbd	-> ptr to disk block descriptor.
 *	pfd	-> ptr to pfdat entry.
 * returns:
 *	none
 */

pinsert(rp, dbd, pfd)
register reg_t	*rp;
register dbd_t	*dbd;
register pfd_t	*pfd;
{
	register dev_t	dev;
	register int	blkno;
	register long	inumber;
	register struct pfdat *pfd1, *p, **phashslot;
	register inode_t	*ip;

	ASSERT(reglocked(rp));

	/* Check page range, see if already on chain
	 */

	if (dbd->dbd_type == DBD_SWAP) {
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
		inumber = 0;
	} else {
		ip = rp->r_iptr;
		ASSERT(ip != NULL);
		inumber = ip->i_number;
		dev = effdev(ip);
		blkno = dbd->dbd_blkno;
	}

	ASSERT(pfd->pf_hchain == NULL);
	ASSERT(pfd->pf_use > 0);

	/*
	 * insert newcomers at tail of bucket
	 */

	phashslot = &phash[(blkno >> DPPSHFT) & phashmask];
	pfd1 = p = *phashslot; 
	for (; pfd1 != (pfd_t*)phashslot; p=pfd1, pfd1=pfd1->pf_hchain) {
		if ((pfd1->pf_blkno == blkno) && (pfd1->pf_dev == dev) &&
		   (pfd1->pf_inumber == inumber)) {
#ifdef	DEBUG
			cmn_err(CE_CONT, "swapdev %x %x %x\n", swapdev,
				pfd1->pf_dev,dev);
			cmn_err(CE_CONT, "blkno %x %x\n",blkno,pfd1->pf_blkno);
			cmn_err(CE_CONT, "swpi %x %x\n", pfd1->pf_swpi,
				pfd->pf_swpi);
			cmn_err(CE_CONT, "pfd %x %x\n", pfd,pfd1);
			cmn_err(CE_CONT, "use %x %x\n",
				pfd->pf_use, pfd1->pf_use);
			cmn_err(CE_CONT, "flags %x %x\n", pfd->pf_flags,
				pfd1->pf_flags);
#endif
			cmn_err(CE_PANIC, "pinsert - pinsert dup");
		}
	}
	if (p == pfd1)
		*phashslot = pfd;
	else
		p->pf_hchain = pfd;
	pfd->pf_hchain = pfd1;

	/*	Set up the pfdat.  Note that only swap pages are
	 *	put on the hash list for now.
	 */

	pfd->pf_dev = dev;
	pfd->pf_inumber = inumber;
	if (dbd->dbd_type == DBD_SWAP) {
		pfd->pf_swpi = dbd->dbd_swpi;
		pfd->pf_flags |= P_SWAP;
	} else {
		pfd->pf_flags &= ~P_SWAP;
	}
	pfd->pf_blkno = blkno;
	pfd->pf_flags |= P_HASH;
}


/*
 * remove page from hash chain
 *	pfd	-> page frame pointer
 * returns:
 *	0	Entry not found.
 *	1	Entry found and removed.
 */
premove(pfd)
register struct pfdat *pfd;
{
	register struct pfdat *pfd1, *p, **phashslot;
	int	rval;
	int	hindex;

	rval = 0;

	phashslot = &phash[(pfd->pf_blkno >> DPPSHFT) & phashmask];
	pfd1 = p = *phashslot;
	for (; pfd1 != (pfd_t*)phashslot; p=pfd1, pfd1 = pfd1->pf_hchain) {
		if (pfd1 == pfd) {
			if (p == pfd1)
				*phashslot = pfd->pf_hchain;
			else
				p->pf_hchain = pfd->pf_hchain;
			if ((pfd->pf_flags & P_QUEUE)&&(phead.pf_next != pfd)) {
				(pfd->pf_prev)->pf_next = pfd->pf_next;
				(pfd->pf_next)->pf_prev = pfd->pf_prev;
				/*
				 * put at head 
				 */
				pfd->pf_next = phead.pf_next;
				pfd->pf_prev = &phead;
				phead.pf_next = pfd;
				pfd->pf_next->pf_prev = pfd;
			}
			rval = 1;
			break;
		}
	}
	/*
	 * Disassociate page from disk and
	 * remove from hash table
	 */
	pfd->pf_blkno = BLKNULL;
	pfd->pf_hchain = NULL;
	pfd->pf_flags &= ~(P_HASH | P_SWAP);
	pfd->pf_dev = 0;
	pfd->pf_swpi = 0;
	pfd->pf_inumber = 0;
	return(rval);
}

/*
 * Allocate system virtual address space and
 * allocate or link  pages.
 */
sptalloc(size, mode, base)
register int size, mode, base;
{
	register i, sp;

	/*
	 * Allocate system virtual address space
	 */
	if ((sp = malloc(sptmap, size))  ==  0) {

#ifdef DEBUG
		cmn_err(CE_WARN, "No kernel virtual space.");
		cmn_err(CE_CONT, "\tsize=%d, mode=%d, base=%d\n",
			size, mode, base);
#endif
		return(NULL);
	}
	/*
	 * Allocate and fill in pages
	 */
	if (base  ==  0)
		if (sptmemall(kvtokptbl(ctob(sp)), size, 0) < 0) {
			mfree(sptmap,size,sp);
			return(NULL);
		}

	/*
	 * Setup page table entries
	 */
	for (i = 0; i < size; i++) {
		if (base > 0)
			pg_setpfn(kvtokptbl(ctob(sp + i)), base++);
		if (mode)
			pg_setvalid(kvtokptbl(ctob(sp + i)));
	}
	return(ctob(sp));
}

sptfree(vaddr, size, flag)
register int size;
{
	register i, sp;

	sp = btoc(vaddr);
	if (flag) {
		spfree(kvtokptbl(vaddr), size);
	} else {
		for (i = 0; i < size; i++)
			pg_setpde(kvtokptbl(ctob(sp + i)), 0);
		mmuflush();
	}
	mfree(sptmap, size, sp);
}

/*
 * Initialize memory map
 *	first	-> first free page #
 *	last	-> last free page #
 * returns:
 *	none
 */

meminit(first, last)
register int first;
{
	register struct pfdat *pfd;
	register int i;

	freemem = (last - first);
	maxmem = freemem;
	availrmem = freemem;
	availsmem = freemem;

	/*
	 * Setup queue of pfdat structures.
	 * One for each page of available memory.
	 */

	pfd = pfntopfdat(first);
	phead.pf_next = &phead;
	phead.pf_prev = &phead;

	/*
	 * Add pages to queue, high memory at end of queue
	 * Pages added to queue FIFO
	 */

	for (i = freemem; --i >= 0; pfd++) {
		pfd->pf_next = &phead;
		pfd->pf_prev = phead.pf_prev;
		phead.pf_prev->pf_next = pfd;
		phead.pf_prev = pfd;
		pfd->pf_flags = P_QUEUE;
	}
}


/*
 * flush all pages associated with a mount device
 *	mp	-> mount table entry
 * returns:
 *	none
 */

punmount(mp)
register struct mount *mp;
{
	register pfd_t		*pfd;
	register pfd_t		*pfdlim;

	pfd    = pfntopfdat(kpbase);
	pfdlim = pfntopfdat(maxclick);
	for (;  pfd < pfdlim  ;  pfd++) {
		if (mp->m_dev == pfd->pf_dev)
			if ((pfd->pf_flags & (P_HASH | P_SWAP)) == P_HASH)
				premove(pfd);
	}
}

/*
 * Find page by looking on hash chain and remove it.
 *	dbd	Ptr to disk block descriptor for block to remove.
 * returns:
 *	0	-> can't find it.
 *	1	-> page found and removed.
 */

pbremove(rp, dbd)
reg_t	*rp;
dbd_t	*dbd;
{
	register struct pfdat	*pfd;
	register struct pfdat	*p;
	register struct pfdat	**phashslot;
	register inode_t	*ip;
	register int		blkno;
	register dev_t		dev;
	register long		inumber;

	/*
	 * Hash on block and look for match
	 */

	if (dbd->dbd_type == DBD_SWAP) {
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
		inumber = 0;
	} else {
		ip = rp->r_iptr;
		ASSERT(ip != NULL);
		dev = effdev(ip);
		inumber = ip->i_number;
		blkno = dbd->dbd_blkno;
	}

	phashslot = &phash[(blkno >> DPPSHFT) & phashmask];
	pfd = p = *phashslot;
	for (; pfd != (pfd_t*)phashslot; p = pfd, pfd = pfd->pf_hchain) {
		if ((pfd->pf_blkno == blkno) && (pfd->pf_dev == dev)
			&& (pfd->pf_inumber == inumber)) {
			if (p != pfd)
				p->pf_hchain = pfd->pf_hchain;
			else
				*phashslot = pfd->pf_hchain;
			if ((pfd->pf_flags & P_QUEUE)&&(phead.pf_next != pfd)) {
				(pfd->pf_prev)->pf_next = pfd->pf_next;
				(pfd->pf_next)->pf_prev = pfd->pf_prev;
				/*
				 * put at head 
				 */
				pfd->pf_next = phead.pf_next;
				pfd->pf_prev = &phead;
				phead.pf_next = pfd;
				pfd->pf_next->pf_prev = pfd;
			}
			pfd->pf_blkno = BLKNULL;
			pfd->pf_hchain = NULL;
			pfd->pf_flags &= ~(P_HASH | P_SWAP);
			pfd->pf_dev = 0;
			pfd->pf_swpi = 0;
			pfd->pf_inumber = 0;
			return(1);
		}
	}
	return(0);
}



/*
 * Reserve size memory pages.  Returns with freemem
 * decremented by size.  Return values:
 *	0 - Memory available immediately
 *	1 - Had to sleep to get memory
 */

memreserve(rp, size)
register reg_t *rp;
{
	register struct proc *p;

	ASSERT(reglocked(rp));

	if (freemem >= size) {
		freemem -= size;
		return(0);
	}
	p = u.u_procp;
	while (freemem < size) {
		regrele(rp);
		p->p_stat = SXBRK;
		++sxbrkcnt;
		if (runout) {
			runout = 0;
			wakeup((caddr_t)&runout);
		}
		swtch();
		reglock(rp);
	}
	freemem -= size;
	return(1);
}

flushpgch(ip)
register struct inode *ip;
{
	/*
	 * Flush the page cache of pages associated with the file
	 * from which this region was initialized. We do it here, rather
	 * than iput or some other file system  routine, because file
	 * system writers shouldn't have to bother with it.
	 */
	register int nblks, blkspp, i;
	register int bsize;
	struct region	reg;
	struct dbd	dbd;

	dbd.dbd_type = DBD_FILE;
	reg.r_iptr = ip;
	bsize = min(NBPP, FSBSIZE(ip));
	nblks = (ip->i_size + bsize - 1)/bsize;
	blkspp = NBPP/bsize;
	nblks = ((nblks + blkspp - 1) / blkspp) * blkspp;
	for (i = 0  ;  i < nblks  ;  i += blkspp) {
		dbd.dbd_blkno = i;
		pbremove(&reg, &dbd);
	}
}

delmem(size)
{
	/*	Delete memory from the available list.  Forces
	**	tight memory situation for load testing.
	*/

	register pfd_t	*pfd;

	/*	Check for first time here.  If so,
	**	initialize deleted memory list to
	**	empty.
	*/

	if (Delmem.pf_next == NULL) {
		Delmem.pf_next = &Delmem;
		Delmem.pf_prev = &Delmem;
	}

	if (freemem < size  ||
	   availrmem - size < tune.t_minarmem  ||
	   availsmem - size < tune.t_minasmem) {
		u.u_error = EINVAL;
		return;
	}

	freemem -= size;
	availrmem -= size;
	availsmem -= size;
	Delmem_cnt += size;

	while (size--) {
		pfd = phead.pf_next;
		ASSERT(pfd != &phead);
		ASSERT(pfd->pf_flags & P_QUEUE);
		ASSERT(pfd->pf_use == 0);
		if (pfd->pf_flags & P_HASH)
			premove(pfd);
		pfd->pf_next->pf_prev = &phead;
		phead.pf_next = pfd->pf_next;
		pfd->pf_next = Delmem.pf_next;
		pfd->pf_prev = &Delmem;
		pfd->pf_next->pf_prev = pfd;
		Delmem.pf_next = pfd;
		pfd->pf_flags &= ~P_QUEUE;
	}
}

addmem(size)
{
	/*	Add back previously deleted memory.
	*/

	register pfd_t	*pfd;

	if (size > Delmem_cnt) {
		u.u_error = EINVAL;
		return;
	}

	Delmem_cnt -= size;
	freemem += size;
	availrmem += size;
	availsmem += size;

	while (size--) {
		pfd = Delmem.pf_next;
		Delmem.pf_next = pfd->pf_next;
		pfd->pf_next->pf_prev = &Delmem;
		pfd->pf_next = phead.pf_next;
		pfd->pf_prev = &phead;
		phead.pf_next = pfd;
		pfd->pf_next->pf_prev = pfd;
		pfd->pf_flags |= P_QUEUE;
	}
}
