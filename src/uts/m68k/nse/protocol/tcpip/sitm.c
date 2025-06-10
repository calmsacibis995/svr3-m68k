/**********************************************************************
*
*	File:		sitm.c
*	Function:	CMC socket/stream driver
*	History:
*	date		modification
*	------		----------------------------------------------
*	09/04/86	add timer routine to detect disconnects
*	09/09/86	scktrk and debug options for trace
*	10/29/86	add s->so_tli transport library state
*	10/29/86	change address to port net addr (len 6)
*	11/01/86	change printf's to strlogs
*	12/18/86	add sck_sanity flag
*
***********************************************************************/

#include "sys/CMC/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/tihdr.h"
#include "sys/tiuser.h"
#include "sys/CMC/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/CMC/errno.h"
#include "sys/CMC/longnames.h"
#include "sys/CMC/netbfr.h"
#include "sys/CMC/ring.h"
#include "sys/CMC/CMC_proto.h"
#include "sys/CMC/CMC_socket.h"
#include "sys/CMC/in.h"
#include "sys/CMC/enp.h"
#include "sys/cmn_err.h"
#include "sys/sitm.h"
#include "sys/strlog.h"
#include "sys/log.h"
#define FNDELAY	04		/* normally located in file.h - but due
				   to dimension overflow put it here */
/*
 * Tracker defines
 */
/* #define	TRACK			/* tracker included			*/
#ifdef TRACK
#include "sys/track.h"
int	scktrk = 1;		/* tracker active			*/
struct		track		sock_track[400];
struct		track		*strk;
struct		track_hd	shd = {0,400};
extern  struct		track		*tracker();
#define SCK		0x05000000
#define SCK_OPEN	SCK + 0x00010000
#define SCK_CLOSE	SCK + 0x00020000
#define SCK_WPUT	SCK + 0x00030000	
#define SCK_WSVC	SCK + 0x00040000	
#define SCK_PRIM	SCK + 0x00050000
#define SCK_CONNINTR	SCK + 0x00060000
#define SCK_RDSRV	SCK + 0x00070000
#define SCK_SEND	SCK + 0x00080000
#define SCK_BACK	SCK + 0x00090000
#define SCK_RDISC	SCK + 0x000a0000
#define SCK_MON		SCK + 0x000b0000
#define SCK_ACTIVE	SCK + 0x000c0000
#define SCK_BCONN	SCK + 0x000d0000
#define SCK_CLEAN	SCK + 0x000e0000
#define SCK_CREQ	SCK_PRIM + T_CONN_REQ		/* 0 */
#define SCK_CRES	SCK_PRIM + T_CONN_RES		/* 1 */
#define SCK_DISCON	SCK_PRIM + T_DISCON_REQ		/* 2 */
#define SCK_DATA	SCK_PRIM + T_DATA_REQ		/* 3 */
#define SCK_INFO	SCK_PRIM + T_INFO_REQ		/* 5 */
#define SCK_BIND	SCK_PRIM + T_BIND_REQ		/* 6 */
#define SCK_UNBIND	SCK_PRIM + T_UNBIND_REQ		/* 7 */
#define SCK_OPTMGT	SCK_PRIM + T_OPTMGMT_REQ	/* 9 */
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
 *  external references
 */
extern enp_in_system;
extern HOSTCTL host;
extern SHARED_RINGS *enp;
extern CHAN *data_chan;
extern PIB p_list[];
extern chan_inuse;

/*
 *  stream data structure definitions
 */
int sckopen(), sckclose(), sckuwput(), sckuwsrv(), sckrdsrv();
extern nulldev();
static struct module_info sck_minfo = {SOCK_ID, "sock", 0, MAX_TCP_DATA, MAX_TCP_DATA/2, MAX_TCP_DATA/4};
static struct qinit sckurinit = { NULL, sckrdsrv, sckopen, sckclose, nulldev, &sck_minfo, NULL};
static struct qinit sckuwinit = { sckuwput, sckuwsrv, sckopen, sckclose, nulldev, &sck_minfo, NULL};
struct streamtab sckinfo = { &sckurinit, &sckuwinit, NULL, NULL };

/*
 *	tli_cmc_map - map tli calls to cmc socket interface routines
 */
extern sck_creq(), sck_cres(), sck_discon(), sck_data(), sck_info(), sck_bind(),
       sck_unbind(), sck_optmgmt(), sck_monitor();

static (*tli_cmc_map[])() = {
	sck_creq,	/* 0 - T_CONN_REQ */
	sck_cres,	/* 1 - T_CONN_RES */
	sck_discon,	/* 2 - T_DISCON_REQ */
	sck_data,	/* 3 - T_DATA_REQ */
	sck_data,	/* 4 - T_EXDATA_REQ */
	sck_info,	/* 5 - T_INFO_REQ */
	sck_bind,	/* 6 - T_BIND_REQ */
	sck_unbind,	/* 7 - T_UNBIND_REQ */
	NULL,		/* 8 - T_UNITDATA_REQ:  not supported */
	sck_optmgmt,	/* 9 - T_OPTMGMT_REQ */
};

/*
 * Mapping of TLI T_primitive types to state machine events
 *
 * T_INFO_REQ is removed since it doesn't affect state, and
 *	T_UNITDATA_REQ and T_ORDREL_REQ are not supported.
 */
#define NPRIM T_ORDREL_REQ+1
#define BADEVENT TE_NOEVENTS
#define NONEVENT 0

int tli_to_cmc[NPRIM] = {
	TE_CONN_REQ,		/*  0 - T_CONN_REQ */
	TE_CONN_RES,		/*  1 - T_CONN_RES */
	TE_DISCON_REQ,		/*  2 - T_DISCON_REQ */
	TE_DATA_REQ,		/*  3 - T_DATA_REQ */
	TE_EXDATA_REQ,		/*  4 - T_EXDATA_REQ */
	NONEVENT,		/*  5 - T_INFO_REQ */
	TE_BIND_REQ,		/*  6 - T_BIND_REQ */
	TE_UNBIND_REQ,		/*  7 - T_UNBIND_REQ */
	BADEVENT,		/*  8 - T_UNITDATA_REQ */
	TE_OPTMGMT_REQ,		/*  9 - T_OPTMGMT_REQ */
	BADEVENT,		/* 10 - T_ORDREL_REQ */
};

/*
 *	TLI state transition table defines
 */
extern char ti_statetbl[TE_NOEVENTS][TS_NOSTATES];
#define NEXTSTATE(X, Y)		ti_statetbl[X][Y]

/*
 *	Write completion queue enable list - flow control mechanism
 */
#define	SCK_ENABLE(q) {\
	int pri;\
	pri = splstr();\
	sckenaq[sckenap++] = q;\
	splx(pri);\
}
#define NQUEUE	196		/* number of queues - see conf.c */
queue_t *sckenaq[NQUEUE];
int	sckenap = 0;		/* current position pointer */

/*
 *	Socket Disconnect Detection
 */
int	sck_timer = 0;		/* flag for polling active */
extern 	sck_sanity;		/* sanity packet flag 1=active  sysgen/kernel */

/*
/************************************************************************
*
*	File:		sitm.c
*	Routine:	sckopen
*	Function:	open virtual circuit by allocation of a socket
*
************************************************************************/

