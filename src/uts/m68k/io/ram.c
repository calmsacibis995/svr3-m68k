/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/* @(#)ram.c	6.2	   */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/var.h"

extern caddr_t bsmear();	/* zeros a block of ram, catches buserr */
/*
 * RAM configuration assumptions.
 * 	One contiguous block of memory expected.
 *	Minimum granuality of memory assumed to be 1k.
 *	It is found in the address range 0-12mb.
 */
#define MAXPHYSCL	12*1024	
#define MINMEMSIZ	1

/*
 * raminit(physclick) - initializes memory.
 *	Zeros memory beginning at physclick to end of contiguous ram.
 *	Returns click number of last physical click found.
 */

int
raminit(physclick)
int	physclick;	/* first click to test and zero */
{
	register unsigned siz, high, low, lastclick, ram_end;

	lastclick = physclick - 1;
	low = 0;
	/*
		Don't go beyond the end of memory
		as defined in dfile - maxpmem (v.v_maxpmem),
		as it will clear the area that should be
		loaded with the ramdisk image.
		v.v_maxpmem is set to a non-zero value
		if a ramdisk is to be installed. This will
		define the end of user/kernel memory and the start
		of ramdisk memory.
	*/
	high = MINMEMSIZ;
	ram_end = (v.v_maxpmem ? v.v_maxpmem : MAXPHYSCL);

    	/* locate and initialize ram */
	for ( ; high <= ram_end; low = high, high += MINMEMSIZ ) {
		if( high < physclick )
			continue;		/* skip past physclick */
		if( low < physclick )
			low = physclick;	/* shrink first chunk */
		siz = high - low;
		
		/* zero chunk of memory */
		if( btoct( bsmear(ctob( low ), ctob( siz ))) != high )
			break;			/* thats all */

		lastclick += siz;
	}
	return( lastclick );
}
