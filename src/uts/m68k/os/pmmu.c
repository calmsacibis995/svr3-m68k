/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/pmmu.c	10.7"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/pfdat.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/var.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"


/*
 * File: pmmu.c
 *  This file WILL contain primitives to support the MC-68851 PMMU chip.
 *  However because of non-availability, the primitives currently support
 *  are the MMB emulation board.  This emulation board is a (slightly
 *  altered) subset of the PMMU.  All future changes to support the PMMU
 *  should be localized to this file and the header file immu.h.
 */

/*
 * System memory management variables
 */

pde_t *mmuupde;	/* pointer to table of pde's mapping the U-Vector */

extern struct map sptmap[];	/* map for dynamiclly allocated virtual space */
extern etext;


/*
 * System table structures 
 * 	The following must be quad longword (hex) aligned.
 */

static char tblarea[FCLTSZ * sizeof(sde_t) + 15];  /* table area */

sde_t *fcltbl;		/* function lookup table */
sde_t *udtbl;		/* user domain table */
sde_t *sdtbl;		/* system domain table */

/*
 * mmuinit(physclick)
 *
 * Sets up the system space for the MMU.
 * Enter this routine running in physical space and exit it running
 * mapped as follows:
 *
 *	AREA			SIZE(CLICKS)	COMMENT
 *
 * 00000000 Kernel Static	physmem 	mapped 1:1
 *
 * 01000000 U-Vector		4k
 * 01001000 Kernel Dynamic	physmem-4k	"syssegs"
 *
 * ff???000 32 Bit Device I/O	I0_32_SZ	mapped 1:1 to last 32 bit addr.
 * fff00000 Debug Monitor ROM
 * ffff0000 VMEbus Short I/O
 *
 * physclick is the first physical click available for tables.
 */
int mmuinit(physclick)
int physclick;
{
	sde_t *sde;
	sde_t *stbl;
	pde_t *pde;
	int   i, size;
	caddr_t addr;

	/*
	 * Set up MMU translation tree for all kernel virtual space.
	 */

	chk_mmu();
	wakemmu();

	/* Allocate user and system domain tables */
	udtbl = (sde_t *)ctob(physclick++);
	sdtbl = (sde_t *)ctob(physclick++);

	/*
	 * Set up function code lookup table.
	 * Note that all pointer and page tables must be 16 BYTE aligned.
	 * Most actually end up page alligned.
	 */
	fcltbl = (sde_t *) (((int) tblarea + 15) & ~15);

	sde = &fcltbl[SUPVDATA];	/* for system function codes */
	setsde(sde, SDTLIM, pnum(sdtbl), PT_VLD );
	fcltbl[SUPVPROG] = *sde;

	sde = &fcltbl[USERDATA];	/* for user function codes */
	setsde(sde, UDTLIM, pnum(udtbl), PT_VLD );
	fcltbl[USERPROG] = *sde;


	/* 
	 * Set up user domain table.
	 */

	setsde(udtbl, USTLIM, pnum(NULLADDR), PT_INV );


	/*
	 * Set up system domain 127 for Debug Rom and I/O
	 */

	/* allocate segment table and attach to domain table  */
	stbl = (sde_t *)ctob(physclick++);
	setsde(&sdtbl[127], SSTLIM, pnum(stbl), PT_VLD );

	/* build page tables */
	addr = (caddr_t) ALLFFFS-(IO_32_SZ*NBPP)+1;
	physclick = mkpgtbl( pnum(doff(addr)), IO_32_SZ, stbl, physclick );

	/* map 1:1 */
	pde = kvtopde(addr);
	for ( i = pnum(addr); i < pnum(addr)+IO_32_SZ; i++, pde++ ) {
		pg_setpfn(pde, i);
		pg_setci(pde);
		pg_setvalid(pde);
	}



	/*
	 * Set up system domain 0 static kernel virtual space.
	 */

	/* allocate segment table and attach to domain 0 */
	stbl = (sde_t *)ctob(physclick++);
	setsde(sdtbl, SSTLIM, pnum(stbl), PT_VLD );


	/* Build page tables to map physical memory */
	size =  physmem;
	physclick = mkpgtbl( pnum(doff(0)), 16*1024, stbl, physclick );


	/* Map all and protect from top of vector table to etext. */
	for ( i = pnum(0), pde = kvtopde(0); i < size; i++, pde++ ) {
		pg_setpfn(pde, i);
		pg_setvalid(pde);
#ifdef TEXTRO
		if ((i >= (BUGRAMSZ+1))  &&  (i < btoct((int)&etext)))
			pg_setcw(pde);
#endif
	}
	for (; i < 16*1024; i++, pde++ ) {   /* Set up VME io area. */
		pg_setpfn(pde, i);
		pg_setci(pde);
		pg_setvalid(pde);
	}

	/*
	 * Set up system domain 0 dynamic kernel virtual space.
	 */

	/* Just build page tables, no mapping yet */
	physclick = mkpgtbl( pnum(doff(SYSSEGS)), size, stbl, physclick );

	kptbl = kvtopde(SYSSEGS);	/* set table address */
	mmuupde = kptbl;		/* set U-Vector pde table address */

	/* Initialize the map used to dynamically allocate
	 * kernel virtual space. Skip room for UBLOCK.
	 */

	
	mapinit(sptmap, v.v_sptmap);
	mfree(sptmap, size-USIZE, btoc(SYSSEGS)+USIZE);

	/* 
	 * Enable mapping
	 */

	mmu_setup();
	mmu_enable();
	return( physclick);
}


