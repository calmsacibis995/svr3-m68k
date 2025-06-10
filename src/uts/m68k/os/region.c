/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/region.c	10.16"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/pfdat.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/sysinfo.h"
#include "sys/ipc.h"
#include "sys/fstyp.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/region.h"
#include "sys/mount.h"
#include "sys/proc.h"
#include "sys/shm.h"
#include "sys/cmn_err.h"
#include "sys/tuneable.h"
#include "sys/message.h"
#include "sys/conf.h"

preg_t	nullpregion;

/*	The following defines the minimum number of
**	regions for a process.  This includes one
**	region each for the text, data, and stack
**	plus one null region pointer to indicate
**	the end of the pregion list.
*/

void
reginit()
{
	register reg_t		*rp;
	extern struct shminfo	shminfo;

	pregpp = MINPREGPP + shmseg() + 2*shlbinfo.shlbs;

	rfree.r_forw = &rfree;
	rfree.r_back = &rfree;

	ractive.r_forw = &ractive;
	ractive.r_back = &ractive;

	for (rp = region; rp < &region[v.v_region]; rp++) {
		rp->r_back = rfree.r_back;
		rp->r_forw = &rfree;
		rfree.r_back->r_forw = rp;
		rfree.r_back = rp;
	}
}

reglock(rp)
register reg_t *rp;
{
	while (rp->r_flags & RG_LOCK) {
		rp->r_flags |= RG_WANTED;
		sleep((caddr_t)rp, PZERO);
	}
	rp->r_flags |= RG_LOCK;
}

regrele(rp)
register reg_t *rp;
{
	ASSERT(rp->r_flags & RG_LOCK);
	if (rp->r_flags & RG_WANTED)
		wakeup((caddr_t)rp);
	rp->r_flags &= ~(RG_LOCK | RG_WANTED);
}

reglocked(rp)
register reg_t *rp;
{
	return(rp->r_flags & RG_LOCK);
}

regwait(rp)
register reg_t	*rp;
{
	ASSERT(rp->r_refcnt == 1);
	if (!(rp->r_flags & RG_DONE)) {
		rp->r_flags |= RG_WAITING;
		sleep((caddr_t)&rp->r_flags, PZERO);
	}
}

regunwait(rp)
register reg_t	*rp;
{
	ASSERT(!(rp->r_flags & RG_DONE));
	ASSERT(rp->r_refcnt == 1);
	if (rp->r_flags & RG_WAITING) {
		rp->r_flags &= ~RG_WAITING;
		wakeup((caddr_t)&rp->r_flags);
	}
}

regdone(rp)
register reg_t *rp;
{
	return(rp->r_flags & RG_DONE);
}

/*
 * Allocate a new region.
 * Returns a locked region pointer or NULL on failure
 * The region is linked into the active list.
 */

reg_t *
allocreg(ip, type, flag)
register struct inode	*ip;
short type;
short flag;
{
	register reg_t *rp;

	if ((rp = rfree.r_forw) == &rfree) {
		cmn_err(CE_WARN, "Region table overflow");
		u.u_error = EAGAIN;
		return(NULL);
	}
	/*
	 * Remove from free list
	 */
	rp->r_back->r_forw = rp->r_forw;
	rp->r_forw->r_back = rp->r_back;

	/* Initialize region fields and bump inode reference
	 * count.  Inode is locked by the caller
	 */

	rp->r_type = type;
	rp->r_iptr = ip;
	rp->r_flags |= flag;
	reglock(rp);

	if (ip != NULL) {
		ASSERT(ip->i_flag & ILOCK);
		ip->i_count++;
	}

	/*
	 * Link onto active list
	 */
	rp->r_forw = ractive.r_forw;
	rp->r_back = &ractive;
	ractive.r_forw->r_back = rp;
	ractive.r_forw = rp;

	return(rp);
}

/*
 * Free an unused region table entry.
 */
