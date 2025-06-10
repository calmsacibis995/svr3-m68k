
/* @(#)tt0.c	6.2	 */
/*
 * Line discipline 0
 * No Terminal Handling
 */
#ident	"@(#)kern-port:isp.tt0/1.c 10.4"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/cmn_err.h"

#define ISP
#define KERNEL 1

#ifdef  ISP
#define  defined_io	1

#include "sys/cstty.h"
/* #include "csintern.h" */

extern struct csttinf *getcsip( );

extern int greek;

#endif  /* ISP */

extern char partab[];


/*
 * routine called on first teletype open.
 * establishes a process group for distribution
 * of quits and interrupts from the tty.
 */
ttopen(tp)
register struct tty *tp;
{
	register struct proc *pp;
#ifdef  ISP
	/*** ISP Change Start ***/
	static short cs_init_done = 0;

	if ( !cs_init_done )
	{
		ttcsinit( );
		cs_init_done++;
	}
	/*** ISP Change End ***/
#endif  /* ISP */

	pp = u.u_procp;
	if ((pp->p_pid == pp->p_pgrp)
	 && (u.u_ttyp == NULL)
	 && (tp->t_pgrp == 0))
	{
		u.u_ttyp = &tp->t_pgrp;
		tp->t_pgrp = pp->p_pgrp;
	}
	ttioctl(tp, LDOPEN, 0, 0);
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
}

ttclose(tp)
register struct tty *tp;
{
	if ((tp->t_state&ISOPEN) == 0)
		return;
	tp->t_state &= ~ISOPEN;
	ttioctl(tp, LDCLOSE, 0, 0);
	tp->t_pgrp = 0;

#ifdef  ISP
	/*** ISP Change Start ***/
	{
		register struct csttinf *csi_p = getcsip( tp );

		if ( csi_p->cs_tt )
		{
			csi_p->cs_tt->cs_nref--;
			csi_p->cs_tt = NULL;
			csi_p->cs_tiopt = 0;
		}
	}
#endif  /* ISP */

}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp)
register struct tty *tp;
{
	register struct csttinf *csip;
	register struct clist *tq;

again:
	tq = &tp->t_canq;

	if (tq->c_cc == 0)
		canon(tp);
	while (u.u_count!=0 && u.u_error==0)
	{
		if (u.u_count >= CLSIZE)
		{
			register n;
			register struct cblock *cp;

			if ((cp = getcb(tq)) == NULL)
				break;
			n = min(u.u_count, cp->c_last - cp->c_first);
			if (copyout(&cp->c_data[cp->c_first], u.u_base, n))
				u.u_error = EFAULT;
			putcf(cp);
			u.u_base += n;
			u.u_count -= n;
		}
		else
		{
			register c;

			if ((c = getc(tq)) < 0)
				break;
			if (subyte(u.u_base++, c))
				u.u_error = EFAULT;
			u.u_count--;
		}
	}
	if (tp->t_state&TBLOCK)
	{
		if (tp->t_rawq.c_cc<TTXOLO)
		{
			(*tp->t_proc)(tp, T_UNBLOCK);
		}
	}
/* In an effort to reduce the bug when no chars are returned in error
   due to the translate tables timing out on bad translation map,
   if line is still open there are characters held in translate
   table array and returned char count is zero and  !(ICANON is off and
   VMIN is zero and VTIME is non-zero and tp->t_state is not RTO) then
   retry the read.
*/
	if( !(u.u_ap[2]-u.u_count) )
/* Simplest test first for speed reasons */
		if( !(
		   !(tp->t_lflag&ICANON)
		   && (tp->t_cc[VMIN]==0)
		   && (tp->t_cc[VTIME] != 0)
			/* Did we timeout on the last read */
		  ))
		{
/* If we did not time out then try again and translate table
still has some chars stored */
			csip=getcsip(tp);
			if(csip->cs_1len > 0) goto again;
		}
}

