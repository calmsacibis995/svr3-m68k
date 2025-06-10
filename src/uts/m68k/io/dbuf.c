/*
 *	dbuf.c: double buffering module.
 *
 *	%W%
 *
 *	This module has been designed and implemented to support
 *	double buffering for drivers, like the MVME350 Steamer Tape
 *	Controller driver, that require it. This module supports
 *	asynchronous write and synchronous read.
 *
 *	This module defines the following procedures:
 *
 *	struct dbuf *dbufopen(size,dev,strategy)	open double buffers
 *	unsigned size;		size of buffer(s)
 *	int	dev;		device minor number
 *	int (*strategy)();	routine to call when buffer full(empty)
 *
 *	dbufclose(dbp)		close dbuf
 *	struct dbuf *dbp;	pointer returned by dbufopen
 *
 *	dbufread(dbp)		read from buffer
 *	struct dbuf *dbp;	pointer returned by dbufopen
 *
 *	dbufwrite(dbp)		write to buffer
 *	struct dbuf *dbp;	pointer returned by dbufopen
 *
 *	dbufioctl(dbp,cmd,arg)	ioctl function
 *	struct dbuf *dbp;	pointer returned by dufopen
 *	int	cmd;		ioctl command (see dbuf.h)
 *	int	arg;		argument to set/address to get
 */

#include "sys/param.h"
#include "sys/fs/s5param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/dbuf.h"
#include "sys/cmn_err.h"

#define	DEBUG
#undef	DEBUG

#ifdef	DEBUG
#define	STATIC
#else
#define	STATIC	static
#endif

#define	DBUFDELAY	10*HZ
#undef	DBUFDELAY

#define	B_EOF	B_DONE
#define	B_FLUSH	B_FORMAT
#define	B_RETRY	B_AGE

#define	OTHER(dbp,dcp)	((dcp == &dbp->d_buf1) ? &dbp->d_buf2 : &dbp->d_buf1)

/*
 *	dbuf: buffer control headers
 *
 *	declared in space.h
 */

extern struct dbuf dbuf[];
extern int	dbuf_cnt;
extern int	dbuf_szstatic;
extern char	dbuf_static[];
extern char	dbuf_busystatic[];

/*
 *	dbufget: reserve dbuf entry
 */

STATIC dbufget(dbp)
register struct dbuf *dbp;
{
	while( dbp->d_flags&B_BUSY ) {
		dbp->d_flags |= B_WANTED;
		sleep( &dbp->d_flags, PRIBIO+1 );
	}
	dbp->d_flags |= B_BUSY;
}

/*
 *	dbufrel: release dbuf entry
 */

STATIC dbufrel(dbp)
register struct dbuf *dbp;
{
	dbp->d_flags &= ~B_BUSY;
	if( dbp->d_flags&B_WANTED ) {
		dbp->d_flags &= ~B_WANTED;
		wakeup( &dbp->d_flags );
	}
}

/*
 *	dbufalloc: allocate buffers
 */

STATIC dbufalloc(dcp,asize)
register struct dbufctl *dcp;
register asize;
{	register char *loc;
	register size;
#ifdef	DBUFDELAY
	register sz;
#endif
	extern char *getcpages();

	asize = btoc(asize);	/* convert to clicks */
	size = asize;

#ifdef	DBUFDELAY
	sz = size;
#endif

	for(;;) {
		if (size <= btoct(dbuf_szstatic)) {
			register int i;		/* get a static buffer */
			register char *p = dbuf_static;

			loc = NULL;
			splhi();
			for (i = 0; i < (dbuf_cnt*2); i++) {
				if (dbuf_busystatic[i] == 0) {
					dbuf_busystatic[i]++;
					spl0();
					loc = p;
					break;
				}
				p += dbuf_szstatic;
			}
			if (loc == NULL)
				cmn_err(CE_PANIC, "no static dbuf available");
			break;
		}
		loc = getcpages(size, 0);
		if (loc != NULL)
			break;

		if( size <= btoct(DBUFMIN) ) {
#ifdef	DBUFDELAY
			delay(DBUFDELAY);	/* wait */
			if( issig() ) {
				u.u_error = ENXIO;
				return(1);
			}
			size = sz;
#else
			u.u_error = ENXIO;
			return(1);
#endif
		} else
			size /= 2;	/* look for half as much */
	}

#ifdef	DEBUG
	if (size != asize)
		cmn_err(CE_NOTE, "dbufalloc requested %d contiguous pages, got %d pages.", asize, size);
#endif
	dcp->d_addr = loc;
	dcp->d_size = ctob(size);
	dcp->d_seek = NULL;
	dcp->d_eaddr = NULL;
	return(0);
}