void
freereg(rp)
register reg_t *rp;	/* pointer to a locked region */
{
	register inode_t	*ip;
	register int		i;
	register int		lim;
	register int		size;
	register pde_t		*pt;
	register int		tsize;
	int			tmp;

	ASSERT(reglocked(rp));
	ASSERT(rp->r_refcnt == 0);

	/*	Free the memory pages and the page tables and
	 *	disk use maps.  These latter are always allocated
	 *	together in pairs in a contiguous 256 (long) word array
	 *	of kernel virtual address space.  Note that the
	 *	pfree for the first page table is special because
	 *	pages may not have been allocated from the beginning
	 *	of the segment.  The last page table is also special
	 *	since it may not have been fully allocated.
	 */
	
	tsize = rp->r_pgoff + rp->r_pgsz;
	i = ctost(rp->r_pgoff);
	lim = ctos(tsize);

	for ( ;  i < lim;  i++) {
		pt = rp->r_list[i];
		ASSERT(pt != 0);
		size = tsize - stoc(i);
		if (size > NPGPT)
			size = NPGPT;
		if (rp->r_pgoff > stoc(i)) {
			tmp = rp->r_pgoff - stoc(i);
			size -= tmp;
			pt += tmp;
		}
		pfree(rp, pt, pt + NPGPT, size);
		sptfree(rp->r_list[i], 2, 1);
	}

	if (rp->r_flags & RG_SSWAPMEM)
		availsmem -= btoc(rp->r_filesz);
	availsmem += rp->r_pgsz;

	ASSERT(rp->r_noswapcnt >= 0);
	if (rp->r_noswapcnt)
		availrmem += rp->r_pgsz;

	/*
	 * Free the list.
	 */

	if (rp->r_listsz)
		sptfree(rp->r_list, rp->r_listsz, 1);

	/*
	 * Remove from active list.
	 */

	rp->r_back->r_forw = rp->r_forw;
	rp->r_forw->r_back = rp->r_back;

	/*
	 * Decrement use count on associated inode
	 * Inode is locked by the caller.
	 */

	if (ip = rp->r_iptr) {
		ASSERT(ip->i_flag & ILOCK);
		iput(ip);
	}

	regrele(rp);

	/*
	 * Clean up region fields.
	 */

	rp->r_flags = 0;
	rp->r_listsz = 0;
	rp->r_pgsz = 0;
	rp->r_pgoff = 0;
	rp->r_nvalid = 0;
	rp->r_type = 0;
	rp->r_filesz = 0;
	rp->r_iptr = NULL;
	rp->r_list = NULL;
	rp->r_noswapcnt = 0;
	rp->r_timestamp = 0;

	/*
	 * Link into free list
	 */

	rp->r_forw = rfree.r_forw;
	rp->r_back = &rfree;
	rfree.r_forw->r_back = rp;
	rfree.r_forw = rp;
}

/*
 * Attach a region to a process' address space
 */
preg_t *
attachreg(rp, up, vaddr, type, prot)
register reg_t *rp;	/* pointer to region to be attached */
struct user *up;	/* pointer to u-block (needed by fork) */
caddr_t	vaddr;		/* virtual address to attach at */
int	type;		/* Type to make the pregion. */
int	prot;		/* permissions for segment table entries. */
{
	register preg_t *prp1, *first;
	register preg_t *prp;
	preg_t  tmp;

	ASSERT(reglocked(rp));

	/*	Check attach address.
	 *	It must be segment aligned.
	 */

	 if ((int)vaddr & SOFFMASK) {
		u.u_error = EINVAL;
		return(NULL);
	 }

	/*	Allocate a pregion.  We should always find a
	 *	free pregion because of the way the system
	 *	is configured.
	 */

	prp = findpreg(up->u_procp, PT_UNUSED);
	if (prp == NULL) {
		u.u_error = EMFILE;
		return(NULL);
	}

	/*
	 * Init a temporary pregion incase chkattach fails
	 */

	prp1 = &tmp;
	prp1->p_reg = rp;
	prp1->p_regva = vaddr;
	prp1->p_type = type;
	prp1->p_flags = 0;
	if (prot == (SEG_RO))
		prp1->p_flags |= PF_RDONLY;

	/*	Check that region does not go beyond end of virtual
	 *	address space.
	 */

	 if (chkattach(up, prp1)) {
		u.u_error = EINVAL;
		return(NULL);
	}

	/*
	 * Load the segment (pointer) table.
	 */

	if (rp->r_pgsz > 0)
		loadstbl(up, prp1, rp->r_pgsz);

	/*
	 * Insert tmp in the pregion list
	 */

	first = (up->u_procp)->p_region;
	for (prp1=prp; prp1!=first; --prp1)
		if ((prp1-1)->p_regva > vaddr)
			*prp1 = *(prp1-1);
		else break;
	*prp1 = tmp;

	++rp->r_refcnt;
	up->u_procp->p_size += rp->r_pgsz;

	return(prp1);
}

