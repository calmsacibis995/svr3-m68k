/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/streamio.c	10.17"

#include "sys/types.h"
#include "sys/file.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/strstat.h"
#include "sys/inode.h"
#include "sys/var.h"
#include "sys/poll.h"
#include "sys/termio.h"
#include "sys/ttold.h"
#include "sys/systm.h"


/*
 * id value used to distinguish between different ioctl messages
 */
static long ioc_id;


/*
 *  Qinit structure and Module_info structures
 *        for stream head read and write queues
 */
int	strrput(), strwsrv();
struct 	module_info strm_info = { 0, "strrhead", 0, INFPSZ, STRHIGH, STRLOW};
struct  module_info stwm_info = { 0, "strwhead", 0, 0, 0, 0 };
struct	qinit strdata = { strrput, NULL, NULL, NULL, NULL, &strm_info, NULL};
struct	qinit stwdata = { NULL, strwsrv, NULL, NULL, NULL, &stwm_info, NULL};


/*
 * open a stream device
 */

stropen(ip, flag)
struct inode *ip;
{
	register struct stdata *stp;
	register struct file *fp;
	register queue_t *qp;
	register ok;

	if (!(ip->i_sptr)) {
		/*
		 * This inode isn't streaming, but another inode
		 * may refer to same device, so look for it in file
		 * table to avoid building 2 streams to 1 device.
		 */
		for (fp = file; fp < (struct file *)v.ve_file; fp++) {
			register struct inode *tip;

			if (fp->f_count) {
				tip = fp->f_inode;
				if ( (tip->i_ftype&IFMT) == IFCHR &&
				     tip->i_rdev == ip->i_rdev   && 
				     tip != ip) {
					ip->i_sptr = tip->i_sptr;
					break;
				}
			}
		}
	}

	/*
	 * If the stream alreay exists, wait for any open in progress
	 * to complete then call the open function of each module and
	 * driver in the stream.  Otherwise create the stream.
	 */
retry:
	if (stp = ip->i_sptr) {

		/*
		 * Waiting for stream to be created to device
		 * due to another open
		 */
		if (stp->sd_flag&STWOPEN) {
			if (sleep((caddr_t)stp, STOPRI|PCATCH)) {
				u.u_error = EINTR;
				return;
			}
			goto retry;  /* could be clone! */
		}

		if (stp->sd_flag&(STRHUP|STRERR)) {
			u.u_error = EIO;
			return;
		}

		stp->sd_flag |= STWOPEN;
		/*
		 * Open all modules and devices down stream to notify
		 * that another user is streaming.
		 * For modules, set the last argument to MODOPEN and
		 * do not pass any open flags.
		 */
		qp = stp->sd_wrq;
		while (qp = qp->q_next) { 
			if (qp->q_flag & QREADR) break;
			if ((*RD(qp)->q_qinfo->qi_qopen)( RD(qp), ip->i_rdev,
					(qp->q_next ? 0 : flag),
					(qp->q_next ? MODOPEN : 0) ) == OPENFAIL) {
				if (!u.u_error)	u.u_error = ENXIO;
				break;
			}
		}
		stp->sd_flag &= ~STWOPEN;
		wakeup((caddr_t)stp);
		return;
	} 

	/* 
	 * Not already streaming so create a stream to driver
	 */

	if (!(qp = allocq())) {
		printf("stropen:out of queues\n");
		u.u_error = ENOSR;
		return;
	}

	for (stp = streams; stp < &streams[v.v_nstream]; stp++)
		if (!(stp->sd_wrq)) {
			stp->sd_wrq = WR(qp);	/* allocates stream head */
			break;
		}

	if (stp >= &streams[v.v_nstream]) {
		strst.stream.fail++;
		printf("stropen: out of streams\n");
		freeq(qp);
		u.u_error = ENOSR;
		return;
	}

	/* 
	 * Initialize stream head
	 */
	BUMPUP(strst.stream);
	stp->sd_pgrp = 0;
	stp->sd_inode = ip;
	stp->sd_flag = 0;
	stp->sd_error = 0;
	stp->sd_wroff = 0;
	stp->sd_iocwait = 0;
	stp->sd_iocblk = NULL;
	stp->sd_pushcnt = 0;
	stp->sd_siglist = NULL;
	stp->sd_pollist = NULL;
	stp->sd_sigflags = 0;
	stp->sd_pollflags = 0;
	setq(qp, &strdata, &stwdata);
	qp->q_ptr = WR(qp)->q_ptr = (caddr_t)stp;
	stp->sd_flag |= STWOPEN;
	stp->sd_strtab = cdevsw[major(ip->i_rdev)].d_str;
	ip->i_sptr = stp;

	/*
	 * Used to find controlling tty
	 */
	if (!u.u_ttyp) stp->sd_flag |= CTTYFLG;


	/*
	 * Open driver and create stream to it (via qattach).  Device opens
	 * may sleep, but must set PCATCH if they do so that signals will
	 * not cause a longjump.  Failure to do this may result in the queues
	 * and stream head not being freed.
	 */
	ok = qattach(stp->sd_strtab, qp, ip->i_rdev, flag);

	/*
	 * Wake up others that are waiting for stream to be created.
	 */
	stp->sd_flag &= ~STWOPEN;
	wakeup((caddr_t)stp);
	if (!ok) {
		stp->sd_flag |= STRHUP;
		stp->sd_wrq = NULL; 		/* free stream */
		strst.stream.use--;

		freeq(qp);
		strst.queue.use--;

		ip->i_sptr = NULL;
		if (!u.u_error)
			u.u_error = ENXIO;
		return;
	}

	/*
	 * Assign process group if controlling tty
	 */
	if (u.u_ttyp && (stp->sd_flag&CTTYFLG)) {
		stp->sd_pgrp = *u.u_ttyp;
		u.u_ttyip = ip;
	}
	stp->sd_flag &= ~CTTYFLG;
}




/*
 * Close a stream.  
 * This is called from closef() on the last close of an open stream.
 * Strclean() will already have removed the siglist and pollist
 * information, so all that remains is to remove all multiplexor links
 * for the stream, pop all the modules (and the driver), and free the
 * stream structure.
 */

strclose(ip, flag)
struct inode *ip;
{
	register s;
	register struct stdata *stp;
	register queue_t *qp;
	mblk_t *mp, *tmp;
	int strtime();
	int id;

	ASSERT(ip->i_sptr);

	stp = ip->i_sptr;
	ip->i_sptr = NULL;

	munlinkall(stp, 1);

	qp = stp->sd_wrq;
	while (qp->q_next) {
		if (!(flag&FNDELAY)) {
			s = splstr();
			stp->sd_flag |= (STRTIME | WSLEEP);
			id = timeout(strtime,stp,STRTIMOUT*HZ);
			/*
			 * sleep until awakened by strwsrv() or strtime() 
			 */
			while((stp->sd_flag &STRTIME) && qp->q_next->q_count) 
				if (sleep((caddr_t)qp, STIPRI|PCATCH)) break;
			untimeout(id);
			stp->sd_flag &= ~(STRTIME | WSLEEP);
			splx(s);
		}
		qdetach(RD(qp->q_next), 1, flag);
	}
	flushq(qp, FLUSHALL);
	qp = RD(qp);
	s = splstr();
	mp = qp->q_first;
	qp->q_first = qp->q_last = NULL;
	qp->q_count = 0;

	if (stp->sd_iocblk) {
		freemsg(stp->sd_iocblk);
		stp->sd_iocblk = NULL;
	}
	stp->sd_wrq = NULL;
	strst.stream.use--;

	/* free stream head queue pair */
	freeq(qp);
	strst.queue.use--;

	splx(s);

	while (mp) {
		tmp = mp->b_next;
		if (mp->b_datap->db_type == M_PASSFP) 
			closef(((struct strrecvfd *)mp->b_rptr)->f.fp);
		freemsg(mp);
		mp = tmp;
	}
}



/*
 * Clean up after a process when it closes a stream.  This is called
 * from closef for all closes, whereas strclose is called only for the 
 * last close on a stream.  The pollist and siglist are scanned for entries
 * for the current process, and these are removed.
 */

strclean(ip)
struct inode *ip;
{
	register struct strevent *sep, *psep, *tsep;
	struct stdata *stp;

	stp = ip->i_sptr;
	psep = NULL;
	sep = stp->sd_siglist;
	while (sep) {
		if (sep->se_procp == u.u_procp) {
			tsep = sep->se_next;
			if (psep) psep->se_next = tsep;
			else stp->sd_siglist = tsep;
			sefree(sep);
			sep = tsep;
		}
		else {
			psep = sep;
			sep = sep->se_next;
		}
	}

	psep = NULL;
	sep = stp->sd_pollist;
	while (sep) {
		if (sep->se_procp == u.u_procp) {
			tsep = sep->se_next;
			if (psep) psep->se_next = tsep;
			else stp->sd_pollist = tsep;
			sefree(sep);
			sep = tsep;
		}
		else {
			psep = sep;
			sep = sep->se_next;
		}
	}
}


/*
 * Read a stream according to the mode flags in sd_flag:
 *
 * (default mode)              - Byte stream, msg boundries are ignored
 * RMSGDIS (msg discard)       - Read on msg boundries and throw away 
 *                               any data remaining in msg
 * RMSGNODIS (msg non-discard) - Read on msg boundries and put back
 *		                 any remaining data on head of read queue
 *
 * Consume readable messages on the front of the queue until u.u_count
 * is satisfied, the readable messages are exhausted, or a message
 * boundary is reached in a message mode.  If no data was read and
 * the stream was not opened with the NDELAY flag, block until data arrives.
 * Otherwise return the data read and update the count.
 *
 * In default mode a 0 length message signifies end-of-file and terminates
 * a read in progress.  The 0 length message is removed from the queue
 * only if it is the only message read (no data is read).
 *
 * Attempts to read an M_PROTO or M_PCPROTO message results in an 
 * EBADMSG error return.
 */

strread(ip)
struct inode *ip;
{
	register s;
	register struct stdata *stp;
	register mblk_t *bp, *nbp;
	int n;
	char rflg;
	

	ASSERT(ip->i_sptr);
	stp = ip->i_sptr;

	if (stp->sd_flag&(STRERR|STPLEX)) {
		u.u_error = ((stp->sd_flag&STPLEX) ? EINVAL : stp->sd_error);
		return;
	}