/*
 *	dbuffree: free buffers
 */

STATIC dbuffree(dcp)
register struct dbufctl *dcp;
{
	register int i;
	register char *p = dbuf_static;

	for (i = 0; i < (dbuf_cnt*2); i++) {	/* check for static buffer */
		if (dcp->d_addr == p) {
			dbuf_busystatic[i] = 0;
			return;
		}
		p += dbuf_szstatic;
	}

	/* found no match in static buffers, must be dynamic */
	freecpages(dcp->d_addr, btoct(dcp->d_size));
}

/*
 *	dbufopen: open dbuf
 */

struct dbuf *dbufopen(size,dev,strategy)
register unsigned size;		/* size of buffer(s) (in bytes) */
register dev;			/* major+minor device */
int (*strategy)();		/* routine to call when buffer full(empty) */
{	register struct dbuf *dbp;
	register struct dbuf *nentry = NULL;
	register n;
	static inited = 0;

	if( inited == 0 ) {
		inited = 1;
		for( n=0; n<dbuf_cnt; n++ )
			/* initialize d_dev field */
			dbuf[n].d_dev = (-1);
	}

	if( size > DBUFMAX )
		size = DBUFMAX;

	for( dbp = &dbuf[0], n=0; n<dbuf_cnt; n++, dbp++ ) {
		if( dbp->d_strat == NULL )
			nentry = dbp;
		if( dbp->d_dev == dev )
			return(dbp);	/* already buffered */
	}

	if( nentry == NULL ) {
		u.u_error = ENXIO;
		return(NULL);
	}
	dbp = nentry;

	if( dbufalloc(&dbp->d_buf1,size) )
		return(NULL);

	if( dbufalloc(&dbp->d_buf2,size) ) {
		dbuffree(&dbp->d_buf1);
		return(NULL);
	}

	dbp->d_buf = NULL;
	dbp->d_flags = 0;
	dbp->d_dev = dev;
	dbp->d_sbuf.b_flags = 0;
	dbp->d_sbuf.b_proc = u.u_procp;
	dbp->d_sbuf.b_dev = minor(dev);
	dbp->d_sbuf.b_blkno = 0;
	dbp->d_sbuf.b_bcount = 0;
	dbp->d_strat = strategy;
	return(dbp);
}

/*
 *	dbufclose: close dbuf
 */

dbufclose(dbp)
register struct dbuf *dbp;	/* pointer returned by dbufopen */
{	register struct buf *bp = &dbp->d_sbuf;

	dbufget(dbp);
	if( dbp->d_strat != NULL ) { 
		if( dbp->d_flags&B_ERROR ) {
			dbp->d_flags &= ~B_ERROR;
		} else if(( dbp->d_flags&B_READ ) == 0 ) {
			dbp->d_flags |= B_FLUSH;
			dbufempty(dbp);
		} else if( bp->b_flags & B_BUSY) 	/* still reading */
			dbufwait(dbp);
		dbuffree(&dbp->d_buf1);
		dbuffree(&dbp->d_buf2);
		dbp->d_strat = NULL;
		dbp->d_buf = NULL;
		dbp->d_flags &= (B_BUSY|B_WANTED);
		dbp->d_dev = (-1);
	}
	dbufrel(dbp);
}

/*
 *	dbufwait: wait for I/O
 */

STATIC dbufwait(dbp)
register struct dbuf *dbp;