sckopen(q, dev, flag, sflag)
  register queue_t *q;
{
	register SOCKET *s;
	register PBLOCK *pb;
	register PROTO	*pr;
	struct PBLOCK pblock;
	pb = &pblock;

	SCKTRK(SCK_OPEN,q,dev,flag,sflag,chan_inuse,0,0);
	STRLOG(SOCK_ID, SCK_NUM(q->q_ptr),0,SL_TRACE,
	       "sckopen: channel count = %d\n",chan_inuse);

	/* test for existance of the board */
	if (!(enp_in_system)) {
		u.u_error = ENXIO;
		return(OPENFAIL);
	}


	/* if already open - all ok */
	if (q->q_ptr) {
		return(SCK_NUM(q->q_ptr));
	}

	/* if first open - start enp and init domain stuff */
	if ((host.h_state & HS_INIT) == 0) {	/* ENP not inited	*/
		SCKTRK(SCK_OPEN+0x01,q,dev,flag,sflag,host.h_state,0,0);
		domaininit();		/* Set up protocol struct FIRST	*/
		init_enp(0, p_list);	/* Init ENP & protos we want	*/
	}

	/* start Socket Disconnect Detection Monitor */
	if (!(sck_timer) && sck_sanity) {
		timeout(sck_monitor, 0, SDD_TIMEOUT);
		sck_timer++;
	}

	/* if clone open - find empty socket and continue */
	if (sflag == CLONEOPEN) {
		for ( s = sock_table; s < &sock_table[NSOCKETS]; s++)
			if (!(s->so_state & SS_OPEN))
				break;
		if (s >= &sock_table[NSOCKETS]) {
			u.u_error = ENXIO;
			return(OPENFAIL);
		}
	}
	else {
		/* if greater than number of sockets */
		if (minor(dev) >= NSOCKETS) { 
			u.u_error = ENODEV;
			return(OPENFAIL);
		}

		s = &sock_table[(short) minor(dev)];
	}

	if (s->so_state & (SS_RESV | SS_OPEN))
		u.u_error = EEXIST;
	else { 
		enp_zero(s, sizeof(SOCKET));
		s->so_state = SS_OPEN;
	}

 	/* hard cord definitions for tcp/ip here - may be changed to
 	   support datagram, rip, or raw ethernet */

	pb->p_domain = AF_INET;
	pb->p_type = SOCK_STREAM;
	pb->p_proto = IPPROTO_TCP;

	pr = (pb->p_proto) ? (PROTO *) pffindproto(pb->p_domain, pb->p_proto) :
			     (PROTO *) pffindtype(pb->p_domain, pb->p_type);

	if ((pr == 0) || (pb->p_type && (pr->pr_type != pb->p_type))) {
		u.u_error = ESOCKTNOSUPPORT;
		return (0);
	}
	
	/* setup streams pointer */
	s->so_proto = pr;
	q->q_ptr = (char *)s;
	WR(q)->q_ptr = (char *)s;
	s->so_strq = (caddr_t)(q);

	/* mark link as inactive but not yet connected */
	s->so_link = SDD_INACTIVE;
	s->so_tli = TS_UNBND;
	return(SCK_NUM(s));
}

/*
/*********************************************************************
*
*	File:		sitm.c
*	Routine:	sckclose()
*	Function:	Terminate virtual circuit/socket
*
**********************************************************************/

sckclose(q)
  register queue_t *q;
{
	register SOCKET	*s;
	register CHAN	*cp;
	register BFR	*bp;
	register int	x;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_CLOSE+0xff,q,q->q_first,q->q_last,q->q_count,0,0,0);
		return;
	}

	SCKTRK(SCK_CLOSE,q,q->q_first,q->q_last,q->q_count,s,s->so_state,q->q_ptr);

	x = splnet();
	enpservice();
	OTHERQ(q)->q_ptr = NULL;
	q->q_ptr = NULL;
	s->so_strq = NULL;

	if ((s->so_state & SS_OPEN) == 0) { 
		u.u_error = EBADF;
		splx(x);
		return;
	}

	if ((s->so_state & SS_RESV) == 0) {	/* No channel allocated	*/
		s->so_state = 0;
		splx(x);
		return;
	}

	cp = &data_chan[(short) s->so_enpid];
	cp->c_state = htons(ntohs(cp->c_state) | SS_FLUSHING);
	enp_assign(cp->c_strsock, NULL);

	(*s->so_proto->pr_close)(s);

	while ((bp = (BFR *) ringget(&cp->c_tohost))) {
		to_enp(enp->e_hostcomplete, bp);
		sringget(&cp->c_sring);
	}
	splx(x);

	--chan_inuse;
	s->so_state = 0;			/* Free the socket	*/
}

/*
/************************************************************************
*
*	File:		sitm.c
*	Routine:	sckclean(q)
*	Function:	Clean up socket and free channel
*
*************************************************************************/

sckclean(q)
  register queue_t *q;
{
	register x;
	register SOCKET *s;
	register CHAN   *cp;
	register BFR    *bp;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_CLEAN+0xff,q,q->q_first,q->q_last,q->q_count,0,0,0);
		return;
	}

	SCKTRK(SCK_CLEAN,q,q->q_first,q->q_last,q->q_count,s,s->so_state,q->q_ptr);

	x = splnet();
	cp = &data_chan[(short) s->so_enpid];
	cp->c_state = htons(ntohs(cp->c_state) | SS_FLUSHING);

	while ((bp = (BFR *) ringget(&cp->c_tohost))) {
		to_enp(enp->e_hostcomplete, bp);
		sringget(&cp->c_sring);
	}
	splx(x);
}

/*
/************************************************************************
*
*	File:		sitm.c
*	Routine:	sckuwput(q,mp)
*	Function:	Handle data requests and map PROTO to cmc
*
*************************************************************************/

