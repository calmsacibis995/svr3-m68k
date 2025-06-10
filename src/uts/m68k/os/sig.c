/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/sig.c	10.15"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/fstyp.h"
#include "sys/inode.h"
#include "sys/nami.h"
#include "sys/file.h"
#include "sys/psw.h"
#include "sys/reg.h"
#include "sys/var.h"
#include "sys/message.h"
#include "sys/conf.h"

/*
 * Priority for tracing
 */

#define	IPCPRI	PZERO

/*
 * Tracing variables.
 * Used to pass trace command from
 * parent to child being traced.
 * This data base cannot be
 * shared and is locked
 * per user.
 */

struct
{
	int	ip_lock;
	int	ip_req;
	int	*ip_addr;
	int	ip_data;
} ipc;

/*
 * Send the specified signal to
 * all processes with 'pgrp' as
 * process group.
 * Called by tty.c for quits and
 * interrupts.
 */

signal(pgrp, sig)
register pgrp;
{
	register struct proc *p;

	if (pgrp == 0)
		return;
	for (p = &proc[1]; p < (struct proc *)v.ve_proc; p++)
		if (p->p_pgrp == pgrp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */

psignal(p, sig)
register struct proc *p;
register sig;
{
	register s = sig - 1;	/* s is used as index into the
				signal array */
	if (s < 0 || s >= NSIG-1)
		return;
	p->p_sig |= 1L << s;
	if (p->p_stat == SSLEEP && ((p->p_flag & SNWAKE) == 0)) {
		if(!(p->p_hold & (1L<<s)))
			setrun(p);
	} else if (p->p_stat == SSTOP) {
		if (sig == SIGKILL)
			setrun(p);
		else if (p->p_wchan && ((p->p_flag & SNWAKE) == 0))
			/*
			 * If process is in the sleep queue at an
			 * interruptible priority but is stopped,
			 * remove it from the sleep queue but don't
			 * set it running yet. The signal will be
			 * noticed when the process is continued.
			 */
			unsleep(p);
	}
}

/*
 * Returns true if the current
 * process has a signal to process,
 * and the signal is not held.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */

issig()
{
	register n;
	register struct proc *p, *q;

	p = u.u_procp;
	if(!(p->p_sig & ~p->p_hold))
		return(0);
	while (n=fsig(p)) {
		if (n == SIGCLD) {
			if (u.u_signal[SIGCLD-1] == SIG_IGN) {
				for (q = p->p_child; q; q = q->p_sibling)
					if (q->p_stat == SZOMB)
						freeproc(q, 0);
			} else if (u.u_signal[SIGCLD-1])
				return(n);
		} else if (n == SIGPWR) {
			if (u.u_signal[SIGPWR-1] && 
				(u.u_signal[SIGPWR-1] != SIG_IGN))
				return(n);
		} else if ((u.u_signal[n-1] != SIG_IGN) || 
			(p->p_flag&STRC))
			return(n);
		p->p_sig &= ~(1L<<(n-1));
	}
	return(0);
}

/*
 * Put the argument process into the stopped state and notify the
 * parent and other interested parties via wakeup and/or signal.
 * 
 */

stop(p)
struct proc *p;
{
	register struct proc *cp, *pp;

	cp = p;
	pp = cp->p_parent;
	if (cp->p_ppid == 1  ||  cp->p_ppid != pp->p_pid)
		exit(fsig(cp));
	cp->p_stat = SSTOP;
	cp->p_flag &= ~SWTED;
	wakeup((caddr_t)pp);
	if (cp->p_trace)
		wakeup((caddr_t)p->p_trace);
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if (issig())
 *		psig();
 */

psig(arg)
{
	register n, mask;
	void(*p)();
	register struct proc *rp = u.u_procp;

	if (rp->p_flag & SPRSTOP) {
		rp->p_flag &= ~SPRSTOP;
		stop(rp);
		swtch();
	}
	if ((rp->p_flag & STRC)
	  || ((rp->p_flag & SPROCTR)
	    && (n = fsig(rp)) && (rp->p_sigmask & (1L<<(n-1))))) {
		/*
		 * If traced, always stop, and stay
		 * stopped until released by parent or tracer.
		 * If tracing via /proc, do not call procxmt.
		 */
		do {
			stop(rp);
			swtch();
		} while ((rp->p_flag&SPROCTR) == 0
		  && !procxmt() && (rp->p_flag & STRC));
	} 
	if ((n = fsig(rp)) == 0)
		return;
	mask = (1L << (n-1));
	rp->p_sig &= ~mask;
	if ((p = u.u_signal[n-1]) != SIG_DFL) {
		if (p == SIG_IGN)
			return;
		u.u_error = 0;
		/* if it is sigset, turn on p_hold bit */
		if(rp->p_chold & mask)
			rp->p_hold |= mask;
		else
			if(n != SIGILL && n != SIGTRAP && n != SIGPWR)
				u.u_signal[n-1] = SIG_DFL;
		sendsig(p, n, arg);
		return;
	}
	switch (n) {

	case SIGQUIT:
	case SIGILL:
	case SIGTRAP:
	case SIGIOT:
	case SIGEMT:
	case SIGFPE:
	case SIGBUS:
	case SIGSEGV:
	case SIGSYS:
		if (core())
			n += 0200;
	}
	exit(n);
}

/*
 * find the unhold signal in bit-position
 * representation in p_sig.
 */

fsig(p)
struct proc *p;
{
	register i;
	register n;

	n = p->p_sig & ~p->p_hold;
	for (i=1; i<NSIG; i++) {
		if (n & 1L)
			return(i);
		n >>= 1;
	}
	return(0);
}

/*
 * Create a core image on the file "core"
 *
 * It writes USIZE block of the
 * user.h area followed by the entire
 * data+stack segments.
 */


core()
{
	register struct inode *ip;
	struct argnamei nmarg;
	extern spath();
	register preg_t	*prp;
	register proc_t *pp;
	register int	gap;

	if (u.u_uid != u.u_ruid)
		return(0);

	u.u_error = 0;
	u.u_dirp = "core";
	u.u_syscall = DUCOREDUMP;
	nmarg.cmd = NI_CREAT;
	nmarg.mode = ((IREAD|IWRITE)>>6)|((IREAD|IWRITE)>>3)|(IREAD|IWRITE);
	nmarg.ftype = 0;
	ip = namei(spath, &nmarg);
	if (u.u_error)
		return(0);

	if (!FS_ACCESS(ip, IWRITE) && ip->i_ftype == IFREG) {

		/*	Put the region sizes into the u-block for the
		 *	dump.
		 */
		pp = u.u_procp;
		if (prp = findpreg(pp, PT_TEXT))
			u.u_tsize = prp->p_reg->r_pgsz;
		else
			u.u_tsize = 0;
		
		/*	In the following, we do not want to write
		**	out the gap but just the actual data.  The
		**	calculation mirrors that in loadreg and
		**	mapreg which allocates the gap and the
		**	actual space separately.  We have to watch
		**	out for the case where the entire data region
		**	was given away by a brk(0).
		*/

		if (prp = findpreg(pp, PT_DATA)) {
			u.u_dsize = prp->p_reg->r_pgsz;
			gap=btoct((caddr_t)u.u_exdata.ux_datorg - prp->p_regva);
			if (u.u_dsize > gap)
				u.u_dsize -= gap;
			else
				u.u_dsize = 0;
		} else {
			u.u_dsize = 0;
		}

		if (prp = findpreg(pp, PT_STACK)) {
			u.u_ssize = prp->p_reg->r_pgsz;
		} else {
			u.u_ssize = 0;
		}

		/*	Check the sizes against the current ulimit and
		**	don't write a file bigger than ulimit.  If we
		**	can't write everything, we would prefer to
		**	write the stack and not the data rather than
		**	the other way around.
		*/

		if (USIZE + u.u_dsize + u.u_ssize > (uint)dtop(u.u_limit)) {
			u.u_dsize = 0;
			if (USIZE + u.u_ssize > (uint)dtop(u.u_limit))
			u.u_ssize = 0;
		}

		FS_ITRUNC(ip);
		u.u_offset = 0;
		u.u_base = (caddr_t)&u;
		u.u_count = ctob(USIZE);
		u.u_segflg = 1;
		u.u_fmode = FWRITE;
		FS_WRITEI(ip);

		/*	Write the data and stack to the dump file.
		 */
		
		u.u_segflg = 0;
		if (u.u_dsize) {
			u.u_base = (caddr_t)u.u_exdata.ux_datorg;
			u.u_count = ctob(u.u_dsize) - poff(u.u_base);
			FS_WRITEI(ip);
			u.u_offset = ctob(btoc(u.u_offset));
		}
		if (u.u_ssize) {
			u.u_count = ctob(u.u_ssize);
			u.u_base = (caddr_t)(USRSTACK - u.u_count);
			FS_WRITEI(ip);
		}

	} else {
		printf("sig access error\n");
		u.u_error = EACCES;
	}
	iput(ip);
	return(u.u_error==0);
}
/*
 * sys-trace system call.
 */

ptrace()
{
	register struct proc *p;
	register struct a {
		int	req;
		int	pid;
		int	*addr;
		int	data;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->req <= 0) {
		u.u_procp->p_flag |= STRC;
		return;
	}
	for (p = u.u_procp->p_child; p; p = p->p_sibling)
		if (p->p_stat == SSTOP
		 && p->p_pid == uap->pid)
			goto found;
	u.u_error = ESRCH;
	return;

    found:
	while (ipc.ip_lock)
		sleep((caddr_t)&ipc, IPCPRI);
	ipc.ip_lock = p->p_pid;
	ipc.ip_data = uap->data;
	ipc.ip_addr = uap->addr;
	ipc.ip_req = uap->req;
	p->p_flag &= ~SWTED;
	setrun(p);
	while (ipc.ip_req > 0)
		sleep((caddr_t)&ipc, IPCPRI);
	if (ipc.ip_req < 0) {
		u.u_error = EIO;
	} else
		u.u_rval1 = ipc.ip_data;
	ipc.ip_lock = 0;
	wakeup((caddr_t)&ipc);
}

/*
 * Code that the child process
 * executes to implement the command
 * of the parent process in tracing.
 */

#define UADDR(u,i)	((int *)&((short *)&u)[i>>1])	/* halfword alignment */
int	ipcreg[] = {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, SP, PC};

procxmt()
{
	register int i;
	int	pswsave;
	register *p;

	i = ipc.ip_req;
	ipc.ip_req = 0;
	switch (i) {

	case 1: /* read user I */
	case 2: /* read user D */

		if ((ipc.ip_data = fuword((caddr_t)ipc.ip_addr)) == -1)
			goto error;
		break;


	case 3: /* read u */

		i = (int)ipc.ip_addr;
		if (i >= 0  &&  i < ctob(USIZE)) {
			ipc.ip_data = *UADDR(u,i);
			break;
		}

		p = (int *)i;
		for (i = 0  ;  i < sizeof(ipcreg)/sizeof(ipcreg[0])  ; i++) {
			if (p == &u.u_ar0[ipcreg[i]]) {
				ipc.ip_data = *p;
				goto out;
			}
		}
		goto error;

	case 4: /* write user I */
	case 5: /* write user D */
	{
		register preg_t *prp;
		register reg_t *rp;
		int rdonly = 0;
		preg_t *vtopreg(), *xdup();

		if ((prp = vtopreg(u.u_procp, ipc.ip_addr)) == NULL)
			goto error;
		rp = prp->p_reg;
		if ((rp->r_type == RT_STEXT) && (u.u_magic == 0407))
			if ((int)ipc.ip_addr < rp->r_filesz)
				goto error; /* Don't write to 407 text pages */
		if ((rp->r_type == RT_STEXT) && (u.u_magic != 0407)) {
			if (rp->r_iptr)
				plock(rp->r_iptr);
			reglock(rp);
			if ((prp = xdup(u.u_procp, prp)) == NULL)
				goto error;
			rp = prp->p_reg;
			if (rp->r_iptr)
				prele(rp->r_iptr);
			regrele(rp);
		}
		if (prp->p_flags & PF_RDONLY) {
			rdonly++;
			reglock(rp);
			chgprot(prp, SEG_RW);
			regrele(rp);
		}
		/*
		 * Zapped address must be on a word boundary; no explicit
		 * check here because the hardware enforces it and suword
		 * will fail on a non-word address.
		 */
		i = suword((caddr_t)ipc.ip_addr, ipc.ip_data);
		if (rdonly) {
			reglock(rp);
			chgprot(prp, SEG_RO);
			regrele(rp);
		}
		if (i < 0)
			goto error;
		break;
	}

	case 6: /* write u */
		p = UADDR(u, (int)ipc.ip_addr);
		for (i=0; i<sizeof(ipcreg)/sizeof(ipcreg[0]); i++)
			if (p == &u.u_ar0[ipcreg[i]])
				goto ok6;
		if (p == &u.u_ar0[PS]) {
			/* don't touch bits: user mode & ipl 0 & system stack */
			ipc.ip_data &= ~(PS_CUR|PS_IPL);
			ipc.ip_data |= PS_M;
			goto ok6;
		}
		goto error;

	ok6:
		*p = ipc.ip_data;
		break;

	/* set signal and continue */
	/* one version causes a trace-trap */
	case 9:
		u.u_ar0[PS] |= PS_T;
	case 7:
		if ((int)ipc.ip_addr != 1)
			u.u_ar0[PC] = (int)ipc.ip_addr;
		u.u_procp->p_sig = 0L;
		if (ipc.ip_data)
			psignal(u.u_procp, ipc.ip_data);
		wakeup(&ipc);
		return(1);

	case 8: /* force exit */

		wakeup(&ipc);
		exit(fsig(u.u_procp));

	default:
	error:
		ipc.ip_req = -1;
	}
out:
	wakeup(&ipc);
	return(0);
}
