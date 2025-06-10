/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/trap.c	1.0"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/reg.h"
#include "sys/psw.h"
#include "sys/immu.h"
#include "sys/trap.h"
#include "sys/region.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/sysinfo.h"
#include "sys/utsname.h"
#include "sys/mvme131.h"
#include "sys/cmn_err.h"

extern unsigned nofulow();
extern unsigned nofuhigh();
extern unsigned errret();

extern int bootstate;	/* rfs state: up or down */

/*
 * Called from the trap handler when a processor trap occurs.
 */
trap(sp,r0,r1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,status,pc)
struct ex_status status;
{
	register type;
	register sig = 0;
	register struct proc *pp;
	time_t syst;
	register code = u.u_ar0[PC];		/* pc is the most common use */
	
	type = (int) status.vec_num & 0xfff;
	syst = u.u_stime;
	pp = u.u_procp;
	u.u_ar0 = &r0;
	u.u_traptype = TRAPNORM;
	sysinfo.trap++;

	if (USERMODE(status.psw))
		type |= USER;
	switch (type) {

	/*
	 * Trap not expected.
	 * Usually a kernel mode bus error.
	 * If user mode, signal trap.
	 */
	default:
		if (USERMODE(status.psw)) {
			sig = SIGILL;
			break;
		}
		spl7();
		cmn_err(CE_WARN,"       *** Kernel Trap *** ");
		cmn_err(CE_WARN,"proc = 0x%x, user = 0x%x",u.u_procp,&u);
		cmn_err(CE_WARN,"pid  = 0x%x, code = 0x%x",u.u_procp->p_pid,code);
		cmn_err(CE_WARN,"pc   = 0x%x, ps   = 0x%x",pc,status.psw);
		cmn_err(CE_WARN,"argp = 0x%x, aps  = 0x%x",&r0,&status);
		cmn_err(CE_PANIC,"   *** Unknown Trap Type = 0x%x ***", type);

	case RESCHED + USER:	/* Allow process switch */
		sysinfo.resched++;
		qswtch();
		goto out;

	case FPETRAP + USER:	/* floating point exception */
		sig = SIGFPE;
		code = r0;
		break;

	case ZDVDERR + USER:	/* zero divide check */
		sig = SIGFPE;
		code = KINTDIV;
		break;

	case CHKTRAP + USER:	/* CHK trap */
		sig = SIGFPE;
		code = KSUBRNG;
		break;

	case ADDRERR + USER:	/* address error */
		sig = SIGBUS;
		code = u.u_ar0[DCFA];
		u.u_traptype = TRAPADDR;
		break;

	case TRAPVFT + USER:	/* TRAPV instruction */
		sig = SIGFPE;
		code = KINTOVF;
		break;

	case IOTTRAP + USER:	/* IOT trap */
		sig = SIGIOT;
		code = r0;
		break;

	case EMTTRAP + USER:	/* EMT trap */
		sig = SIGEMT;
		code = r0;
		break;

	case FMTERR + USER:	/* bad exception format */
		sig = SIGILL;
		break;
	case UNINTR:		/* uninitialized interrupt;fall thru to STRAY */
	case UNINTR + USER:	/* ####### NOTE ##### may move */

#if(defined(m68k) && defined(M68881))
	case FPTBSUN       :	/* FPU exc. - Branch or Set on UNordered cond.*/
		fpu_trap(type); /* BSUN trap needs special processing */
	case FPTINEX       :	/* FPU exc. - INEXact result */
	case FPTDZ         :	/* FPU exc. - Divide by Zero */
	case FPTUNFL       :	/* FPU exc. - UNderFLow */
	case FPTOPER       :	/* FPU exc. - OPerand ERror */
	case FPTOVFL       :	/* FPU exc. - OVerFLow */
	case FPTSNAN       :	/* FPU exc. - Signalling NAN */
		{
		stray(sp,r0,r1,d2,d3,d4,d5,d6,d7,
				a0,a1,a2,a3,a4,a5,a6,status,pc);
				/* Treat kernel mode fpu exceptions as */
		return;		/* stray excs.  Shouldn't happen once  */
				/* all 881 fsave bugs are fixed.       */
		}

	case FPTBSUN + USER:	/* FPU exc. - Branch or Set on UNordered cond.*/
	case FPTINEX + USER:	/* FPU exc. - INEXact result */
	case FPTDZ   + USER:	/* FPU exc. - Divide by Zero */
	case FPTUNFL + USER:	/* FPU exc. - UNderFLow */
	case FPTOPER + USER:	/* FPU exc. - OPerand ERror */
	case FPTOVFL + USER:	/* FPU exc. - OVerFLow */
	case FPTSNAN + USER:	/* FPU exc. - Signalling NAN */
		{
		extern short do_fpu;
		if (do_fpu)
			{
			i = SIGFPE;
			code = fpu_trap(type-USER);
			break;
			}
		}
	   /* Otherwise, fall thru to spurious interrupt processing */
#endif

	case STRAYFT:		/* spurious interrupt */
	case STRAYFT + USER:
		stray(sp,r0,r1,d2,d3,d4,d5,d6,d7,
				a0,a1,a2,a3,a4,a5,a6,status,pc);
		return;

	case COPROTV + USER:	/* coprocessor protocol violation trap */
		sig = SIGILL;
		break;

	case BPTFLT + USER:	/* bpt instruction fault */
		u.u_ar0[PC] -= 2; /* back up pc for simulated breakpoint */
		/* fall thru */

	case TRCTRAP + USER:	/* trace trap */
		u.u_ar0[PS] &= ~(PS_T1 | PS_T0); /* turn off trace bits */
		sig = SIGTRAP;
		break;

	case PRIVFLT + USER:	/* privileged instruction fault */
	case EMT11FT + USER:	/* Line 1111 emulator trap */
	case EMT10FT + USER:	/* Line 1010 emulator trap */
	case INSTERR + USER:	/* Illegal instruction */
		sig = SIGILL;
		break;

	case ACFAIL:		/* VME AC failure interrupt signal */
	case ACFAIL + USER:
		splhi();
		cmn_err(CE_WARN,"System AC failure - No sync!");
		while(1);			/* No sync */
		break;

	case SYSFAIL:		/* VME SYSFAIL signal */
	case SYSFAIL + USER:
		cmn_err(CE_PANIC,"VME SYSFAIL signaled");
		break;
	}