sckuwput(q, mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register SOCKET *s;
	register union T_primitives *prim;
	register mblk_t *tmp;
	struct iocblk *iocp;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_WPUT+0xff,q,mp,mp->b_next,mp->b_prev,mp->b_rptr,mp->b_wptr,mp->b_datap);
		freemsg(mp);
		return;
	}

	SCKTRK(SCK_WPUT,q,mp,mp->b_next,mp->b_prev,mp->b_rptr,mp->b_wptr,mp->b_datap);
	SCKTRK(SCK_WPUT + 0x01, mp->b_datap->db_base,mp->b_datap->db_lim,mp->b_datap->db_class,mp->b_datap->db_type,s,
	       *mp->b_rptr,0);

	switch(mp->b_datap->db_type) {
	case M_DATA: {
		if (!(s->so_state & SS_CONNECTED)) {
			sck_bad_ack(q, mp, TSYSERR, EBADF);
			return;
		}

		/* now put a msg buffer on the front so sck_sendit works */
		if((tmp = allocb(sizeof(struct T_data_req),BPRI_MED)) == NULL) {
			freemsg(mp);
			sck_fatal(q);
			return;
		}
		linkb(tmp,mp);
		prim = (union T_primitives *)tmp->b_rptr;
		prim->data_req.PRIM_type = T_DATA_REQ;
		prim->data_req.MORE_flag = 0;
		tmp->b_wptr = tmp->b_wptr + sizeof(struct T_data_req);

		/* all ok - send data */
		sck_sendit(q, tmp);
		}
		break;

	case M_PROTO:
	case M_PCPROTO:
		/*
	 	 * Check for valid tli primitives and call
		 * appropriate routine for processing
		 */
		prim = (union T_primitives *)mp->b_rptr;
		if (prim->type >= NPRIM) {
			STRLOG(SOCK_ID, SCK_NUM(q->q_ptr),0,SL_ERROR | SL_TRACE,
			       "sckuwput: bad primitive type %d\n", prim->type);
			freemsg(mp);
			sck_fatal(q);
			return;
		}
		if (tli_to_cmc[prim->type] == BADEVENT) {
			STRLOG(SOCK_ID, SCK_NUM(q->q_ptr),0,SL_ERROR | SL_TRACE,
			       "sckuwput: unsupported primitive type %d\n",
				prim->type);
			sck_bad_ack(q, mp, TNOTSUPPORT, 0);
			return;
		}

		(*tli_cmc_map[prim->type])(q, mp);
		return;

	case M_IOCTL:
		/*
		 * blow off ioctl calls for now
		 * maybe of use later so leave stub active
		 */
		iocp = (struct iocblk *)mp->b_rptr;

		switch (iocp->ioc_cmd) {
		case I_LINK: 
		case I_UNLINK:
		default:
			mp->b_datap->db_type = M_IOCNAK;
			break;
		}
		qreply(q, mp);
		return;

 	case M_FLUSH:
		/*
		 * Flush both queues as appropriate and turn
		 * back upstream (not sending on to driver)
		 */
		if (*mp->b_rptr & FLUSHW) {
			flushq(q, FLUSHALL);
			*mp->b_rptr &= ~FLUSHW;
		}
		if (*mp->b_rptr & FLUSHR) {
			flushq(RD(q), FLUSHALL);
			qreply(q, mp);
		} else
			freemsg(mp);
		return;
		
	default:
		STRLOG(SOCK_ID, SCK_NUM(q->q_ptr),0,SL_ERROR | SL_TRACE,
		       "sckuwput: unknown STR msg type %o received\n",
	       		mp->b_datap->db_type);
		freemsg(mp);
		return;
	}
}
/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sckuwsrv(q)
*	Function:	remove data indication messages and sent to enp
*			until flow control stops data flow.  This routine is
*			scheduled when a buffer is placed on the queue, or
*			when the enp responds with this channel number	
*
****************************************************************************/

sckuwsrv(q)
  register queue_t *q;
{
	register SOCKET *s;
	register BFR *bp;
	register PROTO *pr;
	mblk_t *mp,*dmp;
	int sent, i, totalcnt, datacnt;
	SITM shdr, *sptr;
	CONN *conn_ptr;
	union T_primitives *prim;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) 
		return;
	pr = s->so_proto;
	sent = 0;

	SCKTRK(SCK_WSVC,q,q->q_first,q->q_last,q->q_count,s,0,0);

	/* remove messages from queue and send if possible
	   on any flow control situations, place remaining data buffers,
	   along with data request message on the front of the queue */
	while (dmp = getq(q)) {
		while (mp = dmp->b_cont) {
			/* check if possible to place more buffers into ring */
			if (ringfull(enp->h_tonet)) {
				putbq(q,dmp);
				SCK_ENABLE(q);
				return(-1);
			}

			while (mp->b_wptr - mp->b_rptr) {
				if ((bp = (BFR *) get_buf(FNDELAY)) == 0) {
					putbq(q,dmp);
					SCK_ENABLE(q);
					return (-1);
				}

				/* if not data message assume control */
				if (datamsg(dmp->b_datap->db_type))
					shdr.type = SITM_DATA;
				else {
					/* M_CTL message to retry connects and
					   sanity packets that could not get
					   enp buffers first time around */
					prim = (union T_primitives *)dmp->b_rptr;
					switch(prim->type) {
					     case T_DATA_REQ:
						sptr = (SITM *)mp->b_rptr;
						shdr.type = sptr->type;
						break;
					     case T_CONN_REQ:
						conn_ptr = (CONN *)mp->b_rptr;
						fre_buf(bp);
						if((*s->so_proto->pr_connect)(s, conn_ptr->to, conn_ptr->type, conn_ptr->flags))
							STRLOG(SOCK_ID, SCK_NUM(s),0,SL_TRACE,
							       "sckuwsrv: Connect request failed\n");
						mp->b_wptr = mp->b_rptr;
						continue;
					     default:
						STRLOG(SOCK_ID, SCK_NUM(s),0,SL_TRACE,
						       "sckuwsrv: bad M_CTL message type %x\n",prim->type);
						mp->b_wptr = mp->b_rptr;
						continue;
					}
				}

				if(sck_sanity) {
					totalcnt = min((mp->b_wptr - mp->b_rptr) + sizeof(SITM), pr->pr_maxdata);
					datacnt = totalcnt - sizeof(SITM);
					enp_move((char *)&shdr.type, enp_rval(bp->b_addr) + pr->pr_offset,
						sizeof(SITM));
					enp_move(mp->b_rptr, enp_rval(bp->b_addr) + 
				 		pr->pr_offset + sizeof(SITM), datacnt);
				}
				else {
					totalcnt = min((mp->b_wptr - mp->b_rptr), pr->pr_maxdata);
					datacnt = totalcnt;
					enp_move(mp->b_rptr, enp_rval(bp->b_addr) + 
				 		pr->pr_offset, datacnt);
				}

				bp->b_flags = 0;
				bp->b_len = totalcnt;
				bp->b_device = s->so_enpid;
				to_enp(enp->h_tonet, bp);
	
				mp->b_rptr += datacnt;
				sent += datacnt;
			}
			SCKTRK(SCK_WSVC+0x01,q,mp,q->q_first,q->q_count,s,bp,mp->b_cont);
			dmp->b_cont = mp->b_cont;
			freeb(mp);
		}
		freemsg(dmp);
	}
	return(sent);
}
/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sckconnintr(s)
*	Function:	field interrupts after enpservice has determined
*			that the response is for a stream socket. The
*			determination is via the cp->c_strsock field in the
*			channel structure.	
*	Parameters:	s = socket structure pointer
*
****************************************************************************/