	/* loop terminates when u.u_count == 0 */
	rflg = 0;
	for (;;) {
		s = splstr();
		while (!(bp = getq(RD(stp->sd_wrq)))) {
			if (rflg || (stp->sd_flag&STRHUP) || strwaitq(stp, READWAIT, u.u_fmode)) {
				splx(s);
				return;
			}
		}
		splx(s);

		if (qready()) runqueues();

		switch (bp->b_datap->db_type) {

		case M_DATA:

			if ((bp->b_wptr - bp->b_rptr) == 0) {
				/*
				 * if already read data put zero
				 * length message back on queue else
				 * free msg and return 0.
				 */
				if (rflg) 
					putbq(RD(stp->sd_wrq), bp);
				else
					freemsg(bp);
				return;
			}

			rflg = 1;
			while (bp && u.u_count) {
				if (n = min(u.u_count, bp->b_wptr - bp->b_rptr))
					iomove(bp->b_rptr, n, B_READ);
			
				if (u.u_error) {
					freemsg(bp);
					return;
				}

				bp->b_rptr += n;
				while (bp && (bp->b_rptr >= bp->b_wptr)) {
					nbp = bp;
					bp = bp->b_cont;
					freeb(nbp);
				} 
			}
	
	
			/*
			 * The data may have been the leftover of a PCPROTO,
			 * so if none is left reset the STRPRI flag just in case.
			 */
			if (bp) {
				/* 
				 * Have remaining data in message.
				 * Free msg if in discard mode.
				 */
				if (stp->sd_flag & RMSGDIS) {
					freemsg(bp);
					stp->sd_flag &= ~STRPRI;
				} else 
					putbq(RD(stp->sd_wrq),bp);

			} else 
				stp->sd_flag &= ~STRPRI;
	
			/*
			 * Check for signal messages at the front of the read
			 * queue and generate the signal(s) if appropriate.
			 * The only signal that can be on queue is M_SIG at
			 * this point.
			 */
			while (((bp = RD(stp->sd_wrq)->q_first)) &&
				(bp->b_datap->db_type == M_SIG)) {
				bp = getq(RD(stp->sd_wrq));
				switch (*bp->b_rptr) {
				case SIGPOLL:				
					if (stp->sd_sigflags & S_MSG) 
						strsendsig(stp->sd_siglist, S_MSG);
					break;

				default:
					if (stp->sd_pgrp)
						signal(stp->sd_pgrp, *bp->b_rptr);
					break;
				}
				freemsg(bp);
				if (qready()) runqueues();
			}

			if ((u.u_count == 0) || (stp->sd_flag&(RMSGDIS|RMSGNODIS)))
				return;
			continue;
		
		case M_PROTO:
		case M_PCPROTO:
		case M_PASSFP:
			/*
			 * Only data messages are readable.  
			 * Any others generate an error.
			 */
			u.u_error= EBADMSG;
			putbq(RD(stp->sd_wrq), bp);
			return;

		default:
			/*
			 * Garbage on stream head read queue
			 */
			ASSERT(0);
			freemsg(bp);
			break;
		}
	}
}



/*
 * Stream read put procedure.  Called from downstream driver/module
 * with messages for the stream head.  Data, protocol, and in-stream
 * signal messages are placed on the queue, others are handled directly.
 */

strrput(q, bp)
register queue_t *q;
register mblk_t *bp;
{
	register struct stdata *stp;
	register struct iocblk *iocbp;
	struct stroptions *sop;


	stp = (struct stdata *)q->q_ptr;

	ASSERT(!(stp->sd_flag & STPLEX));

	switch (bp->b_datap->db_type) {

	case M_DATA:
	case M_PROTO:
	case M_PCPROTO:
	case M_PASSFP:
		if (bp->b_datap->db_type == M_PCPROTO) {
			/*
			 * Only one priority protocol message is allowed at the
			 * stream head at a time.
			 */
			if (stp->sd_flag & STRPRI) {
				freemsg(bp);
				return;
			}
			stp->sd_flag |= STRPRI;
			if (stp->sd_sigflags & S_HIPRI)
				strsendsig(stp->sd_siglist, S_HIPRI);
			if (stp->sd_pollflags & POLLPRI) 
				strwakepoll(stp, POLLPRI);
		} 
		else if (!q->q_first) {
			if (stp->sd_sigflags & S_INPUT)
				strsendsig(stp->sd_siglist, S_INPUT);
			if (stp->sd_pollflags & POLLIN) 
				strwakepoll(stp, POLLIN);
		}

		/* 
		 * Wake sleeping read/getmsg
		 */
		if (stp->sd_flag & RSLEEP) {
			char oldpri;

			stp->sd_flag &= ~RSLEEP;
			oldpri = curpri;
			curpri = STIPRI; /* no preemption */
			wakeup((caddr_t)q);
			curpri = oldpri;
		}

		putq(q, bp);
		return;


	case M_ERROR:
		/* 
		 * An error has occured downstream, the errno is in the first
		 * byte of the message.
		 */
		if (*bp->b_rptr != 0) {
			stp->sd_flag |= STRERR;
			stp->sd_error = *bp->b_rptr;
			wakeup((caddr_t)q);	/* the readers */
			wakeup((caddr_t)WR(q));	/* the writers */
			wakeup((caddr_t)stp);	/* the ioctllers */

			strwakepoll(stp, POLLERR);
			
			bp->b_datap->db_type = M_FLUSH;
			*bp->b_rptr = FLUSHRW;
			qreply(q, bp);
			return;
		}
		freemsg(bp);
		return;

	case M_HANGUP:
		freemsg(bp);
		stp->sd_error = ENXIO;
		stp->sd_flag |= STRHUP;

		/*
		 * send signal if controlling tty
		 */
		if (stp->sd_pgrp)
			signal(stp->sd_pgrp, SIGHUP);

		wakeup((caddr_t)q);	/* the readers */
		wakeup((caddr_t)WR(q));	/* the writers */
		wakeup((caddr_t)stp);	/* the ioctllers */

		/*
		 * wake up read, write, and exception pollers and
		 * reset wakeup mechanism.
		 */
		strwakepoll(stp, POLLHUP);
		return;


	case M_SIG:
		/*
		 * Someone downstream wants to post a signal.  The
		 * signal to post is contained in the first byte of the
		 * message.  If the message would go on the front of
		 * the queue, send a signal to the process group
		 * (if not SIGPOLL) or to the siglist processes
		 * (SIGPOLL).  If something is already on the queue,
		 * just enqueue the message.
		 */
		if (q->q_first) {
			putq(q, bp);
			return;
		}
		/* flow through */

	case M_PCSIG:
		/*
		 * Don't enqueue, just post the signal.
		 */
		switch (*bp->b_rptr) {
		case SIGPOLL:
			if (stp->sd_sigflags & S_MSG) 
				strsendsig(stp->sd_siglist, S_MSG);
			break;

		default:
			if (stp->sd_pgrp)
				signal(stp->sd_pgrp, *bp->b_rptr);
			break;
		}
		freemsg(bp);
		return;

	case M_FLUSH:
		/*
		 * Flush queues.  The indication of which queues to flush
		 * is in the first byte of the message.  If the read queue 
		 * is specified, then flush it.
		 */
		if (*bp->b_rptr & FLUSHR) {
			mblk_t *mp, *tmp;
			register s;

			s = splstr();
			mp = q->q_first;
			q->q_first = q->q_last = NULL;
			q->q_count = 0;
			q->q_flag &= ~(QFULL | QWANTW);
			splx(s);
			while (mp) {
				tmp = mp->b_next;
				if (mp->b_datap->db_type == M_PASSFP) 
					closef(((struct strrecvfd *)mp->b_rptr)->f.fp);
				freemsg(mp);
				mp = tmp;
			}
		}
		if (*bp->b_rptr & FLUSHW) {
			*bp->b_rptr &= ~FLUSHR;
			qreply(q, bp);
			return;
		}
		freemsg(bp);
		return;

	case M_IOCACK:
	case M_IOCNAK:
		iocbp = (struct iocblk *)bp->b_rptr;
		/*
		 * if not waiting for ACK or NAK then just free msg
		 * if incorrect id sequence number then just free msg
		 * if already have ACK or NAK for user then just free msg
		 */
		if ((stp->sd_flag&IOCWAIT)==0 || stp->sd_iocblk || (stp->sd_iocid != iocbp->ioc_id)) {
			freemsg(bp);
			return;
		}

		/*
		 * assign ACK or NAK to user and wake up
		 */
		stp->sd_iocblk = bp;
		wakeup((caddr_t)stp);
		return;

	case M_SETOPTS:
		/*
		 * Set stream head options (read option, write offset,
		 * min/max packet size, and/or high/low water marks for 
		 * the read side only).
		 */

		ASSERT((bp->b_wptr - bp->b_rptr) == sizeof(struct stroptions));
		sop = (struct stroptions *)bp->b_rptr;
		if (sop->so_flags & SO_READOPT) {
			switch (sop->so_readopt) {
			case RNORM: 
				stp->sd_flag &= ~(RMSGDIS | RMSGNODIS);
				break;
			case RMSGD:
				stp->sd_flag = (stp->sd_flag & ~RMSGNODIS) | RMSGDIS;
				break;
			case RMSGN:
				stp->sd_flag = (stp->sd_flag & ~RMSGDIS) | RMSGNODIS;
				break;
			}
		}
				
		if (sop->so_flags & SO_WROFF) stp->sd_wroff = sop->so_wroff;
		if (sop->so_flags & SO_MINPSZ) q->q_minpsz = sop->so_minpsz;
		if (sop->so_flags & SO_MAXPSZ) q->q_maxpsz = sop->so_maxpsz;
		if (sop->so_flags & SO_HIWAT) q->q_hiwat = sop->so_hiwat;
		if (sop->so_flags & SO_LOWAT) q->q_lowat = sop->so_lowat;

		freemsg(bp);

		if ((q->q_count <= q->q_lowat) && (q->q_flag & QWANTW)) {
			q->q_flag &= ~QWANTW;
			for (q = backq(q); q && !q->q_qinfo->qi_srvp; q = backq(q) );
			if (q) qenable(q);
		}

		return;

	/*
	 * The following set of cases deal with situations where two stream
	 * heads are connected to each other (twisted streams).  These messages
	 * should never originate at a driver or module.
	 */
	case M_BREAK:
	case M_CTL:
	case M_DELAY:
	case M_START:
	case M_STOP:
		freemsg(bp);
		return;

	case M_IOCTL:
		/*
		 * Always NAK this condition
		 * (makes no sense)
		 */
		bp->b_datap->db_type = M_IOCNAK;
		qreply(q, bp);
		return;

	default:
		ASSERT(0);
		freemsg(bp);
		return;
	}
}


