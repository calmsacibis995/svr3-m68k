/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libns:nsrports.c	1.12"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/stropts.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include "nslog.h"
#include "stdns.h"
#include "nsports.h"
#include "nsdb.h"
#include <nserve.h>
#include <sys/tiuser.h>
#include <nsaddr.h>
#include <sys/cirmgr.h>
#include <pn.h>

struct address		*astoa();
extern int		errno;
extern struct address	*Myaddress;	/* address of this machine	*/
extern char		*Net_spec;	/* transport provider name	*/
static int		Listenfd = -1;	/* listen file descriptor	*/
static struct pollfd	Sel[NPORTS+1];
static int		Selcount=0;	/* how many fds are in Sel	*/
#define POLL_EVENTS	POLLIN
#define incr(x)		((x)+1)%(NPORTS + 1)
#define HOLD_SIGS	{ alrmsig = sigset(SIGALRM,SIG_HOLD);\
			  usr2sig = sigset(SIGUSR2,SIG_HOLD); }
#define REL_SIGS	{ sigset(SIGALRM,alrmsig);\
			  sigset(SIGUSR2,usr2sig); }
static struct first_msg	Rem_msg = { VER_HI, REM_MSG, 0};
static struct first_msg	Rec_msg = { VER_HI, REC_MSG, 0};
static struct first_msg	Ok_msg  = { VER_HI, OK_MSG, 0};
static struct first_msg	Nok_msg = { VER_HI, NOK_MSG, 0};

/*
 * rconnect tries to make a connection with
 * the name server defined by addr.  It returns a
 * port id, which is used in subsequent calls.
 * It returns -1 if it fails.
 */
int
rconnect(addr,mode)
struct address	*addr;
int	mode;
{
	int	pd;
	struct nsport	*pptr=NULL;
	char	inblock[sizeof(struct first_msg)];
	char	*bptr = inblock;
	char	*aatos();
	static struct first_msg	*recfm=NULL;
	struct first_msg	rfm;

	LOG4(L_COMM,"(%5d) rconnect: mode = %d, address = %s\n",
		Logstamp, mode, (addr)?aatos(Logbuf,addr,
			KEEP | ((mode==LOCAL)?RAW:HEX)):"NULL");

	if (!addr)
		return(-1);

	if (!addr->protocol)
		addr->protocol = copystr(Net_spec);

	if ((pd = nsgetpd()) == -1)
		return(-1);

	pptr = pdtoptr(pd);

	switch (mode) {
	case RECOVER:
		pptr->p_mode = PENDING;

		if ((pptr->p_fd = snd_connect(addr)) == -1)
			goto bad_out;

		LOG3(L_COMM,"(%5d) rconnect: recovery connection sent fd=%d\n",
			Logstamp,pptr->p_fd);
		break;
	case REMOTE:
		pptr->p_mode = REMOTE;

		if ((pptr->p_fd = att_connect(addr, RFS)) == -1)
			goto bad_out;
		if (rf_request(pptr->p_fd, RF_NS) != VER_HI)
			goto bad_out;

		LOG3(L_COMM,"(%5d) rconnect: connection made fd=%d\n",
			Logstamp,pptr->p_fd);

		if (!recfm) {
			recfm = &Rem_msg;
			aatos(recfm->addr, Myaddress, HEX);
		}
		tcanon(FIRST_FMT,recfm,inblock,FALSE);

		if (ioctl(pptr->p_fd,I_PUSH,TIRDWR_MOD) == -1) {
			LOG3(L_ALL,"(%5d) rconnect: can't push tirdwr, errno=%d\n",
				Logstamp,errno);
			goto bad_out;
		}
		LOG2(L_COMM,"(%5d) rconnect: tirdwr pushed\n",Logstamp);
		if (nswrite(pd,inblock,sizeof(struct first_msg)) == -1) {
			LOG3(L_ALL,"(%5d) rconnect: can't write first msg, errno=%d\n",
				Logstamp,errno);
			goto bad_out;
		}
		if (nsread(pd,&bptr,sizeof(struct first_msg)) == -1) {
			LOG3(L_ALL,"(%5d) rconnect: can't read first reply, errno=%d\n",
				Logstamp,errno);
			goto bad_out;
		}
		fcanon(FIRST_FMT,bptr,(char *) &rfm);
		if (strcmp(rfm.mode,NOK_MSG) == 0) {
			LOG3(L_ALL,"(%5d) rconnect: connection rejected, msg = %s\n",
				Logstamp, rfm.addr);
			goto bad_out;
		}
		break;
	default:
		LOG3(L_COMM,"(%5d) rconnect: unknown mode=%d\n",Logstamp,mode);
		goto bad_out;
	}

	if (addtosel(pptr) == -1)
		goto bad_out;

	LOG2(L_COMM,"(%5d) rconnect: connect succeeded\n",Logstamp);
	return(pd);
bad_out:
	LOG3(L_COMM,"(%5d) rconnect: connect failed, errno=%d\n",
		Logstamp,errno);
	nsrclose(pd);
	return(-1);
}
/*
 * nslisten initializes the listen port for this ns.
 * It returns 0 for success, and -1 for failure.
 */
