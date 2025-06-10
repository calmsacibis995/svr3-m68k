/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/sysm68k.c	1.0"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/sysm68k.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/systm.h"
#include "sys/utsname.h"
#include "sys/swap.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/mvme131.h"
#include "sys/reg.h"

extern short do_fpu;

#define S4_2    0x80

char *saveflag = &u.u_trapflag;		/* Flag to save/restore-the-frame */
long *saveaddr = &u.u_trapsave[0];	/* Area to save-the-frame in	*/
unsigned char stkfmtsz[16] = {		/* (# bytes-8) on stack frame	*/
	 0,  0,  4,  0, 0, 0, 0, 0,	/* for the exceptions		*/
	 0, 12, 24, 84, 0, 0, 0, 0};	/* index is frame type format	*/

/*
 * sysm68k - system call
 */
sysm68k ()
{
	struct a {
		int cmd;
		int arg1;
		int arg2;
	} *uap = (struct a *) u.u_ap;

	switch (uap->cmd) {

 	case S68STACK:	/* grow stack on the MC68000 */
	{
		register i;

		i = uap->arg1;
		if (i>0)
			if (grow(USRSTACK - ctob(u.u_ssize + btoc(i))) != -1)
				u.u_rval1 = USRSTACK - ctob(u.u_ssize);
			else
				u.u_error = ENOMEM;
		break;
	}

	case S68CONT: /* instruction continuation after signal handlers */
	{
		register kind, *regs, *usp;

		regs = u.u_ar0;
		usp = (int *) regs[SP];

		/* find the return address for the 'rtr' */
		/* in the user's stack area */
		/* stack format: */
		/*   0 - _sysm68k return addr */
		/*   1 - command */
		/*   2 - old %d0 */
		/*   3 - <kind of resume, previous trap process status reg> */
		/*   4 - previous trap PC */

 		/* get resume kind */
		kind = (fuword((caddr_t) (usp+3)) >> 16) & 0xffff;
		u.u_traptype = kind; /* make it appear that we just got here */
				     /* for the first time for psignal */

		regs[PC] = fuword((caddr_t) (usp+4)); 
		/* make sure we don't allow transfer to supervisor mode */
		regs[PS] = (regs[PS] & 0xff00) |
			   (fuword((caddr_t) (usp+3)) & 0xff);
		u.u_rval1 = fuword((caddr_t) (usp+2));	/* user's D0 */
		usp += 5;
		if (u.u_traptype) {
			regs[PC] &= ~1;	/* make even again */
			if (u.u_trapsave[0] == 0)	{ /* no valid info */
				/* get rid of any user-specified handler */
				u.u_signal[SIGBUS - 1] = 0;
				psignal(u.u_procp, SIGBUS);
			} else
				u.u_trapflag = 1;	/* restore the frame */
		}
		regs[SP] = (int) usp;

		if (kind == TRAPADDR) { /* addr error */
			/* get rid of any user-specified handler */
			u.u_signal[SIGBUS - 1] = 0;
			psignal(u.u_procp, SIGBUS);
		}

		/* else we can just resume here */
		break;
	}

#if(defined(M68881))
/* The following code supports the "sysm68k" subcommand that
** returns the offending exception operand after an 881 exception.
*/
	case S68FPEX:	/* Floating Point Exception */
	{
		long *ii;
		register int j;

		if (!do_fpu)
			{u.u_error = EINVAL; break;}
			/* no 881, so why is the user doing this?*/

		ii = (long *)((struct a *)u.u_ap)->arg1; /* address to write 
						     **the operand to */

		for (j=0; j<3; j++)		     /* Write 3 longwords. */
		if (suword(ii++,u.u_fpu.excop[j])<0) /* lngword write failed! */
		        {u.u_error = EFAULT; break;}  /* User program bug   */

		break;	/* all is okay */
	}
#endif

	case S68RTODC:	/* read TODC */
	{
		time_t	rtctime;
		extern	time_t clkread();

		if (!suser())
			break;

		rtctime = clkread();
		if (copyout((caddr_t)&rtctime,
		    (caddr_t)uap->arg1, sizeof(time_t)))
			u.u_error = EFAULT;
		break;
	}

	case S68STIME:	/* set internal time, not hardware clock */
		if (suser())
			time = (time_t) uap->arg1;
		break;
 
	case S68SETNAME:	/* rename the system */
	{
		register i, c;

		char	sysnamex[sizeof(utsname.sysname)];

		if (!suser())
			break;

		for (i = 0; (c = fubyte((caddr_t) uap->arg1 + i)) > 0
		    && i < sizeof(sysnamex) - 1; ++i)
			sysnamex[i] = c;

		if (c) {
			u.u_error = c < 0 ? EFAULT : EINVAL;
			break;
		}
		sysnamex[i] = '\0';
		str8cpy(utsname.sysname, sysnamex);
		str8cpy(utsname.nodename, sysnamex);
		break;
	} 

	case S68MEMSIZE:	/*  return the size of memory */
		u.u_rval1 = USRSTACK;
		u.u_rval2 = ctob(physmem);
		break;

	case S68SWAP:
	/*	General interface for adding, deleting, or
	**	finding out about swap files.  See swap.h
	**	for a description of the argument.
	**/
	{
		swpi_t	swpbuf;

		if (copyin(uap->arg1, &swpbuf, sizeof(swpi_t)) < 0) {
			u.u_error = EFAULT;
			return;
		}
		swapfunc(&swpbuf);
		break;
	}

	case S68ICACHEON:	/* Enable internal cache of the MC-68020 */
		if (suser())
			icacheon();
		break;

	case S68ICACHEOFF:	/* Disable internal cache of the MC-68020 */
		if (suser())
			icacheoff();
		break;

	case S68BCACHEON:	/* Enable board cache of the MVME131XT */
		if (suser()) {
			*XCMR = (uap->arg1 & 0x0f);
			*XCCR = 0xf1;
		}
		break;

	case S68BCACHEOFF:	/* Disable board cache of the MVME131XT */
		if (suser())
			*XCCR = 0xFF;
		break;

	case S68DELMEM:
	/*	Delete memory from the available list.  Forces
	**	tight memory situation for load testing.
	*/
		if (suser())
			delmem(uap->arg1);
		break;

	case S68ADDMEM:		/* Add back previously deleted memory */
		if (suser())
			addmem(uap->arg1);
		break;

	case S68FPHW:	/* Tell a user whether we have a coprocessor or not.  */
		if (do_fpu) {
			if (suword(uap->arg1, MC68881) == -1)
				u.u_error = EFAULT;
		} else {
			if (suword(uap->arg1, NOFPHW) == -1)
				u.u_error = EFAULT;
		}
		break;

	case 74391:     /* You really don't want to know why - D. Farnsworth */
		u.u_rval1 = 0;
		if ( *STAT1 & S4_2 )
			u.u_rval1 = 1;
		break;
	default:
		u.u_error = EINVAL;
	}
}