/*
 * Send SIGPOLL signal to all processes registered on the given signal
 * list that want a signal for the specified event.
 */

strsendsig(siglist, event)
register struct strevent *siglist;
register event;
{
	struct strevent *sep;

	for (sep = siglist; sep; sep = sep->se_next) {
		if (sep->se_events & event)
			psignal(sep->se_procp, SIGPOLL);
	}
}


/*
 * Wake up all processes sleeping on a poll for the given events
 * on this stream.  POLLERR and POLLHUP cause a wakeup of all processes
 * regardless of the events they were looking for.  Remove all of
 * the event cells matching the given events from the pollist.
 */

strwakepoll(stp, events)
struct stdata *stp;
{
	register struct strevent *sep, *psep;
	register s;

	stp->sd_pollflags &= ~events;
	sep = stp->sd_pollist;
	psep = NULL;
	while (sep) {
		if ((sep->se_events & events) || (events & (POLLHUP|POLLERR|POLLNVAL))) {
			s = splstr();
			if (sep->se_procp->p_wchan == (caddr_t)&pollwait)
				setrun(sep->se_procp);
			else
				sep->se_procp->p_flag &= ~SPOLL;
			splx(s);
			if (psep){
				psep->se_next = sep->se_next;
				sefree(sep);
				sep = psep->se_next;
			} else  {
				stp->sd_pollist = sep->se_next;
				sefree(sep);
				sep = stp->sd_pollist;
			}
		} else {
			psep = sep;
			sep = sep->se_next;
		}
	}
}



/*
 * Write attempts to break the read request into messages conforming
 * with the minimum and maximum packet sizes set downstream.  
 *
 * Write will always attempt to get the largest buffer it can to satisfy the
 * message size. If it can not, then it will try up to 2 classes down to try
 * to satisfy the write. Write will not block if downstream queue is full and
 * O_NDELAY is set, otherwise it will block waiting for the queue to get room.
 * 
 * A write of zero bytes gets packaged into a zero length message and sent
 * downstream like any other message.
 *
 * If buffers of the requested sizes are not available, the write will
 * sleep until the buffers become available.
 *
 * Write (if specified) will supply a write offset in a message if it
 * makes sense. This can be specified by downstream modules as part of
 * a M_SETOPTS message. Write will not supply the write offset if it
 * cannot supply any data in a buffer. In other words, write will never
 * send down a empty packet due to a write offset.
 */

strwrite(ip)
struct inode *ip;
{
	register s;
	register struct stdata *stp;
	mblk_t *mp;
	struct strbuf mctl, mdata;
	short rmin, rmax;
	char waitflag;
	mblk_t *strmakemsg();

	ASSERT(ip->i_sptr);
	stp = ip->i_sptr;

	if (stp->sd_flag & (STRERR|STRHUP|STPLEX)) {
		u.u_error = ((stp->sd_flag&STPLEX) ? EINVAL : stp->sd_error);
		return;
	}

	mctl.len = -1;

	/*
	 * Check the min/max packet size constraints.  If min packet size
	 * is non-zero, the write cannot be split into multiple messages
	 * and still guarantee the size constraints. 
	 */
	rmin = stp->sd_wrq->q_next->q_minpsz;
	rmax = stp->sd_wrq->q_next->q_maxpsz;
	ASSERT(rmax);
	if (rmax == 0) return;
	if (rmax == INFPSZ) 
		rmax = strmsgsz;
	else
		rmax = min(strmsgsz, rmax);

	if ((rmin > 0) && ((u.u_count < rmin) || (u.u_count > rmax))) {
		u.u_error = ERANGE;
		return;
	}

	/*
	 * do until count satisfied or error
	 */
	waitflag = WRITEWAIT;
	do {
		s = splstr();
		while (!canput(stp->sd_wrq->q_next))
			if (strwaitq(stp, waitflag, u.u_fmode)) {
				splx(s);
				return;
			}
		splx(s);

		/*
		 * Determine the size of the next message to be
		 * packaged.  May have to break write into several
		 * messages based on max packet size.
		 */
		mdata.len = min(u.u_count, rmax);
		mdata.buf = u.u_base;

		if (!(mp = strmakemsg(&mctl, &mdata, stp->sd_wroff, 0))) return;

		u.u_base += mdata.len;
		u.u_count -= mdata.len;

		/*
		 * Put block downstream
		 */
		(*stp->sd_wrq->q_next->q_qinfo->qi_putp)(stp->sd_wrq->q_next, mp);
		waitflag |= NOINTR;
		if (qready()) runqueues();

	} while (u.u_count);
}



/*
 * Stream head write service routine.
 * Its job is to wake up any sleeping writers when a queue
 * downstream needs data (part of the flow control in putq and getq).
 * It also must wake anyone sleeping on a poll().
 * For stream head right below mux module, it must also invoke put procedure
 * of next downstream module
 */

strwsrv(q)
register queue_t *q;
{
	register struct stdata *stp;

	stp = (struct stdata *)q->q_ptr;

	ASSERT(!(stp->sd_flag & STPLEX));

	if (stp->sd_flag & WSLEEP) {
		char oldpri;

		stp->sd_flag &= ~WSLEEP;
		oldpri = curpri;
		curpri = STOPRI;
		wakeup((caddr_t)q);
		curpri = oldpri;
	}
	if (stp->sd_sigflags & S_OUTPUT) strsendsig(stp->sd_siglist, S_OUTPUT);
	if (stp->sd_pollflags & POLLOUT) strwakepoll(stp, POLLOUT);
}


/*
 * ioctl for streams
 */