/*
 *	Detach a region from a process' address space.
 */

void
detachreg(prp, up)
register preg_t *prp;
register user_t	*up;
{
	register reg_t	*rp;
	register int	i;
	inode_t		*ip;
	int		change;

	rp = prp->p_reg;

	ASSERT(rp);
	ASSERT(reglocked(rp));

	/*
	 * If shared 407 region with sticky text, remove data portion.
	 * Size of text is found in r_filesz of region.
	 */

	if ((u.u_magic == 0407) && (rp->r_type == RT_STEXT)) {
		rp->r_filesz -= (int)prp->p_regva;
		if (rp->r_flags & RG_NOFREE) {
			change = btoc(rp->r_filesz) - rp->r_pgsz;
			ASSERT(change <= 0);
			if (growreg(prp, change, DBD_DZERO) < 0)
				cmn_err(CE_WARN, "detach: growreg failed\n");
		}
	}

	/*
	 * Invalidate segment (pointer) table entries pointing at the region
	 */

	if (rp->r_pgsz > 0)
		loadstbl(up, prp, 0);

	/*	Clear the proc region we just detached.
	 */
	
	for (i = prp - up->u_procp->p_region; i < pregpp-1; i++, prp++) {
		*prp = *(prp+1);
	}
	*prp = nullpregion;

	/*
	 *	Decrement process size by size of region.
	 */

	up->u_procp->p_size -= rp->r_pgsz;

	/*
	 * Decrement use count and free region if zero
	 * and RG_NOFREE is not set, otherwise unlock.
	 */
	if (--rp->r_refcnt == 0 && !(rp->r_flags & RG_NOFREE)) {
		freereg(rp);
	} else {
		if (ip = rp->r_iptr)
			prele(ip);
		regrele(rp);
	}
}

/*
 * Duplicate a region
 */

