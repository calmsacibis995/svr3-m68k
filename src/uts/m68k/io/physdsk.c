/*Copyright (c) 1984 AT&T	*/
/*	All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/physdsk.c	10.4"
/* 
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/region.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/debug.h"

short physlock;				/* controls use of "physbuf" buffer */
short physwanted;			/* set while sleeping on physbuf */

#define PHYSBSIZE (64 * 1024)		/* 64K physio buffer */
char physbuf[PHYSBSIZE];

caddr_t
getphysbuf(size)
unsigned size;
{
	if (size > PHYSBSIZE)
		return(0);
	spl6();
	while(physlock) {		/* wait for availability of "physbuf" */
		physwanted=1;
		sleep((caddr_t)physbuf, PRIBIO);
	}
	physlock = 1;
	spl0();
	return(physbuf);
}

freephysbuf(addr, size)
caddr_t addr;
unsigned size;
{
	physlock = 0;
	if (physwanted){
		physwanted = 0;
		wakeup((caddr_t)physbuf);
	}
}

/*
 * Calls the designated strategy routine to 
 * perform the I/O transfer described in the supplied bp.
 * A large static buffer is used to store and forward consecutive disk
 * data to/from physically discontiguous pages of user memory rather than
 * perform a separate disk request for each disjoint section of user memory.
 */

dma_breakup(strat, bp, sectsize)
int (*strat)(); 		/* address of device driver strategy routine */
register struct buf *bp;	/* buffer header from physical buffer list */
int sectsize; 			/* sector size */
{
	register caddr_t bufaddr;	/* kernel buffer address */
	register int count;		/* user requested byte count */
	register int rw;

	count = bp->b_bcount;
	rw = bp->b_flags & B_READ;

	if ((count & (sectsize - 1)) || ((bufaddr = getphysbuf(count)) == 0)) {
		bp->b_flags |= B_ERROR | B_DONE;
  		bp->b_error = ENXIO;
		return;
	}
	bp->b_un.b_addr = bufaddr;

	if (rw == B_READ) {
		doio(strat, bp);
		iomove(bufaddr, count, rw);
	}
	else {
		iomove(bufaddr, count, rw);
		doio(strat, bp); 
	}

	freephysbuf(bufaddr, count);
}

doio(strat, bp)
int (*strat)();
struct buf *bp;
{
	bp->b_flags &= ~(B_DONE | B_ERROR);

	(*strat)(bp);
	spl6();
	while ((bp->b_flags & B_DONE) == 0) {
		sleep(bp, PRIBIO);
	}
	spl0();
}