	if (sig)
		psignal(pp, sig);

	if (runrun != 0)
		qswtch();
out:
	curpri = pp->p_pri = calcppri(pp);

	if ((pp->p_flag & SPRSTOP) || (pp->p_sig && issig())) {
		psig(code);
		curpri = pp->p_pri = calcppri(pp);
	}

	if(u.u_prof.pr_scale)
		addupc((caddr_t)u.u_ar0[PC], &u.u_prof, (int)(u.u_stime-syst));
}

/*
 * Called from the trap handler when a processor syscall trap occurs.
 */
syscall(sp,r0,r1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,status,pc)
struct ex_status status;
{
	register		code;
	register 		i;
	register int		*ap;
	register struct sysent	*callp;
	register struct proc	*pp;
	time_t			syst;
	short			pid;

	syst = u.u_stime;
	pid = u.u_procp->p_pid;
	u.u_traptype = TRAPNORM;
	u.u_ar0 = &r0;
	sysinfo.syscall++;
	u.u_error = 0;
	u.u_ar0[PS] &= ~PS_C;
	ap = (int *)u.u_ar0[SP];
	ap++;			/* skip word with param count */
	code = u.u_ar0[R0];
	i = code&0377;
	if (i >= NSYSENT)
		i = 0;
	else if (i==0) {	/* indirect */
		i = fuword(ap++)&0377;
		if (i >=NSYSENT)
			i = 0;
	}
	u.u_syscall = i;
	callp = &sysent[i];
	for (i = 0; i < callp->sy_narg; i++) {
		u.u_arg[i] = fuword(ap++);
	}
	u.u_dirp = (caddr_t)u.u_arg[0];
	u.u_rval1 = 0;
	u.u_rval2 = u.u_ar0[R1];
	u.u_ap = u.u_arg;

	if ((bootstate || !callp->sy_setjmp) && setjmp(u.u_qsav)) {
		if (!(u.u_rflags & U_RSYS) && !u.u_error)
			u.u_error = EINTR;
		spl0();
	} else
		(*callp->sy_call)();