reg_t *
dupreg(rp, force, rsize)
register reg_t *rp;
int force;
register int rsize;
{
	register int	i;
	register int	j;
	register int	size;
	register pde_t	*ppte;
	register pde_t	*cpte;
	register reg_t	*rp2;
	register struct inode *ip;
	register int	tmp;

	ASSERT(reglocked(rp));
	ASSERT(rsize <= rp->r_pgsz);

	/* If region is shared, and we're not forcing a duplicate,
	 * there is no work to do.
	 * Just return the passed region.  The region reference
	 * counts are incremented by attachreg
	 */

	if (rp->r_type != RT_PRIVATE && force == 0)
		return(rp);
	
	/*	Make sure we have enough space to duplicate
	 *	this region without potential deadlock.
	 */

	ip = rp->r_iptr;
	if (availsmem - rsize < tune.t_minasmem) {
		nomemmsg("dupreg", rsize, 0, 0);
		u.u_error = EAGAIN;
		if (ip)
			prele(ip);
		return(NULL);
	}

	availsmem -= rsize;

	/*
	 * Need to copy the region.
	 * Allocate a region descriptor
	 */
	if ((rp2 = allocreg(ip, force ? RT_PRIVATE : rp->r_type,
		0)) == NULL) {
		availsmem += rsize;
		u.u_error = EAGAIN;
		if (ip)
			prele(ip);
		return(NULL);
	}

	/*	Allocate a list for the new region.
	 */

	rp2->r_listsz = rp->r_listsz;
	rp2->r_list = (pde_t **)sptalloc(rp2->r_listsz, PG_VALID, 0);
	rp2->r_flags = rp->r_flags;
	if (rp2->r_list == NULL) {
		rp2->r_listsz = 0;
		freereg(rp2);
		availsmem += rsize;
		u.u_error = EAGAIN;
		return(NULL);
	}

	/*
	 * Copy pertinent data to new region
	 */

	bzero(rp2->r_list, ctob(rp2->r_listsz));
	rp2->r_pgsz = rsize;
	rp2->r_pgoff = rp->r_pgoff;
	rp2->r_filesz = rp->r_filesz;
	rp2->r_nvalid = rp->r_nvalid;

	/* Scan the parents page table  list and fix up each page table.
	 * Allocate a page table and map table for the child and
	 * copy it from the parent.
	 */

	for (i = 0  ;  i < ctos(rsize + rp->r_pgoff)  ;  i++) {
		/*
		 * Allocate page descriptor (table) and map table for the child.
		 */

		if ((cpte = (pde_t*) sptalloc(2, PG_VALID, 0)) == 0) {
			rp2->r_pgsz = stoc(i);
			rp2->r_pgoff = 0;
			freereg(rp2);
			u.u_error = EAGAIN;
			return(NULL);
		}

		/* Invalidate the page table and zero the map table
		*/

		bzero(cpte, ctob(2));

		/* Set pointer to the newly allocated page descriptor (table)
		 * and dbd into the child's list.  Then get a
		 * pointer to the parents page descriptor (table) and dbd.
		 */

		rp2->r_list[i] = cpte;
		if (i < ctost(rp->r_pgoff))
			continue;
		ppte = rp->r_list[i];

		/* Get the total number of unmapped pages remaining.
		 * This is the total size of the region minus the
		 * number of segments for which we have allocated
		 * page tables already.
		 */

		size = rp->r_pgoff + rsize - stoc(i);

		/* If this size is greater than a segment, then
		 * we will only process a segment.
		 */

		if (size > NPGPT)
			size = NPGPT;

		/* Check for the first segment after the offset.
		 * This is not a full segment in general.
		 */

		if (rp->r_pgoff > stoc(i)) {
			tmp = rp->r_pgoff - stoc(i);
			size -= tmp;
			ppte += tmp;
			cpte += tmp;
		}

		/* Check each parents page and then copy it to
		 * the childs pte.  Also check the map table
		 * entries.
		 */

		for (j = 0  ;  j < size  ;  j++, ppte++, cpte++) {
			struct pfdat *pfd;
			dbd_t	*dbd;

			/*	If the page is in core, then
			 *	increment the page use count.
			 */

			if (pg_tstvalid(ppte)) {
				pfd = pdetopfdat(ppte);
				ASSERT(pfd->pf_use > 0); 
				pfd->pf_use++;
			}

			/* Increment the swap use count for pages which
			 * are on swap.  This bug took a long time to find.
			 * If you understand why this changed, you truly
			 * understand the paging mechanism.     K. Rusnock
			 */

			dbd = (dbd_t *)(ppte + NPGPT);
			if (dbd->dbd_type == DBD_SWAP) {
				ASSERT(swpuse(dbd) > 0);
				if (pg_tstmod(ppte) && pg_tstvalid(ppte)) {
					ASSERT(swpuse(dbd) == 1);
					if (!pbremove(rp, dbd))
						cmn_err(CE_PANIC,
							"dupreg - pbremove");
					swfree1(dbd);
					dbd->dbd_type = DBD_NONE;
				} else {
					if (!swpinc(dbd, "dupreg")) {
					     /* The swap use count overflowed
					      * so free the region and return
					      * an error.
					      */
					     ((dbd_t*)(cpte+NPGPT))->dbd_type = 
							DBD_NONE;
					     rp2->r_pgsz = stoc(i+1);
					     rp2->r_pgoff = 0;
					     freereg(rp2);
					     u.u_error = EAGAIN;
					     return(NULL);
					}
				}
			}

			/* Set copy-on-write bit
		 	 */

			pg_setcw(ppte);
			pg_clrmod(ppte);

			/*	Copy parents page to child.
			 */

			*cpte = *ppte;

			/*	Copy parents dbd to child.
			*/

			*(dbd_t *)(cpte + NPGPT) = *dbd;
		}
	}
	mmuflush();
	return(rp2);
}

/*
 * Change the size of a region
 *  change == 0  -> no-op
 *  change  < 0  -> shrink
 *  change  > 0  -> expand
 * For expansion, you get (change) demand zero pages
 * For shrink, the caller must flush the ATB
 * Returns 0 on no-op, -1 on failure, and 1 on success.
 */

