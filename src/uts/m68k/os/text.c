/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/text.c	10.18"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"

/*	Allocate text region for a process
 */
struct inode *
xalloc(exec_data, tpregtyp)
register struct exdata *exec_data;
{
	register struct inode   *ip = exec_data->ip;
	register reg_t		*rp;
	register reg_t		*rp2;
	register preg_t		*prp;
	register int		size = exec_data->ux_tsize;
	register caddr_t	org = (caddr_t)(exec_data->ux_toffset);
	register int		base = exec_data->ux_txtorg;
	register int		regva = base & ~SOFFMASK;
	extern short dufstyp;

	/* Kind a KLUDGY, but reduces number of DU hooks.
	** If the ip is remote, overwrite magic number to 410
	*/

	if ((ip->i_fstyp == dufstyp) && (exec_data->ux_mag != 0407))
		exec_data->ux_mag = 0410;

	/*
	 * Make sure the map is built.
	 */

	if (exec_data->ux_mag == 0413 || exec_data->ux_mag == 0443) {
		if (FS_ALLOCMAP(ip) == 0) {
			iput(ip);
			return(NULL);
		}
	}

	if (size == 0)
		return(ip);

	/*	Search the region table for the text we are
	 *	looking for.
	 */

loop:
	for (rp = ractive.r_forw ; rp != &ractive ; rp = rp->r_forw) {
		if ((rp->r_type == RT_STEXT) && (rp->r_iptr == ip)) {
check:
			reglock(rp);
			if ((rp->r_type != RT_STEXT) || (rp->r_iptr != ip)) {
				regrele(rp);
				goto loop;
			}
			/* Since we have the inode locked, we should
			 * never get into xalloc with RG_DONE not set.
			 */
			if (!regdone(rp)) {
				regrele(rp);
				regwait(rp);
				goto check;
			}
			if (exec_data->ux_mag == 0407) { /* MCS special */
				if (rp->r_refcnt != 0) {
				    ASSERT(rp->r_filesz==ctob(btoc(base+size)));
					availsmem += btoc(rp->r_filesz - regva);
					rp2 = dupreg(rp, FORCE, btoc(size));
					regrele(rp);
					if (rp2 == NULL) {
					  availsmem -= btoc(rp->r_filesz-regva);
					  goto out;
					}
					rp = rp2;	/* use new region */
					rp->r_type = RT_STEXT;
					rp->r_flags |= RG_SSWAPMEM;
					rp->r_flags &= ~RG_NOFREE;
				} else 
					rp->r_filesz += regva;
			}
			prp = attachreg(rp, &u, regva, tpregtyp, SEG_RO);
			regrele(rp);
			if (prp == NULL) {
				iput(ip);
				return(NULL);
			}
			return(ip);
		}
	}
	
	/*	Text not currently being executed.  Must allocate
	 *	a new region for it.
	 *	The inode is locked by getxfile;  it expects us to
	 *	do an iput on every failure.  Since allocreg waits,
	 *	we must unwait here.  Don't set RG_NOFREE until the end.
	 */

	if ((rp = allocreg(ip, RT_STEXT, 0)) == NULL) {
		iput(ip);
		return(NULL);
	}

	/*	Attach the region to our process.
	 */
	
	if ((prp = attachreg(rp, &u, regva, tpregtyp, SEG_RW)) == NULL) {
		regunwait(rp);
		freereg(rp);
		goto out;
	}
	
	/*	Load the region or map it for demand load.
	 */

	switch (exec_data->ux_mag) {

	case 0413:
	case 0443:
		if (mapreg(prp, base, ip, org, size) < 0)
			goto bad;
		size = exec_data->ux_doffset + exec_data->ux_dsize;
		if (size > rp->r_filesz)
			rp->r_filesz = size;
		break;

	case 0410:	/* fall thru case if REMOTE */
		if (loadreg(prp, base, ip, org, size) < 0) 
			goto bad;
		break;

	case 0407:	/* degraded 410, shared text and data in one region */
		if (loadreg(prp, base, ip, org, size) < 0)
			goto bad;
		/* Special use of r_filesz with 407 text pages. */
		rp->r_filesz = ctob(btoc(base + size));
		markcw(prp);		/* protect the text pages */
		break;

	default:
		cmn_err(CE_PANIC, "xalloc - bad magic");
	}

	if (!FS_ACCESS(ip, ISVTX))
		rp->r_flags |= RG_NOFREE;
	ASSERT(prp->p_reg->r_listsz != 0);
	chgprot(prp, SEG_RO);
	regunwait(rp);
	rp->r_flags |= RG_DONE;		/* region finished */
	regrele(rp);
	return(ip);

bad:
	regunwait(rp);
	detachreg(prp, &u);
out:
	plock(ip);		/* freereg performs an iput, must relock ip */
	iput(ip);
	return(NULL);
}


/*	Free the swap image of all unused shared text regions
 *	which are from device dev (used by umount system call).
 */

