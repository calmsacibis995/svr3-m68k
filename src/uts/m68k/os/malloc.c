/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/malloc.c	10.3"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/map.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"

/*
 * Allocate 'size' units from the given map.
 * Return the base of the allocated space.
 * In a map, the addresses are increasing and the
 * list is terminated by a 0 size.
 * The swap map unit is 512 bytes.
 * Algorithm is first-fit.
 */
unsigned int
malloc(mp, size)
register struct map *mp;
register int size;
{
	register unsigned int a;
	register struct map *bp;

	if (size == 0)
		return(NULL);
	ASSERT(size > 0);
	for (bp = mapstart(mp); bp->m_size; bp++) {
		if (bp->m_size >= size) {
			a = bp->m_addr;
			bp->m_addr += size;
			if ((bp->m_size -= size) == 0) {
				do {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
				} while ((bp-1)->m_size = bp->m_size);
				mapsize(mp)++;
			}
			return(a);
		}
	}
	return(0);
}

/*
 * Free the previously allocated space a
 * of size units into the specified map.
 * Sort a into map and combine on
 * one or both ends if possible.
 */
mfree(mp, size, a)
register struct map *mp;
register int	size;
register unsigned int a;
{
	register struct map *bp;
	register unsigned int t;

	if (size == 0)
		return;
	ASSERT(size > 0);
	bp = mapstart(mp);
	for (; bp->m_addr<=a && bp->m_size!=0; bp++);
	if (bp>mapstart(mp) && (bp-1)->m_addr+(bp-1)->m_size == a) {
		(bp-1)->m_size += size;
		if(bp->m_addr){		/* m_addr==0 end of map table */
			ASSERT(a+size <= bp->m_addr);
			if (a+size == bp->m_addr) {

				/* compress adjacent map addr entries */
				(bp-1)->m_size += bp->m_size;
				while (bp->m_size) {
					bp++;
					(bp-1)->m_addr = bp->m_addr;
					(bp-1)->m_size = bp->m_size;
				}
				mapsize(mp)++;
			}
		}
	} else {
		if (a+size == bp->m_addr && bp->m_size) {
			bp->m_addr -= size;
			bp->m_size += size;
			ASSERT(bp == mapstart(mp)  ||
				((bp - 1)->m_addr + (bp - 1)->m_size) <
				 bp->m_addr);
		} else {
			ASSERT(size);
			if (mapsize(mp) == 0) {
				cmn_err(CE_WARN,"mfree map overflow %x.\
	Lost %d items at %d\n",mp,size,a) ;
				return;
			}
			do {
				t = bp->m_addr;
				bp->m_addr = a;
				a = t;
				t = bp->m_size;
				bp->m_size = size;
				bp++;
			} while (size = t);
			mapsize(mp)--;
		}
	}
	if (mapwant(mp)) {
		mapwant(mp) = 0;
		wakeup((caddr_t)mp);
	}
}