growreg(prp, change, type)
register preg_t *prp;
{
	register pde_t	*pt;
	register int	i;
	register reg_t	*rp;
	register int	size;
	register int	osize;
	int		lotohi;
	int		start, end;
	int		tmp;

	rp = prp->p_reg;
	ASSERT(rp);
	ASSERT(reglocked(rp));
	ASSERT(change >= 0 || (-change <= rp->r_pgsz));
	ASSERT(rp->r_refcnt == 1);

	if (change == 0)
		return(0);

	/* Stack grows downwards in memory */
	lotohi = (prp->p_type != PT_STACK);

	osize = rp->r_pgoff + rp->r_pgsz;

	ASSERT(rp->r_noswapcnt >= 0);

	if (change < 0) {

		/*	The region is being shrunk.  Compute the new
		 *	size and free up the unneeded space.
		 */

		if (lotohi) {
			start = osize + change;
			end = osize;
		} else {
			start = rp->r_pgoff;
			end = rp->r_pgoff - change;
		}

		availsmem -= change;	/* change < 0.	*/
		if (rp->r_noswapcnt)
			availrmem -= change;

		for ( i = ctost(start);  i < ctos(end)  ;  i++) {
			/*	Free up the allocated pages for
			 *	this segment.
			 */

			pt = rp->r_list[i];
			size = end - stoc(i);
			if (size > NPGPT)
				size = NPGPT;
			if (start > stoc(i)) {
				tmp = start - stoc(i);
				size -= tmp;
				pt += tmp;
			}
			pfree(rp, pt, pt + NPGPT, size);
		}

		/*	Free up the page tables which we no
		 *	longer need.
		 */

		(void) ptexpand(prp, change, lotohi);
	} else {
		/*	We are expanding the region.  Make sure that
		 *	the new size is legal and then allocate new
		 *	page tables if necessary.
		 */

		if (availsmem - change < tune.t_minasmem) {
			nomemmsg("growreg", change, 0, 0);
			u.u_error = EAGAIN;
			return(-1);
		}
		if (rp->r_noswapcnt  &&
		   availrmem - change < tune.t_minarmem) {
			nomemmsg("growreg", change, 0, 1);
			u.u_error = EAGAIN;
			return(-1);
		}
		if (chkgrowth(prp, change, lotohi) ||
		   ptexpand(prp, change, lotohi)) {
			u.u_error = ENOMEM;
			return(-1);
		}
		availsmem -= change;
		if (rp->r_noswapcnt)
			availrmem -= change;

		/*	Initialize the new page tables and allocate
		 *	pages if required.
		 */

		if (lotohi) {
			start = osize;
			end = osize + change;
		} else {
			start = rp->r_pgoff;
			end = rp->r_pgoff + change;
		}

		for ( i = ctost(start); i < ctos(end) ; i++) {
			pt = rp->r_list[i];
			size = end - stoc(i);
			if (size > NPGPT)
				size = NPGPT;
			if (start > stoc(i)) {
				tmp = start - stoc(i);
				size -= tmp;
				pt += tmp;
			}

			while (--size >= 0) {
				((dbd_t *)(pt + NPGPT))->dbd_type = type;
				pt++;
			}
		}
	}

	rp->r_pgsz += change;
	loadstbl(&u, prp, change);
	u.u_procp->p_size += change;
	return(1);
}

/*
 * Check that grow of a pregion is legal
 * returns 0 if legal, -1 if illegal.
 */

chkgrowth(prp, change, lotohi)
register preg_t	*prp;
int	change;	/* Size in pages. */
int	lotohi;	/* Direction of change. */
{
	register preg_t  *oprp;
	register reg_t   *rp;
	register caddr_t start, last, vaddr, end;

	if (change <= 0)
		return(0);	/* if region is shrinking, everything's ok */

	rp = prp->p_reg;
	start = prp->p_regva;
	last  = start + ctob(rp->r_pgoff + rp->r_pgsz);

	if (lotohi) {
		last  += ctob(change);
		oprp = prp + 1;
	} else {
		if (rp->r_pgoff >= change)	/* region addr won't change */
			return(0);
		start -= stob(ctos(rp->r_pgsz + change) - ctos(change)); 
		oprp = prp - 1;
	}

	if (((int)last > USRSTACK) || ((int)start < 0)) /* inside addr space */
		return(-1);

	if ((rp = oprp->p_reg) != NULL) {
		vaddr = oprp->p_regva;
		if (start == vaddr)
			return(-1);
		end   = vaddr + ctob(rp->r_pgoff + rp->r_pgsz);
		if ((start < end) && (last > vaddr))
			return(-1);
	}
	return(0);
}

/*
 * Check all pregions of a process against the pregion specified.
 */