/*
 * Called from device's write routine after it has
 * calculated the tty-structure given as argument.
 */
ttwrite(tp)
register struct tty *tp;
{
	register int oldpri;
#ifdef ISP
/* Start of ISP differences ttwrite-1 */
	register struct csttinf *csip = getcsip( tp );
/* End of ISP differences ttwrite-1 */
#endif /* ISP */

	if (!(tp->t_state&CARR_ON))
		return;
	while (u.u_count)
	{
		if (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD])
		{
			oldpri=spltty();
			while (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD])
			{
				(*tp->t_proc)(tp,T_OUTPUT);
				tp->t_state |= OASLP;
				if(sleep((caddr_t)&tp->t_outq,(TTOPRI|PCATCH))){
					splx(oldpri);
					goto out;
				}
			}
			splx(oldpri);
		}
#ifdef ISP
/* Start of ISP differences ttwrite-2 */
	
	    if (csip->cs_tiopt&CSTRANS)
	    {
		register struct cstthdr *cstt = csip->cs_tt;
		register struct cblock *cp;
		register int ch;
		register int pos;
		register int csave;

		if (u.u_count >= (CLSIZE/2))
		{
		    if ((cp = getcf()) == NULL)
		    	break;
		    csip->cs_cb = cp;
		    cp->c_last = cp->c_first;
		}
		else 
		    csip->cs_cb = NULL;

		do
		{
		    if (CSTATE&CS_4RESET)
		    {
		        if (cstt && cstt->cs_nextcs)
			    for (pos= *CSTTF(ushort, cstt, cstt->cs_pextcs);
			        ch= *CSTTF(char, cstt, pos++); )
			        ttrxput(csip, ch);
		        CSTATE &= ~ CS_4RESET;
		    }
		    csave = 1; 
		    if (cstt != NULL && (ch = csip->cs_4acc) != 0xffff && !CS_ACCENT(ch))
		    { 
		        csip->cs_4acc = 0xffff;
		        csave--;
		    }
		    if (!csave || csip->cs_3save > 0)
		    {
		        if (ttri2e(csip, ch, csave) < 0)
			{
			    u.u_error = EFAULT;
			    break;
		        }
		    }
		    else
		    {
		        for (;;)
			{
			    ch = fubyte(u.u_base++);
			    if (ch < 0)
			    {
			        u.u_error = EFAULT;
				goto out; /* Gotos again */
			    }
			    u.u_count--;
			    if (csip->cs_tiopt&CSFMT7)
			   	goto trans;
			    if (!(CSTATE&(CS_3F377|CS_3DS16)) && !cselect && ch < 0200)  
			        goto norm1;

		     trans: ch = ttrxs2i(csip, ch);
			    if (ch > 0)
			    {
			        if ((CSTATE&CS_3DS16) && csip->cs_4acc == 0xffff && ch < 0200)
				    goto norm;    			
				if (ttri2e(csip, ch, 0) < 0) 
				    u.u_error = EFAULT;
			        break;
			    } 
			    continue;

	              norm1: if (csip->cs_cb)
				{
				if(cp->c_last >= CLSIZE)
					cmn_err(CE_PANIC,"ttwrite: Over the top 1");
				cp->c_data[cp->c_last++] = ch;
				}
			    else
			norm:
			        ttxput(csip->cs_tp, (int)ch, 0);
			    break;
		        }
		    }
		} while (csip->cs_cb != NULL && cp->c_last < (CLSIZE-CLIST_SPACE) && u.u_count);
		if (csip->cs_cb)
		    ttxput(tp, cp, cp->c_last);
	    }
	    else