{	register struct buf *bp = &dbp->d_sbuf;
	register bcnt;

	iowait(bp);
	bcnt = bp->b_bcount-bp->b_resid;
	bp->b_blkno += bcnt/BSIZE;
	if( bp->b_error&B_ERROR ) {		/* set up retry */
		u.u_error = bp->b_error;
		dbp->d_flags |= B_ERROR;
		bp->b_bcount -= bcnt;
		bp->b_un.b_addr += bcnt;
		return(-1);			/* error */
	}
	bp->b_flags &= ~B_BUSY;
	return(bcnt);				/* no error */
}

/*
 *	dbuffill: fill (read into) buffer(s)
 */

STATIC dbuffill(dbp)
register struct dbuf *dbp;
{	register struct dbufctl *dcp;
	register struct buf *bp = &dbp->d_sbuf;
	register char *offptr;
	register bcnt;
	register thistime;

	/* if not in READ state, flush the buffer */
	if(( dbp->d_flags&B_READ ) == 0 ) {	/* Not READ state ? */
		dbp->d_flags |= B_FLUSH;
		dbufempty(dbp);			/* flush out write data */
		dbp->d_buf = NULL;
		dbp->d_flags |= B_READ;		/* Now in READ state */
		bp->b_blkno = 0;
	}


	/* current buffer empty ? */
	if(( dcp = dbp->d_buf ) != NULL ) {
		offptr = (char *)((unsigned)dcp->d_addr +
				u.u_offset - dcp->d_offset );

		if((dcp->d_seek >= dcp->d_eaddr)||(u.u_offset < dcp->d_offset)
		  ||(offptr >= dcp->d_eaddr)||(offptr < dcp->d_seek)) {

			/* this buffer now empty */
		emptyagain:
			dcp->d_seek = NULL;
			/* get other buffer */
			dcp = OTHER(dbp,dcp);
			if(( dcp->d_seek != NULL )/* something in this buffer */
			|| ( bp->b_flags&B_BUSY ))/* this buffer being filled */
				dbp->d_buf = dcp; /* ok, wait for this data */
			else {			  /* no buffered data here */
				dbp->d_buf = NULL;
				dcp = NULL;		/* no data */
			}
		} else
			dcp->d_seek = offptr;	/* adjust seek pointer */
	}

	/* no current buffer */
	if( dcp == NULL ) {
		/* if an EOF has been detected */
		if( dbp->d_flags&B_EOF ) {
			dbp->d_flags &= ~B_EOF;
			return(1);		/* at EOF */
		}
		/* start a read into buffer 1 */
		dcp = &dbp->d_buf1;
		dcp->d_seek = NULL;
		bp->b_error = 0;
		bp->b_flags = B_BUSY|B_READ|B_PHYS;
		bp->b_un.b_addr = (caddr_t)dcp->d_addr;
		bp->b_bcount = dcp->d_size;
		bp->b_blkno = u.u_offset/BSIZE;
		dcp->d_offset = u.u_offset&(~(BSIZE-1));
		(*(dbp->d_strat))(bp);
		thistime = 1;
		goto dowait;
	}

	/* I/O done yet ? */
	if( dcp->d_seek == NULL ) {
		thistime = 0;
	dowait:
		if(( bcnt = dbufwait(dbp) ) <= 0 ) {
			dbp->d_buf = NULL;
			return(1);		/* at EOF or ERROR */
		}
		dbp->d_buf = dcp;
		dcp->d_eaddr = dcp->d_addr + bcnt;
		if( bcnt < bp->b_bcount )
			dbp->d_flags |= B_EOF;

		if( ! thistime ) {
			offptr = (char *)((unsigned)dcp->d_addr +
					u.u_offset - dcp->d_offset );

			if((dcp->d_seek >= dcp->d_eaddr)
			  ||(u.u_offset < dcp->d_offset)
			  ||(offptr >= dcp->d_eaddr)||(offptr < dcp->d_seek))
				goto emptyagain;
			dcp->d_seek = offptr;
		} else
			dcp->d_seek = dcp->d_addr;
	}

	/*
	 *	At this point, there is data in the buffer to be read
	 *	and "dbp->d_buf != NULL".
	 *
	 *	Now do read ahead if possible.
	 */

	/* if at EOF */
	if( dbp->d_flags&B_EOF )
		return(0);	/* no read ahead wanted */

	/* compute offset of next buffer */
	bcnt = dcp->d_offset + (dcp->d_eaddr - dcp->d_addr);

	/* get other buffer */
	dcp = OTHER(dbp,dcp);

	/* other buffer empty ? */
	if(( dcp->d_seek == NULL ) && (( bp->b_flags&B_BUSY ) == 0 )) {
		/* start up next I/O */
		bp->b_error = 0;
		bp->b_un.b_addr = (caddr_t)dcp->d_addr;
		bp->b_bcount = dcp->d_size;
		bp->b_flags = B_BUSY|B_READ|B_PHYS;
		dcp->d_offset = bcnt;
		(*(dbp->d_strat))(bp);		/* get next I/O started */
	}
	return(0);
}