/*
 * loadstbl(up, prp, change)
 *
 * Change the content of a process's segment descriptor entries (pointer tbl):
 *    	change > 0 -> load sdt table using the affected entries of prp->r_list.
 *    	change = 0 -> invalidate all sdt table entries for this prp.
 *	change < 0 -> invalidate the affected sdt table entries. 
 *
 * Region data always represents the new state when loadstbl() is called. 
 *  Exception: Special case when (change == 0), then invalidate all entries.
 *   In this case, region has not change to new state or size.
 */

loadstbl(up, prp, change)
user_t		*up;
preg_t		*prp;
register int	change;
{
	register reg_t	*rp;
	register sde_t	*st;
	register proc_t *p;
	int	seg;	/* first segment to be modified (pttbl index) */
	int	rseg;	/* first segment to be modified (rlist index) */
	int	lim;	/* first segment not to be modified */
	int	newend, oldend, lowend, highend;
	int	osize;

	p = up->u_procp;
	rp = prp->p_reg;

	osize = btoct(prp->p_regva) + rp->r_pgoff;

	if (prp->p_type != PT_STACK) {
		newend = osize + rp->r_pgsz;
		if (change)
			oldend = newend - change;
		else
			oldend = osize;
	} else {
		newend = osize;
		if (change)
			oldend = newend + change;
		else
			oldend = osize + rp->r_pgsz;
	}

	lowend  = min(newend, oldend);
	highend = max(newend, oldend);


	lim = ctos(highend);			/* round up */

	if (change <= 0) {	/* shrinking region,  clear sde's */
		seg = ctos(lowend - rp->r_pgoff);	/* round up */
		st = &p->p_pttbl[seg];		/* first seg to change */
		for (; seg < lim; seg++, st++) {	
			pt_clrvalid(st);	
			pt_setpfn(st, 0);
		}
	} else {			/* expanding region, set up sde's */
		seg  = ctost(lowend - rp->r_pgoff);	/* round down */
		st = &p->p_pttbl[seg];		/* first seg to change */
		rseg = ctost(lowend - rp->r_pgoff - btoc(prp->p_regva) );
		for (; seg < lim; seg++, rseg++, st++) {    
			pt_setup(st);
			pt_setpfn(st, kvtopfn(rp->r_list[rseg]));
			pt_setvalid(st,PT_VSD);
			if (prp->p_flags & PF_RDONLY)
				pt_setprot(st);
		}
	}
	mmuflush();
}


/*
 * mmucswtch(p)
 *
 * Sets up the MMU to switch to process p.
 * Requires a change of the single descriptor in the 
 * user domain table (udtbl).
 * The MMU must be properly flushed after the change.
 */
mmucswtch(p)
register proc_t	*p;
{
	register int	ipl;

	ASSERT(p->p_pttbl != NULL);
	pt_setpfn(udtbl,kvtopfn(p->p_pttbl));
	pt_setvalid(udtbl,PT_VLD);
	/*
	 * mmuflush not required here as long as resume
	 * must flush using mmuflush.  With PMMU the USERPROG
	 * and USERDATA function code related accesses should
	 * be flushed here.
	 */
}



/*	The one and only MMB flush option is to flush the entire ATC
 *	by rewriting the root pointer.
 *	Note that kvtophys(fcltbl) would be appropriate but we know
 *	the kernel is mapped 1:1 so the kvtophys() is skipped
 *	...we have enough overhead already.
 */

/*
 * mmuflush() flushes entire MMU.
 * Probably wouldn't need to flush supervisor program space.
 */
mmuflush()
{
	mmbstore(MMB_RP,fcltbl);
}


/*
 * mkpgtbl( firstclick, size, stbl, nextfree )
 *
 * Makes page tables as required to map the specified 
 * virtual address range from firstclick for size clicks 
 * within a domain whose segment table is pointed to by stbl,
 * taking clicks for tables from a zeroed click heap beginning at nextfree. 
 *
 * Returns nextfree, the next free click for tables.
 * The new page table is invalidated by default since the heap is zeroed.
 */
int
mkpgtbl( firstclick, size, stbl, nextfree )
int	firstclick;	/* click offset into domain */
int	size;
sde_t	*stbl;		/* segment table */
int	nextfree;
{
	sde_t *sde, *lastsde;

	ASSERT( firstclick + size <= 32*1024);
	ASSERT( size > 0 );

	/* For each segment descriptor within the range */

	lastsde = stbl + ctost(firstclick + size -1);
	for ( sde = stbl + ctost(firstclick); sde <= lastsde; sde++ ) {

		if ( !pt_tstvalid(sde,PT_VSD) ) { /* if the sde is not valid */

			/* allocate and attach page table */
			setsde(sde, PGTLIM, pnum((pde_t *)ctob(nextfree++)), PT_VSD);
		}
	}

	return( nextfree );
}



/*
 * setsde(sde, lim, pfn, val )
 *
 * Sets default values, limit and pfn for specified sde.
 * If val is set, validates the sde using the value of val.
 */
setsde(sde, lim, pfn, val )
sde_t	*sde;
int	lim;
int	pfn;
int	val;
{
	pt_setup(sde);
	pt_setlim(sde,lim);
	pt_setpfn(sde,pfn);
	if( val )
		pt_setvalid(sde,val);
}