/* End of ISP differences ttwrite-2 */
#endif /* ISP */
		if (u.u_count >= (CLSIZE/2) )
		{
			register n;
			register struct cblock *cp;

			if ((cp = getcf()) == NULL)
				break;
			n = min(u.u_count, cp->c_last);
			if (copyin(u.u_base, cp->c_data, n))
			{
				u.u_error = EFAULT;
				putcf(cp);
				break;
			}
			u.u_base += n;
			u.u_count -= n;
			cp->c_last = n;
			ttxput(tp, cp, n);
		}
		else
		{
			register c;

			c = fubyte(u.u_base++);
			if (c<0)
			{
				u.u_error = EFAULT;
				break;
			}
			u.u_count--;
			ttxput(tp, c, 0);
		}
	}
out:
	oldpri=spltty();
	(*tp->t_proc)(tp, T_OUTPUT);
	splx(oldpri);
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 */
ttin(tp, code)
register struct tty *tp;
{
	register c;
	register flg1;
	register flg;
	char *cp;
	ushort nchar, nc;

	ushort print=0;
	register struct csttinf *csip;
	struct cblock *old_cb;

	if(greek)
		csip = getcsip(tp);


	if (code == L_BREAK)
	{
		signal(tp->t_pgrp, SIGINT);
		ttyflush(tp, (FREAD|FWRITE));
		return;
	}

	nchar = tp->t_rbuf.c_size - tp->t_rbuf.c_count;
	/* reinit rx control block */
	tp->t_rbuf.c_count = tp->t_rbuf.c_size;
	if (nchar==0)
		return;
	flg = tp->t_iflag;
	flg1 = lobyte(tp->t_lflag);
	/* KMC does all but IXOFF */
	if (tp->t_state&EXTPROC)
		flg &= IXOFF;
	nc = nchar;
	cp = tp->t_rbuf.c_ptr;

	if ( (greek && (nc < cfreelist.c_size || (flg & (INLCR|IGNCR|ICRNL|IUCLC))
		|| csip -> cs_tiopt&CSTRANS)) ||

	(!greek && (nc < cfreelist.c_size || (flg & (INLCR|IGNCR|ICRNL|IUCLC)))))
	{
			/* must do per character processing */

		if( greek && csip->cs_tiopt&CSTRANS )
		{
			old_cb = csip->cs_cb;
			csip->cs_cb = NULL;
			/* Save and zero current output Queue stuff 
		 	as may have interrupted a write or may be using
			junk information */
			while ((c = ttre2i(csip,&nc,&cp)) != -1 ) {
			/* -1 is only illegal char */
			/* Handle special canonical options before passing through tables */
				if( c == CS_SWITCH )
				{
					flip(csip);
					continue;
				}
				if (c == '\n' && flg&INLCR)
					c = '\r';
				else if (c == '\r')
					if (flg&IGNCR)
						continue;
					else if (flg&ICRNL)
						 c = '\n';
				if (csip->cs_tiopt&CS_OFF) {
					csip->cs_tiopt &= ~CS_OFF;
					if (csip->cs_tiopt&CST16) {
					    if (CSTATE&CS_2RESET) {
						CSTATE &= ~CS_2RESET;
						CSTATE &= ~CS_2INTSO;
						putc(0xff, &tp->t_rawq);
						putc(0xff, &tp->t_rawq);
						putc(0, &tp->t_rawq);
					    }
					    putc(CNUL, &tp->t_rawq);
					}
					putc(c, &tp->t_rawq);
				} else
					/* Copy char with whatever options to raw queue */
					ttri2xs(csip, c);
/* start post -processing loop */
				if(flg1)
				{
					if (flg1&ISIG) {
						if (c == tp->t_cc[VINTR]) {
							signal(tp->t_pgrp, SIGINT);
							if (!(flg1&NOFLSH))
								ttyflush(tp, (FREAD|FWRITE));
							continue;
						}
						if (c == tp->t_cc[VQUIT]) {
							signal(tp->t_pgrp, SIGQUIT);
							if (!(flg1&NOFLSH))
								ttyflush(tp, (FREAD|FWRITE));
							continue;
						}
						if (c == tp->t_cc[VSWTCH])
						{
							if (!(flg1&NOFLSH))
								ttyflush(tp, FREAD|FWRITE);
							(*tp->t_proc)(tp, T_SWTCH);
							continue;
						}
					}
					if (flg1&ICANON) {
						if (c == '\n') {
							if (flg1&ECHONL)
								flg1 |= ECHO;
							tp->t_delct++;
						} else if (c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
							tp->t_delct++;
						if (!(tp->t_state&CLESC)) {
							if (c == '\\')
								tp->t_state |= CLESC;
							if (c == tp->t_cc[VERASE] && flg1&ECHOE) {
								if (flg1&ECHO)
									ttxput(tp, '\b', 0);
								flg1 |= ECHO;
								ttxput(tp, ' ', 0);
								c = '\b';
							} else if (c == tp->t_cc[VKILL] && flg1&ECHOK) {
								if (flg1&ECHO)
									ttxput(tp, c, 0);
								flg1 |= ECHO;
								c = '\n';
							} else if (c == tp->t_cc[VEOF]) {
								flg1 &= ~ECHO;
								tp->t_delct++;
							}
						} else {
							if (c != '\\' || (flg1&XCASE))
								tp->t_state &= ~CLESC;
						}
					}
					if (flg1&ECHO) {
/* First attempt to translate the echo */
						ttri2e(csip,c,0);
/*						ttxput(tp, (ushort) c, 0); */
						print=1;
					}
				}
/* End post processing loop */
			}
			csip->cs_cb = old_cb;
			/* Restore cblock pointer before return */
			if(print)
				(*tp->t_proc)(tp, T_OUTPUT);
		}
		else  if(!greek || ( greek && !(csip->cs_tiopt&CSTRANS )))
		{
			for ( ;nc--; cp++)
			{
				c = *cp;
				if (c == '\n' && flg&INLCR)
					*cp = c = '\r';
				else if (c == '\r')
					if (flg&IGNCR)
						continue;
					else if (flg&ICRNL)
						*cp = c = '\n';
				if (flg&IUCLC && 'A' <= c && c <= 'Z')
					c += 'a' - 'A';
				if (putc(c, &tp->t_rawq))
					continue;
				sysinfo.rawch++;
			}
		}
		cp = tp->t_rbuf.c_ptr;
	}
	else
	{
		/* may do block processing */
		putcb(CMATCH((struct cblock *)cp), &tp->t_rawq);
		sysinfo.rawch += nc;
		/* allocate new rx buffer */
		if ((tp->t_rbuf.c_ptr = getcf()->c_data)
			== ((struct cblock *)NULL)->c_data)
		{
			tp->t_rbuf.c_ptr = NULL;
			return;
		}
		tp->t_rbuf.c_count = cfreelist.c_size;
		tp->t_rbuf.c_size = cfreelist.c_size;
	}


	if (tp->t_rawq.c_cc > TTXOHI)
	{
		if (flg1&IXOFF && !(tp->t_state&TBLOCK))
			(*tp->t_proc)(tp, T_BLOCK);
		if (tp->t_rawq.c_cc > TTYHOG)
		{
			ttyflush(tp, FREAD);
			return;
		}
	}
	if ((greek && flg1 && !(csip->cs_tiopt&CSTRANS)) ||
		(!greek && flg1))
		while (nchar--)
	{
		c = *cp++;
		if (flg1&ISIG)
		{
			if (c == tp->t_cc[VINTR])
			{
				signal(tp->t_pgrp, SIGINT);
				if (!(flg1&NOFLSH))
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VQUIT])
			{
				signal(tp->t_pgrp, SIGQUIT);
				if (!(flg1&NOFLSH) )
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VSWTCH])
			{
				if (!(flg1&NOFLSH) )
					ttyflush(tp, FREAD|FWRITE);
				(*tp->t_proc)(tp, T_SWTCH);
				continue;
			}
		}
		if (flg1&ICANON)
		{
			if (c == '\n') {
				if (flg1&ECHONL)
					flg1 |= ECHO;
				tp->t_delct++;
			} else if (c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
				tp->t_delct++;
			if (!(tp->t_state&CLESC)) {
				if (c == '\\')
					tp->t_state |= CLESC;
				if (c == tp->t_cc[VERASE] && flg1&ECHOE) {
					if (flg1&ECHO)
						ttxput(tp, '\b', 0);
					flg1 |= ECHO;
					ttxput(tp, ' ', 0);
					c = '\b';
				} else if (c == tp->t_cc[VKILL] && flg1&ECHOK) {
					if (flg1&ECHO)
						ttxput(tp, c, 0);
					flg1 |= ECHO;
					c = '\n';
				} else if (c == tp->t_cc[VEOF]) {
					flg1 &= ~ECHO;
					tp->t_delct++;
				}
			} else {
				if (c != '\\' || (flg1&XCASE))
					tp->t_state &= ~CLESC;
			}
		}
		if (flg1&ECHO)
		{
			ttxput(tp, c, 0);
			(*tp->t_proc)(tp, T_OUTPUT);
		}
	}
	if (!(flg1&ICANON))
	{
		tp->t_state &= ~RTO;
		if (tp->t_rawq.c_cc >= tp->t_cc[VMIN])
			tp->t_delct = 1;
		else if (tp->t_cc[VTIME])
		{
			if (!(tp->t_state&TACT))
				tttimeo(tp);
		}
	}
	if (tp->t_delct && (tp->t_state&IASLP))
	{
		tp->t_state &= ~IASLP;
		wakeup((caddr_t)&tp->t_rawq);
	}
}

