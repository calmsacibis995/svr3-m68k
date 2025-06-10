/*	Copyright (c) 1984 by Communication Machinery Corporation 
 *
 *	This file contains material which is proprietary to
 *	Communication Machinery Corporation (CMC) and which
 *	may not be divulged without the written permission
 *	of CMC.
 */

/*
 *	File:	enp.c		Module: HOST Resident
 *	ENP:	All ENP models
 *	Unix:	System V
 *
 *	This file implements the generic support procedures utilized by
 *	sock_driver.c. The only "dependent" portion for enp.c is which
 *	enp_addr.h file is included.
 */

 /*	HISTORY:
  *		Initial coding (for Sys V and ENP-30) July 1984 by John Mullen
  *		Add hooks for stream interface 041686 dtk
  */

/* #define VERBOSE */
#include <sys/param.h>
#include <sys/CMC/types.h>
#include <sys/signal.h>
#include <sys/fs/s5dir.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/CMC/errno.h>
#include <sys/CMC/signal.h>
#include <sys/CMC/enp_addr.h>
#include <sys/CMC/netbfr.h>
#include <sys/CMC/ring.h>
#include <sys/CMC/CMC_proto.h>
#include <sys/CMC/CMC_socket.h>
#include <sys/CMC/in.h>
#include <sys/CMC/enp.h>
#include <sys/stream.h>
#include <sys/cmn_err.h>

#ifdef TRACK
#include <sys/track.h>
extern struct track sock_track[];
extern struct track *strk;
extern struct track_hd shd;
extern struct track *tracker();
extern int    scktrk;
#define ENP		0x06000000
#define ENP_SERV	ENP + 0x00010000
#define SCKTRK(id,t1,t2,t3,t4,t5,t6,t7)  {\
		if (scktrk) {\
			strk = tracker(sock_track, &shd);\
			strk->trc_id = id;\
			strk->trc_f1 = (int) (t1);\
			strk->trc_f2 = (int) (t2);\
			strk->trc_f3 = (int) (t3);\
			strk->trc_f4 = (int) (t4);\
			strk->trc_f5 = (int) (t5);\
			strk->trc_f6 = (int) (t6);\
			strk->trc_f7 = (int) (t7);\
		}\
}	
#else		/* tracker not active */
#define SCKTRK(id,t1,t2,t3,t4,t5,t6,t7)
#endif

/*
 *	Useful constants
 */

# define K *1024
# define ENPBUFSIZE 1550

/*
 *	Globals
 */

SHARED_RINGS	*enp;			/* Pointer to ...		*/
SHARED_RINGS	enp_rings;		/* pointers to the rings	*/
CHAN		*data_chan;		/* Data channel array		*/
RING32		h_freebfr;		/* Our free buffer pool		*/

extern HOSTCTL	host;			/* Host/ENP initailization	*/

extern
unsigned long	MYHOSTPARAM;		/* Our Internet address		*/
extern char	MYNODEPARAM[];		/* Our Ethernet address		*/

/*
 *	socket/stream write completion queue for flow control
 */

extern	queue_t *sckenaq[];
extern	int	sckenap;


/*
 *		enpservice()
 *
 *	The "real" interrupt routine - We service ENP interrupts by
 *	first returning any buffers found on the h_netcomplete ring to
 *	the h_freebfr ring. We then process any incoming buffers on the
 *	e_tohost ring (data from the network). The process wakeup and TCP
 *	urgent rings are serviced last.
 *
 *	Called from enpintr().
 */