/*
 *	dbufread: read from buffer
 */

dbufread(dbp)
register struct dbuf *dbp;	/* pointer returned by dbufopen */
{	register struct dbufctl *dcp;
	register unsigned n;

	dbufget(dbp);

	if(( dbp->d_flags&B_ERROR ) || ( dbp->d_strat == NULL ))
		u.u_error = ENXIO;

	else while( u.u_count != 0 ) {
		if( dbuffill(dbp) )
			break;
		dcp = dbp->d_buf;
		if(( n = (dcp->d_eaddr - dcp->d_seek)) > u.u_count )
			n = u.u_count;
		if( copyout( dcp->d_seek, u.u_base, n ) )
			break;
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
		dcp->d_seek += n;
	}

	dbufrel(dbp);
}

/*
 *	dbufempty: empty buffers
 */

STATIC dbufempty(dbp)
register struct dbuf *dbp;

{	register struct dbufctl *dcp;
	register struct buf *bp = &dbp->d_sbuf;
	register bcnt;

	/* check for partial buffer in transit */
	if( dbp->d_flags&B_RETRY ) {
		dbp->d_flags &= ~B_RETRY;
		bp->b_flags = B_BUSY|B_PHYS;	/* implied B_WRITE */
		(*(dbp->d_strat))(bp);		/* do retry */

		if(( dcp = dbp->d_buf ) == NULL )
			cmn_err(CE_PANIC, "buffer write error");

	} else {
		/* if no data is buffered */
		if(( dcp = dbp->d_buf ) == NULL ) {
			/* if not flushing, then set up a buffer */
			if(( dbp->d_flags&B_FLUSH ) == 0 ) {
				dcp = &dbp->d_buf1;
				dbp->d_buf = dcp;
				dcp->d_seek = dcp->d_addr;
				dcp->d_eaddr = dcp->d_addr + dcp->d_size;
			}
			return(0);
		}

		/*
		 *	if the current buffer is not yet full,
		 *	and we are not flushing.
		 */

		if((dcp->d_seek<dcp->d_eaddr) && ((dbp->d_flags&B_FLUSH) == 0))
			return(0);
	}

	/*
	 *	If the buffer is busy, then we must wait until it is
	 *	done before we can write out the current buffer.
	 *	That is, the write operation for the previous I/O
	 *	is still in progress.
	 */

	if(( bp->b_flags&B_BUSY )&&(( bcnt = dbufwait(dbp)) < bp->b_bcount )) {
		dbp->d_flags |= B_ERROR;
		bp->b_bcount -= bcnt;
		bp->b_un.b_addr += bcnt;
		return(1);
	}

	if( ( bcnt = (dcp->d_seek - dcp->d_addr) ) == 0 )
		return(1);		/* buffer empty ... no I/O */

	/*
	 *	If buffer is not filled to block boundary, then pad.
	 */

	if( bcnt & (BSIZE-1)) {
		register n = BSIZE - (bcnt % BSIZE);

		bzero(dcp->d_seek,n);
		bcnt += n;
		dcp->d_seek += n;
	}
	bp->b_bcount = bcnt;
	bp->b_un.b_addr = (caddr_t)dcp->d_addr;
	bp->b_error = 0;
	bp->b_flags = B_BUSY;
	(*(dbp->d_strat))(bp);

	/*
	 *	We've started this I/O, only wait around if
	 *	this call is a flush.
	 */

	if( dbp->d_flags&B_FLUSH ) {
		if(( bcnt = dbufwait(dbp)) < bp->b_bcount ) {
			dbp->d_flags |= B_ERROR;
			bp->b_bcount -= bcnt;
			bp->b_un.b_addr += bcnt;
		}
		dbp->d_flags &= ~B_FLUSH;
	}

	/*
	 *	set up other buffer
	 */

	dcp = OTHER(dbp,dcp);
	dbp->d_buf = dcp;
	dcp->d_seek = dcp->d_addr;
	dcp->d_eaddr = dcp->d_addr + dcp->d_size;
	return(0);
}