strioctl(ip, cmd, arg, flag)
struct inode *ip;
int arg;
int flag;
{
	register struct stdata *stp;
	struct strioctl strioc;

	ASSERT(ip->i_sptr);
	stp = ip->i_sptr;

	if (stp->sd_flag & (STRERR|STPLEX)) {
		u.u_error = ((stp->sd_flag & STPLEX) ? EINVAL : stp->sd_error);
		return;
	}

	switch (cmd) {

	default:
		if (((cmd&IOCTYPE) == LDIOC) || 
		    ((cmd&IOCTYPE) == tIOC) ||
		    ((cmd&IOCTYPE) == TIOC) ) {

			/*
			 * The ioctl is a tty ioctl - set up strioc buffer 
			 * and call strdoioctl() to do the work.
			 */
			if (stp->sd_flag & STRHUP) {
				u.u_error = ENXIO;
				return;
			}
			strioc.ic_cmd = cmd;
			strioc.ic_timout = 0;

			switch (cmd) {
				case TCXONC:
				case TCSBRK:
				case TCFLSH:
				case TCDSET:
					strioc.ic_len = sizeof(int);
					strioc.ic_dp = (char *)&arg;
					strdoioctl(stp, &strioc, K_TO_K);
					return;

				case TCSETA:
				case TCSETAW:
				case TCSETAF:
					strioc.ic_len = sizeof(struct termio);
					strioc.ic_dp = (char *)arg;
					strdoioctl(stp, &strioc, U_TO_K);
					return;

				case LDSETT:
					strioc.ic_len = sizeof(struct termcb);
					strioc.ic_dp = (char *)arg;
					strdoioctl(stp, &strioc, U_TO_K);
					return;

				case TIOCSETP:
					strioc.ic_len = sizeof(struct sgttyb);
					strioc.ic_dp = (char *) arg;
					strdoioctl(stp, &strioc, U_TO_K);
					return;

				case TCGETA:
				case LDGETT:
				case TIOCGETP:
					strioc.ic_len = 0;
					strioc.ic_dp = (char *)arg;
					strdoioctl(stp, &strioc, U_TO_K);
					return;
			}
		}
		u.u_error = EINVAL;
		return;

	case I_STR:
		/*
		 * Stream ioctl.  Read in an strioctl buffer from the user
		 * along with any data specified and send it downstream.
		 * Strdoioctl will wait allow only one ioctl message at
		 * a time, and waits for the acknowledgement.
		 */

		if (stp->sd_flag & STRHUP) {
			u.u_error = ENXIO;
			return;
		}
		if (copyin((caddr_t)arg, (caddr_t)&strioc, sizeof(struct strioctl))) {
			u.u_error = EFAULT;
			return;
		}
		if (strioc.ic_len < 0 || strioc.ic_timout < -1) {
			u.u_error = EINVAL;
			return;
		}

		strdoioctl(stp, &strioc, U_TO_K);

		if (copyout((caddr_t)&strioc, arg, sizeof(struct strioctl)))
			u.u_error = EFAULT;
		return;


	case I_NREAD:
		/*
		 * return number of bytes of data in first message
		 * in queue in "arg" and return the number of messages
		 * in queue in return value
		 */
		{
			int size = 0;
			mblk_t *bp;

			if (bp = RD(stp->sd_wrq)->q_first)
				size = msgdsize(bp);
			if (copyout(&size, (int *)arg, sizeof(size)))
				u.u_error = EFAULT;
			u.u_rval1 = qsize(RD(stp->sd_wrq));
			return;
		}

	case I_FIND:
		/*
		 * get module name
		 */
		{
			char mname[FMNAMESZ+1];
			queue_t *q;
			int i;

			if (copyin((caddr_t)arg, mname, FMNAMESZ+1)) {
				u.u_error = EFAULT;
				return;
			}
	
			/*
			 * find module in fmodsw
			 */
			if ((i = findmod(mname)) < 0) {
				u.u_error = EINVAL;
				return;
			}
	
			u.u_rval1 = 0;
	
			/* look downstream to see if module is there */
			for (q = stp->sd_wrq->q_next; 
			     q && (fmodsw[i].f_str->st_wrinit != q->q_qinfo); 
			     q = q->q_next);

			u.u_rval1 = ( q ? 1 : 0);
			return;
		}

	case I_PUSH:
		/*
		 * Push a module
		 */

		{
			char mname[FMNAMESZ+1];
			int i;

			if (stp->sd_flag & STRHUP) {
				u.u_error = ENXIO;
				return;
			}
			if (stp->sd_pushcnt > nstrpush) {
				u.u_error = EINVAL;
				return;
			}
			
			/*
			 * get module name and look up in fmodsw
			 */
			if (copyin((caddr_t)arg, mname, FMNAMESZ+1)) {
				u.u_error = EFAULT;
				return;
			}
			if ((i = findmod(mname)) < 0) {
				u.u_error = EINVAL;
				return;
			}
	
			while (stp->sd_flag & STWOPEN) {
				if (sleep((caddr_t)stp, STOPRI|PCATCH)) {
					u.u_error = EINTR;
					return;
				}
				if (stp->sd_flag & (STRERR|STRHUP|STPLEX)) {
					u.u_error = ((stp->sd_flag&STPLEX) ? EINVAL : stp->sd_error);
					return;
				}
			}
			stp->sd_flag |= STWOPEN;

			/*
			 * Set up to test if the push creates a controlling tty.
			 */
			if (!u.u_ttyp) stp->sd_flag |= CTTYFLG;

			/*
			 * push new module and call its open routine via qattach
			 */
			if (!qattach(fmodsw[i].f_str, RD(stp->sd_wrq), ip->i_rdev, 0)) {
				if (!u.u_error)	u.u_error =  ENXIO;
			} else {
				/*
				 * if controlling tty established mark the 
				 * process group and tty inode pointer.
				 */
				if (u.u_ttyp && (stp->sd_flag&CTTYFLG)) {
					stp->sd_pgrp = *u.u_ttyp;
					u.u_ttyip = ip;
				}
				stp->sd_pushcnt++;
			}
			stp->sd_flag &= ~(CTTYFLG | STWOPEN);
			wakeup((caddr_t)stp);
			return;
		}


	case I_POP:
		/*
		 * Pop module ( if module exists )
		 */
		if (stp->sd_flag&STRHUP) {
			u.u_error = ENXIO;
			return;
		}
		if (stp->sd_wrq->q_next->q_next && 
		    !(stp->sd_wrq->q_next->q_next->q_flag & QREADR)) {
			qdetach(RD(stp->sd_wrq->q_next), 1, 0);
			stp->sd_pushcnt--;
			return;
		}
		u.u_error = EINVAL;
		return;


	case I_LOOK:
		/*
		 * Get name of first module downstream
		 * If no module (return error)
		 */
		{
			int i;

			for (i=0; i<fmodcnt; i++)
				if(fmodsw[i].f_str->st_wrinit==stp->sd_wrq->q_next->q_qinfo) {
					if (copyout(fmodsw[i].f_name,(char *)arg,FMNAMESZ+1))
						u.u_error = EFAULT;
					return;
				}
			u.u_error = EINVAL;
			return;
		}


	case I_LINK:
		/* 
		 * link a multiplexer 
		 */
		{
			struct file *fpdown;
			struct linkblk *linkblkp, *alloclink();
			struct stdata *stpdown;
			queue_t *rq;

			/*
			 * Test for invalid upper stream
			 */
			if (stp->sd_flag & STRHUP) {
				u.u_error = ENXIO;
				return;
			}
			if (!stp->sd_strtab->st_muxwinit) {
				u.u_error = EINVAL;
				return;
			}

			fpdown = getf(arg);
			if (u.u_error)	return;
	
			/*
			 * Test for invalid lower stream 
			 */
			if (!(stpdown = fpdown->f_inode->i_sptr) ||	
			    (stpdown == stp) ||
			    (stpdown->sd_flag & (STPLEX|STRHUP|STRERR|IOCWAIT)) ||
			    linkcycle(getendq(stp->sd_wrq), stpdown->sd_wrq) ) {
				u.u_error = EINVAL;
				return;
			}

			if (!(linkblkp = alloclink(getendq(stp->sd_wrq), 
					     stpdown->sd_wrq, fpdown - &file[0]))) {
				u.u_error = EAGAIN;
				return;
			}
			strioc.ic_cmd = I_LINK;
			strioc.ic_timout = 0;
			strioc.ic_len = sizeof(struct linkblk);
			strioc.ic_dp = (char *) linkblkp;
		
			/* Set up queues for link */
			rq = RD(stpdown->sd_wrq);
			setq(rq, stp->sd_strtab->st_muxrinit,
				stp->sd_strtab->st_muxwinit);
			rq->q_ptr = WR(rq)->q_ptr = NULL;
			rq->q_flag |= QWANTR;	
			WR(rq)->q_flag |= QWANTR;

			strdoioctl(stp, &strioc, K_TO_K);

			if (u.u_error) {
				linkblkp->l_qtop = NULL;
				setq(rq, &strdata, &stwdata);
				rq->q_ptr = WR(rq)->q_ptr = (caddr_t)stpdown;
				return;
			}
			stpdown->sd_flag |= STPLEX;
			fpdown->f_count++;
			/*
			 * Wake up any other processes that may have been waiting
			 * on the lower stream.  These will all error out.
			 */
			wakeup((caddr_t)rq);
			wakeup((caddr_t)WR(rq));
			wakeup((caddr_t)stpdown);
			u.u_rval1 = fpdown - &file[0];
			return;
		}
	
	
	case I_UNLINK:
		/*
		 * Unlink a multiplexer.
		 * If arg is -1, unlink all links for which this is the
		 * controlling stream.  Otherwise, arg is a index number
		 * for a link to be removed.
		 */
		{
			struct file *fpdown;
			struct linkblk *linkblkp;
			struct stdata *stpdown;

			if (stp->sd_flag & STRHUP) {
				u.u_error = ENXIO;
				return;
			}
			if (arg == -1) {
				munlinkall(stp, 0);
			} else {
				fpdown = &file[arg];
				if ((fpdown < file) || 
				    (fpdown >= (struct file *)v.ve_file) ||
				    !fpdown->f_inode || 
				    !(stpdown = fpdown->f_inode->i_sptr) ||
				    !(stpdown->sd_flag & STPLEX) ||
				    !(linkblkp = findlinks(NULL,
					     	stpdown->sd_wrq, arg))) {
					/* invalid user supplied index number */
					u.u_error = EINVAL;
					return;
				}
				(void) munlink(stp, fpdown, linkblkp, 0);
			}
			return;
		}

	case I_FLUSH:
		/*
		 * send a flush message downstream
		 * flush message can indicate 
		 * FLUSHR - flush read queue
		 * FLUSHW - flush write queue
		 * FLUSHRW - flush read/write queue
		 */
		if (stp->sd_flag & STRHUP) {
			u.u_error = EINVAL;
			return;
		}
		if (arg & ~FLUSHRW) {
			u.u_error = EINVAL;
			return;
		}
		if (!putctl1(stp->sd_wrq->q_next, M_FLUSH, arg))
			u.u_error = EAGAIN;
		else
			if (qready()) runqueues();
		return;

	case I_SRDOPT:
		/*
		 * Set read options
		 *
		 * RNORM - default stream mode
		 * RMSGN - message no discard
		 * RMSGD - message discard
		 */
		switch (arg) {
		case RNORM: 
			stp->sd_flag &= ~(RMSGDIS | RMSGNODIS);
			return;
		case RMSGD:
			stp->sd_flag = (stp->sd_flag & ~RMSGNODIS) | RMSGDIS;
			return;
		case RMSGN:
			stp->sd_flag = (stp->sd_flag & ~RMSGDIS) | RMSGNODIS;
			return;
		default:
			u.u_error = EINVAL;
			return;
		}


	case I_GRDOPT:
		/*
		 * Get read option and return the value
		 * to spot pointed to by arg
		 */
		{
			int rdopt;

			rdopt = ( (stp->sd_flag & RMSGDIS ? RMSGD :
				  (stp->sd_flag & RMSGNODIS ? RMSGN : RNORM)) );

			if (copyout(&rdopt, (int *)arg, sizeof(rdopt)))
				u.u_error = EFAULT;
			return;
		}

	case I_SETSIG:
		/*
		 * Register the calling proc to receive the SIGPOLL
		 * signal based on the events given in arg.  If
		 * arg is zero, remove the proc from register list.
		 */
		{
			struct strevent *sep, *psep;

			psep = NULL;

			for (sep = stp->sd_siglist; 
			     sep && (sep->se_procp != u.u_procp);
			     psep = sep, sep = sep->se_next);

			if (arg) {
				if (arg & ~(S_INPUT|S_HIPRI|S_OUTPUT|S_MSG)) {
					u.u_error = EINVAL;
					return;
				}

				/*
				 * If proc not already registered, add it to list
				 */
				if (!sep) {
					if (!(sep = sealloc(SE_SLEEP))) {
						u.u_error = EAGAIN;
						return;
					}
					if (psep) psep->se_next = sep;
					else stp->sd_siglist = sep;
					sep->se_procp = u.u_procp;
				}
				/*
				 * set events 
				 */
				sep->se_events = arg;
				stp->sd_sigflags |= arg;
				return;
			} 
			/*
			 * Remove proc from register list
			 */
			if (sep) {
				if (psep) psep->se_next = sep->se_next;
				else stp->sd_siglist = sep->se_next;
				sefree(sep);
				/*
				 * recalculate OR of sig events
				 */
				stp->sd_sigflags = 0;
				for (sep = stp->sd_siglist; sep; sep = sep->se_next)
					stp->sd_sigflags |= sep->se_events;
				return;
			}
			u.u_error = EINVAL;
			return;
		}

	case I_GETSIG:
		/*
		 * Return (in arg) the current registration of events
		 * for which the calling proc is to be signaled.
		 */
		{
			struct strevent *sep;

			for (sep = stp->sd_siglist; sep; sep = sep->se_next)
				if (sep->se_procp == u.u_procp) {
					if (copyout((caddr_t)&sep->se_events, 
						         (int *)arg, sizeof(int)))
						u.u_error = EFAULT;
					return;
				};
			u.u_error = EINVAL;
			return;
		}

	case I_PEEK:
		{
			mblk_t *bp;
			struct strpeek strpeek;
			int save_ucnt, n;

			if (copyin((caddr_t)arg, (caddr_t)&strpeek, sizeof(strpeek))) {
				u.u_error = EFAULT;
				return;
			}

			if (!(bp = RD(stp->sd_wrq)->q_first) ||
			   ((strpeek.flags & RS_HIPRI) && (queclass(bp) == QNORM))){
				u.u_rval1 = 0;
				return;
			}
	
			if (bp->b_datap->db_type == M_PCPROTO) 
				strpeek.flags = RS_HIPRI;
			else
				strpeek.flags = 0;


			/*
			 * First process PROTO blocks, if any
			 */
			u.u_base = strpeek.ctlbuf.buf;
			u.u_count = strpeek.ctlbuf.maxlen;
			u.u_segflg = 0;
			while (bp && bp->b_datap->db_type!=M_DATA && u.u_count) {
				if (n = min(u.u_count, bp->b_wptr - bp->b_rptr))
					iomove(bp->b_rptr, n, B_READ);
				if (u.u_error)
					return;
				bp = bp->b_cont;
			}
			save_ucnt = u.u_count;
		
			/*
			 * Now process DATA blocks, if any
			 */
			u.u_base = strpeek.databuf.buf;
			u.u_count = strpeek.databuf.maxlen;
			u.u_segflg = 0;
			while (bp && u.u_count) {
				if (n = min(u.u_count, bp->b_wptr - bp->b_rptr))
					iomove(bp->b_rptr, n, B_READ);
				if (u.u_error)
					return;
				bp = bp->b_cont;
			}


			strpeek.ctlbuf.len = strpeek.ctlbuf.maxlen - save_ucnt;
			strpeek.databuf.len = strpeek.databuf.maxlen - u.u_count;
			if (copyout((caddr_t)&strpeek, (caddr_t)arg, sizeof(strpeek))) {
				u.u_error = EFAULT;
				return;
			}
			u.u_rval1 = 1;
			return;
		}

	case I_FDINSERT:
		{
			struct strfdinsert strfdinsert;
			struct file *resftp;
			struct stdata *resstp;
			queue_t *q;
			mblk_t *mp;
			register s;
			register msgsize;
			short rmin, rmax;
			mblk_t *strmakemsg();

			if (stp->sd_flag & (STRERR|STRHUP|STPLEX)) {
				u.u_error = ((stp->sd_flag&STPLEX) ? EINVAL : stp->sd_error);
				return;
			}
			if (copyin((caddr_t)arg, (caddr_t)&strfdinsert, 
							sizeof(strfdinsert))) {
				u.u_error = EFAULT;
				return;
			}
			if ( strfdinsert.offset < 0 || 
			    (strfdinsert.offset % sizeof(queue_t *)) != 0 ) {
				u.u_error = EINVAL;
				return;
			}
			if (!(resftp = getf(strfdinsert.fildes)) ||
			    !(resstp = resftp->f_inode->i_sptr)) {
				u.u_error = EINVAL;
				return;
			}
	
			/* get read queue of stream terminus */
			for (q = resstp->sd_wrq->q_next; q->q_next; q = q->q_next);
			q = RD(q);
	
			if (strfdinsert.ctlbuf.len < 
					(strfdinsert.offset + sizeof(queue_t *))) {
				u.u_error = EINVAL;
				return;
			}

			/*
			 * Check for legal flag value
			 */
			if (strfdinsert.flags & ~RS_HIPRI) {
				u.u_error = EINVAL;
				return;
			}

			/*
			 * make sure ctl and data sizes together fall within 
			 * the limits of the max and min receive packet sizes 
			 * and do not exceed system limit.  A negative data length
			 * means that no data part is to be sent.
			 */
			rmin = stp->sd_wrq->q_next->q_minpsz;
			rmax = stp->sd_wrq->q_next->q_maxpsz;
			if (rmax == INFPSZ) rmax = strmsgsz;
			else rmax = min(rmax, strmsgsz);
			msgsize = strfdinsert.databuf.len;
			if (msgsize < 0) msgsize = 0;
			if ((msgsize<rmin) || (msgsize>rmax) ||
			    (strfdinsert.ctlbuf.len>strctlsz)) {
				u.u_error = ERANGE;
				return;
			}

			s = splstr();
			while (!(strfdinsert.flags & RS_HIPRI) && 
			       !canput(stp->sd_wrq->q_next)) 
				if (strwaitq(stp, WRITEWAIT, flag)) {
					splx(s);
					return;
				}
			splx(s);

			if (!(mp = strmakemsg(&strfdinsert.ctlbuf, 
					      &strfdinsert.databuf, stp->sd_wroff,
					      strfdinsert.flags))) 
				return;

			/*
			 * place pointer to queue 'offset' bytes from the
			 * start of the control portion of the message 
			 */

			*((queue_t **)(mp->b_rptr + strfdinsert.offset)) = q;

			/*
			 * Put message downstream
			 */
			(*stp->sd_wrq->q_next->q_qinfo->qi_putp)(stp->sd_wrq->q_next, mp);
			if (qready()) runqueues();
			return;
		}

	case I_SENDFD:
		{
			register queue_t *qp;
			register mblk_t *mp;
			register struct strrecvfd *srf;
			struct file *fp;

			if (stp->sd_flag & STRHUP) {
				u.u_error = ENXIO;
				return;
			}
			for (qp = stp->sd_wrq; qp->q_next; qp = qp->q_next);
			if (qp->q_qinfo != &strdata) {
				u.u_error = EINVAL;
				return;
			}
		 	if (!(fp = getf(arg))) return;
			if ((qp->q_flag & QFULL) || 
			    !(mp = allocb(sizeof(struct strrecvfd), BPRI_MED))) {
				u.u_error = EAGAIN;
				return;
			}
			srf = (struct strrecvfd *)mp->b_rptr;
			mp->b_wptr += sizeof(struct strrecvfd);
			mp->b_datap->db_type = M_PASSFP;
			srf->f.fp = fp;
			srf->uid = u.u_uid;
			srf->gid = u.u_gid;
			fp->f_count++;
			strrput(qp, mp);
			return;
		}

	case I_RECVFD:
		{
			register s;
			register mblk_t *mp;
			register struct strrecvfd *srf;
			register i;

			if (stp->sd_flag & (STRERR|STPLEX)) {
				u.u_error = ((stp->sd_flag & STPLEX) ? EINVAL :stp->sd_error);
				return;
			}
			s = splstr();
			while (!(mp = getq(RD(stp->sd_wrq)))) {
				if (stp->sd_flag&STRHUP) {
					splx(s);
					u.u_error = ENXIO;
					return;
				}
				if (strwaitq(stp, READWAIT, flag)) {
					splx(s);
					return;
				}
			}
			if (mp->b_datap->db_type != M_PASSFP) {
				putbq(RD(stp->sd_wrq), mp);
				splx(s);
				u.u_error = EBADMSG;
				return;
			}
			splx(s);
			srf = (struct strrecvfd *)mp->b_rptr;
			if ((i = ufalloc(0)) < 0) {
				putbq(RD(stp->sd_wrq), mp);
				return;
			}
			u.u_ofile[i] = srf->f.fp;
			srf->f.fd = i;
			if (copyout((caddr_t)srf, (caddr_t)arg, sizeof(struct strrecvfd))) {
				u.u_error = EFAULT;
				srf->f.fp = u.u_ofile[i];
				putbq(RD(stp->sd_wrq), mp);
				u.u_ofile[i] = NULL;
				return;
			}
			freemsg(mp);
			u.u_rval1 = 0;	/* reset value set by ufalloc() */
			return;
		}
	}
}