/*
 * Put character(s) on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the base level for output, and from
 * interrupt level for echoing.
 */
ttxput(tp, ucp, ncode)
register struct tty *tp;
union {
	ushort	ch;
	struct cblock *ptr;
} ucp;
{
	register c;
	register flg;
	register unsigned char *cp;
	register char *colp;
	int ctype;
	int cs;
	struct cblock *scf;

	if (tp->t_state&EXTPROC) {
		if (tp->t_lflag&XCASE)
			flg = OPOST;
		else
			flg = 0;
	} else
		flg = tp->t_oflag;
	if (ncode == 0) {
		ncode++;
		cp = (unsigned char *)&ucp.ch + 3; /* char is in least significant byte of first param */
		if (!(flg&OPOST)) {
			sysinfo.outch++;
			c = *cp;
			putc(c, &tp->t_outq);
			return;
		}
		scf = NULL;
	} else {
		if (!(flg&OPOST)) {
			sysinfo.outch += ncode;
			putcb(ucp.ptr, &tp->t_outq);
			return;
		}
		cp = (unsigned char *)&ucp.ptr->c_data[ucp.ptr->c_first];
		scf = ucp.ptr;
	}
	while (ncode--) {
		c = *cp++;
		if (c >= 0200 && !(tp->t_state&EXTPROC)) {
	/* spl5-0 */
			if (c == QESC)
				putc(QESC, &tp->t_outq);
			sysinfo.outch++;
			putc(c, &tp->t_outq);
			continue;
		}
		/*
		 * Generate escapes for upper-case-only terminals.
		 */
		if (tp->t_lflag&XCASE) {
			colp = "({)}!|^~'`\\\0";
			while (*colp++)
				if (c == *colp++) {
					ttxput(tp, '\\', 0);
					c = colp[-2];
					break;
				}
			if ('A' <= c && c <= 'Z')
				ttxput(tp, '\\', 0);
		}
		if (flg&OLCUC && 'a' <= c && c <= 'z')
			c += 'A' - 'a';
		cs = c;
		/*
		 * Calculate delays.
		 * The numbers here represent clock ticks
		 * and are not necessarily optimal for all terminals.
		 * The delays are indicated by characters above 0200.
		 */
		ctype = partab[c];
		colp = &tp->t_col;
		c = 0;
		switch (ctype&077) {

		case 0:	/* ordinary */
			(*colp)++;

		case 1:	/* non-printing */
			break;

		case 2:	/* backspace */
			if (flg&BSDLY)
				c = 2;
			if (*colp)
				(*colp)--;
			break;

		case 3:	/* line feed */
			if (flg&ONLRET)
				goto cr;
			if (flg&ONLCR) {
				if (!(flg&ONOCR && *colp==0)) {
					sysinfo.outch++;
					putc('\r', &tp->t_outq);
				}
				goto cr;
			}
		nl:
			if (flg&NLDLY)
				c = 5;
			break;

		case 4:	/* tab */
			c = 8 - ((*colp)&07);
			*colp += c;
			ctype = flg&TABDLY;
			if (ctype == TAB0) {
				c = 0;
			} else if (ctype == TAB1) {
				if (c < 5)
					c = 0;
			} else if (ctype == TAB2) {
				c = 2;
			} else if (ctype == TAB3) {
				sysinfo.outch += c;
				do
					putc(' ', &tp->t_outq);
				while (--c);
				continue;
			}
			break;

		case 5:	/* vertical tab */
			if (flg&VTDLY)
				c = 0177;
			break;

		case 6:	/* carriage return */
			if (flg&OCRNL) {
				cs = '\n';
				goto nl;
			}
			if (flg&ONOCR && *colp == 0)
				continue;
		cr:
			ctype = flg&CRDLY;
			if (ctype == CR1) {
				if (*colp)
					c = max((*colp>>4) + 3, 6);
			} else if (ctype == CR2) {
				c = 5;
			} else if (ctype == CR3) {
				c = 9;
			}
			*colp = 0;
			break;

		case 7:	/* form feed */
			if (flg&FFDLY)
				c = 0177;
			break;
		}
		sysinfo.outch++;
		putc(cs, &tp->t_outq);
		if (c) {
			if ((c < 32) && flg&OFILL) {
				if (flg&OFDEL)
					cs = 0177;
				else
					cs = 0;
				putc(cs, &tp->t_outq);
				if (c > 3)
					putc(cs, &tp->t_outq);
			} else {
				putc(QESC, &tp->t_outq);
				putc(c|0200, &tp->t_outq);
			}
		}

	}
	if (scf != NULL)
		putcf(scf);
}