xumount(mp)
register struct mount *mp;
{
	register reg_t		*rp;
	register reg_t		*nrp;
	register int		count;
	register inode_t	*ip;

	count = 0;
loop:
	for (rp = ractive.r_forw ; rp != &ractive ; rp = nrp) {
		if (rp->r_type != RT_STEXT)
			nrp = rp->r_forw;
		else {
			/*
			 * There is a race here.  At this point the region
			 * is not locked, and while we sleep waiting for
			 * the inode lock, the region may be freed and the
			 * inode count may drop to zero; when we finally
			 * get the inode we panic on the assertion in plock()
			 * which says that the count must not be zero.
			 *
			 * The correct fix is to lock the region before locking
			 * the inode, but to avoid deadlocks it has to be done
			 * in the same order throughout the system.  Next
			 * release, guys.  Workaround for the moment: put the
			 * code for plock() and prele() inline here, without
			 * the i_count assertions.  (I don't like it, either.)
			 */
			if (ip = rp->r_iptr) {
				/* Temporary: plock(ip) */
#ifdef DEBUG
				ilog(ip, 3, xumount);
#endif
				while (ip->i_flag & ILOCK) {
					ip->i_flag |= IWANT;
					sleep((caddr_t)ip, PINOD);
				}
				ip->i_flag |= ILOCK;
				ASSERT(addilock(ip) == 0);
			}
			reglock(rp);
			if (rp->r_type != RT_STEXT) {
				regrele(rp);
				if (ip) {
					/* Temporary: prele(ip) */
#ifdef DEBUG
					ilog(ip, 4, xumount);
#endif
					ASSERT(ip->i_flag & ILOCK);
					ip->i_flag &= ~ILOCK;
					if (ip->i_flag & IWANT) {
						ip->i_flag &= ~IWANT;
						wakeup((caddr_t)ip);
					}
					ASSERT(rmilock(ip) == 0);
				}
				goto loop;
			}
			nrp = rp->r_forw;
			if (mp == ip->i_mntdev && rp->r_refcnt == 0) {
				freereg(rp);
				count++;
				goto loop;
			} else {
				regrele(rp);
				if (ip)
					prele(ip);
			}
		}
	}
	return(count);
}

/*	Remove a shared text region associated with inode ip from
 *	the region table, if possible.
 */
xrele(ip)
register struct inode *ip;
{
	register reg_t	*rp;
	register reg_t	*nrp;

	ASSERT(ip->i_flag & ILOCK);
	if ((ip->i_flag&ITEXT) == 0)
		return;
	
loop:
	for (rp = ractive.r_forw ; rp != &ractive ; rp = nrp) {
		if (rp->r_type != RT_STEXT || ip != rp->r_iptr)
			nrp = rp->r_forw;
		else {
			reglock(rp);
			if (rp->r_type != RT_STEXT || ip != rp->r_iptr) {
				regrele(rp);
				goto loop;
			}
			nrp = rp->r_forw;
			if (rp->r_refcnt == 0) {
				freereg(rp);
				plock(ip);
				goto loop;
			} else {
				regrele(rp);
			}
		}
	}
}


/*	Try to removed unused sticky regions in order to free up swap space.
 *	This routine can not sleep on any lock: inode, region, sysreg, etc.
 */

swapclup()
{
	register reg_t		*rp;
	register reg_t		*nrp;
	register int		rval;
	register inode_t	*ip;

	rval = 0;
loop:
	for (rp = ractive.r_forw ; rp != &ractive ; rp = nrp) {
		nrp = rp->r_forw;
		if (rp->r_type == RT_SHMEM)
			continue;
		if (ip = rp->r_iptr) {
			if (ip->i_flag & ILOCK)
				continue;
			plock(ip);
		}
		if (rp->r_flags & RG_LOCK) {
			if (ip)
				prele(ip);
			continue;
		}
		reglock(rp);
		if (rp->r_type == RT_UNUSED) {
			regrele(rp);
			if (ip)
				prele(ip);
			continue;
		}
		if (rp->r_refcnt == 0) {
			freereg(rp);
			rval = 1;
			goto loop;
		} else {
			regrele(rp);
			if (ip)
				prele(ip);
		}
	}
	return(rval);
}

/*
 * Prepare object process text region for writing.  If there is only
 * one reference to the region, and it's not sticky (RG_NOFREE is not
 * set), change the region type to private (instead of shared-text)
 * to prevent it from being found by any subsequent text searches.
 * Otherwise, make a private copy-on-write duplicate, detach the
 * original region from the object process, and attach the duplicate
 * in its place.  The pregion pointer of the new (private) text region
 * is returned.
 *
 * The original region and the associated inode (if any) must be locked
 * before entry to xdup(); on return the new region will have been
 * locked and the inode will be (still) locked.  On error, xdup()
 * returns NULL and unlocks both the region and the inode.
 */
preg_t *
xdup(p, prp)
register struct proc *p;
register preg_t *prp;
{
	register reg_t *rp = prp->p_reg, *nrp;
	register struct inode *ip;
	register preg_t *nprp;
	caddr_t va;
	register user_t *up;
	user_t *winublock();

	ASSERT(rp->r_flags & RG_LOCK);
	ASSERT(rp->r_type == RT_STEXT);
	if (rp->r_refcnt == 1 && (rp->r_flags & RG_NOFREE) == 0) {
		nprp = prp;
		rp->r_type = RT_PRIVATE;
	} else {
		ip = rp->r_iptr;
		va = prp->p_regva;
		if ((nrp = dupreg(rp, FORCE, rp->r_pgsz)) == NULL) {
			regrele(rp);
			return(NULL);
		}
		ip = nrp->r_iptr;
		up = winublock(p);
		detachreg(prp, up);
		/* detachreg() unlocks ip */
		if (ip)
			plock(ip);
		if ((nprp = attachreg(nrp, up, va, PT_TEXT, SEG_RO)) == NULL
		  && ip)
			prele(ip);
		winubunlock(up);
	}
	return(nprp);
}
