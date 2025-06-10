/*	Copyright (c) 1986 Motorola
 *	All Rights Reserved	
 *
 *
 *	@(#)ramdisk.c	6.1
 *
 *	ramdisk.c: ramdisk driver
 *
 */

/*	INCLUDE files 	*/

#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/map.h"
#include "sys/immu.h"
#include "sys/var.h"


#define	DEBUG
/* #undef	DEBUG */



#ifdef	DEBUG		/* debugging code */

#define CLRMEM
#define	STATIC
#define	DEBUGPR(x,d,s)	{ if( x & ramdDebug ) ramdprint(d,s); }
#define PRINT(a,b)	{ if(DBPRINT & ramdDebug) printf(a,b);}

int	ramdDebug = 0;

#define	DBINIT		(1<<0)
#define	DBOPEN		(1<<1)
#define	DBCLOSE		(1<<2)
#define	DBSTRAT		(1<<3)
#define	DBREAD		(1<<4)
#define	DBWRITE		(1<<5)
#define	DBPRINT		(1<<6)

#else			/* no debugging code */

#define	STATIC	static
#define	DEBUGPR(x,d,s)
#define PRINT(a,b)

#endif

/*
 *	from io.h
 */

struct size
{
	daddr_t nbytes;		/* size of slice in bytes */
	daddr_t byteoff;	/* first physical byte in slice */
};

extern struct size ramd_sizes[];

/*
 *	conf.c
 */

extern	int	ramd_cnt;

/*
 *
 *	ramdinit: initialize the ramdisk driver
 */

ramdinit()
{
	register int maxpmem;
	
	DEBUGPR(DBINIT,0,"init");


	maxpmem = v.v_maxpmem;
	if(maxpmem)
	{
		/*
			maxpmem is a count
			byteoff is an address

			maxpmem - 1 would the address of the end of memory
				  in clicks
		*/
		if((maxpmem-1) >= btoc(ramd_sizes[0].byteoff))
		{
			/* not enough room for root and swap */
			ramd_sizes[0].byteoff = 0;
			ramd_sizes[1].byteoff = 0;
			printf("Not enough room for ramdisk\n");
		}
	}
	else
	{
		/* no memory, so no ramdisk */
		ramd_sizes[0].byteoff = 0;
		ramd_sizes[1].byteoff = 0;
	}
}


/*
 *	ramdopen: open a ramdisk device
 */

ramdopen(dev)
register dev;
{
	DEBUGPR(DBOPEN,dev,"open");

	/*
	 *	if the device is nonexistent, then error
	 */

	if( ramd_sizes[minor(dev)].byteoff == 0 ) {
		u.u_error = ENXIO;
		return;
	}

	DEBUGPR(DBOPEN,dev,"open successful");
}

/*
 *	ramdclose: close device
 */
ramdclose(dev)
{
	DEBUGPR(DBCLOSE,dev,"close");
}

/*
 *	ramdstrategy: perform an I/O
 */

ramdstrategy(bp)
register struct buf *bp;
{
	register struct size *sp = &ramd_sizes[minor(bp->b_dev)];
	register last;		/* last block in slice */
	register unsigned ram;
	register unsigned src;
	register unsigned dst;

	DEBUGPR(DBSTRAT,bp->b_dev,"strategy");

	last = sp->nbytes >> BSHIFT;

	PRINT("last: %x\n", last);
	PRINT("bp->b_blkno: %x\n", bp->b_blkno);

	if ((bp->b_blkno < 0) || (bp->b_blkno >= last)) {
		if ((bp->b_blkno == last) && (bp->b_flags&B_READ))
			bp->b_resid = bp->b_bcount;
		else {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}

	} else {
		ram = (unsigned)(sp->byteoff + (bp->b_blkno << BSHIFT));

		PRINT("ram: %x\n", ram);

		if(ram > (sp->byteoff+sp->nbytes))
		{
			PRINT("ram: %x > exceeded slice size\n", ram);
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}

		if(( last = bp->b_bcount) > sp->nbytes )
			last = sp->nbytes;

		PRINT("u.u_segflg: %x\n", u.u_segflg);
		PRINT("b_flags: %x\n", bp->b_flags);

		if( last > 0)
		{
			if(bp->b_flags & B_PHYS)
			{
				iomove(ram,last,
					((bp->b_flags&B_READ)?B_READ:B_WRITE));	
			}
			else
			{
				if(bp->b_flags & B_READ)
				{
					src = ram;
					dst = paddr(bp);
				}
				else
				{
					src = paddr(bp);
					dst = ram;
				}
				bcopy(src, dst, last);
			}
			bp->b_resid = bp->b_bcount - last;
		}
	}
	iodone(bp);
}

/*
 *	ramdread: character device read
 */

ramdread(dev)
register dev;
{
	register struct size *sp = &ramd_sizes[minor(dev)];

	DEBUGPR(DBREAD,dev,"read");

	/* guarantee block alignment of both offset and count */
	if ((u.u_offset & BMASK) || (u.u_count & BMASK)) {
		u.u_error = EINVAL;
		return;
	}

	if( physck( (sp->nbytes >> BSHIFT),B_READ))
		physio( ramdstrategy, 0, dev, B_READ );
}

/*
 *	ramdwrite: character device write
 */

ramdwrite(dev)
register dev;
{
	register struct size *sp = &ramd_sizes[minor(dev)];

	DEBUGPR(DBREAD,dev,"write");

	/* guarantee block alignment of both offset and count */
	if ((u.u_offset & BMASK) || (u.u_count & BMASK)) {
		u.u_error = EINVAL;
		return;
	}

	if( physck( (sp->nbytes >> BSHIFT),B_WRITE))
		physio( ramdstrategy, 0, dev, B_WRITE );
}

/*
 *	ramdprint: driver print procedure
 */

ramdprint(dev,s)
{	register x;
	x = spl6();
	printf("RAMDISK: %s on slice %d\n", s, minor(dev) );
	splx(x);
}

/*
 *	ramddump: dump routine
 */

ramddump()
{
}