/*
 * Send an ioctl message downstream and wait for acknowledgement
 */

strdoioctl(stp, strioc, copyflg)
struct stdata *stp;
struct strioctl *strioc;
int copyflg;
{
	register mblk_t *bp;
	register s;
	struct iocblk *iocbp;
	extern str2time(), str3time();
	int id;
	
	if ((strioc->ic_len < 0) || (strioc->ic_len > strmsgsz)) {
		u.u_error = EINVAL;
		return;
	}

	while (!(bp = allocb(sizeof(struct iocblk), BPRI_HI))) 
		if (strwaitbuf(sizeof(struct iocblk), BPRI_HI)) return;

	iocbp = (struct iocblk *)bp->b_wptr;
	iocbp->ioc_count = strioc->ic_len;
	iocbp->ioc_cmd = strioc->ic_cmd;
	iocbp->ioc_uid = u.u_uid;
	iocbp->ioc_gid = u.u_gid;
	iocbp->ioc_error = 0;
	iocbp->ioc_rval = 0;
	bp->b_datap->db_type = M_IOCTL;
	bp->b_wptr += sizeof(struct iocblk);


	/*
	 * If there is data to copy into ioctl block, do so
	 */
	if (iocbp->ioc_count && !putiocd(bp, strioc->ic_dp, copyflg)){
			freemsg(bp);
			return;
	}
	s = splstr();


	/*
	 * Block for up to STRTIMOUT sec if there is a outstanding
	 * ioctl for this stream already pending.  All processes
	 * sleeping here will be awakened as a result of an ACK
	 * or NAK being received for the outstanding ioctl, or
	 * as a result of the timer expiring on the outstanding
	 * ioctl (a failure), or as a result of any waiting
	 * process's timer expiring (also a failure).
	 */
	stp->sd_flag |= STR2TIME;
	id = timeout(str2time, stp, STRTIMOUT*HZ);

	while (stp->sd_flag & IOCWAIT) {
		stp->sd_iocwait++;
		if (sleep((caddr_t)&stp->sd_iocwait,STIPRI|PCATCH) ||
				    !(stp->sd_flag & STR2TIME)) {
			stp->sd_iocwait--;
			u.u_error = (stp->sd_flag&STR2TIME ? EINTR : ETIME);
			if (!stp->sd_iocwait) stp->sd_flag &= ~STR2TIME;
			splx(s);
			untimeout(id);
			freemsg(bp);
			return;
		}
		stp->sd_iocwait--;
		if (stp->sd_flag & (STRHUP|STRERR|STPLEX)) {
			u.u_error = ((stp->sd_flag & STPLEX) ? EINVAL : stp->sd_error);
			if (!stp->sd_iocwait) stp->sd_flag &= ~STR2TIME;
			splx(s);
			untimeout(id);
			freemsg(bp);
			return;
		}
	}
	untimeout(id);
	if (!stp->sd_iocwait) stp->sd_flag &= ~STR2TIME;

	/*
	 * Have control of ioctl mechanism.
	 * Send down ioctl packet and wait for
	 * response
	 */
	if (stp->sd_iocblk) {
		freemsg(stp->sd_iocblk);
		stp->sd_iocblk = NULL;
	}
	stp->sd_flag |= IOCWAIT;

	/* 
	 * assign sequence number
	 */
	iocbp->ioc_id = (stp->sd_iocid = ++ioc_id);

	splx(s);
	(*stp->sd_wrq->q_next->q_qinfo->qi_putp)(stp->sd_wrq->q_next, bp);
	if (qready()) runqueues();


	/*
	 * Timed wait for acknowledgment.  The wait time is limited by the
	 * timeout value, which must be a positive integer (number of seconds
	 * to wait, or 0 (use default value of STRTIMOUT seconds), or -1
	 * (wait forever).  This will be awakened either by an ACK/NAK
	 * message arriving, the timer expiring, or the timer expiring 
	 * on another ioctl waiting for control of the mechanism.
	 */
	s = splstr();
	if ( strioc->ic_timout >= 0)
		id = timeout(str3time, stp, (strioc->ic_timout ? strioc->ic_timout: STRTIMOUT) * HZ);

	stp->sd_flag |= STR3TIME;
	/*
	 * If the reply has already arrived, don't sleep.  If awakened from
	 * the sleep, fail only if the reply has not arrived by then.
	 * Otherwise, process the reply.
	 */
	while (!stp->sd_iocblk) {
		if (stp->sd_flag & (STRERR|STPLEX)) {
			u.u_error = ((stp->sd_flag&STPLEX) ? EINVAL : stp->sd_error);
			stp->sd_flag &= ~(STR3TIME|IOCWAIT);
			if (strioc->ic_timout >= 0) untimeout(id);
			splx(s);
			wakeup((caddr_t)&(stp->sd_iocwait));
			return;
		}

		if (sleep((caddr_t)stp,STIPRI|PCATCH) || 
		    !(stp->sd_flag & STR3TIME))  {
			u.u_error = ((stp->sd_flag&STR3TIME) ? EINTR : ETIME);
			stp->sd_flag &= ~(STR3TIME|IOCWAIT);
			if (stp->sd_iocblk) {
				freemsg(stp->sd_iocblk);
				stp->sd_iocblk = NULL;
			}
			if (strioc->ic_timout >= 0) untimeout(id);
			splx(s);
			wakeup((caddr_t)&(stp->sd_iocwait));
			return;
		}
	}
	ASSERT(stp->sd_iocblk);
	bp = stp->sd_iocblk;
	stp->sd_iocblk = NULL;
	stp->sd_flag &= ~(STR3TIME|IOCWAIT);
	if (strioc->ic_timout >= 0) untimeout(id);
	splx(s);
	wakeup((caddr_t)&(stp->sd_iocwait));


	/*
	 * Have recieved acknowlegment
	 */

	iocbp = (struct iocblk *)bp->b_rptr;
	switch (bp->b_datap->db_type) {
	case M_IOCACK:
		/*
		 * Positive ack
		 */

		/*
		 * set error if indicated
		 */
		if (iocbp->ioc_error) {
			u.u_error = iocbp->ioc_error;
			break;
		}

		/*
		 * set return value
		 */
		u.u_rval1 = iocbp->ioc_rval;

		/*
		 * Data may have been returned in ACK message (ioc_count > 0).
		 * If so, copy it out to the user's buffer.
		 */
		if (iocbp->ioc_count) 
			if (!getiocd(bp, strioc->ic_dp)) break;
		strioc->ic_len = iocbp->ioc_count;
		break;

	case M_IOCNAK:
		/*
		 * Negative ack
		 *
		 * The only thing to do is set error as specified
		 * in neg ack packet
		 */
		u.u_error = (iocbp->ioc_error ? iocbp->ioc_error : EINVAL);
		break;

	default:
		ASSERT(0);
		break;
	}

	freemsg(bp);
}