chkattach(up, prp)
user_t *up;
preg_t *prp;
{
	register preg_t  *oprp;
	register reg_t   *rp;
	register caddr_t start, last, vaddr, end;

	rp = prp->p_reg;
	start = prp->p_regva;
	last  = start + ctob(rp->r_pgoff + rp->r_pgsz);

	if ((int)start & SOFFMASK)	/* segment aligned */
		return(-1);
	if (((int)last > USRSTACK) || ((int)start < 0)) /* inside addr space */
		return(-1);

	for (oprp = (up->u_procp)->p_region; rp = oprp->p_reg; ++oprp) {
		ASSERT(oprp != prp);
		vaddr = oprp->p_regva;
		if (start == vaddr)
			return(-1);
		if (last < vaddr)
			break;
		end   = vaddr + ctob(rp->r_pgoff + rp->r_pgsz);
		if ((start < end) && (last > vaddr))
			return(-1);
	}
	return(0);
}

/*
 * Expand user page tables for a region 
 */
 
ptexpand(prp, change, lotohi)
register preg_t *prp;
{
	register reg_t  *rp;
	register pde_t	**lp;
	register int	osize;
	register int	nsize;
	register pde_t	**lp1, **lp2;
	register int	i;

	/* Calculate the new size in pages.
	 */

	rp = prp->p_reg;
	if (lotohi)
		osize = rp->r_pgoff + rp->r_pgsz;
	else
		osize = rp->r_pgsz;
	nsize = osize + change;

	/*	If we are shrinking the region, then free up
	 *	the page tables and map tables.  Use a smaller
	 *	list if possible.
	 */

	if (ctos(nsize) < ctos(osize)) {
		if (lotohi) {
			lp  = &rp->r_list[ctos(nsize)];
			lp1 = &rp->r_list[ctos(osize)];
			for (;  lp < lp1;  lp++) {
				sptfree(*lp, 2, 1);
				*lp = 0;
			}
		} else {
			int diff;

			diff = ctos(osize) - ctos(nsize);
			lp1 = rp->r_list;
			lp2 = &rp->r_list[diff];
			lp  = &rp->r_list[ctos(osize)];
			while (lp2 < lp) {
				sptfree(*lp1, 2, 1);
				*lp1++ = *lp2;
				*lp2++ = 0;
			}
			while (*lp1 != 0) {
				sptfree(*lp1, 2, 1);
				*lp1++ = 0;
			}
			rp->r_pgoff  -= stoc(diff);
			prp->p_regva += stob(diff);
		}

		i = ctos(ctos(nsize));
		if (i < rp->r_listsz) {
			if (nsize > 0) {
				lp2 = (pde_t **)sptalloc(i, PG_VALID, 0);
				/* If we are shrinking and can't get space
				 * for a smaller list, just keep the larger
				 * list and return ok.
				 */
				if (lp2 == NULL)
					goto out;
				bzero(lp2, ctob(i));
				bcopy(rp->r_list, lp2, ctos(nsize)*NBPW);
			} else {
				lp2 = 0;
			}
			sptfree(rp->r_list, rp->r_listsz, 1);
			rp->r_list = lp2;
			rp->r_listsz = i;
		}
	}

	/*	If the region shrunk or didn't grow by enough to
	 *	require any more page tables, then we are done.
	 */
out:
	if (change <= 0) {
		if (!lotohi)
			rp->r_pgoff -= change;
		return(0);
	}

	/*	If the region grew into the next segment,  then we 
	 *	must allocate one or more new page and map tables.
	 *	See if we have enough space in the list for the
	 *	new segments.  If not, allocate a new list and
	 *	copy over the old data.
	 */

	i = ctos(ctos(nsize));
	if (i > rp->r_listsz) {
		if ((lp2 = (pde_t **)sptalloc(i, PG_VALID, 0)) == 0)
			return(-1);
		bzero(lp2, ctob(i));
		if (rp->r_list) {
			bcopy(rp->r_list, lp2, ctos(osize)*NBPW);
			sptfree(rp->r_list, rp->r_listsz, 1);
		}
		rp->r_list = lp2;
		rp->r_listsz = i;
	}

	/*
	 *	Allocate a new set of page tables and disk maps.
	 */

	if (lotohi) {
		lp1 =lp= &rp->r_list[(rp->r_pgsz) ? ctos(osize) : ctost(osize)];
		lp2 = &rp->r_list[ctos(nsize)];
	} else {
		int diff;

		diff = ctos(nsize) - ctos(osize);
		lp1 = &rp->r_list[ctos(osize)];
		lp2 = &rp->r_list[ctos(nsize)];
		while (lp1 > rp->r_list)
			*(--lp2) = *(--lp1);
		lp = lp1;
		rp->r_pgoff  += stoc(diff);
		prp->p_regva -= stob(diff);
	}

	for ( ;  lp < lp2;  lp++) {
		*lp = (pde_t *)sptalloc(2, PG_VALID, 0);
		if (*lp == NULL) {
			while (--lp >= lp1)
				sptfree(*lp, 2, 1);
			return(-1);
		}
		bzero(*lp, ctob(2));
	}

	if (!lotohi)
		rp->r_pgoff -= change;
	return(0);
}

