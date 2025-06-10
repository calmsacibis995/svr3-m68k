/*	Copyright (c) 1984 by Communication Machinery Corporation
 *
 *	This file contains material which is proprietary to
 *	Communication Machinery Corporation (CMC) and which
 *	may not be divulged without the written permission
 *	of CMC.
 */

#include <sys/CMC/ring.h>
#include <sys/CMC/enp.h>

/* 
 * routines to synchronize enp and host 
 */

ringinit( rp,size )
register RING *rp;
{
	rp->r_rdidx = rp->r_wrtidx= 0;
	rp->r_size = size;
}

ringempty( rp )
register RING *rp;
{
	return( rp->r_rdidx == rp->r_wrtidx );
}

ringfull( rp )
register RING *rp;
{
	register short idx;

	idx = (rp->r_wrtidx + 1) & (rp->r_size-1);
	return( idx == rp->r_rdidx );
}

ringput( rp,v )
register RING *rp;
{
	register int idx;

	idx = (rp->r_wrtidx + 1) & (rp->r_size-1);
	if( idx != rp->r_rdidx )
	{
		enp_assign(rp->r_slot[ rp->r_wrtidx ], v);
		rp->r_wrtidx = idx;
		return( 1 );
	}
	return( 0 );
}

ringget( rp )
register RING *rp;
{
	register int i = 0;

	if( rp->r_rdidx != rp->r_wrtidx )
	{
		i = enp_rval(rp->r_slot[ rp->r_rdidx ]);
		rp->r_rdidx = (rp->r_rdidx+1) & (rp->r_size-1);
	}
	return( i );
}

fir( rp )
register RING *rp;
{
	if( rp->r_rdidx != rp->r_wrtidx )
		return( enp_rval(rp->r_slot[ rp->r_rdidx ]) );
	else
		return( 0 );
}