sckconnintr(s)
  SOCKET *s;
{
	register struct T_conn_ind *conn_ind;
	register struct T_conn_con *conn_con;
	register queue_t *q;
	register BFR *bp;
	register OPN *op;
	mblk_t *ok;
	
	q = (queue_t *)s->so_strq;
	SCKTRK(SCK_CONNINTR,q,q->q_first,q->q_last,q->q_count,s,s->so_state,0);

	/* if in listen state, send connection indication to tli
	   DO NOT set SS_CONNECTED until accept is complete.  This 
	   will block received data until stream is ready */
	if (s->so_state & SS_LISTEN) {
		if ((ok = allocb(sizeof(struct T_conn_ind) 
		     + SCK_ADDR_SIZE, BPRI_HI)) == NULL) {
			STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
			       "sckconnintr: couldn't alloc buffer for conn_ind\n");
			return;
		}

		/* send connection indication to fulfil recv by tli user */
		ok->b_rptr = ok->b_datap->db_base;
		conn_ind = (struct T_conn_ind *)ok->b_rptr;
		conn_ind->PRIM_type = T_CONN_IND;
		conn_ind->SRC_length = SCK_ADDR_SIZE;
		conn_ind->SRC_offset = sizeof(struct T_conn_ind);
		conn_ind->OPT_length = 0;
		conn_ind->OPT_offset = 0;
		conn_ind->SEQ_number = 0;
		ok->b_datap->db_type = M_PROTO;
		ok->b_wptr = ok->b_rptr + sizeof(struct T_conn_ind);
		sck_gname(s, ok->b_wptr);
		ok->b_wptr += SCK_ADDR_SIZE;
		putnext(q,ok);
		
		/* set tli state to waiting response to connection indication */
		s->so_tli = TS_WRES_CIND;

		/* release the buffer for the listener */
		bp = s->so_o_bfr;
	}
	/* if connecting, send response for T_CONN_REQ to tli */
	else if (s->so_state & SS_CONNECTING) {
		op = s->so_o_opn;
		bp = s->so_o_bfr;

		/* debug for possible window */
		if (op->o_status == -1) {
			SCKTRK(SCK_CONNINTR + 0x02,op,bp,op->o_status,0,0,0,0);
			STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
			       "sckconnintr: WARNING buffer still active in enp\n");
		}

		/* if status -
			       check for connection refused or not alive
			       if status still = not complete panic
			       if printf and no panic then window exists */
		if (op->o_status) {
			SCKTRK(SCK_CONNINTR + 0x01,op,bp,op->o_status,0,0,0,0);
			STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
			       "sckconnintr: Buffer still active 1\n");
			if (op->o_status == -1) {
				SCKTRK(SCK_CONNINTR + 0x03,op,bp,op->o_status,0,0,0,0);
				STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
				       "sckconnintr: Buffer still active 3\n");
			}
			sck_rcvdiscon(WR(q));
		} else {
			if ((ok = allocb(sizeof(struct T_conn_con) 
			          + SCK_ADDR_SIZE, BPRI_HI)) == NULL) {
				STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
				       "sckconnintr: couldn't alloc buffer for conn_con\n");
				bufcall(sizeof(struct T_conn_con) 
				        + SCK_ADDR_SIZE, BPRI_HI, 
				        sckconnintr,q);
				return;
			}

			/* mark connected - tested in get name routine */
			s->so_state |= SS_CONNECTED;

			/* send connection confirmation to waiting ti user */
			ok->b_rptr = ok->b_datap->db_base;
			conn_con = (struct T_conn_con *)ok->b_rptr;
			conn_con->PRIM_type = T_CONN_CON;
			conn_con->OPT_length = 0;
			conn_con->OPT_offset = 0;
			conn_con->RES_length = SCK_ADDR_SIZE;
			conn_con->RES_offset = sizeof(struct T_conn_con);
			ok->b_datap->db_type = M_PROTO;
			ok->b_wptr = ok->b_rptr + sizeof(struct T_conn_con);
			sck_gname(s, ok->b_wptr);
			ok->b_wptr += SCK_ADDR_SIZE;
			putnext(q,ok);

			/* set tli state to data transfer */
			s->so_tli = TS_DATA_XFER;
		}

	} else
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
		       "sckconnintr: Not listening or Connecting\n");

	/* connect action is over - free buffer and reset state */
	s->so_state &= ~SS_CONNECTING;
	s->so_o_opn = 0;
	s->so_o_bfr = 0;
	fre_buf(bp);
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sckrdsrv(q)
*	Function:	process data from network by placing into 
*			a stream queue	
*
****************************************************************************/

sckrdsrv(q)
  queue_t *q;
{
	register union T_primitives *prim;
	register CHAN *cp;
	register priflg = 0;
	register pri;
	mblk_t *mp,*dmp;
  	SOCKET *s;
  	BFR *buf;
	SITM shdr;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_RDSRV+0xff,q,0,0,0,0,0,0);
		return;
	}
	cp = &data_chan[(short)s->so_enpid];

	/* test if connection has completed - if not wait for accept to
	   re-schedule this routine.  This routine should allow CMC flow
	   control mechanism between enp and host to continue functioning */
	if (s->so_state & SS_CONNECTED) {
		if (ntohs(cp->c_state) & SS_BROKEN) {
			SCKTRK(SCK_RDSRV+0xfe,q,0,0,0,0,0,0);
			sck_rcvdiscon(WR(q));
			return;
		}
		pri = splnet();
		while((canput(q->q_next)) && (buf=(BFR *)fir(&cp->c_tohost))) {
			SCKTRK(SCK_RDSRV,s,q,cp,buf,buf->b_msglen,s->so_state,0);

			/* received buffer - mark this connection SDD_ACTIVE */
			s->so_link = SDD_ACTIVE;

			if(sck_sanity) {
				/* take buffer from enp and remove sitm header
				   If monitor packet, send back a good response */
				enp_move(enp_rval(buf->b_addr), (char *)&shdr.type, sizeof(SITM));
				buf->b_msglen -= sizeof(SITM);
				enp_assign(buf->b_addr, enp_rval(buf->b_addr) + sizeof(SITM));
				switch (shdr.type) {
					case SITM_ACTIVE:
						sckactive(WR(q),s,SITM_CTL);
					case SITM_CTL:
						SCKTRK(SCK_RDSRV + 0x01,cp,buf,buf->b_msglen,shdr.type,enp_rval(buf->b_addr),0,0);
						to_enp(enp->e_hostcomplete, buf);
						ringget(&cp->c_tohost);
						sringget(&cp->c_sring);
						continue;
						break;
					case SITM_DATA:
						break;
					default:
						STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
					       	       "sckrdsrv: unknown SITM header\n");
						to_enp(enp->e_hostcomplete, buf);
						ringget(&cp->c_tohost);
						sringget(&cp->c_sring);
						continue;
						break;
				}
			}
			if ((dmp = allocb(sizeof(struct T_data_ind),BPRI_HI)) == NULL) {
				STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
				       "sckrdsrv: couldn't alloc buffer for T_data_ind\n");
				splx(pri);
				return;
			}

			/* build buffer for data indication */
			dmp->b_rptr = dmp->b_datap->db_base;
			dmp->b_datap->db_type = M_PROTO;
			prim = (union T_primitives *)dmp->b_rptr;
		
			/* check for normal or expidited flow */
			if (priflg) 
				prim->data_ind.PRIM_type = T_EXDATA_IND;
			else
				prim->data_ind.PRIM_type = T_DATA_IND;
	
			prim->data_ind.MORE_flag = 0;
			dmp->b_wptr = dmp->b_rptr + sizeof(struct T_data_ind);

			if ((mp = allocb(buf->b_msglen, BPRI_HI)) == NULL) {
				STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
				       "sckrdsrv: couldn't alloc buffer for data\n");
				sck_bad_ack(q, mp, TSYSERR,EAGAIN);
				splx(pri);
				return;
			}

			mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
			mp->b_datap->db_type = M_DATA;
			enp_move(enp_rval(buf->b_addr)
				 , mp->b_wptr
				 , buf->b_msglen);
			mp->b_wptr += buf->b_msglen;

			/* free enp buffers and clean out shadow ring */
			to_enp(enp->e_hostcomplete, buf);
			ringget(&cp->c_tohost);
			sringget(&cp->c_sring);

			/* link data buffers to Data_indication message */
			linkb(dmp,mp);
			putnext(q,dmp);
		}
		splx(pri);
	}
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_creq()
*	Function:	Process tli primitive T_CONN_REQ
*
****************************************************************************/

