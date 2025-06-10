/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/startup.c	10.10"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/pfdat.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/buf.h"
#include "sys/reg.h"
#include "sys/tty.h"
#include "sys/var.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"

extern user_t	*winuvad[];
extern pde_t	*winupde[];

/* Routine called before main, to initialize various data structures.
** The arguments are:
**	physclick - The click number of the first available page
**		    of memory.
*/

mlsetup(physaddr)
int	physaddr;
{
	register unsigned	nextclick;
	register unsigned	physclick;
	register unsigned	i;
	extern int		firstfree;	/* First free click.	*/
	extern int		maxfree;	/* Last free click + 1.	*/

	/* 	Size and zero memory starting at the first 
	 *	available page.
	 */
	physclick = btoc(physaddr);
	i = raminit(physclick);

	/*	Set up memory parameters.
	*/
	if (v.v_maxpmem  &&  (v.v_maxpmem < i))
		i = v.v_maxpmem;
	physmem = i;
	maxclick = i;
	firstfree = physclick;
	maxfree = maxclick;

	ASSERT(physclick < maxclick);

	/*	Initialize memory mapping for sysseg,
	**	the segment from which the kernel
	**	dynamically allocates space for itself.
	**	Also initialize virtual address space 
	**	table and turn on MMU.
	*/

	nextclick = mmuinit(physclick);

	/*	Initialize the kernel window segments.
	*/

	for (i = 0; i < NUBLK; ++i) {
		winuvad[i] = (struct user *)sptalloc(USIZE, 0, -1);
		winupde[i] = (pde_t *)kvtokptbl(winuvad[i]);
	}

	/*	Note that reginit must be called before
	**	mktables because reginit calculates the
	**	value of pregpp which is used by mktables.
	*/

	reginit();
	
	/*	Allocate some kernel tables.
	*/

	nextclick = mktables(nextclick);

	/*	Initialize the physical memory manager.
	*/

	kpbase = nextclick;
	meminit(nextclick, maxclick);

	/*	Initialize process 0.
	*/

	p0init();
}

/*
 * Create system space to hold page allocation and
 * buffer mapping structures and hash tables
 */

mktables(nextfree)
{
	register int	m;
	register int	i;
	register preg_t	*prp;
	extern int	pregpp;

	/*	Allocate space for the pfdat.
	*/

	i = btoc((maxclick - nextfree) * sizeof(struct pfdat));
	pfdat = ((struct pfdat *)sptalloc(i, PG_VALID, nextfree));
	pfdat = &pfdat[-nextfree];
	nextfree +=i;
	
	/*	Compute the smallest power of two larger than
	 *	the size of physical memory.
	 */

	m = physmem;
	while (m & (m - 1))
		 m = (m | (m - 1)) + 1;
	phashmask = (m>>3) - 1;

	/*	Allocate space for the page hash bucket
	 *	headers.
	 */

	i = btoc((m >> 3) * sizeof(*phash));
	phash = ((struct pfdat **)sptalloc(i, PG_VALID, nextfree));
	nextfree +=i;

	for (i = 0; i <= phashmask; ++i)
		phash[i] = (struct pfdat *)&phash[i];

	/*	Allocate space for the pregion tables for each process
	 *	and link them to the process table entries.
	 *	The maximum number of regions allowed for is process is
	 *	3 for text, data, and stack plus the maximum number
	 *	of shared memory regions allowed.
	 */
	
	i = btoc(pregpp * sizeof(preg_t) * v.v_proc);
	prp = (preg_t *)sptalloc(i, PG_VALID, nextfree);
	nextfree +=i;
	for (i = 0  ;  i < v.v_proc  ;  i++, prp += pregpp)
		proc[i].p_region = prp;
	
	/*	Return next available physical page.
	*/

	return(nextfree);
}

/*	Setup proc 0
*/

extern	pde_t	*mmuupde;

p0init()
{
	register int	i;
	register pde_t	*upde = mmuupde;
	extern 	 int	cdlimit;

	/* Allocate the proc[0]'s segment table
	*/

	if ((proc[0].p_pttbl = (sde_t*)sptalloc(SEGSIZE, PG_VALID, 0)) == NULL)
		cmn_err( CE_PANIC, "Can't allocate proc[0]'s ptr table");
	bzero((char*)proc[0].p_pttbl, ctob(SEGSIZE));

	/*	Setup proc 0's ublock
	*/

	if (sptmemall(proc[0].p_ubpgtbl, USIZE, PG_VALID) < 0)
		cmn_err( CE_PANIC, "Can't allocate proc[0]'s ublock");

	for (i=0; i<USIZE; ++i) {
		pg_setpde(upde++, pg_getpde(&proc[0].p_ubpgtbl[i]));
	}

	/*	Initialize proc 0's ublock
	*/

	u.u_tsize   = 0;
	u.u_dsize   = 0;
	u.u_ssize   = 0;
	u.u_procp   = &proc[0];
	u.u_cmask   = CMASK;
	u.u_limit   = cdlimit;

	/*	initialize process data structures
	*/

	curproc = &proc[0];
	curpri = 0;

	proc[0].p_size = USIZE;
	proc[0].p_stat = SRUN;
	proc[0].p_flag = SLOAD | SSYS;
	proc[0].p_nice = NZERO;
}

/*	Machine-dependent startup code
*/

startup()
{
	extern int	nofiles_min;
	extern int	nofiles_max;
	extern int	nofiles_cfg;

	nofiles_cfg = v.v_nofiles;
	v.v_nofiles = min(v.v_nofiles, nofiles_max);
	v.v_nofiles = max(v.v_nofiles, nofiles_min);
	u.u_pofile  = (char*)&u.u_ofile[v.v_nofiles];

	/*	Check to see if the configured value of NOFILES
	**	was illegal and changed.  If so, print a message.
	*/

	if (nofiles_cfg != v.v_nofiles) {
	    if (nofiles_cfg < nofiles_min)
		   cmn_err(CE_NOTE,
		      "Configured value of NOFILES (%d) is less than min (%d)",
			  nofiles_cfg, nofiles_min);
	    else
		  cmn_err(CE_NOTE,
		    "Configured value of NOFILES (%d) is greater than max (%d)",
			 nofiles_cfg, nofiles_max);
	    cmn_err(CE_CONT, "        NOFILES set to %d.", v.v_nofiles);
	}

	/*	The following string is used to designate the end of the
	**	u-structure and helps determine stack over runs.
	*/

	strcpy( (char*)&u.u_pofile[v.v_nofiles], "This string marks the end of the u-structure.  The kernel stack should not extend beyond this string.");
}
