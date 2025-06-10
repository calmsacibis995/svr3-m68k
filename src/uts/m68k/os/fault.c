/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/fault.c	10.9"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/mount.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/psw.h"
#include "sys/getpages.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"

/*	"Fault" gets called because there is a problem with a user page.
 *	Determine the cause of the page fault and return the appropriate
 *	signal action to be sent.
 *	    	 0	   Faulting page has been fixed, continue inst.
 *	    	value	   Appropriate signal to be sent.
 */

fault(vaddr, p, write)
register proc_t	*p;	/* faulting process */
{
	register preg_t	*prp;	/* faulting pregion */
	register reg_t	*rp;	/* faulting region  */
	register pde_t	*pd;	/* faulting pde     */
	register dbd_t	*dbd;	/* faulting dbd     */
	register typesig = 0;	/* signal type      */
	int	caddr;		/* click address    */
	preg_t *vtopreg();

	minfo.fault++;

	/*	Get a pointer to the region which the faulting
	 *	virtual address is in.  Attempt to grow the
	 *	stack to resolve region.
	 */

	if ((prp = vtopreg(p, vaddr)) == NULL)
	    if ((grow(u.u_ar0[SP]) != 1) || (prp = vtopreg(p, vaddr)) == NULL)
		return(SIGSEGV);

	rp = prp->p_reg;
	reglock(rp);
	caddr = btoct((caddr_t)vaddr - prp->p_regva);
	pd = &rp->r_list[caddr/NPGPT][caddr%NPGPT];
	dbd = (dbd_t *)pd + NPGPT;

	/*	Check to see that the pde hasn't been modified
	 *	while waiting for the lock.  This also checks to
	 *	see if another process read in the page while we
	 *	were waiting for the region lock above.
	 */

	if (!pg_tstvalid(pd)) {
		/*	Check for an unassigned page.  This is a real
	 	 *	error.
		 */
		if (dbd->dbd_type == DBD_NONE) {
			typesig = SIGSEGV;
			goto out;
		}

		if (typesig = vfault(rp, pd, dbd))
			goto out;
	}

	/*	Now check for a real protection error as opposed
	 *	to a copy on write.
	 */

	if (write) {
		if (prp->p_flags & PF_RDONLY) {
			typesig = SIGSEGV;
			goto out;
		}

		/*	Now Check for a copy on write page fault
		 */
		if (pg_tstcw(pd)) {
			if ((u.u_magic == 0407) && (vaddr < rp->r_filesz)) {
				typesig = SIGSEGV;
				goto out;
			}
			pfault(rp, pd, dbd);
		}
	}
	mmuflush();
out:
	regrele(rp);
	return(typesig);
}

/*	Protection fault handler
 */

pfault(rp, pd, dbd)
register reg_t	*rp;	/* faulting region	*/
register pde_t	*pd;	/* faulting pde.	*/
register dbd_t	*dbd;	/* faulting dbd.	*/
{
	register pfd_t	*pfd;
	pde_t		pttmp;

	minfo.pfault++;

	ASSERT(reglocked(rp));
	ASSERT((rp->r_type == RT_PRIVATE) || (u.u_magic == 0407));
	/* rp->r_type can be either RT_STEXT (407 files) or RT_PRIVATE */

	pfd = pdetopfdat(pd);
	ASSERT(pfd->pf_use > 0);
	ASSERT(pg_tstcw(pd));

	/*	Keep all pages in the cache that could reduce
	**	I/O because they represent disc blocks.	   K. Rusnock
	*/

	if  (pfd->pf_use == 1) {
		if (dbd->dbd_type == DBD_SWAP) {
			if (swfree1(dbd) == 0)
				premove(pfd);
			dbd->dbd_type = DBD_NONE;
		}

		if ((pfd->pf_flags & P_HASH) == 0) {
			minfo.steal++;
			goto out;
		}
	}

	minfo.cw++;
	pg_setpde(&pttmp, 0);		/* invalidate pde */

	/*	We are locking the page we faulted on
	**	before calling ptmemall because
	**	ptmemall may unlock the region.  If
	**	it does, then the page could be stolen
	**	and we would be copying incorrect
	**	data into our new page.
	*/

	pg_setlock(pd);
	pfd->pf_rawcnt++;
	ptmemall(rp, &pttmp, 1, 1);

	/*	Its O.K. to unlock the page now since
	**	ptmemall has locked the region again.
	*/

	ASSERT(reglocked(rp));
	ASSERT(pg_tstlock(pd));
	ASSERT(pfd == pdetopfdat(pd));
	ASSERT(pfd->pf_rawcnt > 0);

	if (--pfd->pf_rawcnt == 0)
		pg_clrlock(pd);

	bcopy(pfntokv(pg_getpfn(pd)), pfntokv(pg_getpfn(&pttmp)), NBPP);

	pfree(rp, pd, dbd, 1);
	*pd = pttmp;

	/*	Set the modify bit here before the region is unlocked
	 *	so that getpages will write the page to swap if necessary.
	 */
out:
	pg_setmod(pd);
	pg_setref(pd);
	pg_clrcw(pd);
	return(0);
}

/*	Translation fault handler
 */

