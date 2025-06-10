/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/fork.c	10.14"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/pfdat.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/acct.h"
#include "sys/tuneable.h"

/*	The following is used to lock the kernel window which
**	is used to address a u-block other than the u-block of
**	the current process.  This lock is needed since
**	procdup can sleep while using the window.
*/

pde_t	*winupde[NUBLK];
user_t	*winuvad[NUBLK];
int	win_ublk_lock = 0;

struct user*
winublock(p)
register struct proc *p;
{
	register int i, k;
	register pde_t *mmupte;
	register pde_t *ubpte = p->p_ubpgtbl;

	for(;;) {
		for(k = 1, i = 0; i < NUBLK; ++i, k<<=1) {
			if ((win_ublk_lock & k) == 0) {
				win_ublk_lock |= k;
				mmupte = winupde[i];
				for (k=0; k< USIZE; ++k)
					*mmupte++ = *ubpte++;
				return(winuvad[i]);
			}
		}
		win_ublk_lock |= UB_WANTED;
		sleep(&win_ublk_lock, PZERO);
	}
}

winubunlock(window)
struct user* window;
{
	register int i, k;
	register pde_t *mmupte;

	for(k = 1, i = 0; i < NUBLK; ++i, k<<=1) {
		if (winuvad[i] == window) {
			ASSERT(win_ublk_lock & k);
			mmupte = winupde[i];
			if (win_ublk_lock & UB_WANTED)
				wakeup(&win_ublk_lock);
			win_ublk_lock &= ~(UB_WANTED | k);
			for (i=0; i< USIZE; ++i)
				pg_setpde(mmupte++,0);
			mmuflush();
			return;
		}
	}
	cmn_err(CE_PANIC,"winubunlock - bad window");
}

/*
 * fork system call.
 */

fork()
{
	sysinfo.sysfork++;

	/*
	 * Disallow if
	 *  No processes at all; or
	 *  not su and too many procs owned; or
	 *  not su and would take last slot; or
	 * Check done in newproc().
	 */

	switch (newproc(1)) {
		case 1:  /* child -- successful newproc */
			u.u_rval1 = u.u_procp->p_ppid;
			u.u_rval2 = 1; /* child */
			u.u_start = time;
			u.u_ticks = lbolt;
			u.u_mem = u.u_procp->p_size;
			u.u_ior = u.u_iow = u.u_ioch = 0;
			u.u_cstime = 0;
			u.u_stime = 0;
			u.u_cutime = 0;
			u.u_utime = 0;
			u.u_acflag = AFORK;
			u.u_lock = 0;
			ASSERT(noilocks() == 0);
			return;
		case 0: /* parent, rval1 setup by newproc */
			/* u.u_rval1 = pid_of_child; */
			break;
		default:	/* couldn't fork */
			u.u_error = EAGAIN;
			break;
	}

	u.u_rval2 = 0;	/* parent */
}


/*
 * Create a new process-- the internal version of
 * sys fork.
 *
 * This changes the new proc structure and
 * alters only u.u_procp kf the uarea.
 *
 * It returns 1 in the new process, 0 in the old.
 */

int	mpid;

