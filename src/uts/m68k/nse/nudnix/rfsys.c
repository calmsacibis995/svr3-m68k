/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/rfsys.c	1.11"
/*
 *  Miscellaneous system calls for file sharing.
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/stream.h"
#include "sys/comm.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/psw.h"
#include "sys/user.h"
#include "sys/inode.h"
#include "sys/message.h"
#include "sys/mount.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/sysinfo.h"
#include "sys/recover.h"
#include "sys/rfsys.h"

char	Domain[MAXDNAME];		/* this machine's domain	*/
int	rfs_vflg=0;

extern int rfs_vhigh;
extern int rfs_vlow;
extern int bootstate;

extern sema_t rfadmin_sema;
extern struct bqctrl_st rfmsgq;		/* queue for user-level daemon */
extern int rfmsgcnt;			/* length of rfmsgq */

rfsys ()
{
	struct uap {
		int opcode;
	} *uap;

	void fumount(), send_rfmsg(), get_rfmsg();

	uap = (struct uap *) u.u_ap;
	DUPRINT1 (DB_RFSYS, "rfsys system call\n");
	if (uap->opcode != RF_GETDNAME && !suser())
		return;

	switch (uap->opcode) {
	case RF_FUMOUNT:	/* forced unmount */
		{
		fumount ();
		break;
		}
	case RF_SENDUMSG:	/* send message to remote user-level */
		{
		send_rfmsg ();
		break;
		}
	case RF_GETUMSG: 	/* wait for message from remote user-level */
		{
		get_rfmsg ();
		break;
		}
	case RF_LASTUMSG:	/* wake up from GETUMSG */
		{
		DUPRINT1 (DB_RFSYS, "RF_LASTUMSG \n");
		cvsema (&rfadmin_sema);
		break;
		}
	case RF_FWFD:		/* force working file descriptor */
	        {
		register struct a {
			unsigned opcode;
			int 	fd;
			struct token	*token;
			struct gdpmisc	*gdpmisc;
		} *uap = (struct a *) u.u_ap;
		register queue_t *qp;
		register int i;
		struct token	name;
		struct gdpmisc gdpmisc;
		queue_t	*get_circuit();

		if (bootstate != DU_UP) {
			u.u_error = ENONET;
			break;
		}

		if (uap->fd < 0) {
			u.u_error = EINVAL;
			break;
		}

		if (copyin(uap->token, &name, sizeof(struct token))) {
			u.u_error = EFAULT;
			DUPRINT1(DB_MNT_ADV,"fwfd token copyin failed...\n");
			break;
		}
		DUPRINT3(DB_MNT_ADV,"fwfd: token.t_id=%x, token.t_uname=%s\n",
			name.t_id, name.t_uname);
		if (copyin(uap->gdpmisc, &gdpmisc, sizeof (struct gdpmisc))) {
			u.u_error = EFAULT;
			DUPRINT1(DB_MNT_ADV,"fwfd gdpmisc copyin failed...\n");
			break;
		}
		if (qp = get_circuit(uap->fd, &name)) {

			register struct gdp *gdpp;

			gdpp = GDP(qp);
			gdpp->token = name;
			gdpp->hetero = gdpmisc.hetero;
			gdpp->version = gdpmisc.version;
			gdpp->idmap[0] = 0;
			gdpp->idmap[1] = 0;
		DUPRINT4(DB_MNT_ADV,"fwfd exits: gdpp->t_id=%x, ver = %d, t_uname=%s\n",
			gdpp->token.t_id, gdpp->version, gdpp->token.t_uname);
		}
		break;
	    }
	case RF_SETDNAME:	/* set domain name	*/
		{
		register struct uap {
			int	opcode;
			char	*dname;
			int	size;
		} *uap = (struct uap *) u.u_ap;

		if (bootstate != DU_DOWN) {
			u.u_error = EEXIST;
			break;
		}
		if (uap->size > MAXDNAME) {
			u.u_error = EINVAL;
			break;
		}
		switch (upath(uap->dname,Domain,uap->size)) {
		case -2:	/* too long	*/
		case  0:	/* too short	*/
			u.u_error = EINVAL;
			break;
		case -1:	/* bad user address	*/
			u.u_error = EFAULT;
			break;
		}
		DUPRINT3(DB_RFSYS,"SETDNAME: name = %s, u.u_error = %d\n",
			Domain,u.u_error);
		break;
		}
	case RF_GETDNAME:	/* get domain name	*/
		{
		register struct uap {
			int	opcode;
			char	*dname;
			int	size;
		} *uap = (struct uap *) u.u_ap;
		if (uap->size > MAXDNAME || uap->size <= 0) {
			u.u_error = EINVAL;
			break;
		}
		if (*Domain == '\0') {
			u.u_error = ENOENT;
			break;
		}
		if (copyout(Domain,uap->dname,uap->size) == -1)
			u.u_error = EFAULT;

		DUPRINT3(DB_RFSYS,"GETDNAME: name = %s, u.u_error = %d\n",
			Domain,u.u_error);
		break;
		}
	case RF_SETIDMAP:	/* add an id map	*/
		{
		register struct uap {
			int	opcode;
			char	*name;
			int	flag;
			char	*map;
		} *uap = (struct uap *) u.u_ap;

		setidmap(uap->name,uap->flag,uap->map);
		/* DO NOT print uap->name as a string as it is a user pointer
		   value used in setidmap so display that */
		DUPRINT4(DB_RFSYS,"SETIDMAP: name=%x, flag=0x%x, u.u_error=%d\n",
			uap->name,uap->flag,u.u_error);
		break;
		}
        case RF_VFLAG:		/* handle verification option	*/
		{
                struct a {
                        int opcode;
                        int vcode;
                } *uap;

                uap = (struct a *)u.u_ap;
                switch (uap->vcode) {
                case V_SET:
                        rfs_vflg = 1;
                        break;
                case V_CLEAR:
                        rfs_vflg = 0;
                        break;
                case V_GET:
                        u.u_rval1 = rfs_vflg;
                        break;
                default:
                        u.u_error = EINVAL;
                } /* end switch */
		break;
		}
        case RF_VERSION:		/* handle version information */
		{
		int	uvhigh;
		int	uvlow;
                struct a {
                        int opcode;
                        int vcode;
			int *vhigh;
			int *vlow;
                } *uap = (struct a *) u.u_ap;

                switch (uap->vcode) {
                case VER_CHECK:
			if ((uvhigh = fuword(uap->vhigh)) < 0 ||
		    	    (uvlow = fuword(uap->vlow)) < 0)
				u.u_error = EFAULT;
			else if (uvhigh < uvlow)
				u.u_error = EINVAL;
			else if (uvhigh < rfs_vlow || uvlow > rfs_vhigh)
				u.u_error = ERANGE;
			else
				u.u_rval1 = (uvhigh < rfs_vhigh)?uvhigh:rfs_vhigh;
                        break;
                case VER_GET:
			if (suword(uap->vhigh,rfs_vhigh) || suword(uap->vlow,rfs_vlow))
				u.u_error = EFAULT;
                        break;
                default:
                        u.u_error = EINVAL;
                } /* end switch */
		break;
		}
	default:
		DUPRINT2 (DB_RFSYS, "illegal opcode %d \n", uap->opcode);
		u.u_error = EINVAL;
	}

}