/*
 * Get the next message from the read queue.  If the message is 
 * priority, STRPRI will have been set by strrput().  This flag
 * should be reset only when the entire message at the front of the
 * queue as been consumed.
 */
int
strgetmsg(ip, mctl, mdata, flag)
register struct inode *ip;
register struct strbuf *mctl;
register struct strbuf *mdata;
register flag;
{
	register s;
	register struct stdata *stp;
	register mblk_t *bp, *nbp;
	mblk_t *savemp = NULL;
	mblk_t *savemptail = NULL;
	int rflag;
	int flg = 0;
	int more = 0;
	int n;
	int bcnt;
	char *ubuf;


	ASSERT(ip->i_sptr);
	stp = ip->i_sptr;

	if (stp->sd_flag & (STRERR|STPLEX)) {
		u.u_error = ((stp->sd_flag&STPLEX) ? EINVAL : stp->sd_error);
		return(0);
	}
	if (copyin((int *)flag, (int *)&rflag, sizeof(int))) {
		u.u_error = EFAULT;
		return(0);
	}
	if (rflag & (~RS_HIPRI)) {
		u.u_error = EINVAL;
		return(0);
	}

	s = splstr();
	while ( ((rflag & RS_HIPRI) && !(stp->sd_flag & STRPRI)) ||
			!(bp = getq(RD(stp->sd_wrq))) ) {
		/*
		 * If STRHUP, return 0 length control and data
		 */
		if (stp->sd_flag & STRHUP) {
			mctl->len = mdata->len = 0;
			if (copyout(&flg, (int *)flag, sizeof(int)))
				u.u_error = EFAULT;
			splx(s);
			return(0);
		} 
		if (strwaitq(stp, READWAIT, u.u_fmode)) {
			splx(s);
			return(0);
		}
	}
	splx(s);
	
	if (bp->b_datap->db_type == M_PASSFP) {
		putbq(RD(stp->sd_wrq), bp);
		u.u_error = EBADMSG;
		return(0);
	}

	if (qready()) runqueues();

	/*
	 * Set HIPRI flag if message is priority.
	 */
	if (stp->sd_flag & STRPRI) flg |= RS_HIPRI;

	/*
	 * First process PROTO or PCPROTO blocks, if any.
	 */
	if (mctl->maxlen >= 0 && bp && bp->b_datap->db_type != M_DATA) {
		bcnt = mctl->maxlen;
		ubuf = mctl->buf;
		while (bp && bp->b_datap->db_type != M_DATA && bcnt >= 0) {
			if ( (n = min(bcnt, bp->b_wptr - bp->b_rptr)) &&
			     copyout(bp->b_rptr, ubuf, n) ) {
				u.u_error = EFAULT;
				stp->sd_flag &= ~STRPRI;
				more = 0;
				freemsg(bp);
				goto getmout;
			}
			ubuf += n;
			bp->b_rptr += n;
			if (bp->b_rptr >= bp->b_wptr) {
				nbp = bp;
				bp = bp->b_cont;
				freeb(nbp);
			}
			if ((bcnt -= n) <= 0) break;
		}
		mctl->len = mctl->maxlen - bcnt;
	} else
		mctl->len = -1;
	
		
	if (bp && bp->b_datap->db_type != M_DATA) {	
		/*
		 * more PROTO blocks in msg 
		 */
		more |= MORECTL;
		savemp = bp;
		while (bp && bp->b_datap->db_type!=M_DATA) {
			savemptail = bp;
			bp = bp->b_cont;
		}
		savemptail->b_cont = NULL;
	}

	/*
	 * Now process DATA blocks, if any
	 */
	if (mdata->maxlen >= 0 && bp) {
		bcnt = mdata->maxlen;
		ubuf = mdata->buf;
		while (bp && bcnt >= 0) {
			if ( (n = min(bcnt, bp->b_wptr - bp->b_rptr)) &&
			     copyout(bp->b_rptr, ubuf, n) ) {
				u.u_error = EFAULT;
				stp->sd_flag &= ~STIPRI;
				more = 0;
				freemsg(bp);
				goto getmout;
			}
			ubuf += n;
			bp->b_rptr += n;
			if (bp->b_rptr >= bp->b_wptr) {
				nbp = bp;
				bp = bp->b_cont;
				freeb(nbp);
			}
			if ((bcnt -= n) <= 0) break;
		}
		mdata->len = mdata->maxlen - bcnt;
	} else
		mdata->len = -1;

	if (bp) {			/* more data blocks in msg */
		more |= MOREDATA;
		if (savemp)
			savemptail->b_cont = bp;
		else
			savemp = bp;
	} 

	if (savemp) 
		putbq(RD(stp->sd_wrq), savemp);
	else 
		stp->sd_flag &= ~STRPRI;

	if (copyout(&flg, (int *)flag, sizeof(int)))
		u.u_error = EFAULT;

	/*
	 * Getmsg cleanup processing - if the state of the queue has changed
	 * some signals may need to be sent and/or poll awakened.
	 */
getmout:
	while ((bp = RD(stp->sd_wrq)->q_first) && (bp->b_datap->db_type==M_SIG)) {
		bp = getq(RD(stp->sd_wrq));
		switch (*bp->b_rptr) {
		case SIGPOLL:
			if (stp->sd_sigflags & S_MSG)
				strsendsig(stp->sd_siglist, S_MSG);
			break;

		default:
			if (stp->sd_pgrp)
				signal(stp->sd_pgrp, *bp->b_rptr);
			break;
		}
		freemsg(bp);
		if (qready()) runqueues();
	}
	/*
	 * If we have just received a high priority message and a
	 * regular message is now at the front of the queue, send
	 * signals in S_INPUT processes and wake up processes polling
	 * on POLLIN.
	 */
	if (RD(stp->sd_wrq)->q_first && 
	    !(stp->sd_flag & STRPRI) && (flg & RS_HIPRI)) {
		if (stp->sd_sigflags & S_INPUT) 
			strsendsig(stp->sd_siglist, S_INPUT);
		if (stp->sd_pollflags & POLLIN)
			strwakepoll(stp, POLLIN);
	}
	return(more);
}



/*
 * Put a message downstream
 */