	u.u_rflags &= ~(U_RSYS | U_DOTDOT | U_LBIN);

	if (u.u_error) {
		u.u_ar0[R0] = u.u_error;
		u.u_error = 0;
		u.u_ar0[PS] |= PS_C;
		if (++u.u_errcnt > 16) {
			u.u_errcnt = 0;
			runrun++;
		}
	} else {
		u.u_ar0[R0] = u.u_rval1;
		u.u_ar0[R1] = u.u_rval2;
	}

	pp = u.u_procp;
	curpri = pp->p_pri = calcppri(pp);

/*	if the system call trap was executed with the T bit set,
 *	the trace trap will be ignored.  So, the bit is reset
 *	and the SIGTRAP signal is sent to the process.
 */

	if (u.u_ar0[PS] & (PS_T1 | PS_T0)) {	/* were trace bits set ? */
		u.u_ar0[PS] &= ~(PS_T1 | PS_T0); /* reset the trace bits */
		psignal(pp, SIGTRAP);	/* send signal to process */
	}

	if (runrun != 0)
		qswtch();

	if (!u.u_trapflag && ((pp->p_flag & SPRSTOP)||(pp->p_sig && issig()))) {
		psig(code);
		curpri = pp->p_pri = calcppri(pp);
	}

	/*	If pid != pp->p_pid, then we are the child
	**	returning from a fork system call.  In this
	**	case, ignore syst since our time was reset
	**	in fork.
	*/

	if (u.u_prof.pr_scale)
		addupc((caddr_t)u.u_ar0[PC], &u.u_prof, 
			pid == pp->p_pid ? (int)(u.u_stime - syst)
					 : (int)u.u_stime);
}


/*
 * Called from the trap handler when a processor bus error occurs.
 *	    return value		Meaning
 *	    	 0	   Faulting page has been fixed, continue inst.
 *	    	value	   Abort cycle.  Clean up stack frame.
 *				New pc has been assigned.
 */