newproc(failok)
{
	register struct proc *cp, *pp, *pend;
	register n, a;

	/*
	 * First, just locate a slot for a process
	 * and copy the useful info from this process into it.
	 * The panic "cannot happen" because fork has already
	 * checked for the existence of a slot.
	 */

retry:
	mpid++;
	if (mpid >= MAXPID) {
		mpid = 0;
		goto retry;
	}
	pp = &proc[0];
	cp = NULL;
	n = (struct proc *)v.ve_proc - pp;
	a = 0;
	do {
		if (pp->p_stat == NULL) {
			if (cp == NULL)
				cp = pp;
			continue;
		}
		if (pp->p_pid == mpid)
			goto retry;
		if (pp->p_uid == u.u_ruid)
			a++;
		pend = pp;
	} while (pp++, --n);
	if (cp == NULL) {
		if ((struct proc *)v.ve_proc >= &proc[v.v_proc]) {
			if (failok) {
				syserr.procovf++;
				u.u_error = EAGAIN;
				return(-1);
			} else
				cmn_err(CE_PANIC, "newproc - no procs");
		}
		cp = (struct proc *)v.ve_proc;
	}
	if (cp > pend)
		pend = cp;
	pend++;
	v.ve_proc = (char *)pend;
	if (u.u_uid && u.u_ruid) {
		if (cp == &proc[v.v_proc-1] || a > v.v_maxup) {
			u.u_error = EAGAIN;
			return(-1);
		}
	}
	/*
	 * make proc entry for new proc
	 */
	pp = u.u_procp;
	cp->p_uid = pp->p_uid;
	cp->p_suid = pp->p_suid;
	cp->p_sgid = pp->p_sgid;
	cp->p_pgrp = pp->p_pgrp;
	cp->p_nice = pp->p_nice;
	cp->p_chold = pp->p_chold;
	cp->p_sig = pp->p_sig;
	cp->p_hold = pp->p_hold;
	cp->p_stat = SIDL;
	cp->p_clktim = 0;
	cp->p_flag = SLOAD | (pp->p_flag & (SSEXEC|SPROCTR));
	cp->p_pid = mpid;
	cp->p_epid = mpid;
	cp->p_ppid = pp->p_pid;
	cp->p_time = 0;
	cp->p_cpu = 0;
	cp->p_pri = PUSER + pp->p_nice - NZERO;
	cp->p_sigmask = pp->p_sigmask;

	/* link up to parent-child-sibling chain---
	 * no need to lock generally since only a free proc call
	 * (done by same parent as newproc) diddles with child chain.
	 */

	cp->p_sibling = pp->p_child;
	cp->p_parent = pp;
	pp->p_child = cp;
	cp->p_sysid = pp->p_sysid;	/* REMOTE */

	/*
	 * make duplicate entries
	 * where needed
	 */

	for (n=0; n<v.v_nofiles; n++)
		if (u.u_ofile[n] != NULL)
			u.u_ofile[n]->f_count++;

	plock(u.u_cdir);
	u.u_cdir->i_count++;
	prele(u.u_cdir);
	if (u.u_rdir) {
		plock(u.u_rdir);	/*lock root directory*/
		u.u_rdir->i_count++;
		prele(u.u_rdir);
	}
	
	/*
	 * Copy process.
	 */
	switch (procdup(cp, pp)) {
	case 0:
		/* Successful copy */
		break;
	case -1:
		if (!failok)
			cmn_err(CE_PANIC, "newproc - fork failed\n");

		/* reset all incremented counts */

		pexit();

		/* clean up parent-child-sibling pointers--
		* No lock necessary since nobody else could
		* be diddling with them here.
		*/

		pp->p_child = cp->p_sibling;
		cp->p_parent = NULL;
		cp->p_sibling = NULL;
		cp->p_stat = NULL;
		cp->p_ppid = 0;
		u.u_error = EAGAIN;
		return(-1);
	case 1:
		/* Child resumes here */
		return(1);
	}

	u.u_rval1 = cp->p_pid;		/* parent returns pid of child */

	/* have parent give up processor after
	 * its priority is recalculated so that
	 * the child runs first (its already on
	 * the run queue at sufficiently good
	 * priority to accomplish this).  This
	 * allows the dominant path of the child
	 * immediately execing to break the multiple
	 * use of copy on write pages with no disk home.
	 * The parent will get to steal them back
	 * rather than uselessly copying them.
	 */
	runrun = 1;
	return(0);
}

/*
 * Create a duplicate copy of a process, everything but stack.
 */

procdup(cp, pp)
struct proc	*cp;
struct proc	*pp;
{
	register preg_t		*p_prp;
	register preg_t		*c_prp;
	register user_t		*uservad;
	register inode_t	*ip;
	int			tmppsw;
	struct 	 user 		*setuctxt();
	extern	struct	proc	*lastproc;

	/*
	 * When the resume is executed for the new process,
	 * here's where it will resume.
	 */

	if (save(u.u_rsav)) {
		if (lastproc->p_stat == SZOMB)
			ubfree(lastproc);
		return(1);
	}

	/*	Setup child u-block and return address of locked window
	 */

