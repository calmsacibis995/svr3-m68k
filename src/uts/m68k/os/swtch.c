/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/swtch.c	10.9"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/region.h"
#include "sys/proc.h"

/*
 * put the current process on
 * the Q of running processes and
 * call the scheduler. (called by trap.c)
 */

qswtch()
{
	setrq(u.u_procp);
	swtch();
}

/*
 * This routine is called to reschedule the CPU.
 * if the calling process is not in RUN state,
 * arrangements for it to restart must have
 * been made elsewhere, usually by calling via sleep.
 * There is a race here. A process may become
 * ready after it has been examined.
 * In this case, idle() will be called and
 * will return in at most 1HZ time.
 * i.e. its not worth putting an spl() in.
 */

char		runrun;
int		switching;
struct proc	*lastproc;
extern		runqueues();
extern char	qrunflag;

swtch()
{
	register struct proc *p;
	struct proc *disp();

	sysinfo.pswitch++;
	if (save(u.u_rsav)) {
		if (lastproc->p_stat == SZOMB)
			ubfree(lastproc);
		return;
	}

	switching = 1;
	lastproc = u.u_procp;
loop:
	spl6();

	/*
	 * Run the STREAM's queues, if anything is pending.
	 */

	if (qrunflag)
		runqueues();

	/*
	 * Search for highest-priority runnable process
	 * If no process is runnable, idle.
	 */

	runrun = 0;
	if ((p = disp()) == NULL) {
		curpri = PIDLE;
		curproc = &proc[0];
		idle();
		goto loop;
	}

	/*
	 * The rsav contents are interpreted in the new address space
	 */

	spl0();
	mmucswtch(p);
	switching = 0;
	resume(p->p_ubpgtbl, u.u_rsav);
}