/*
 *	Construct and send message to remote user-level, and wait for reply.
 */

void
send_rfmsg ()
{
	struct uap {
		int opcode;
		int cl_sysid;	/* destination sysid */
		char *buf;	/* message to send */
		unsigned size;	/* size of message */
	} *uap;

	queue_t *cl_queue, *sysid_to_queue();
	struct	request	*request;
	struct	response *resp;
	mblk_t	*bp, *in_bp;
	sndd_t	sdp;
	int	size;
	extern	set_sndd();
 
	uap = (struct uap *) u.u_ap;
	DUPRINT4 (DB_RFSYS,
		"send_rfmsg: sysid %d, buf %x, size %d \n",
		uap->cl_sysid, uap->buf, uap->size);

	if ((cl_queue = sysid_to_queue ((sysid_t) uap->cl_sysid)) == NULL) {
		u.u_error = ECOMM;
		return;
	}
	if ((sdp = cr_sndd()) == NULL) {
		u.u_error = ENOMEM;
		DUPRINT1 (DB_RFSYS, "rfmsg: cannot create sd\n");
		return;
	}
	set_sndd (sdp, cl_queue, RECOVER_RD, RECOVER_RD);
	bp = alocbuf(sizeof(struct request), BPRI_LO);
	if (bp == NULL) {
		u.u_error = EINTR;
		goto out;
	}
	request = (struct request *) PTOMSG(bp->b_rptr);
	request->rq_type = REQ_MSG;
	request->rq_opcode = REC_MSG;
	size = (uap->size > ULINESIZ) ? ULINESIZ : uap->size;
	request->rq_count = size;

	if (copyin (uap->buf, request->rq_data, size) < 0) {
		DUPRINT1 (DB_RFSYS, "rfmsg: copyin failed\n");
		u.u_error = EFAULT;
		freemsg (bp);
		goto out;
	}
	if (rsc (sdp, bp, sizeof(struct request), &in_bp, (sndd_t)NULL)
	    != SUCCESS) {
		DUPRINT1 (DB_RFSYS, "rfmsg: rsc failed\n");
		goto out;
	}
	resp = (struct response *)PTOMSG(in_bp->b_rptr);
	if (u.u_error = resp->rp_errno)  {
		DUPRINT2 (DB_RFSYS,"rfmsg: rsc response has error %d\n",
			resp->rp_errno);
	}
	freemsg (in_bp);
out:
	free_sndd (sdp);
	return;
}