/*
 * Get next packet from output queue.
 * Called from xmit interrupt complete.
 */

ttout(tp)
register struct tty *tp;
{
	register struct ccblock *tbuf;
	register c;
	register char *cptr;
	register retval;
	extern ttrstrt();

	if (tp->t_state&TTIOW && tp->t_outq.c_cc==0) {
		tp->t_state &= ~TTIOW;
		wakeup((caddr_t)&tp->t_oflag);
	}
delay:
	tbuf = &tp->t_tbuf;
	if (hibyte(tp->t_lflag)) {
		if (tbuf->c_ptr) {
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
			tbuf->c_ptr = NULL;
		}
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, (hibyte(tp->t_lflag)&0177)+6);
		hibyte(tp->t_lflag) = 0;
		return(0);
	}
	retval = 0;

	if ((tp->t_state&EXTPROC) || (!(tp->t_oflag&OPOST))) {
		if (tbuf->c_ptr)
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
		if ((tbuf->c_ptr = (char *)getcb(&tp->t_outq)) == NULL)
			return(0);
		tbuf->c_count = ((struct cblock *)tbuf->c_ptr)->c_last -
				((struct cblock *)tbuf->c_ptr)->c_first;
		tbuf->c_size = tbuf->c_count;
		tbuf->c_ptr = &((struct cblock *)tbuf->c_ptr)->c_data
				[((struct cblock *)tbuf->c_ptr)->c_first];
		retval = CPRES;
	} else {		/* watch for timing	*/
		if (tbuf->c_ptr == NULL) {
			if ((tbuf->c_ptr = getcf()->c_data)
				== ((struct cblock *)NULL)->c_data) {
				tbuf->c_ptr = NULL;
				return(0);	/* Add restart? */
			}
		}
		tbuf->c_count = 0;
		cptr = tbuf->c_ptr;
		while ((c=getc(&tp->t_outq)) >= 0) {
			if (c == QESC) {
				if ((c = getc(&tp->t_outq)) < 0)
					break;
				if (c > 0200) {
					hibyte(tp->t_lflag) = c;
					if (!retval)
						goto delay;
					break;
				}
			}
			retval = CPRES;
			*cptr++ = c;
			tbuf->c_count++;
			if (tbuf->c_count >= cfreelist.c_size)
				break;
		}
		tbuf->c_size = tbuf->c_count;
	}

