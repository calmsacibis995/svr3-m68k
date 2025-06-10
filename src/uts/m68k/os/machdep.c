/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/machdep.c	10.15"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/psw.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/user.h"
#include "sys/var.h"
#include "sys/reg.h"

/*
 *	Read time of day chip
 */
extern time_t time;

time_t clkread()
{
	return(time);
}

/*
/*
 * Send an interrupt to process
 */
sendsig(hdlr, signo, arg)
caddr_t hdlr;
{
	register *usp, *regs;

	regs = u.u_ar0;
	usp = (int *)regs[SP];
	if (u.u_traptype) {
		regs[PC] |= 1;	/* make odd to prevent sucessful 'rtr' */
	}
	if (grow((unsigned)(usp-4)) == -1) {		/* 5 integers */
		return;
	}
	/* simulate an interrupt on the user's stack */
	suword((caddr_t)--usp, regs[PC]);
	suword((caddr_t)--usp, (regs[PS] & 0xffff) | (u.u_traptype << 16));
	suword((caddr_t)--usp, arg);
	suword((caddr_t)--usp, signo);

	regs[SP] = (int)usp;
	regs[PS] &= ~(PS_T1 | PS_T0);
	regs[PC] = (int)hdlr;
}

/*
 * Clear registers on exec
 */

setregs()
{
	register int	i;
	void (* *rp)();

	for (i = 0; i < 15; i++) 
		u.u_ar0[i] = 0;	/* ignore PS and PC */

		/* special registers for 68000; initial pc */
	u.u_ar0[PC] = u.u_exdata.ux_entloc;

		/* Initial stack region */
	u.u_rval1 = u.u_ar0[R8] = USRSTACK - ctob(u.u_ssize);
	u.u_rval2 = 0;

	u.u_procp->p_chold = 0;	/* clear p_chold */

	/* Any pending signal remain held, so don't clear p_hold and
	p_sig */	

	/* If the action was to catch the signal, then the action
	must be reset to SIG_DFL */

	for (rp = &u.u_signal[0]; rp < &u.u_signal[NSIG]; rp++)
		if (*rp != SIG_IGN)
			*rp = SIG_DFL;

	for (i=0; i<v.v_nofiles; i++)
		if ((u.u_pofile[i] & EXCLOSE) && u.u_ofile[i] != NULL) {
			closef(u.u_ofile[i]);
			u.u_ofile[i] = NULL;
		}
}

addupc(pc, p, incr)
unsigned pc;
register struct
{
	short	*pr_base;
	unsigned pr_size;
	unsigned pr_off;
	unsigned pr_scale;
} *p;
{
	register int   curval;	/* current value of word @ 'slot' */
	register int   tmpval;
	register short *slot;

	slot = &p->pr_base[((((pc - p->pr_off) * p->pr_scale) >> 16))>>1];
	if ((caddr_t)slot >= (caddr_t)(p->pr_base) &&
		(caddr_t)slot < (caddr_t)((unsigned)p->pr_base + p->pr_size))
		{
			tmpval = (int)fubyte((char *)slot);
			tmpval = (int)(tmpval<<8 | fubyte((char *)slot+1));
			if ((curval = tmpval) == -1)
				u.u_prof.pr_scale = 0;	/* turn off */
			else
			{
			    /*
			    ** Inc curval;  Ovfl lets loword (short) wraparound.
			    */
				curval += incr;
				subyte((char *)slot, curval>>8);
				subyte((char *)slot+1, curval&0xff);
			}
		}
}