sck_creq(q,mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register SOCKET *s;
	register union T_primitives *prim;
	register CHAN *cp;
	sockaddr_in toname;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_CREQ+0xff,q,0,0,0,0,0,0);
		return;
	}

	prim = (union T_primitives*)mp->b_rptr;

 	/* zero address holder toname to allow for 8 byte rfs address */
 	bzero(&toname, sizeof(sockaddr_in));

	SCKTRK(SCK_CREQ,q,s,&toname,prim->conn_req.DEST_length,0,0,0);
	
	if (prim->conn_req.DEST_length != SCK_ADDR_SIZE) {
		sck_bad_ack(q, mp, TBADADDR, 0);
		return;
	}

	if (prim->conn_req.OPT_length != 0) {
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
		       "sck_creq:  connect options requested\n");
		sck_bad_ack(q, mp, TBADOPT, 0);
		return;
	}
	toname.sin_family = AF_INET;
	bcopy((char *)(mp->b_rptr + prim->conn_req.DEST_offset),
		 (char *)&toname.sin_port, SCK_ADDR_SIZE);

	/* do protocol connect - set -1 to avoid sleep in tcpconnect */
	if ((s->so_enpid = chan_alloc(s)) == -1) {
		sck_bad_ack(q, mp, TBADADDR, 0);
		return;
	}
	s->so_state |= SS_RESV;
	cp = &data_chan[(short)s->so_enpid];
	enp_assign(cp->c_strsock, (caddr_t)s);

	/* connect may fail due to no enp buffers 
	   to retry, place M_CTL message on queue and try on buffer
	   free
	*/
	if((*s->so_proto->pr_connect)(s, &toname, 0, -1)) 
		sckconnect(q,s,&toname,0,-1);

	/* return ok ack to tli - this allows async connections */
	s->so_tli = TS_WCON_CREQ;
	sck_ok_ack(q, mp, T_CONN_REQ);
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_cres()
*	Function:	Process tli primitive T_CONN_RES
*
****************************************************************************/

sck_cres(q,mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register SOCKET *s, *s2;
	register union T_primitives *prim;
	register queue_t *s2q;
	register CHAN *cp;
	short save_enpid;

	prim = (union T_primitives *)mp->b_rptr;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_CRES+0xff,q,0,0,0,0,0,0);
		return;
	}

	SCKTRK(SCK_CRES,q,mp,prim->conn_res.QUEUE_ptr,prim->conn_res.OPT_length,
		s->so_proto->pr_type,s->so_state,prim->conn_res.QUEUE_ptr->q_ptr);
	SCKTRK(SCK_CRES,0x01,s->so_enpid,0,0,0,0,0);

	if (prim->conn_res.OPT_length != 0) {
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
		       "sck_cres: Connection response options requested\n");
		sck_bad_ack(q, mp, TBADOPT, 0);
		return;
	}

	if (s->so_proto->pr_type != SOCK_STREAM) {
		sck_bad_ack(q, mp, TBADF, 0);
		return;
	}

	if (!(s->so_state & SS_LISTEN)) {
		sck_bad_ack(q, mp, TBADF, 0);
		return;
	}

	s2q = prim->conn_res.QUEUE_ptr;
	if (!(s2 = (SOCKET *)s2q->q_ptr)) {
		sck_bad_ack(q, mp, TBADF, 0);
		return;
	}
		
	/* connection is established - move socket info to accepter 
	 * update the channel structure to reflect correct socket
	 * structure.  socket structure is updated to reflect new q
	 * (ie listener) instead of the acceptors q
	 * and respond with an ok_ack */

	/* move listen information to acceptor */
	*s2 = *s;
	s2->so_state = (s2->so_state & ~SS_LISTEN) | SS_CONNECTED;
	s2->so_strq = (char *)s2q;
	s2->so_lim_listen = NULL;
	cp = &data_chan[(short)s2->so_enpid];
	enp_assign(cp->c_strsock, (char *)s2);

	/* get a new channel for listener and add stream specific information */
	s->so_state &= ~SS_CONNECTED;
	sck_ok_ack(q, mp, T_CONN_RES);

	/* enable RD(s2q) to start any data pending on the acception */
	s2->so_tli = TS_DATA_XFER;
	qenable(s2q);

	/* if this is the listen socket, re-send connect to enp
	   to field next connection */
	if (s->so_lim_listen)  {
		if ((s->so_enpid = chan_alloc(s)) == -1) {
			SCKTRK(SCK_CONNINTR + 0x03,s,q,s2,s2q,s->so_state,0,0);
			STRLOG(SOCK_ID, SCK_NUM(q->q_ptr),0,SL_TRACE | SL_ERROR | SL_NOTIFY,
	       		"sck_cres: NO Channels for listen \nchannel count = %d\n",chan_inuse);
			return;
		}
		s->so_state |= SS_RESV;
		cp = &data_chan[(short)s->so_enpid];
		enp_assign(cp->c_strsock, (caddr_t)s);
		s->so_tli = TS_WACK_CREQ;

		/* connect may fail due to out of enp buffer condition */
		if((*s->so_proto->pr_connect)(s, 0, SS_LISTEN, -1))
			sckconnect(q,s,0,SS_LISTEN,-1);
	}
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_discon()
*	Function:	Process tli primitive T_DISCONN_REQ
*
****************************************************************************/

sck_discon(q,mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register SOCKET *s;
	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_DISCON+0xff,q,0,0,0,0,0,0);
		return;
	}

	/* mark socket closed for i/o and respond with an ok ack */
	SCKTRK(SCK_DISCON,q,mp,s,s->so_state,s->so_tli,0,0);
	s->so_state |= SS_NOREAD | SS_NOWRITE;
	sckclean(q);

	if (!sck_flush(RD(q))) {
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
		       "sck_discon: send_flush failed\n");
		sck_bad_ack(q, mp, TSYSERR, EPROTO);
	}
	sck_ok_ack(q,mp,T_DISCON_REQ);
	s->so_tli = TS_IDLE;
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_rcvdiscon(q)
*	Function:	socket channel marked SS_BROKEN - send T_DISCON_IND	
*
****************************************************************************/

sck_rcvdiscon(q)
  register queue_t *q;
{
	register struct T_discon_ind *discon_ind;
	mblk_t *dis;
	register SOCKET *s;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_RDISC+0xff,q,0,0,0,0,0,0);
		return;
	}


	if ((dis = allocb(sizeof(struct T_discon_ind), BPRI_HI)) == NULL) {
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
		       "sck_rcvdiscon: couldn't allocate buffer for discon\n");
		return;
	}

	if (!sck_flush(RD(q))) 
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
		       "sck_rcvdiscon: send_flush failed\n");
	
	SCKTRK(SCK_RDISC,q,dis,s,s->so_tli,0,0,0);
	sckclean(q);
	dis->b_wptr = dis->b_rptr + sizeof(struct T_discon_ind);
	dis->b_datap->db_type = M_PROTO;
	discon_ind = (struct T_discon_ind *)dis->b_rptr;
	discon_ind->PRIM_type = T_DISCON_IND;
	discon_ind->DISCON_reason = 0;
	discon_ind->SEQ_number = -1;
	
	qreply(q,dis);
	s->so_tli = TS_IDLE;
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_data()
*	Function:	Process tli primitive T_DATA_REQ
*
****************************************************************************/