strputmsg(ip, mctl, mdata, flag)
register struct inode *ip;
register struct strbuf *mctl;
register struct strbuf *mdata;
register flag;
{
	register struct stdata *stp;
	mblk_t *mp;
	register s;
	register msgsize;
	short rmin, rmax;
	mblk_t *strmakemsg();

	ASSERT(ip->i_sptr);
	stp = ip->i_sptr;

	if (stp->sd_flag & (STRHUP|STRERR|STPLEX)) {
		u.u_error = ((stp->sd_flag&STPLEX) ? EINVAL : stp->sd_error);
		return;
	}

	/*
	 * Check for legal flag value
	 */
	if ((flag & ~RS_HIPRI) || ((flag & RS_HIPRI) && (mctl->len < 0))) {
		u.u_error = EINVAL;
		return;
	}

	/*
	 * make sure ctl and data sizes together fall within the limits of the
	 * max and min receive packet sizes and do not exceed system limit
	 */
	rmin = stp->sd_wrq->q_next->q_minpsz;
	rmax = stp->sd_wrq->q_next->q_maxpsz;
	if (rmax == INFPSZ) rmax = strmsgsz;
	else rmax = min(rmax, strmsgsz);
	msgsize = mdata->len;
	if (msgsize < 0) {
		msgsize = 0;
		rmin = 0;	/* no range check for NULL data part */
	}
	if ((msgsize<rmin) || (msgsize>rmax) || (mctl->len>strctlsz)) {
		u.u_error = ERANGE;
		return;
	}

	s = splstr();
	while (!(flag & RS_HIPRI) && !canput(stp->sd_wrq->q_next)) {
		if (strwaitq(stp, WRITEWAIT, u.u_fmode)) {
			splx(s);
			return;
		}
	}
	splx(s);

	if (!(mp = strmakemsg(mctl, mdata, stp->sd_wroff, flag))) return;

	/*
	 * Put message downstream
	 */
	(*stp->sd_wrq->q_next->q_qinfo->qi_putp)(stp->sd_wrq->q_next, mp);
	if (qready()) runqueues();
}



/*
 * Determines whether the necessary conditions are set on a stream
 * for it to be readable, writeable, or have exceptions.
 */
int
strpoll(stp, events, anyyet)
register struct stdata *stp;
short events;
{
	register retevents = 0;
	register s;
	register struct strevent *sep, *psep;
	register queue_t *tq;

	if (stp->sd_flag & STPLEX)
		return(POLLNVAL);
	
	s = splstr();
	if (stp->sd_flag&STRERR) {
		splx(s);
		return(POLLERR);
	}

	if (stp->sd_flag & STRHUP) retevents |= POLLHUP;

	for (tq = stp->sd_wrq->q_next; 
	     tq->q_next && !tq->q_qinfo->qi_srvp;  tq = tq->q_next) ;
	if ( (events & POLLOUT) &&
	     !(tq->q_flag & QFULL) &&
	     !(stp->sd_flag & STRHUP) )
			retevents |= POLLOUT;

	if ( (events & POLLIN) &&
	     !(stp->sd_flag & STRPRI) &&
	     RD(stp->sd_wrq)->q_first )
			retevents |= POLLIN;

	if ( (events & POLLPRI) &&
	     (stp->sd_flag & STRPRI) )
			retevents |= POLLPRI;

	ASSERT((retevents & (POLLPRI|POLLIN)) != (POLLPRI|POLLIN));

	if (retevents) {
		splx(s);
		pollreset(stp);
		return(retevents);
	}


	/*
	 * if poll() has not found any events yet, set up event cell
	 * to wake up the poll if a requested event occurs on this
	 * stream.  This will occaisionally result in adding an event
	 * cell on top of one that already there, but both will get 
	 * cleaned up in the pollout: section of poll() in sys2.c.
	 * Hence, it is not worth checking for here.
	 */
	if (!anyyet) {
		if (sep = sealloc(SE_SLEEP)) {
			sep->se_procp = u.u_procp;
			sep->se_events = events;
			sep->se_next = stp->sd_pollist;
			stp->sd_pollist = sep;
			stp->sd_pollflags |= events;
			splx(s);
			return(0);
		}
		splx(s);
		pollreset(stp);
		u.u_error = EAGAIN;
		return(0);
	}
	/*
	 * If we get here the poll has already found an event but
	 * there are no return events for this stream. Remove
	 * any event cell for this process from the pollist, and
	 * recalculate the pollflags.  
	 * A cell will exist if this stream was previously scanned
	 * in this poll().
	 */
	splx(s);
	pollreset(stp);
	return(retevents);
}


/*
 * Attach a stream device or module.
 * qp is a read queue; the new queue goes in so its next
 * read ptr is the argument, and the write queue corresponding
 * to the argument points to this queue.  Return 1 on success,
 * 0 on failure.
 */

int
qattach(qinfo, qp, dev, flag)
register struct streamtab *qinfo;
register queue_t *qp;
dev_t dev;
{
	register queue_t *rq;
	register s;
	int sflg;

	if (!(rq = allocq())) return(0);

	sflg = 0;
	s = splstr();
	rq->q_next = qp;
	WR(rq)->q_next = WR(qp)->q_next;
	if (WR(qp)->q_next) {
		OTHERQ(WR(qp)->q_next)->q_next = rq;
		sflg = MODOPEN;
	}
	WR(qp)->q_next = WR(rq);
	setq(rq, qinfo->st_rdinit, qinfo->st_wrinit);
 	rq->q_flag |= QWANTR;
	WR(rq)->q_flag |= QWANTR;

	/*
	 * Open the attached module or driver.
	 * The open may sleep, but it must always return here.  Therefore,
	 * all sleeps must set PCATCH or ignore all signals to avoid a 
	 * longjump if a signal arrives.
	 */
	if ((*rq->q_qinfo->qi_qopen)(rq, dev, flag, sflg) == OPENFAIL) {
		qdetach(rq, 0, 0);
		splx(s);
		return(0);
	}
	splx(s);
	return(1);
}

/*
 * Detach a stream module or device.
 * If clmode == 1 then the module or driver was opened and its
 * close routine must be called.  If clmode == 0, the module
 * or driver was never opened or the <open failed, and so its close
 * should not be called.
 */

qdetach(qp, clmode, flag)
register queue_t *qp;
{
	register s;
	register queue_t *q, *prev = NULL;

	if (clmode) {
		if (qready()) runqueues();
		s = splstr();
		(*qp->q_qinfo->qi_qclose)(qp, (qp->q_next ? 0 : flag));
		/*
		 * Check if queues are still enabled, and remove from 
		 * runlist if necessary.
		 */
		if ((qp->q_flag | WR(qp)->q_flag) & QENAB) {
			for (q = qhead; q; q = q->q_link)  {
				if (q == qp || q == WR(qp)) {
					if (prev)
						prev->q_link = q->q_link;
					else
						qhead = q->q_link;
					if (q == qtail)
						qtail = prev;
				}
				prev = q;
			}
		}
		flushq(qp, FLUSHALL);
		flushq(WR(qp), FLUSHALL);
	} else 
		s = splstr();

	if (WR(qp)->q_next)
		backq(qp)->q_next = qp->q_next;
	if (qp->q_next)
		backq(WR(qp))->q_next = WR(qp)->q_next;
	freeq(qp);
	splx(s);
	strst.queue.use--;
}


/*
 * This function is placed in the callout table to wake up a process
 * waiting to close a stream that has not completely drained.
 */

strtime(stp)
struct stdata *stp;
{

	if (stp->sd_flag & STRTIME) {
		wakeup(stp->sd_wrq);
		stp->sd_flag &= ~STRTIME;
	}
}

/*
 * This function is placed in the callout table to wake up all
 * processes waiting to send an ioctl down a particular stream,
 * as well as the process whose ioctl is still outstanding.  The
 * process placing this function in the callout table will remove
 * it if he gets control of the ioctl mechanism for the stream -
 * this should only run if there is a failure.  This wakes up
 * the same processes as str3time below.
 */

str2time(stp)
struct stdata *stp;
{

	if (stp->sd_flag & STR2TIME) {
		wakeup(&stp->sd_iocwait);
		stp->sd_flag &= ~STR2TIME;
	}
}

/*
 * This function is placed on the callout table to wake up the
 * the process that has an outstanding ioctl waiting acknowledgement
 * on a stream, as well as any processes waiting to send their
 * own ioctl messages.  It should be removed from the callout table
 * when the acknowledgement arrives.  If this function runs, it
 * is the result of a failure.  This wakes up the same processes
 * as str2time above.
 */

str3time(stp)
struct stdata *stp;
{

	if (stp->sd_flag & STR3TIME) {
		wakeup(stp);
		stp->sd_flag &= ~STR3TIME;
	}
}

/*
 *  Put ioctl data from user land to ioctl buffers.  Return 0 for failure,
 *  1 for success.
 */
int
putiocd(bp, arg, copymode)
register mblk_t *bp;
caddr_t arg;
int copymode;
{
	register mblk_t *tmp;
	register int count, n;

	count = ((struct iocblk *)bp->b_rptr)->ioc_count;

	/*
	 * strdoioctl validates ioc_count, so if this assert fails it
	 * cannot be due to user error.
	 */
	ASSERT(count >= 0);

	while (count) {
		n = min(MAXIOCBSZ,count);
		if (!(tmp = allocb(n, BPRI_HI))) {
			u.u_error = EAGAIN;
			return(0);
		}
		switch (copymode) {
			case K_TO_K:
				bcopy((caddr_t)arg, tmp->b_wptr, n);
				break;

			case U_TO_K:
				if (copyin((char *)arg, tmp->b_wptr, n)) {
					freeb(tmp);
					u.u_error = EFAULT;
					return(0);
				}
				break;

			default:
				ASSERT(0);
				freeb(tmp);
				return(0);
		}
		arg += n;
		tmp->b_datap->db_type = M_DATA;
		tmp->b_wptr += n;
		count -= n;
		bp = (bp->b_cont = tmp);
	}
	return(1);
}

/*
 * copy ioctl data to user land.  Return 0 for failure, 1 for success.
 */
int
getiocd(bp, arg)
register mblk_t *bp;
caddr_t arg;
{
	register int count,n;

	count = ((struct iocblk *)bp->b_rptr)->ioc_count;
	ASSERT(count >= 0);

	for(bp = bp->b_cont; bp && count; count -= n, bp = bp->b_cont ,arg += n) {
		n = min(count, bp->b_wptr - bp->b_rptr);
		if (copyout(bp->b_rptr, arg, n)) {
			u.u_error = EFAULT;
			return(0);
		}
	}
	ASSERT(count==0);
	return(1);
}



/* 
 * allocate a linkblk table entry for the triple:
 * (write queue of bottom module of top stream, write queue of stream head of
 * bottom stream, file table index number)
 *
 * linkblk table entries are freed by nulling the l_qtop field.
 */

struct linkblk *
alloclink(qup, qdown, index)
queue_t *qup, *qdown;
int index;
{
	register struct linkblk *linkblkp;

	for (linkblkp = &linkblk[0];   linkblkp < &linkblk[nmuxlink];   linkblkp++)
		if (!linkblkp->l_qtop) {
			linkblkp->l_qtop = qup;
			linkblkp->l_qbot = qdown;
			linkblkp->l_index = index;
			return(linkblkp);
		}

	return(NULL);
}