vfault(rp, pd, dbd)
register reg_t	*rp;	/* faulting region	*/
register pde_t	*pd;	/* faulting pde.	*/
register dbd_t	*dbd;	/* faulting dbd.	*/
{
	register pfd_t	*pfd;
	register inode_t *ip;

	ASSERT(u.u_procp->p_flag & SLOAD);
	ASSERT(reglocked(rp));
	ASSERT(!pg_tstvalid(pd));

	minfo.vfault++;

	/*	Allocate a page in case we need it.  We must
	 *	do it now because it is not convenient to
	 *	wait later if no memory is available.  If
	 *	ptmemall does a wait and some other process
	 *	allocates the page first, then we have
	 *	nothing to do.
	 */
	
	if (ptmemall(rp, pd, 1, 0)) {
		ASSERT(pg_tstvalid(pd));
		return(0);
	}

	/*	See what state the page is in.
	 */

	switch (dbd->dbd_type) {
	case DBD_DFILL:
	case DBD_DZERO:{

		/* 	Demand zero or demand fill page.
		 */

		minfo.demand++;
		if (dbd->dbd_type == DBD_DZERO)
			bzero(pfntokv(pg_getpfn(pd)), NBPP);
		dbd->dbd_type = DBD_NONE;
		pg_clrcw(pd);
		break;
	}
	case DBD_SWAP:
	case DBD_FILE:
	case DBD_LSTFILE:{

		/*	Page is on swap or in a file.  See if a
		 *	copy is in the hash table.
		 */

		if (pfd = pfind(rp, dbd)) {
			pde_t	pde_save;

			/*	Page is in cache.
			 *	If it is also on the free list,
			 *	remove it.
			 */

			minfo.cache++;
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

			/*	Free the page we allocated above
			 *	since we don't need it.
			 */

			pde_save = *pd;
			pg_setvalid(&pde_save);
			pfree(rp, &pde_save, NULL, 1);
			rp->r_nvalid++;
			pfd->pf_use++;
			pg_setpfn(pd, pfdattopfn(pfd));

			/*	If the page has not yet been read
			 *	in from swap or file, then wait for
			 *	the I/O to complete.
			 */

			while (!(pfd->pf_flags & P_DONE)) {
				pfd->pf_flags |= P_WAITING;
				sleep(pfd, PZERO);
				if (pfd->pf_flags & P_BAD) {
					return(SIGKILL);
				}
			}

		} else {

			/*	Must read from swap or a file.
			 *	Get the pfdat for the newly allocated
			 *	page and insert it in the hash table.
			 *	Note that it cannot already be there
			 *	because the pfind above failed.
			 */
			
			pfd = pdetopfdat(pd);
			ASSERT((pfd->pf_flags & P_HASH) == 0);
			pinsert(rp, dbd, pfd);

			if (dbd->dbd_type == DBD_SWAP) {
				pglst_t		pglist;

				minfo.swap++;
				pglist.gp_ptptr = pd;
				swap(&pglist, 1, B_READ);

			} else {	/* Read from file */
				int vaddr;
				int offset;
				int retval;
				int i;

				minfo.file++;
				vaddr = pfntokv(pg_getpfn(pd));
				ip = rp->r_iptr;
				offset = dbd->dbd_blkno * min(NBPP,FSBSIZE(ip));

				retval = FS_READMAP(ip, offset, NBPP, vaddr, 1);

				if (retval <= 0) {
					killpage(rp, pd);
					return(SIGKILL);
				}

				/*
				 * clear the last bytes of a partial page
				 */

				if (dbd->dbd_type == DBD_LSTFILE) {
				     if (btoct(offset) == btoct(rp->r_filesz)) {
						i = poff(rp->r_filesz);
						if (i > 0)
							bzero(vaddr+i, NBPP-i);
				     }
				}
			}

			/*	Mark the I/O done in the pfdat and
			 *	awaken anyone who is waiting for it.
			 */

			pfd->pf_flags |= P_DONE;
			if (pfd->pf_flags & P_WAITING) {
				pfd->pf_flags &= ~P_WAITING;
				wakeup(pfd);
			}
		}
		break;
	}
	default:
		cmn_err(CE_PANIC, "vfault - bad dbd_type");
	}
	pg_setvalid(pd);
	pg_clrmod(pd);
	return(0);
}


/*
 * Clean up after a read error during vfault processing.
 * This code frees the previously allocated page, and marks
 * the pfdat as bad.  It leaves the pde, and dbd in their original
 * state.  It assumes the pde is presently invalid.
 */
killpage(rp, pd)
reg_t *rp;
register pde_t *pd;
{
	register pde_t save_pde;
	register pfd_t *pfd;

	save_pde = *pd;
	pg_setvalid(pd);
	pfd = pdetopfdat(pd);
	if (pfd->pf_flags & P_HASH)
		premove(pfd);
	pfd->pf_flags |= (P_DONE|P_BAD);
	if (pfd->pf_flags & P_WAITING) {
		pfd->pf_flags &= ~P_WAITING;
		wakeup(pfd);
	}
	pfree(rp, pd, NULL, 1);
	*pd = save_pde;
}