sck_data(q,mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register SOCKET *s;
	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_DATA+0xff,q,0,0,0,0,0,0);
		return;
	}

	/*
	SCKTRK(SCK_DATA,q,mp,s,s->so_state,s->so_proto->pr_flags,0,0);
	*/

	if (!(s->so_state & SS_BOUND)) {
		sck_bad_ack(q, mp, TBADF, 0);
		return;
	}

	if ((s->so_proto->pr_flags & PR_CONNREQUIRED) 
	    && ((s->so_state & SS_CONNECTED) == 0)) {
		sck_bad_ack(q, mp, TOUTSTATE,  0);
		return;
	}

	sck_sendit(q, mp);
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_info()
*	Function:	Process tli primitive T_INFO_REQ
*
****************************************************************************/

sck_info(q,mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register struct T_info_ack *info_ack;
	register SOCKET *s;
	mblk_t *ack;

	if ((ack = allocb(sizeof(struct T_info_ack), BPRI_HI)) == NULL) {
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
		       "sck_info: couldn't allocate buffer for info_ack\n");
		sck_bad_ack(q, mp, TSYSERR, EAGAIN);
		return;
	}
	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_INFO+0xff,q,0,0,0,0,0,0);
		return;
	}

	SCKTRK(SCK_INFO,q,mp,ack,s->so_tli,0,0,0);

	freemsg(mp);
	ack->b_wptr = ack->b_rptr + sizeof(struct T_info_ack);
	ack->b_datap->db_type = M_PCPROTO;
	info_ack = (struct T_info_ack *)ack->b_rptr;
	info_ack->PRIM_type = T_INFO_ACK;
	info_ack->TSDU_size = MAX_TCP_DATA;
	info_ack->ETSDU_size = ETSDU_SIZE;
	info_ack->CDATA_size = CDATA_SIZE;
	info_ack->DDATA_size = DDATA_SIZE;
	info_ack->ADDR_size = ADDR_SIZE;
	info_ack->OPT_size = OPT_SIZE;
	info_ack->TIDU_size = TIDU_SIZE;
	info_ack->SERV_type = T_COTS;

	info_ack->CURRENT_state =  s->so_tli;
	qreply(q, ack);
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_bind()
*	Function:	Process tli primitive T_BIND_REQ
*	Flow:		if bind address is not provided - get it by calling
*			protocol nmck.  After filling out sin_addr/port
*			check to make sure port is not bound. If all is
*			ok, send bind to enp and send back bind ack.
*
****************************************************************************/

sck_bind(q,mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register union T_primitives *prim;
	register SOCKET *s;
	sockaddr_in *inp;
	register PROTO *pr;
	register CHAN *cp;

	prim = (union T_primitives *)mp->b_rptr;
	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_BIND+0xff,q,0,0,0,0,0,0);
		return;
	}
	pr = s->so_proto;

	/* DANGER - this is the listen/connect/sequence magic */
	s->so_lim_listen = min(prim->bind_req.CONIND_number, SOMAXCONN);

	SCKTRK(SCK_BIND,q,mp,prim->bind_req.ADDR_length,s->so_lim_listen,s,s->so_tli,0);

	/* setup address for internet address */
	inp = (sockaddr_in *) s->so_name;
	inp->sin_family = AF_INET;

	if (prim->bind_req.ADDR_length == 0) {
		register mblk_t *new;

		/* set for internet family - if listener mark identity
		   as IPPORT_RFS, otherwise just use a free port id */
		if (s->so_lim_listen) 
			inp->sin_port = IPPORT_RFS;

		/* check for valid name and fill in sin_addr */
		if ((*pr->pr_nmck)(s)) {
			sck_bad_ack(q, mp, TBADF, 0);
			return;
		}
		s->so_nmlen = sizeof(struct sockaddr_in);

		if ((new = allocb(sizeof(struct T_bind_ack) 
			+ SCK_ADDR_SIZE, BPRI_HI)) == NULL) {
			s->so_lim_listen = 0;
			STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
			       "sck_bind: couldn't allocbuffer for bind_ack\n");
			sck_bad_ack(q, mp, TSYSERR, EAGAIN);
			return;
		}
		freemsg(mp);
		mp = new;
		prim = (union T_primitives *)mp->b_rptr;

		prim->bind_ack.ADDR_offset = sizeof(struct T_bind_ack);
		prim->bind_ack.ADDR_length = SCK_ADDR_SIZE;
		mp->b_wptr = mp->b_rptr + prim->bind_ack.ADDR_offset;
		bcopy(&inp->sin_port, mp->b_wptr, SCK_ADDR_SIZE);
		mp->b_wptr += SCK_ADDR_SIZE;
	} else {
		if (prim->bind_req.ADDR_length != SCK_ADDR_SIZE) {
			s->so_lim_listen = 0;
			sck_bad_ack(q, mp, TBADADDR, 0);
			return;
		}
		bcopy((char *)(mp->b_rptr + prim->bind_req.ADDR_offset),
			&inp->sin_port, SCK_ADDR_SIZE);

		/* if listener pass port definition - else have pr_nmck
		   pick a port for us.  Listener is only port needed */
		if (!(s->so_lim_listen)) 
			inp->sin_port = NULL;

		/* check for valid name and fill in sin_port if needed */
		if ((*pr->pr_nmck)(s)) {
			sck_bad_ack(q, mp, TBADF, 0);
			return;
		}
		s->so_nmlen = sizeof(struct sockaddr_in);
		
	}

	/* check for unbound condition */
	if (namecheck(pr, s->so_name, s->so_nmlen)) {
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
		       "sckbind: name check failed\n");
		sck_bad_ack(q, mp, TBADADDR, 0);
		return;
	}

	/* do the actual bind to enp */
	if ((*pr->pr_bind)(s)) {
		sck_bad_ack(q, mp, TBADADDR, 0);
		return;
	}
	s->so_state |= SS_BOUND;
	s->so_tli = TS_IDLE;

	/* if this is the listen queue - start off enp with a connect */
	if (s->so_lim_listen) {
		s->so_state |= SS_LISTEN;
		if ((s->so_enpid = chan_alloc(s)) == -1) {
			sck_bad_ack(q, mp, TBADADDR, 0);
			return;
		}
		s->so_state |= SS_RESV;
		cp = &data_chan[(short)s->so_enpid];
		enp_assign(cp->c_strsock, (caddr_t)s);
		s->so_tli = TS_WACK_CREQ;

		/* connect may fail due to out of buffer condition in enp */
		if((*s->so_proto->pr_connect)(s,0,SS_LISTEN,-1))
			sckconnect(q,s,0,SS_LISTEN,-1);
	}

	/* respond to tli with bind complete */
	prim->bind_ack.PRIM_type = T_BIND_ACK;
	prim->bind_ack.CONIND_number = s->so_lim_listen;
	mp->b_datap->db_type = M_PCPROTO;
	qreply(q, mp);
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_unbind()
*	Function:	Process tli primitive T_UNBIND_REQ
*
****************************************************************************/

sck_unbind(q,mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register union T_primitives *prim;
	register SOCKET *s;

	prim = (union T_primitives *)mp->b_rptr;
	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_UNBIND+0xff,q,0,0,0,0,0,0);
		return;
	}
	SCKTRK(SCK_UNBIND,q,mp,s,prim,s->so_tli,0,0);
	
	if (!sck_flush(RD(q))) {
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
		       "sck_unbind: send_flush failed\n");
	}
	s->so_lim_listen = 0;
	s->so_tli = TS_UNBND;
	sck_ok_ack(q, mp, T_UNBIND_REQ);
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_optmgmt()
*	Function:	Process tli primitive T_OPTMGMT_REQ
*
****************************************************************************/