/*
 *	User-level daemon sleeps here, waiting for something to happen.
 *	Things that can happen: link down to or fumount of remote resource
 *	(copyout name of resource), message for user-level daemon from
 *	remote user-level (copyout message), exit.
 */

static void
get_rfmsg ()
{
	struct uap {
		int opcode;
		char *buf;		/* where to write messages */
		unsigned size;		/* size of buf */
	} *uap;
	mblk_t	*bp, *deque();
	struct u_d_msg *request;
	int	size, s;

	uap = (struct uap *) u.u_ap;

	DUPRINT1 (DB_RFSYS, "get_rfmsg \n");
	/* If already got a message, don't sleep. */
	s = splrf ();
	if ((bp = deque (&rfmsgq)) != NULL) {
		goto gotbuf;
	}
	psema (&rfadmin_sema, PZERO);
	if ((bp = deque (&rfmsgq)) == NULL) {
		DUPRINT1 (DB_RFSYS, "get_rfmsg wakeup with null deque \n");
		u.u_rval1 = RF_LASTUMSG;
		splx (s);
		return;
	}

gotbuf:
	DUPRINT1 (DB_RFSYS, "get_rfmsg awakened \n");
	splx (s);
	request = (struct u_d_msg *) (bp->b_wptr);
	size = (uap->size > request->count) ? request->count : uap->size;
	u.u_rval1 = request->opcode;
	switch (u.u_rval1) {
	case RF_DISCONN:	/* link is down */
	case RF_FUMOUNT:	/* forced unmount */
	case RF_GETUMSG: 	/* message for user level from remote user */
		if (copyout (request->res_name, uap->buf, size) < 0) {
			u.u_error = EFAULT;
			DUPRINT1 (DB_RFSYS, "get_rfmsg copyout failed \n");
		}
		break;
	default: 
		DUPRINT2 (DB_RFSYS, "get_rfmsg: unknown opcode %d\n",
				request->opcode);
		u.u_error = EINVAL;
		break;
	}
	--rfmsgcnt;
	freemsg (bp);
}