/*
 * Check for a potential linking cycle.  
 * Qup is the upper queue in a multiplexor that is going to be linked.
 * Qdown is initially the queue to be linked below the multiplexor.
 * Linkcycle() is called to recursively scan the tree of links
 * rooted at qdown to determine if qup is contained in that tree.
 */
int
linkcycle(qup, qdown)
register queue_t *qup, *qdown;
{
	register struct linkblk *linkblkp;

	for (linkblkp = &linkblk[0]; linkblkp < &linkblk[nmuxlink]; linkblkp++) {
		if (linkblkp->l_qtop == qdown) 
			if ((linkblkp->l_qbot == qup) ||
			    linkcycle(qup, linkblkp->l_qbot))
				return(1);
	}
	return(0);
}


/* 
 * find linkblk table entry corresponding to triple.
 * A NULL parameter means any value matches that member of the triple.
 * Return pointer to linkblk entry.
 */
struct linkblk *
findlinks(qup, qdown, index)
register queue_t *qup, *qdown;
int index;
{
	register struct linkblk *linkblkp;

	ASSERT(qup || qdown || index);

	for (linkblkp = &linkblk[0];   linkblkp < &linkblk[nmuxlink];   linkblkp++) {
		if ( linkblkp->l_qtop &&
		     (!qup || (qup == linkblkp->l_qtop))   &&
		     (!qdown || (qdown == linkblkp->l_qbot)) &&
		     (!index || (index == linkblkp->l_index)) )

			return(linkblkp);
	}
	return(NULL);
}


/* 
 * given a queue ptr, follow the chain of q_next pointers until you reach the
 * last queue on the chain and return it
 */
queue_t *
getendq(q)
register queue_t *q;
{
	while (q->q_next) q = q->q_next;
	return(q);
}


/*
 * unlink a multiplexer link.  Stp is the controlling stream for the
 * link, fpdown is the file pointer for the lower stream, and
 * linkblkp points to the link's entry in the linkblk table.
 */
int
munlink(stp, fpdown, linkblkp, cflag)
struct stdata *stp;
struct file *fpdown;
struct linkblk *linkblkp;
int cflag;
{
	struct strioctl strioc;
	struct stdata *stpdown;
	queue_t *rq;

	strioc.ic_cmd = I_UNLINK;
	strioc.ic_timout = 0;
	strioc.ic_len = sizeof(struct linkblk);
	strioc.ic_dp = (char *) linkblkp;
	
	strdoioctl(stp, &strioc, K_TO_K);

	/*
	 * If there was an error and this is not called via strclose, 
	 * return to the user.  Otherwise, pretend there was no error 
	 * and close the link.  
	 */
	if (u.u_error) {
		if (cflag) {
			printf("KERNEL: munlink: could not perform unlink ioctl, closing anyway\n");
			u.u_error = 0;	
			stp->sd_flag &= ~STRERR; /* allows strdoioctl() to work */
		}
		else return(-1);
	}

	stpdown = fpdown->f_inode->i_sptr;
	stpdown->sd_flag &= ~STPLEX;
	rq = RD(stpdown->sd_wrq);
	setq(rq, &strdata, &stwdata);
	rq->q_ptr = WR(rq)->q_ptr = (caddr_t)stpdown;
	linkblkp->l_qtop = NULL;
	closef(fpdown);
	return(0);
}



/*
 * unlink all multiplexer links for which stp is the controlling stream.
 */
munlinkall(stp, cflag)
struct stdata *stp;
int cflag;
{

	struct file *fpdown;
	struct linkblk *linkblkp;
	queue_t *qup;

	qup = getendq(stp->sd_wrq);
	while (linkblkp = findlinks(qup, NULL, NULL)) {
		fpdown = &file[linkblkp->l_index];
		ASSERT((fpdown >= file) && (fpdown < (struct file *)v.ve_file));
		if (munlink(stp, fpdown, linkblkp, cflag) == -1) {
			ASSERT(0);
			return;
		}
	}
	return;
}

/*
 * Set the interface values for a pair of queues (qinit structure,
 * packet sizes, water marks).
 */
setq(rq, rinit, winit)
queue_t *rq;
struct qinit *rinit, *winit;
{
	register queue_t  *wq;

	wq = WR(rq);

	rq->q_qinfo = rinit;
	rq->q_hiwat = rinit->qi_minfo->mi_hiwat;
	rq->q_lowat = rinit->qi_minfo->mi_lowat;
	rq->q_minpsz = rinit->qi_minfo->mi_minpsz;
	rq->q_maxpsz = rinit->qi_minfo->mi_maxpsz;
	wq->q_qinfo = winit;
	wq->q_hiwat = winit->qi_minfo->mi_hiwat;
	wq->q_lowat = winit->qi_minfo->mi_lowat;
	wq->q_minpsz = winit->qi_minfo->mi_minpsz;
	wq->q_maxpsz = winit->qi_minfo->mi_maxpsz;
}




/*
 * Make a protocol message given control and data buffers
 */
mblk_t *
strmakemsg(mctl, mdata, wroff, flag)
register struct strbuf *mctl;
register struct strbuf *mdata;
int wroff;
long flag;
{
	register mblk_t *mp = NULL;
	register mblk_t *bp;
	int count;
	caddr_t base;
	int pri;
	int msgtype;
	int offlg = 0;

	if (flag & RS_HIPRI) pri = BPRI_MED;
	else pri = BPRI_LO;

	/*
	 * Create control part of message, if any
	 */
	if (mctl->len >= 0) {
		if (flag & RS_HIPRI) 
			msgtype = M_PCPROTO;
		else 
			msgtype = M_PROTO;

		count = mctl->len;
		base = mctl->buf;

		/*
		 * range checking has already been done, simply try
		 * to allocate a message block for the ctl part.
		 */
		while (!(bp = allocb(count, pri))) 
			if (strwaitbuf(count, pri)) return(NULL);

		bp->b_datap->db_type = msgtype;
		if (copyin(base, bp->b_wptr, count)) {
			u.u_error = EFAULT;
			freeb(bp);
			return(NULL);
		}
		bp->b_wptr += count;
		mp = bp;
	}

	/*
	 * Create data part of message, if any
	 */
	if (mdata->len >= 0) {

		count = mdata->len;
		base = mdata->buf;
		do {
			register size;
			register class;

			size = count + (offlg ? 0 : wroff);

getbp:
			if (size < QBSIZE) {
				if (bp = allocb(size, pri)) goto gotbp;
			} else {
				if ((class = getclass(size)) == NCLASS) {
					class = NCLASS-1;
					size = rbsize[class];
				}
				if (bp = allocb(rbsize[class], pri)) goto gotbp;
				if (bp = allocb(rbsize[--class], pri)) goto gotbp;
				if (bp = allocb(rbsize[--class], pri)) goto gotbp;
			}
			if (strwaitbuf(size, pri)) {
				freemsg(mp);
				return(NULL);
			}
			goto getbp;

gotbp:
			if (wroff && !offlg++ &&
			    (wroff < bp->b_datap->db_lim - bp->b_wptr)) {
				bp->b_rptr += wroff;
				bp->b_wptr += wroff;
			}
			if ((size = min(count, bp->b_datap->db_lim - bp->b_wptr)) &&
			    copyin(base, bp->b_wptr, size)) {
				u.u_error = EFAULT;
				freeb(bp);
				freemsg(mp);
				return(NULL);
			}
			bp->b_wptr += size;
			base += size;
			count -= size;
			if (!mp)
				mp = bp;
			else
				linkb(mp, bp);

		} while (count);
	}
	return(mp);
}

/*
 * Wait for a buffer to become available.  Return 1 if not able to wait,
 * 0 if buffer is probably there.
 */
strwaitbuf(size, pri)
int size, pri;
{
	extern setrun();

	if (!bufcall(size, pri, setrun, u.u_procp)) {
		u.u_error = EAGAIN;
		return(1);
	}
	if (sleep((caddr_t)&(u.u_procp->p_flag), STOPRI|PCATCH)) {
		strunbcall(size, pri, u.u_procp);
		u.u_error = EINTR;
		return(1);
	}
	strunbcall(size, pri, u.u_procp);
	return(0);
}

/*
 * Remove a setrun for the given process from the bufcall list for
 * the given buffer size and allocation priority.
 */
strunbcall(size, pri, p)
int size, pri;
struct proc *p;
{
	register s;
	struct dbalcst *dbp;
	struct strevent *sep, **prvp;
	extern setrun();
	extern struct dbalcst dballoc[];
	
	ASSERT((size >= 0) && (size <= rbsize[NCLASS-1]));
	dbp = &dballoc[getclass(size)];
	prvp = ( pri == BPRI_HI ? &dbp->dba_hip :
		(pri == BPRI_LO ? &dbp->dba_lop : &dbp->dba_medp));
	s = splstr();
	if (!(sep = *prvp)) return;
	while (sep) {
		if ((sep->se_func == setrun) && (sep->se_arg == (long)p)) {
			*prvp = sep->se_next;
			splx(s);
			sefree(sep);
			return;
		}
		prvp = &sep->se_next;
		sep = sep->se_next;
	}
	splx(s);
}


/*
 * This function waits for a read or write event to happen on a stream.
 */

strwaitq(stp, flag, fmode)
register struct stdata *stp;
int flag;
int fmode;
{
	int slpflg, slppri, errs;
	caddr_t slpadr;

	if (fmode&FNDELAY) {
		if (!(flag & NOINTR)) u.u_error = EAGAIN;
		return(1);
	}
	if (flag & READWAIT) {
		slpflg = RSLEEP;
		slpadr = (caddr_t)RD(stp->sd_wrq);
		slppri = STIPRI;
		errs = STRERR|STPLEX;
	} else {
		slpflg = WSLEEP;
		slpadr = (caddr_t)stp->sd_wrq;
		slppri = STOPRI;
		errs = STRERR|STRHUP|STPLEX;
	}
	
	stp->sd_flag |= slpflg;
	if (sleep(slpadr, slppri)) {
		stp->sd_flag &= ~slpflg;
		wakeup(slpadr);
		if (!(flag & NOINTR)) u.u_error = EINTR;
		return(1);
	}
	if (stp->sd_flag & errs) {
		u.u_error = ((stp->sd_flag & STPLEX) ? EINVAL : stp->sd_error);
		return(1);
	}
	return(0);
}