sck_optmgmt(q,mp)
  register queue_t *q;
  register mblk_t *mp;
{
	register union T_primitives *prim;
	register SOCKET *s;

	prim = (union T_primitives *)mp->b_rptr;
	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_OPTMGT+0xff,q,0,0,0,0,0,0);
		return;
	}

	switch(prim->optmgmt_req.MGMT_flags) {

	case T_DEFAULT: {
		register mblk_t *tmp;

		if ((tmp = allocb(sizeof(struct T_optmgmt_ack), BPRI_HI)) == NULL) {
			STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE | SL_ERROR,
			       "sck_optmgmt: couldn't alloc buffer for optmgmt_ack\n");
			sck_bad_ack(q, mp, TSYSERR, EAGAIN);
			return;
		}
		freemsg(mp);
		prim = (union T_primitives *)tmp->b_rptr;
		tmp->b_datap->db_type = M_PCPROTO;
		tmp->b_wptr = tmp->b_rptr + sizeof(struct T_optmgmt_ack);
		prim->optmgmt_ack.MGMT_flags = T_DEFAULT;
		prim->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
		prim->optmgmt_ack.OPT_length = NULL;
		prim->optmgmt_ack.OPT_offset = NULL;
		qreply(q, tmp);
		return;
		}

	case T_CHECK:
		if ((prim->optmgmt_req.OPT_length != NULL) ||
		    (prim->optmgmt_req.OPT_offset < sizeof(struct T_optmgmt_req))) {
			sck_bad_ack(q, mp, TBADOPT, 0);
			return;
		}

		/*
		 * Any values are fine.
		 */
		prim->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
		prim->optmgmt_ack.MGMT_flags |= T_SUCCESS;
		mp->b_datap->db_type = M_PCPROTO;
		qreply(q, mp);
		return;
		
	case T_NEGOTIATE: {
		/*
		 * No negotiation -- accept any values sent down
		 */
		if ((prim->optmgmt_req.OPT_length != NULL) ||
		    (prim->optmgmt_req.OPT_offset < sizeof(struct T_optmgmt_req))) {
			sck_bad_ack(q, mp, TBADOPT, 0);
			return;
		}
		prim->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
		mp->b_datap->db_type = M_PCPROTO;
		qreply(q, mp);
		return;
		}
		
	default:
		sck_bad_ack(q, mp, TBADFLAG, 0);
		return;
	}
}

/*
/***************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_sendit(q, mp)
*	Function:	general purpose send to enp routine
*			place message on queue for sckuwsrv
*
****************************************************************************/

sck_sendit(q,dmp)
  register queue_t *q;
  register mblk_t *dmp;
{
	register int i;
	register SOCKET *s;

	/* if disconnected - q_ptr will already be zero */
	if (!(s = (SOCKET *)q->q_ptr)) {
		SCKTRK(SCK_SEND+0xff,q,0,0,0,0,0,0);
		return;
	}

	SCKTRK(SCK_SEND,q,dmp,s,s->so_state,dmp->b_cont,0,0);

	if (s->so_state & SS_NOWRITE) {
		freemsg(dmp);
		sck_fatal(q);
		return (-1);
	}

	if (ntohs(data_chan[(short) s->so_enpid].c_state) & SS_BROKEN) {
		SCKTRK(SCK_SEND+0xfe,q,0,0,0,0,0,0);
		sck_rcvdiscon(q);
		freemsg(dmp);
		return (-1);
	}

	i = splnet();
	enpservice();
	splx(i);

	/* place on queue for sckuwsrv */
	putq(q,dmp);
	return(0);
}

/*
/*************************************************************************
*
*	File:		sitm.c
*	Routine:	sck_bad_ack(q,mp,tli_error,unix_error)
*	Function:	return bad news to stream head
*
***************************************************************************/

sck_bad_ack(q, mp, tli_error, unix_error)
  queue_t *q;
  register mblk_t *mp;
  long tli_error;
  long unix_error;
{
	mblk_t *tmp;
	long type;
	register union T_primitives *prim;

	SCKTRK(SCK_BACK,q,mp,tli_error,unix_error,0,0,0);

	prim = (union T_primitives *)mp->b_rptr;
	type = prim->type;
	/*
	 * is message large enough to send
	 * up a T_ERROR_ACK primitive
	 */
	 if ((mp->b_datap->db_lim - mp->b_datap->db_base) < sizeof(struct T_error_ack)) {
		if ((tmp=allocb(sizeof(struct T_error_ack), BPRI_HI)) == NULL) {
			STRLOG(SOCK_ID,SCK_NUM(q->q_ptr),0,SL_TRACE | SL_ERROR,
			       "sck_bad_ack: couldn't allocate buffer for error_ack\n");
			freemsg(mp);
			sck_fatal(q);
			return;
		}
		freemsg(mp);
		mp = tmp;
	 }
	 mp->b_rptr = mp->b_datap->db_base;
	 mp->b_wptr = mp->b_rptr + sizeof(struct T_error_ack);
	 prim = (union T_primitives *)mp->b_rptr;
	 prim->error_ack.ERROR_prim = type;
	 prim->error_ack.TLI_error = tli_error;
	 prim->error_ack.UNIX_error = unix_error;
	 prim->error_ack.PRIM_type = T_ERROR_ACK;
	 mp->b_datap->db_type = M_PCPROTO;
	 freemsg(unlinkb(mp));
	 if (q->q_flag&QREADR)
		putnext(q, mp);
	 else
		qreply(q, mp);
	 return;
}

/*
/*********************************************************************
*
*	File:		sitm.c
*	Routine:	sck_fatal(q)
*	Function:	send bad news to stream head and quit
*
**********************************************************************/

sck_fatal(q)
  register queue_t *q;
{
	register mblk_t *mp;

	/* if mp is null reschedule for later when we have some buffers */
	if((mp = allocb(sizeof(struct T_data_req),BPRI_HI)) == NULL) {
		bufcall(sizeof(struct T_data_req),BPRI_HI,sck_fatal,q);
		return;
	}

	mp->b_datap->db_type = M_ERROR;
	*mp->b_datap->db_base = EPROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_datap->db_base + sizeof(char);
	freemsg(unlinkb(mp));
	if (q->q_flag&QREADR)
		putnext(q, mp);
	else
		qreply(q, mp);
}


/*
/*********************************************************************
*
*	File:		sitm.c
*	Routine:	sck_ok_ack(q,mp,type)
*	Function:	send good news to stream head 
*
**********************************************************************/

sck_ok_ack(q, mp, type)
  queue_t *q;
  register mblk_t *mp;
  long type;
{
	mblk_t *tmp;
	register struct T_ok_ack *ok_ack;

	/*
	 * is message large enough to send
	 * up a T_OK_ACK primitive
	 */
	 if ((mp->b_datap->db_lim - mp->b_datap->db_base) < sizeof(struct T_ok_ack)) {
		if ((tmp = allocb(sizeof(struct T_ok_ack), BPRI_HI)) == NULL) {
			STRLOG(SOCK_ID,SCK_NUM(q->q_ptr),0,SL_TRACE | SL_ERROR,
			       "sck_ok_ack: couldn't alloc buffer for ok_ack\n");
			freemsg(mp);
			sck_fatal(q);
			return;
		}
		freemsg(mp);
		mp = tmp;
	 }
	 mp->b_rptr = mp->b_datap->db_base;
	 mp->b_wptr = mp->b_rptr + sizeof(struct T_ok_ack);
	 ok_ack = (struct T_ok_ack *)mp->b_rptr;
	 ok_ack->CORRECT_prim = type;
	 ok_ack->PRIM_type = T_OK_ACK;
	 mp->b_datap->db_type = M_PCPROTO;
	 freemsg(unlinkb(mp));
	 if (q->q_flag&QREADR)
		putnext(q, mp);
	 else
		qreply(q, mp);
	 return;
}