out:
	if (tp->t_state&OASLP &&
		tp->t_outq.c_cc<=ttlowat[tp->t_cflag&CBAUD]) {
		tp->t_state &= ~OASLP;
		wakeup((caddr_t)&tp->t_outq);
	}
	return(retval);
}

tttimeo(tp)
register struct tty *tp;
{
	tp->t_state &= ~TACT;
	if (tp->t_lflag&ICANON || tp->t_cc[VTIME] == 0)
		return;
	if (tp->t_rawq.c_cc == 0 && tp->t_cc[VMIN])
		return;
	if (tp->t_state&RTO) {
		tp->t_delct = 1;
		if (tp->t_state&IASLP) {
			tp->t_state &= ~IASLP;
			wakeup((caddr_t)&tp->t_rawq);
		}
	} else {
		tp->t_state |= RTO|TACT;
		timeout(tttimeo, tp, tp->t_cc[VTIME]*(HZ/10));
	}
}

/*
 * I/O control interface
 */
ttioctl(tp, cmd, arg, mode)
register struct tty *tp;
{
	ushort	chg;
	register int	oldpri;

	switch (cmd) {
	case LDOPEN:
		if (tp->t_rbuf.c_ptr == NULL) {
			/* allocate RX buffer */
			while ((tp->t_rbuf.c_ptr = getcf()->c_data)
				== ((struct cblock *)NULL)->c_data) {
				tp->t_rbuf.c_ptr = NULL;
				cfreelist.c_flag = 1;
				sleep(&cfreelist, TTOPRI);
			}
			tp->t_rbuf.c_count = cfreelist.c_size;
			tp->t_rbuf.c_size  = cfreelist.c_size;
			(*tp->t_proc)(tp, T_INPUT);
		}
		break;

	case LDCLOSE:
		oldpri = spltty();
		(*tp->t_proc)(tp, T_RESUME);
		splx(oldpri);
		ttywait(tp);
		ttyflush(tp, FREAD);
		if (tp->t_tbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_tbuf.c_ptr));
			tp->t_tbuf.c_ptr = NULL;
		}
		if (tp->t_rbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_rbuf.c_ptr));
			tp->t_rbuf.c_ptr = NULL;
		}
		break;

	case LDCHG:
		chg = tp->t_lflag^arg;
		if (!(chg&ICANON))
			break;
		oldpri = spltty();
		if (tp->t_canq.c_cc) {
			if (tp->t_rawq.c_cc) {
				tp->t_canq.c_cc += tp->t_rawq.c_cc;
				tp->t_canq.c_cl->c_next = tp->t_rawq.c_cf;
				tp->t_canq.c_cl = tp->t_rawq.c_cl;
			}
			tp->t_rawq = tp->t_canq;
			tp->t_canq = ttnulq;
		}
		tp->t_delct = tp->t_rawq.c_cc;
		splx(oldpri);
		break;

	default:
		break;
	}
}