loadreg(prp, vaddr, ip, off, count)
register preg_t		*prp;
caddr_t			vaddr;
register struct inode	*ip;
{
	register reg_t	*rp;
	register int	gap;
	register int	size;

	/*	Make sure that we are not trying to read
	 *	beyond the end of the file.  This can
	 *	happen for a bad a.out where the header
	 *	lies and says the file is bigger than
	 *	it actually is.
	 */

	if (off + count > ip->i_size)
		return(-1);

	/*	Grow the region to the proper size to load the file.
	 */

	rp = prp->p_reg;
	ASSERT(reglocked(rp));
	gap = vaddr - prp->p_regva - ctob(rp->r_pgoff + rp->r_pgsz);
	if (gap < 0)
		gap = 0;

	if (size = btoct(gap)) {
		if (growreg(prp, 1, DBD_DZERO) < 0)	/* zero @ virt addr 0 */
			return(-1);
		if (growreg(prp, size - 1, DBD_NONE) < 0)
			return(-1);
	}

	if (growreg(prp, btoc(count+gap) - size, DBD_DFILL) < 0) 
		return(-1);

	/*	Set up to do the I/O.
	 */

	u.u_segflg = 0;
	u.u_base = vaddr;
	u.u_count = count;
	u.u_offset = off;

	/*	We must unlock the region here because we are going
	 *	to fault in the pages as we read them.  No one else
	 *	will try to use the region before we finish because
	 *	the RG_DONE flag is not set yet.
	 */

	regrele(rp);

	FS_READI(ip);

	if (u.u_error) {
		reglock(rp);
		return(-1);
	}

	/*	Clear the last (unused)  part of the last page.
	 */

	vaddr += count;
	count = ctob(1) - poff(vaddr);
	if (count > 0  &&  count < ctob(1))
		uzero(vaddr, count);

	reglock(rp);
	if (u.u_count)
		return(-1);
	return(0);
}

mapreg(prp, vaddr, ip, off, count)
preg_t	*prp;
caddr_t		vaddr;
struct inode	*ip;
int		off;
register int	count;
{
	register int	i;
	register int	j;
	register int	blkspp;
	register reg_t	*rp;
	register dbd_t	*dbd;
	int		seglim;
	int		tmp;
	int		gap;
	int		size;

	/*	Make sure that we are not trying to map
	 *	beyond the end of the file.  This can
	 *	happen for a bad a.out where the header
	 *	lies and says the file is bigger than
	 *	it actually is.
	 */

	if (off + count > ip->i_size)
		return(-1);

	/*	Get region pointer and effective device number.
	 */

	rp = prp->p_reg;
	ASSERT(reglocked(rp));

	/*	Compute the number of file system blocks in a page.
	 *	This depends on the file system block size.
	 */

	blkspp = max(NBPP, FSBSIZE(ip))/FSBSIZE(ip);

	/*	Allocate invalid pages for the gap at the start of
	 *	the region and demand-fill pages for the actual
	 *	text.
	 */

	gap = vaddr - prp->p_regva - ctob(rp->r_pgoff + rp->r_pgsz);
	if (gap < 0)
		gap = 0;

	if (size = btoct(gap)) {
		if (growreg(prp, 1, DBD_DZERO) < 0)	/* zero @ virt addr 0 */
			return(-1);
		if (growreg(prp, size - 1, DBD_NONE) < 0)
			return(-1);
	}

	if (growreg(prp, btoc(count + gap) - size, DBD_DFILL) < 0)
		return(-1);

	rp->r_filesz = count + off;
	size = rp->r_pgoff + rp->r_pgsz;

	/*	Build block list pointing to map table.
	 */

	gap = btoct(gap);		/* Gap in pages */
	off = btoct(off) * blkspp;	/* File offset in blocks. */
	seglim = ctos(size);
	ASSERT(gap >= rp->r_pgoff);

	for (i = ctost(gap);  i < seglim  ;  i++) {
		register int	lim;
		register pde_t	*pt;

		pt = rp->r_list[i];
		ASSERT(pt != 0);
		lim = size - stoc(i);
		if (lim > NPGPT)
			lim = NPGPT;
		if (gap > stoc(i)) {
			tmp = gap - stoc(i);
			pt += tmp;
			lim -= tmp;
		}
		dbd = (dbd_t *)pt + NPGPT;

		for (j = 0;  j < lim;  j++, pt++, dbd++) {

			/*	Set the copy-on-write for all pages,
			 *	since all pages are placed in the hash table.
			 */

			pg_setcw(pt);
			dbd->dbd_type  = DBD_FILE;
			dbd->dbd_blkno = off;
			off += blkspp;
		}
	}

	/*	Mark the last page for special handling
	 */
	
	dbd[-1].dbd_type = DBD_LSTFILE;
	return(0);
}