int
nslisten()
{
	int		fd[2];
	struct stat	sbuf;
	int		i;
	char		cbuf[BUFSIZ];


	LOG2(L_COMM,"(%5d) nslisten: \n",Logstamp);

	if (Listenfd != -1) return(-1);

	
	if (spipe(fd) < 0) {
		perror("spipe");
		return(-1);
	}

	fstat(fd[0], &sbuf);

	unlink(NS_PIPE);

	if (mknod(NS_PIPE, 0020660, sbuf.st_rdev) == -1) {
		perror(NS_PIPE);
		return(-1);
	}

	sprintf(cbuf,"chgrp bin %s\n",NS_PIPE);
	system(cbuf);

	Sel[0].fd = Listenfd = fd[1];
	Sel[0].events = POLLIN;
	Sel[0].revents = 0;
	/* set up entire poll structure	*/
	if (!Selcount) {
		for (i=1; i < NPORTS+1; i++) {
			Sel[i].fd = -1;
			Sel[i].events = 0;
			Sel[i].revents = 0;
		}
		Selcount = NPORTS+1;
	}

	SET_NODELAY(Listenfd);

	LOG2(L_COMM,"(%5d) nslisten: listen returns success\n",Logstamp);
	return(0);
}
/*
 * nswait is the name server poll, it sets pd to the newly
 * assigned port, and returns one of:
 *
 *	LOC_REQ:	request from a local command
 *	REM_REQ:	request from a remote name server
 *	REC_IN:		input on recovery stream
 *	REC_HUP:	hang up on recovery stream
 *	REC_CON:	recovery connect request
 *	REC_ACC:	recovery connect indication recvd and accepted
 *	FATAL:		fatal error (must be -1)
 *	NON_FATAL:	non-fatal error
 */