/*
/*********************************************************************
*
*	File:		sitm.c
*	Routine:	sck_flush(q)
*	Function:	flush queue	
*
**********************************************************************/

sck_flush(q)
  queue_t *q;
{
	mblk_t *mp;

	if ((mp = allocb(1, BPRI_HI)) == NULL)
		return(0);
	mp->b_wptr++;
	mp->b_datap->db_type = M_FLUSH;
	*mp->b_rptr = FLUSHRW;
	putnext(q, mp);
	return(1);
}

/*
/*********************************************************************
*
*	File:		sitm.c
*	Routine:	sck_monitor()
*	Function:	periodically poll all stream connections
*			to determine if a disconnect has taken place
*
**********************************************************************/

sck_monitor()
{
	register SOCKET *s;
	register str_going = NULL;

	/* look at all sockets that meet this criterian:
		1 - stream based socket useage
		2 - socket is connected
		3 - sck_sanity is active		*/
	SCKTRK(SCK_MON,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0,0,0);
	if(sck_sanity) {
		for (s = sock_table; s < &sock_table[NSOCKETS]; s++) {
			if (s->so_strq) {
				if (s->so_state & SS_CONNECTED) {
					switch(s->so_link) {
						case SDD_INACTIVE:
							s->so_link = SDD_PEND;
							sckactive(WR((queue_t *)s->so_strq),s,SITM_ACTIVE);
							break;
						case SDD_PEND:
							s->so_link = SDD_DEAD;
							sck_rcvdiscon(WR((queue_t *)s->so_strq));
							break;
						case SDD_ACTIVE:
							s->so_link = SDD_INACTIVE;
							break;
						case SDD_DEAD:
						default:
							break;
					}
				}
				str_going = 1;
			}
		} 
	}
	/* if no streams queues active - stop this periodic polling */
	if (str_going)
		timeout(sck_monitor, 0, SDD_TIMEOUT);
	else
		sck_timer = 0;
}
/*
/*********************************************************************
*
*	File:		sitm.c
*	Routine:	sckactive(q,s)
*	Function:	send Active packet to force remote to
*			respond with a control packet.
*
**********************************************************************/

sckactive(q,s,type)
  register queue_t *q;
  register SOCKET *s;
  register short type;
{
	register SITM *sitm_ptr;
	register mblk_t *mp, *tmp;
	register union T_primitives *prim;

	SCKTRK(SCK_ACTIVE,q,s,type,s->so_link,0,0,0);

	/* get buffer and build a M_CTL message for sanity packet */
	if((tmp = allocb(sizeof(struct T_data_req),BPRI_HI)) == NULL) {
		SCKTRK(SCK_ACTIVE+0x01,q,s,type,s->so_link,0,0,0);
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
		       "sckactive:  No buffers for sanity control packet\n");
		sck_fatal(q);
		return;
	}
	tmp->b_datap->db_type = M_CTL;
	prim = (union T_primitives *)tmp->b_rptr;
	prim->data_req.PRIM_type = T_DATA_REQ;
	prim->data_req.MORE_flag = 0;
	tmp->b_wptr = tmp->b_wptr + sizeof(struct T_data_req);

	/* get buffer for control type field */
	if((mp = allocb(sizeof(SITM),BPRI_HI)) == NULL) {
		SCKTRK(SCK_ACTIVE+0x02,q,s,type,s->so_link,tmp,0,0);
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
		       "sckactive:  No buffers for sanity packet\n");
		freemsg(tmp);
		sck_fatal(q);
		return;
	}
	sitm_ptr = (SITM *)mp->b_rptr;
	sitm_ptr->type = type;
	mp->b_wptr = mp->b_wptr + sizeof(SITM);

	/* all ok - send data */
	linkb(tmp,mp);
	sck_sendit(q, tmp);
}
/*
/*********************************************************************
*
*	File:		sitm.c
*	Routine:	sckconnect(q,s,,toname,type,flags)
*	Function:	If out of enp buffers, place request in queue
*			and when buffers are released, grab one
*
**********************************************************************/

sckconnect(q,s,toname,type,flags)
  register queue_t *q;
  register SOCKET *s;
  register char *toname;
  register type;
  register flags;
{
	register CONN *conn_ptr;
	register mblk_t *mp, *tmp;
	register union T_primitives *prim;

	SCKTRK(SCK_BCONN,q,s,type,flags,toname,0,0);

	/* get buffer and build a M_CTL message for control packet */
	if((tmp = allocb(sizeof(struct T_data_req),BPRI_HI)) == NULL) {
		SCKTRK(SCK_BCONN+0x01,q,s,type,flags,toname,0,0);
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
		       "sckconnect:  No buffers for connect control packet\n");
		sck_fatal(q);
		return;
	}
	tmp->b_datap->db_type = M_CTL;
	prim = (union T_primitives *)tmp->b_rptr;
	prim->data_req.PRIM_type = T_CONN_REQ;
	prim->data_req.MORE_flag = 0;
	tmp->b_wptr = tmp->b_wptr + sizeof(struct T_data_req);

	/* get buffer for control type field */
	if((mp = allocb(sizeof(SITM),BPRI_HI)) == NULL) {
		SCKTRK(SCK_BCONN+0x02,q,s,type,flags,tmp,0,0);
		STRLOG(SOCK_ID,SCK_NUM(s),0,SL_TRACE,
		       "sckconnect:  No buffers for connect packet\n");
		freemsg(tmp);
		sck_fatal(q);
		return;
	}
	conn_ptr = (CONN *)mp->b_rptr;

	/* handle case of toname = NULL gracefully */
	if (toname) {
		conn_ptr->to = conn_ptr->toname;
		bcopy(toname,(char *) conn_ptr->toname, sizeof(struct sockaddr_in)/2);
	} else
		conn_ptr->to = NULL;

	conn_ptr->type = type;
	conn_ptr->flags = flags;
	mp->b_wptr = mp->b_wptr + sizeof(CONN);

	/* all ok - send data */
	linkb(tmp,mp);
	sck_sendit(q, tmp);
}
/*
/*********************************************************************
*
*	File:		sitm.c
*	Routine:	sck_gname(s, place)
*	Function:	copy address to place
*
**********************************************************************/

sck_gname(s, place)
  register SOCKET *s;
  char *place;
{
	register sockaddr_in *csin;
	register LOCAL_ADDR *ap;

	if (s->so_state & SS_BOUND)
		enp_move(s->so_name, place, sizeof(sockaddr_in));
	else
		enp_zero(place, sizeof(sockaddr_in));

	ap = (LOCAL_ADDR *) place;
	csin = (sockaddr_in *) data_chan[(short) s->so_enpid].c_name;
	ap->port = ntohs(csin->sin_port);
	ap->addr = ntohl(enp_rval(csin->sin_addr.s_addr));

	return (SCK_ADDR_SIZE);
}