/*	Find the region corresponding to a virtual address.
 */

reg_t	*
findreg(p, vaddr)
register struct proc	*p;
register caddr_t	vaddr;
{
	register preg_t	*prp;
	preg_t *vtopreg();

	prp = vtopreg(p, vaddr);
	if (!prp)
		return(NULL);
	reglock(prp->p_reg);
	return(prp->p_reg);
}

/*	Find the pregion of a particular type.
 */

preg_t *
findpreg(pp, type)
register proc_t	*pp;
register int	type;
{
	register preg_t	*prp;

	for (prp = pp->p_region ; prp->p_reg ; prp++) {
		if (prp->p_type == type)
			return(prp);
	}

	/*	We stopped on an unused region.  If this is what
	 *	was called for, then return it unless it is the
	 *	last region for the process.  We leave the last
	 *	region unused as a marker.
	 */

	if ((type == PT_UNUSED)  &&  (prp < &pp->p_region[pregpp - 1]))
		return(prp);
	return(NULL);
}

/*
 * Change protection of pdes for a region
 */
void
chgprot(prp, prot)
preg_t	*prp;
{
	if (prot == SEG_RO)
		prp->p_flags |= PF_RDONLY;
	else
		prp->p_flags &= ~PF_RDONLY;

	loadstbl(&u, prp, prp->p_reg->r_pgsz);
}

/* Locate process region for a given virtual address. */

preg_t *
vtopreg(p, vaddr)
register struct proc *p;
register caddr_t vaddr;
{
	register preg_t *prp;
	register reg_t *rp;
	register caddr_t hi;
	register caddr_t lo;

	for (prp = p->p_region; rp = prp->p_reg; prp++) {
		lo = prp->p_regva + ctob(rp->r_pgoff);
		hi = lo + ctob(rp->r_pgsz);
		if ((unsigned long)vaddr >= (unsigned long)lo
		  && (unsigned long)vaddr < (unsigned long)hi)
			return(prp);
	}
	return(NULL);
}

/*
 * Set copy-on-write bits in all page descriptors
 */

markcw(prp)
register preg_t *prp;
{
	register reg_t	*rp;
	register pde_t	*pd;
	register int	i;
	register int	j;
	register int	size;
	register int	seglim;
	register int	lim;
	register int	tmp;

	rp = prp->p_reg;
	size = rp->r_pgoff + rp->r_pgsz;
	seglim = ctos(size);
	for (i = ctost(rp->r_pgoff); i < seglim; i++) {
		pd = rp->r_list[i];
		lim = size - stoc(i);
		if (lim > NPGPT)
			lim = NPGPT;
		if (rp->r_pgoff > stoc(i)) {
			tmp = rp->r_pgoff - stoc(i);
			pd += tmp;
			lim -= tmp;
		}
		for (j = 0; j < lim; j++, pd++) {
			pg_setcw(pd);
		}
	}
	mmuflush();
}