int
nswait(pd)
int	*pd;	/* return port id	*/
{
	static int	i=0;
	int	temp;
	char	inblock[sizeof(struct first_msg)];
	char	*bptr = inblock; /* just to get the ptr to ptr	*/
	struct first_msg	first;
	static struct first_msg	*recfm=NULL;
	struct first_msg	rfm;
	struct strrecvfd	sfd;
	int	ret;
	int	retval;
	void	(*alrmsig)();
	void	(*usr2sig)();
	struct nsport	*pptr;
	char	garbage[BUFSIZ];
	int	t_val;

	LOG2(L_COMM,"(%5d) nswait: enter\n",Logstamp);

	if (Sel[0].fd == -1) {
		PLOG1("nswait: Listen fd not initialized\n");
		return(FATAL);
	}
	if ((ret = poll(Sel, Selcount, SEL_TIME)) == -1) {
		switch(errno) {
		case EINTR:
		case EAGAIN:
			return(NON_FATAL);
		default:
			PLOG3("(%5d) nswait: poll returns -1, errno=%d\n",
				Logstamp,errno);
			return(FATAL);
		}
	}
	LOG3(L_COMM,"(%5d) nswait: poll returns %d\n",Logstamp,ret);
	if (ret == 0)
		return(NON_FATAL);

	HOLD_SIGS;

	temp = i;
	do {
		i = incr(i);
		if (!Sel[i].revents)
			continue;
		if (i == 0)
			break;	/*things coming in on port 0 are handled */
				/* by the code after the loop... 	 */

		if ((pptr = pdtoptr(fdtopd(Sel[i].fd))) == NULL) {
			PLOG4("(%5d) nswait: bad fd (%d) on Sel[%d]\n",
				Logstamp, Sel[i].fd, i);
			REL_SIGS;
			return(FATAL);
		}

		*pd = ptrtopd(pptr);

		LOG5(L_COMM,"(%5d) nswait: poll port %d, mode=%d, revents=0x%x\n",
			Logstamp, *pd, pptr->p_mode, Sel[i].revents);


		switch (pptr->p_mode) {
		case REMOTE:
		case LOCAL:
			if (Sel[i].revents & POLLIN) {
				if (pptr->p_mode == REMOTE) {
					REL_SIGS;
					return(REM_REQ);
				}
				else {
					REL_SIGS;
					return(LOC_REQ);
				}
			}
			/* can only be POLLHUP or POLLERR now	*/
			nsrclose(*pd);
			*pd = -1;
			REL_SIGS;
			return(NON_FATAL);
		case RECOVER:
			if (Sel[i].revents & POLLIN) {
				REL_SIGS;
				return(REC_IN);
			}
			REL_SIGS;
			return(REC_HUP);
		case PENDING:
			if (!(Sel[i].revents & POLLIN) ||
			    rcv_connect(pptr->p_fd,RFS) == -1 ||
			    rf_request(pptr->p_fd, RF_NS) != VER_HI) {
				/*
				 * t_look() is necessary if rcv_connect()
				 * failed. This is to remove pending disconnect
				 * put back in the look buffer. If we are here
				 * from passing any
				 * other of the tests, the t_look() will not
				 * hurt.
				 */
				t_val = t_look(pptr->p_fd);
				LOG4(L_ALL,"(%5d) nswait: rconnect failed, pd =%d, t_look() = %d\n",
					Logstamp, *pd, t_val);
				REL_SIGS;
				return(REC_HUP);
			}

			if (!recfm) {
				recfm = &Rec_msg;
				aatos(recfm->addr, Myaddress, HEX);
			}
			tcanon(FIRST_FMT,recfm,inblock,FALSE);

			if (ioctl(pptr->p_fd,I_PUSH,TIRDWR_MOD) == -1) {
			    LOG3(L_ALL,"(%5d) nswait: can't push tirdwr, errno=%d\n",
				Logstamp,errno);
			    REL_SIGS;
			    return(REC_HUP);
			}
			LOG2(L_COMM,"(%5d) nswait: tirdwr pushed\n",Logstamp);
			if (nswrite(*pd,inblock,sizeof(struct first_msg)) == -1) {
			    LOG3(L_ALL,
				"(%5d) nswait: can't write first msg, errno=%d\n",
				Logstamp,errno);
			    REL_SIGS;
			    return(REC_HUP);
			}
			LOG2(L_COMM,"(%5d) nswait: first msg written\n",Logstamp);
			pptr->p_mode = PENDING2;
			REL_SIGS;
			return(REC_CON);

		case PENDING2:
			if (nsread(*pd,&bptr,sizeof(struct first_msg)) == -1) {
			    LOG3(L_ALL,"(%5d) nswait: nsread first_msg, errno=%d\n",
				Logstamp,errno);
			    REL_SIGS;
			    return(REC_HUP);
			}
			fcanon(FIRST_FMT,bptr,(char *) &rfm);
			if (strcmp(rfm.mode,NOK_MSG) == NULL) {
			    LOG3(L_ALL,"(%5d) nswait: conn rejected, msg = %s\n",
				Logstamp, rfm.addr);
			    REL_SIGS;
			    return(REC_HUP);
			}
			pptr->p_mode = RECOVER;
			REL_SIGS;
			return(REC_ACC);
		default:
			/* shouldn't get here	*/
			PLOG5("(%5d) nswait: bad mode (%d) fd (%d) on Sel[%d]\n",
				Logstamp, pptr->p_mode, Sel[i].fd, i);
			REL_SIGS;
			return(FATAL);
		}
	} while ( i != temp );

	/* check Listenfd (port 0) here...		*/

	if (Sel[0].revents) {
		LOG3(L_COMM,"(%5d) nswait: poll on Listenfd, revents=0x%x\n",
			Logstamp, Sel[0].revents);
		if (!(POLLIN & Sel[0].revents)) {
			PLOG3("(%5d) event 0x%x on Listenfd, FATAL error\n",
				Logstamp,Sel[0].revents);
			REL_SIGS;
			return(FATAL);
		}
		if (ioctl(Listenfd, I_RECVFD, &sfd) < 0) {
			switch (errno) {
			case EBADMSG:
			    /* someone wrote to this fd, just throw it out	*/
			    read(Listenfd,garbage,BUFSIZ);
			    /* fall through	*/
			case EAGAIN:
			    REL_SIGS;
			    return(NON_FATAL);
			default:
			    PLOG3("(%5d) nswait: I_RECVFD Listenfd fails, errno=%d\n",
				Logstamp, errno);
			    REL_SIGS;
			    return(FATAL);
			}
		}

		/* at this point, it must be a connect request	*/
		if ((*pd = nsgetpd()) == -1) {
			PLOG2("(%5d) nswait: nsgetpd failed\n",Logstamp);
			close(sfd.fd);
			REL_SIGS;
			return(NON_FATAL);
		}
	
		pptr = pdtoptr(*pd);
		pptr->p_fd = sfd.fd;

		if ((ret = nsread(*pd, &bptr, sizeof(struct first_msg))) == -1) {
			LOG3(L_ALL,"(%5d) nswait: Can't read first msg, errno=%d\n",
				Logstamp, errno);
			nsrclose(*pd);
			*pd = -1;
			REL_SIGS;
			return(NON_FATAL);
		}

		fcanon(FIRST_FMT,inblock,(char *) &first);

		if (first.version < VER_LO || first.version > VER_HI) {
			LOG3(L_COMM,"(%5d) nswait: version mismatch, version # %d\n",
				Logstamp, first.version);
			first = Nok_msg;
			strcpy(first.addr,BAD_VERSION);
			tcanon(FIRST_FMT,(char *) &first,inblock,FALSE);
			nswrite(*pd, inblock, sizeof(struct first_msg));
			nsrclose(*pd);
			*pd = -1;
			REL_SIGS;
			return(NON_FATAL);
		}
		if (Caddress) {
			if (Caddress->protocol)
				free(Caddress->protocol);
			if (Caddress->addbuf.buf)
				free (Caddress->addbuf.buf);
			free(Caddress);
		}

		addtosel(pptr);

		if (strcmp(first.mode, LOC_MSG) == NULL) {
			pptr->p_mode = LOCAL;
			Caddress = NULL;
			retval = LOC_REQ;
		}
		else if (strcmp(first.mode, REM_MSG) == NULL) {
			pptr->p_mode = REMOTE;
			Caddress = astoa(first.addr,NULL);
			retval = REM_REQ;
		}
		else if (strcmp(first.mode, REC_MSG) == NULL) {
			pptr->p_mode = RECOVER;
			Caddress = astoa(first.addr,NULL);
			retval = REC_CON;
		}
		else {
			LOG3(L_ALL,"(%5d) nswait: unknown mode = %s\n",
				Logstamp,first.mode);
			
			Caddress = NULL;
			first = Nok_msg;
			strcpy(first.addr,UNK_MODE);
			tcanon(FIRST_FMT,(char *) &first,inblock,FALSE);
			nswrite(*pd, inblock, sizeof(struct first_msg));
			nsrclose(*pd);
			*pd = -1;
			REL_SIGS;
			return(NON_FATAL); 
		}
		first = Ok_msg;
		strcpy(first.addr,Net_spec);
		LOG3(L_COMM,"(%5d) nswait: send back net_spec = %s\n",
			Logstamp,first.addr);
		tcanon(FIRST_FMT,(char *) &first,inblock,FALSE);
		if (nswrite(*pd, inblock, sizeof(struct first_msg)) == -1) {
			nsrclose(*pd);
			*pd = -1;
			retval = NON_FATAL;
		}
		REL_SIGS;
		return(retval);
	}
	REL_SIGS;
	return(NON_FATAL);
}
/*
 * Add port to poll structure.
 * Reserve slot 0 for Listenfd.
 * unused slots have -1 as the file descriptor.
 */
addtosel(pptr)
struct nsport	*pptr;
{
	int	i;

	if (ptrtopd(pptr) == -1 || pptr->p_fd == -1)
		return(-1);

	for (i=1; i < NPORTS+1; i++)
		if (Sel[i].fd == -1)
			break;

	if (i == NPORTS+1)
		return(-1);


	Sel[i].fd = pptr->p_fd;
	Sel[i].events = POLL_EVENTS;
	Sel[i].revents = 0;
	return(i);
}
/*
 * Remove port from poll structure.
 */
remfrsel(pptr)
struct nsport	*pptr;
{
	int	i;

	if (ptrtopd(pptr) == -1 || pptr->p_fd == -1)
		return(-1);

	for (i=1; i < NPORTS+1; i++)
		if (pptr->p_fd == Sel[i].fd) {
			Sel[i].fd = -1;
			Sel[i].events = 0;
			Sel[i].revents = 0;
			return(i);
		}

	return(-1);
}
nsrclose(pd)
int	pd;
{
	struct nsport	*pptr;

	if ((pptr = pdtoptr(pd)) != NULL)
		remfrsel(pptr);

	np_clean(pd);
	nsclose(pd);
	return;
}