enpservice()
{
	register CHAN	*cp;
	register RING	*rp;
	register BFR	*bp;
	register int	addr, x;
	register int newbufs = 0;
	register SOCKET *s;
	register sck_found;

	if ((host.h_state & HS_INIT) == 0) {
		init_enp(1, 0);
		return;
	}

	x = ringempty(&h_freebfr);
	while (bp = (BFR *) ringget(enp->h_netcomplete)) {
		hostbfr(bp);
		ringput(&h_freebfr, bp);
		newbufs = 1;
	}
	if (x)
		wakeup(&h_freebfr);

	/* check for write completion for socket/stream flow control */
	if (newbufs && sckenap) {
		for (x=0; x< sckenap; x++) {
			SCKTRK(ENP_SERV + 0x09,sckenaq[x],0,0,0,0,0,0);
			qenable(sckenaq[x]);
		}
		sckenap = NULL;
	}

	while (bp = (BFR *) ringget(enp->e_tohost)) {
		hostbfr(bp);
		cp = &data_chan[(short) bp->b_device];

		SCKTRK(ENP_SERV + 0x01,bp,cp,enp_rval(cp->c_strsock),bp->b_device,0,0,0);

		/* test for stream socket active */
		if (s = (SOCKET *)enp_rval(cp->c_strsock)) {
			rp = (RING *) &cp->c_tohost;
			if (ringput(rp, bp) == 0)
				cmn_err (CE_PANIC,"enpservice: stream ringput failed!",NULL);
			qenable(s->so_strq);
		}
		else
		{
			rp = (RING *) &cp->c_tohost;
			if (ringempty(rp))
				wakeup(rp);
			if (ringput(rp, bp) == 0)
				cmn_err (CE_PANIC,"enpservice: socket ringput failed!",NULL);

#ifdef SELECT
			if (cp->c_selproc) {
				selwakeup(cp->c_selproc, ntohs(cp->c_state)&SS_SELCOLL);
				cp->c_selproc = 0;
				cp->c_state = htons(ntohs(cp->c_state) & SS_SELCOLL);
			}
#endif
		}
	}
	
	/* process outstanding items that are sleeping on buffers 
	 * this normally is connection responses.  If the stream socket is
	 * active, go to special stream handler */
	/*  loop through waiting connections for buffer addresses */
	while ((addr = ringget(enp->e_wakeaddr)) != 0) {
		SCKTRK(ENP_SERV + 0x02,addr,0,0,0,0,0,0);
		for (sck_found = 0, s = &sock_table[0]; s < &sock_table[NSOCKETS]; s++) {
			if (s->so_strq) {
				/* test for connects */
				if ( (unsigned)addr == (unsigned)(s->so_o_bfr)) {
					bp = (BFR *)addr;
					cp = &data_chan[(short)bp->b_device];
					SCKTRK(ENP_SERV + 0x04,bp,cp,s->so_strq,0,0,0,0);
					sckconnintr(s);
					sck_found = 1;
					break;
				}

				/* test for disconnect */
				cp = &data_chan[(short)s->so_enpid];
				if((unsigned)addr == (unsigned)&(cp->c_tohost)) { 
					SCKTRK(ENP_SERV + 0x05,cp,s->so_strq,cp->c_state,0,0,0,0);
					if((cp->c_state & (SS_BROKEN | SS_CONNECTED)) == (SS_BROKEN | SS_CONNECTED)) {
						sck_rcvdiscon(WR((queue_t *)s->so_strq));
						sck_found = 1;
						break;
					}
				}
			}
		}

		/* must be CMC connection - use wakeup mechanism */
		if (!(sck_found))
			wakeup(addr);
	}

	/* Who knows where these signals go.... just don't do any
	 * to a stream for now */
	while ((cp = (CHAN *) ringget(enp->e_urg)) != 0) {
		SCKTRK(ENP_SERV + 0x03,cp,enp_rval(cp->c_strsock),0,0,0,0,0);
		if (!(enp_rval(cp->c_strsock)))
			signal(cp->c_pgrp, SIGURGENT);
	}
}


/*
 *		init_enp()
 *
 *	  The Host/ENP initialization "hand-shake" routine. This routine
 *	will be called twice - First time is from the device driver on
 *	the initial open; we set up some shared stuff and build the list
 *	of INTERFACE level protocols we want up in the ENP, we then
 *	interrupt the ENP (so it can initialize) and wait for it to
 *	signal completion by setting sp->enpbase to non-zero and doing
 *	a "wakeup" on the address of SHARED.
 *
 *	  Second entry is from the interrupt routine (with sp->enpbase
 *	non-zero); we do a wakeup on the shared pointer (which actually
 *	wakes up this routine! - which was sleeping from the first entry).
 *	On the second entry we will initialize our portion of the shared
 *	ring structure and using the buffer allocation from the ENP, build
 *	our free buffer pool (h_freebfr).
 */