	if ((uservad = setuctxt(pp, cp)) == NULL)
		return(-1);

	/*	Duplicate all the regions of the process.
	 */

	p_prp = pp->p_region;
	c_prp = cp->p_region;

	for (; p_prp->p_reg ; p_prp++, c_prp++) {
		register int		prot;
		register reg_t		*rp;
		int			force;

		prot = (p_prp->p_flags & PF_RDONLY ? SEG_RO : SEG_RW);
		rp = p_prp->p_reg;
		if (ip = rp->r_iptr)
			plock(ip);
		reglock(rp);
		force = ((u.u_magic == 0407) && (rp->r_type == RT_STEXT));
		if ((rp = dupreg(rp, force, rp->r_pgsz)) == NULL) {
			regrele(p_prp->p_reg);
			goto bad;
		}

		if (force) {	/* 407 files only */
			availsmem += btoc(rp->r_filesz - (int)p_prp->p_regva);
			rp->r_type  =  RT_STEXT;
			rp->r_flags |= RG_SSWAPMEM;
			rp->r_flags &= ~RG_NOFREE;
		}

		if (attachreg(rp, uservad, p_prp->p_regva,
			     p_prp->p_type, prot) == NULL) {
			if (rp->r_refcnt) {
				if (ip = rp->r_iptr)
					prele(ip);
				regrele(rp);
			} else
				freereg(rp);
			if (rp != p_prp->p_reg) {
				regrele(p_prp->p_reg);

				/* Note that we don't want to
				** do a prele(ip) here since
				** rp will have had the same
				** ip value and the freereg
				** will have unlocked it.
				*/
			}
			goto bad;
		}
		if (ip)
			prele(ip);
		regrele(p_prp->p_reg);
		if (rp != p_prp->p_reg)
			regrele(rp);
	}

	/*	Set up values for child to
	**	return to "newproc".
	**	Put the child on the run queue.
	*/

	winubunlock(uservad);
	cp->p_stat = SRUN;
	setrq(cp);
	return(0);
bad:
	while (c_prp-- > cp->p_region) {
		if (ip = c_prp->p_reg->r_iptr) 
			plock(ip);
		reglock(c_prp->p_reg);
		detachreg(c_prp, uservad);
	}
	winubunlock(uservad);
	ubfree(cp);
	return(-1);
}

/*	Setup context of child process
*/

struct user*
setuctxt(pp, cp)
register struct proc *pp;	/* parent proc pointer */
register struct proc *cp;	/* child proc pointer */
{
	register int	i;
	register user_t *uservad;
	label_t		tsav;

	/* Copy the save area for the child before entering the 
	 * memory allocation routines, which may sleep, thus
	 * trashing the current save area contents.
	 */
	for (i=0; i<LABELSIZE; i++) 
		tsav[i] = u.u_rsav[i];

	/* Allocate the childs segment table
	*/

	if ((cp->p_pttbl = (sde_t*)sptalloc(SEGSIZE, PG_VALID, 0)) == NULL)
		return(NULL);
	bzero((char*)cp->p_pttbl, ctob(SEGSIZE));

	/* Allocate the U block
	 */

	if (sptmemall(cp->p_ubpgtbl, USIZE, PG_VALID) < 0) {
		sptfree(cp->p_pttbl, SEGSIZE, 1);
		cp->p_pttbl = 0;
		return(NULL);
	}

	/* Map and lock the U block window
	*/

	uservad = winublock(cp);

	/*	Copy u-block 
	 */

	bcopy(&u, uservad, ctob(USIZE));

	/*	Reset the save registers and the u_procp.
	*/

	for (i=0; i<LABELSIZE; i++)
		uservad->u_rsav[i] = tsav[i];

	uservad->u_procp = cp;
	return(uservad);
}

/*
 * Ubfree will free the ublock and the segment table for the given process
 */

ubfree(p)
struct proc *p;
{
	ASSERT(u.u_procp != p);
	ASSERT(p->p_pttbl != NULL);

	sptfree( p->p_pttbl, SEGSIZE, 1);
	p->p_pttbl = 0;
	spfree(p->p_ubpgtbl, USIZE);
}