/*
 *	dbufwrite: write to buffers
 */

dbufwrite(dbp)
register struct dbuf *dbp;	/* pointer returned by dbufopen */
{	register struct dbufctl *dcp;
	register unsigned n;

	dbufget(dbp);

	if(( dbp->d_flags&B_ERROR ) || ( dbp->d_strat == NULL ))
		u.u_error = ENXIO;
	else {

		if( dbp->d_flags&B_READ ) {	/* read state ? */
			dbp->d_flags &= ~B_READ;
			dbp->d_buf = NULL;
			dbp->d_buf1.d_seek = NULL;
			dbp->d_buf1.d_eaddr =
				dbp->d_buf1.d_addr+dbp->d_buf1.d_size;
			dbp->d_buf2.d_seek = NULL;
			dbp->d_buf2.d_eaddr =
				dbp->d_buf2.d_addr+dbp->d_buf2.d_size;
		}

		while(( u.u_count != 0 ) && ( dbufempty(dbp) == 0 )) {
			dcp = dbp->d_buf;
			if(( n = dcp->d_eaddr - dcp->d_seek ) > u.u_count )
				n = u.u_count;
			if( copyin( u.u_base, dcp->d_seek, n ) )
				break;
			u.u_offset += n;
			u.u_base += n;
			u.u_count -= n;
			dcp->d_seek += n;
		}
	}

	dbufrel(dbp);
}

/*
 *	dbufioctl: perform dbuf control
 */

dbufioctl(dbp,cmd,arg)
register struct dbuf *dbp;	/* pointer returned by dbufopen */
register cmd;
register arg;
{	long	minsize;
	register addr1;
	register addr2;
	register size1;
	register size2;

	if( dbp == NULL ) {
		u.u_error = EINVAL;
		return;
	}
		
	dbufget(dbp);

	switch(cmd) {

	case DBUFSET:
		if( arg > DBUFMAX )
			arg = DBUFMAX;

		if(( dbp->d_flags&B_READ ) == 0 ) {
			dbp->d_flags |= B_FLUSH;
			dbufempty(dbp);
		}

		/* I/O in progress? */
		if( dbp->d_sbuf.b_flags&B_BUSY )
			dbufwait(dbp);	/* wait until done */

		dbuffree(&dbp->d_buf1);
		dbuffree(&dbp->d_buf2);

		if( arg != 0 ) {
			if( dbufalloc(&dbp->d_buf1,arg) == 0 )  {
				if( dbufalloc(&dbp->d_buf2,arg) == 0 )
					break;

				dbuffree(&dbp->d_buf1);
			}
			u.u_error = ENXIO;
		}
		else
			dbp->d_dev = (-1);
		dbp->d_strat = NULL;
		dbp->d_buf = NULL;
		dbp->d_flags &= (B_BUSY|B_WANTED);
		break;
	
	case DBUFGET:
		if(( minsize = dbp->d_buf1.d_size ) > dbp->d_buf2.d_size )
			minsize = dbp->d_buf2.d_size;
		copyout( &minsize, arg, sizeof(minsize) );
		break;

	case DBUFRETRY:
		dbp->d_flags &= ~B_ERROR;
		dbp->d_flags |= B_RETRY;
		if( arg >= 0 )
			dbp->d_sbuf.b_blkno = arg;
		break;

	case DBUFFLUSH:
		dbufget(dbp);
		if( dbp->d_flags&B_READ )
			dbp->d_buf = NULL;
		else {
			dbp->d_flags |= B_FLUSH;
			dbufempty(dbp);
		}
		break;

	default:
		u.u_error = EINVAL;
		break;
	}

	dbufrel(dbp);
}