init_enp(pass, pb)
register int	pass;
register PIB	*pb;
{
	register SHARED		*sp = (SHARED *) SHAREBASE;
	register SHARED_RINGS	*sr = (SHARED_RINGS *) (sp + 1);
	register PIB		*pb2 = (PIB *) (sr + 1);
	register PKT		*pp;
	register PROTO		*p;
	register int		x;

	int			nchan;

#ifdef VERBOSE
	printf("ENP Init Pass %d\n", pass);
#endif

	if (pass == 1) {		/* from the interrupt routine */
		wakeup(sp);
		return;
	}

	enp_assign(sp->hostbase, ENPBASE);
	enp_assign(sp->enpbase, 0);

	nchan = 0;
	while (pb->pib_name[0] != 0) {
		p = (PROTO *) pffindtype(pb->pib_family, pb->pib_type);
		if (p) {
			/* Set host's variables for protocol		*/

			p->pr_channel = pb->pib_start;	/* Chan. offset	*/
			p->pr_next = pb->pib_start;	/* First alloc	*/
			p->pr_end = pb->pib_start + pb->pib_num;
			nchan += pb->pib_num;

			/* Set ENP's variables for protocol		*/

			pb2->pib_family = htons(pb->pib_family);
			pb2->pib_type = htons(pb->pib_type);
			enp_assign(pb2->pib_start, htonl(pb->pib_start));
			enp_assign(pb2->pib_num, htonl(pb->pib_num));
			enp_move(pb->pib_name, pb2->pib_name,
				 sizeof(pb2->pib_name));

			pb2++;
		}
		pb++;
	}
	pb2->pib_name[0] = 0;		/* Terminate list	*/

	intenp();			/* Interrupt ENP	*/

	x = splnet();
	while (enp_rval(sp->enpbase) == 0)
		sleep(sp, PENP | PCATCH);
	splx(x);
	if (sp->status == -1)		/* ENP failed init	*/
		cmn_err(CE_PANIC,"ENP FAILED INIT",NULL);

	MYHOSTPARAM = ntohl(enp_rval(sp->inetaddr));
	enp_move(sp->enetaddr, MYNODEPARAM, 6);
	data_chan = (CHAN *) (enp_rval(sp->chanaddr));
#ifdef VERBOSE
	printf("Chan addr = 0x%x\n", data_chan);
#endif

/*	Copy in the ADDRESSES of the shared rings		*/

	enp = &enp_rings;
	enp->h_open         = (RING *) enp_rval(sr->h_open);
	enp->h_tonet        = (RING *) enp_rval(sr->h_tonet);
	enp->h_netcomplete  = (RING *) enp_rval(sr->h_netcomplete);
	enp->e_tohost       = (RING *) enp_rval(sr->e_tohost);
	enp->e_hostcomplete = (RING *) enp_rval(sr->e_hostcomplete);
	enp->e_wakeaddr     = (RING *) enp_rval(sr->e_wakeaddr);
	enp->e_urg          = (RING *) enp_rval(sr->e_urg);

/*	Initialize our rings and buffers			*/

	ringinit(&h_freebfr, 32);
	ringinit(enp->h_open, 8);
	ringinit(enp->h_tonet, 16);
	ringinit(enp->h_netcomplete, 16);

	pp = (PKT *) enp_rval(sp->hostbfraddr);

#ifdef VERBOSE
	printf("Rings at 0x%x, Channels at 0x%x, %d Buffers at 0x%x\n",
		enp, data_chan, enp_rval(sp->numhostbfr), pp);
#endif

	for (x = enp_rval(sp->numhostbfr); x > 0; x--)
		ringput(&h_freebfr, pp++);

	for (x = 0; x < nchan; x++) {
		ringinit(&data_chan[(short) x].c_tohost, CHANSIZE);
		sringinit(x);
		data_chan[(short) x].c_state = 0;
	}

	host.h_state = HS_INIT;
}


/*
 *	Utility routines used by the driver, etc.
 */

/*
 *		get_buf()
 *
 *	Get a buffer from the free pool, if none available and "wait"
 *	specified, sleep till someone frees a buffer.
 */

get_buf(dontwait)
{
	register BFR	*bp;
	register int	x;

	x = splnet();
	if (ringempty(&h_freebfr))
		enpservice();
	while ((bp = (BFR *) ringget(&h_freebfr)) == 0) {
		if (dontwait != 0)	
			break;
		sleep(&h_freebfr, PENP);
	}
	splx(x);

	if (bp)
		enp_assign(bp->b_addr, (char *) (bp + 1));

	return ((int) bp);
}


/*
 *		fre_buf()
 *
 *	Return a buffer to the free pool, if pool was empty - do a
 *	wakeup on it.
 *
 *	NOTE: This routine is NOT used by the "main" routine which frees
 *	      buffers found on h_netcomplete - It is intended to be used
 *	      by routines which will only be freeing a single buffer.
 */

fre_buf(bp)
register BFR	*bp;
{
	register int	x;

	x = splnet();
	if (ringempty(&h_freebfr))
		wakeup(&h_freebfr);
	ringput(&h_freebfr, bp);
	splx(x);
}


/*
 *		enpbfr(bp)
 *
 *	Ensure correct byte/word ordering in BFR control fields in
 *	BFR going TO the ENP.
 */

enpbfr(bp)
register BFR	*bp;
{
	bp->b_flags = htons(bp->b_flags);
	bp->b_len = htons(bp->b_len);
	bp->b_msglen = htons(bp->b_msglen);
	bp->b_level = htons(bp->b_level);
	bp->b_device = htons(bp->b_device);
	return ((int) bp);
}


/*
 *		hostbfr(bp)
 *
 *	Ensure correct byte/word ordering in BFR control fields in
 *	BFR coming FROM the ENP.
 */

hostbfr(bp)
register BFR	*bp;
{
	bp->b_flags = ntohs(bp->b_flags);
	bp->b_len = ntohs(bp->b_len);
	bp->b_msglen = ntohs(bp->b_msglen);
	bp->b_level = ntohs(bp->b_level);
	bp->b_device = ntohs(bp->b_device);
}


/*
 *		to_enp()
 *
 *	Send BFR to the ENP by placing it into the proper ring.
 */

to_enp(rp, bp)
register RING	*rp;
register BFR	*bp;
{
	register int	x;

	x = splnet();
	while (ringfull(rp))
		sleep(rp, PENP);
	splx(x);

	ringput(rp, enpbfr(bp));
}


enp_move(src, dest, cnt)
register char	*src;
register char	*dest;
register int	cnt;
{
	while (cnt--)
		*dest++ = *src++;
}


enp_zero(dest, cnt)
register char	*dest;
{
	while (cnt--)
		*dest++ = 0;
}