buserr(sp,r0,r1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,Status,pc)
struct ex_status Status;
unsigned pc;
{
	register typesig = 0;	/* signal type		*/
	register proc_t	*p;	/* faulting process	*/
	register int	ssw;	/* special status work	*/
	register int	vaddr;	/* faulting virt addr	*/
	register int	berrsrc;/* STAT2 bus error reg	*/
	register int	write;	/* write flag		*/
	register int	*ppc;	/* pointer to pc	*/
	time_t syst;		/* time of fault	*/
	int *old_ar0;		/* old ar0		*/
	int old_hold;		/* old held signals	*/

	minfo.buserr++;
	p = u.u_procp;
	u.u_traptype = TRAPNORM;
	old_ar0 = u.u_ar0;	/* save old ar0 and restore on return	      */
	u.u_ar0 = &r0;		/* set up new ar0 for current regs	      */
	ppc = &u.u_ar0[PC];	/* address of pc - referenced very often      */
	ssw = u.u_ar0[SSW];	/* write only if data fault and write bit clr */
	write = ((ssw & (SSW_DF|SSW_RW)) == SSW_DF);	/* otherwise a read   */

	/*
	 * Calculate the address of the bus error 
	 *	This is the logical address to the MMU
	 */

	if (ssw & SSW_DF) {			/* If data cycle fault,       */
		vaddr = u.u_ar0[DCFA];		/* use data cycle fault addr  */
	} else {				/* else check frame type.     */
		if (Status.vec_num == SHRTBUSSTKFRM)	/* If short frame     */
			vaddr = *ppc + 4;	/* use pc+4 for B stage addr, */
		else				/* otherwise pick up stage B  */
			vaddr = u.u_ar0[SBA];	/* fault address directly.    */
		if (ssw & SSW_FC)		/* If fault on stage C then   */
			vaddr -= 2;		/* use B address - 2 as addr. */
	}

	/*
	 * Determine the source of the bus error
	 *
	 *	The STAT2 bus error status register holds the source of the
	 * bus error.  However, this register has some problems.  On the vme131
	 * card stray bus errors are seen.  This is due to the 68020 
	 * architecture.  When a prefetch fault and an operand (data) fault
	 * occur at the same time, it is possible to get here and the bus error
	 * status register will be zero.  In this case, we will default to
	 * processing the bus fault normally.  Parity errors are checked
	 * only if the VME, VMX, or LBTO bits are set in the status register.
	 */

	berrsrc = ~(*STAT2);
	if (berrsrc & (STAT2_VMEBERR | STAT2_VMXBERR | STAT2_LBTO)) {
		if (chkparity(vaddr, USERMODE(Status.psw))) {
			if (USERMODE(Status.psw)) {
				psignal(p, SIGKILL);
				psig(vaddr);
			} else {
				parbuserr(berrsrc, vaddr, *ppc, Status);
			}
		}
	}

	/*
	 *	Stray bus errors and MMU generated faults are treated 
	 * the same.  Since there is no status register on the MMB, we must
	 * first determine what caused the fault and call the appropriate fault
	 * handler.  If the program can not continue, determine the appropriate
	 * error handling.
	 */

	if (USERMODE(Status.psw)) {	/* user mode bus error */

		syst = u.u_stime;
		if (typesig = fault(vaddr, p, write)) {
			u.u_traptype = TRAPBUS;	/* bus error trap type */
			u.u_trapflag = 1;	/* copy stack frame to u area */

			psignal(p, typesig);	/* post bus error signal */
			old_hold = p->p_hold;	/* Hold off all other signals */
			p->p_hold = ~( (1L<<(SIGSEGV-1)) | (1L<<(SIGKILL-1)) );
						/* except SIGSEGV & SIGKILL */
			if ((p->p_flag & SPRSTOP) || (p->p_sig && issig()))
				psig(vaddr);

			p->p_hold = old_hold;	/* reset held signals */
		}

		curpri = p->p_pri = calcppri(p);

		if (runrun != 0)
			qswtch();

		if (u.u_prof.pr_scale)
		   addupc((caddr_t)*ppc ,&u.u_prof, (int)(u.u_stime-syst));

	} else {			/* kernel mode bus error */

		if (*ppc > (unsigned)nofulow && *ppc < (unsigned)nofuhigh) {
			if (ssw & FCD_KERNEL) {
				typesig = -1;
			} else {
				if (typesig = fault(vaddr, p, write))
					typesig = -1;
				curpri = p->p_pri = calcppri(p);
			}
		} else {
			cmn_err(CE_WARN,"      *** Kernel Bus Error ***");
			cmn_err(CE_WARN,"      %s virtual address 0x%x",
			       (write) ? "Writing" : "Reading", vaddr);
			cmn_err(CE_WARN,"      at location PC = 0x%x",*ppc);
			cmn_err(CE_PANIC,"  Kernel Bus Error System Panic");
		}
	}

	if (typesig == -1) {
		*ppc = (unsigned)errret;
		if (u.u_caddrflt)
			*ppc = (unsigned)u.u_caddrflt;
	}

	u.u_ar0 = old_ar0;
	return(typesig);
}

/*
 * nonexistent system call-- signal bad system call.
 */
nosys()
{
	psignal(u.u_procp, SIGSYS);
}

stray(usp,d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,Status,pc)
struct ex_status Status;
{
	register int vector = (int)Status.vec_num;
	cmn_err(CE_WARN, " ***** Stray interrupt at vector number 0x%x *****\n", vector);
	cmn_err(CE_WARN, " PC = 0x%x, usp = 0x%x, psw = 0x%x\n",
		pc, usp, Status.psw);
}

parbuserr(berrsrc, vaddr, pc, Status)
struct ex_status Status;
{
	cmn_err(CE_WARN,"Kernel Parity Error (PC=0x%x, PS=0x%x)",pc,Status.psw);
	switch (berrsrc & (STAT2_VMEBERR | STAT2_VMXBERR | STAT2_LBTO)) {
	case STAT2_VMEBERR:
		cmn_err(CE_PANIC,"VME131: VME bus error at 0x%x",vaddr);
	case STAT2_VMXBERR:
		cmn_err(CE_PANIC,"VME131: VMX bus error at 0x%x",vaddr);
	case STAT2_LBTO:
		cmn_err(CE_PANIC,"VME131: Local bus time out at 0x%x",vaddr);
	default:
		cmn_err(CE_PANIC,"VME131: Unknown bus error (0x%x) at 0x%x",berrsrc,vaddr);
	}
}
