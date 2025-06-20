/* @(#)tty.c	6.2	 */
/*
 * general TTY subroutines
 */
#ident	"@(#)Kern-port:isp.tty.c 10.7"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/ttold.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/map.h"

#define ISP
#define KERNEL 1

#ifdef ISP

#define  defined_io	1

#include "sys/cstty.h"
/* #include "csintern.h" */

extern struct csttinf *getcsip( );

extern int greek;

/* cstrans off buffer corresponding to canonb */
unsigned char 	csoff[CANBSIZ];

#endif /* ISP */

/*
 * tty low and high water marks
 * high < TTYHOG
 */
int	tthiwat[16] = {
	0, 60, 60, 60,
	60, 60, 60, 120,
	120, 180, 180, 240,
	240, 240, 100, 100,
};
int	ttlowat[16] = {
	0, 20, 20, 20,
	20, 20, 20, 40,
	40, 60, 60, 80,
	80, 80, 50, 50,
};

char	ttcchar[NCC] = {
	CINTR,
	CQUIT,
	CERASE,
	CKILL,
	CEOF,
	0,
	0,
	0
};

/* null clist header */
struct clist ttnulq;

/* canon buffer */
#ifdef ISP
ushort	canonb[CANBSIZ];
#else
char	canonb[CANBSIZ];
#endif /* ISP */
/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */
char	maptab[] = {
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,'|',000,000,000,000,000,'`',
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,'~',000,
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};

/*
 * common ioctl tty code
 */
ttiocom(tp, cmd, arg, mode)
register struct tty *tp;
{
	extern sxtin();
	register flag;
	struct termio cb;
	struct sgttyb tb;
#ifdef  ISP
	static csttname csnoname = { "\0\0\0\0\0\0\0\0", "\0\0\0\0\0\0" };
#endif  /* ISP */

	switch(cmd)
	{
	case IOCTYPE:
		u.u_rval1 = TIOC;
		break;

	case TCSETAW:
	case TCSETAF:
		ttywait(tp);
		if (cmd == TCSETAF)
			ttyflush(tp, (FREAD|FWRITE));
	case TCSETA:
		if (copyin(arg, &cb, sizeof cb))
		{
			u.u_error = EFAULT;
			break;
		}
		if (tp->t_line != cb.c_line) {
			if (cb.c_line < 0 || cb.c_line >= linecnt ||
				/* don't allow user to specify SXT LD */
				linesw[cb.c_line].l_input == sxtin) {
				u.u_error = EINVAL;
				break;
			}
			(*linesw[tp->t_line].l_ioctl)(tp, LDCLOSE, 0, mode);
		}
		flag = tp->t_lflag;
		tp->t_iflag = cb.c_iflag;
		tp->t_oflag = cb.c_oflag;
		tp->t_cflag = cb.c_cflag;
		tp->t_lflag = cb.c_lflag;
		bcopy(cb.c_cc, tp->t_cc, NCC);
		if (tp->t_line != cb.c_line)
		{
			tp->t_line = cb.c_line;
			(*linesw[tp->t_line].l_ioctl)(tp, LDOPEN, 0, mode);
		}
		else if (tp->t_lflag != flag)
		{
			(*linesw[tp->t_line].l_ioctl)(tp, LDCHG, flag, mode);
		}
		return(1);

	case TCGETA:
		cb.c_iflag = tp->t_iflag;
		cb.c_oflag = tp->t_oflag;
		cb.c_cflag = tp->t_cflag;
		cb.c_lflag = tp->t_lflag;
		cb.c_line = tp->t_line;
		bcopy(tp->t_cc, cb.c_cc, NCC);
		if (copyout(&cb, arg, sizeof cb))
			u.u_error = EFAULT;
		break;

	case TCSBRK:
		ttywait(tp);
		if (arg == 0)
			(*tp->t_proc)(tp, T_BREAK);
		break;

	case TCXONC:
		switch (arg)
		{
		case 0:
			(*tp->t_proc)(tp, T_SUSPEND);
			break;
		case 1:
			(*tp->t_proc)(tp, T_RESUME);
			break;
		case 2:
			(*tp->t_proc)(tp, T_BLOCK);
			break;
		case 3:
			(*tp->t_proc)(tp, T_UNBLOCK);
			break;
		default:
			u.u_error = EINVAL;
		}
		break;

	case TCFLSH:
		switch (arg)
		{
		case 0:
		case 1:
		case 2:
			ttyflush(tp, (arg - FOPEN)&(FREAD|FWRITE));
			break;

		default:
			u.u_error = EINVAL;
		}
		break;

#ifdef  ISP
/* Start ISP additions ttiocom-2 */
	case CSGETTT:		/* request translate table data */
	{
		register struct cstthdr *cstt;
		register int		 n;
		struct csttiohdr 	 iohdr;
		
		if (copyin(arg, &iohdr, sizeof(iohdr)))
		{
			u.u_error = EFAULT;
			break;
		}
		if ((n = iohdr.cs_h.cs_tnum) < 0 || n >= cs_info.cs_imap)
		{
			u.u_error = ENXIO;
			break;
		}
		if (cstt = cs_ttmap[n])
			n = copyout(cstt, arg +
				(int)(&iohdr.cs_h) - (int)(&iohdr),
				min(iohdr.cs_tmax, cstt->cs_tlen));
		else
		{
			iohdr.cs_h.cs_tlen = 0;
			n = copyout(&iohdr, arg, sizeof(iohdr));
		}
		if (n)
			u.u_error = EFAULT;
		break;
	}

	case CSSETTT:		/* change translate table data */
	{
		register struct cstthdr	*cstt;
		register ushort		 ntt;
		struct csttiohdr	 iohdr;
		short			 roundseg;
		short			 tloc;

		if (! suser()) break;
		if (copyin(arg, &iohdr, sizeof(iohdr)))
		{
		    u.u_error = EFAULT;
		    break;
		}
		if ((ntt = iohdr.cs_h.cs_tnum) >= cs_info.cs_imap)
		{
		    u.u_error = ENXIO;
		    break;
		}
		cstt = cs_ttmap[ntt];
		if (iohdr.cs_h.cs_tlen)
		{	/* to load a new table */
		    if (cstt)
		    {
			u.u_error = EEXIST;
			break;
		    }

		    roundseg =  CSSEGS(iohdr.cs_h.cs_tlen);
		    cstt = (struct cstthdr *) malloc(csmap, roundseg);
		    if (cstt == 0)
		    {
			printf( "ttiocom: Can't allocate %d segments\n", roundseg );
			u.u_error = ENOMEM;
			break;
		    }
		    if (copyin(arg + (int)(&iohdr.cs_h) - (int)(&iohdr),
			cstt, iohdr.cs_h.cs_tlen))
		    {
			    u.u_error = EFAULT;
			    mfree(csmap, roundseg, cstt);
			    break;
		    }
		    cs_ttmap[ntt] = cstt;
		    cstt->cs_nref = 0;

		}
		else if (cstt)
		{		/* to remove existing table */
		    if (cstt->cs_nref)
		    {
			u.u_error = EBUSY;
			break;
		    }

		    mfree(csmap, CSSEGS(cstt->cs_tlen), cstt);
		    cs_ttmap[ntt] = NULL;
		}
		break;
	}

	case CSGETO:		/* request char set options */
	{
		register struct csttinf	*csip;
		struct csopt 		 opt;
		
		csip = getcsip(tp);
		opt.cs_options = csip->cs_tiopt;
		bcopy((csip->cs_tt) ? &csip->cs_tt->cs_tname : &csnoname,
		    &opt.cs_name, sizeof(opt.cs_name));
		if (copyout(&opt, arg, sizeof(opt)))
		    u.u_error = EFAULT;
		break;
	}

	case CSSETOW:		/* change char set options after wait */
	case CSSETOF:		/* change char set options and flush */
		ttywait(tp);
		if (cmd == CSSETOF)
		    ttyflush(tp, FREAD|FWRITE);

	case CSSETO:		/* change char set options */
	{
		register struct csttinf *csip;
		register struct cstthdr *cstt;
		register char 		*ttnp;
		register ushort		 ntt;
		struct csopt 		 opt;
		
		/* get char set translation tty info structure */
		csip = getcsip(tp);

		if (copyin(arg, &opt, sizeof(opt)))
		{
		    u.u_error = EFAULT;
		    break;
		}

		for (ntt = 0; ntt < cs_info.cs_imap; ntt++)
		{
		    cstt = cs_ttmap[ntt];
		    if (! ttmatch(&cstt->cs_tname, &opt.cs_name))
			cstt = NULL;
		    else
			break;
		}

		if (cstt == NULL && ! ttmatch(&opt.cs_name, &csnoname))
		{
		    u.u_error = EINVAL;
		    break;
		}

		if (cstt != csip->cs_tt)
		{
		    if (csip->cs_tt)
			csip->cs_tt->cs_nref--;
		    if (cstt)
			cstt->cs_nref++;
		    csip->cs_tt = cstt;

		      /* clear the state variables */
		    CSTATE &= ~(CSEXTSO|CS_1SAVE|CS_1XLAT|CS_1CNT);
		    CSTATE |= cstt->cs_ttflag&(CSEXTSO|CS_4RESET);
		    csip->cs_1pos = 0;
		    csip->cs_1len = 0;
		    csip->cs_1char = -1;
		}
		if (opt.cs_options&CST16 && ! (csip->cs_tiopt&CST16))
		    CSTATE |= CS_2RESET;
		csip->cs_tiopt = opt.cs_options;
		if (csip->cs_tiopt&CS040)
		{
		    csip->cs_tiopt &= ~ CST16;
		    csip->cs_3set = CSMOTPRI;
		}
		else
		    csip->cs_3set = 0;
		cselect = 0;

		break;
	}
/* End ISP addition ttiocom-2 */
#endif  /* ISP */

/* conversion aide only */
	case TIOCSETP:
		ttywait(tp);
		ttyflush(tp, (FREAD|FWRITE));
		if (copyin(arg, &tb, sizeof(tb)))
		{
			u.u_error = EFAULT;
			break;
		}
		tp->t_iflag = 0;
		tp->t_oflag = 0;
		tp->t_lflag = 0;
		tp->t_cflag = (tb.sg_ispeed&CBAUD)|CREAD;
		if ((tb.sg_ispeed&CBAUD)==B110)
			tp->t_cflag |= CSTOPB;
		tp->t_cc[VERASE] = tb.sg_erase;
		tp->t_cc[VKILL] = tb.sg_kill;
		flag = tb.sg_flags;
		if (flag&O_HUPCL)
			tp->t_cflag |= HUPCL;
		if (flag&O_XTABS)
			tp->t_oflag |= TAB3;
		else if (flag&O_TBDELAY)
			tp->t_oflag |= TAB1;
		if (flag&O_LCASE)
		{
			tp->t_iflag |= IUCLC;
			tp->t_oflag |= OLCUC;
			tp->t_lflag |= XCASE;
		}
		if (flag&O_ECHO)
			tp->t_lflag |= ECHO;
		if (!(flag&O_NOAL))
			tp->t_lflag |= ECHOK;
		if (flag&O_CRMOD)
		{
			tp->t_iflag |= ICRNL;
			tp->t_oflag |= ONLCR;
			if (flag&O_CR1)
				tp->t_oflag |= CR1;
			if (flag&O_CR2)
				tp->t_oflag |= ONOCR|CR2;
		}
		else
		{
			tp->t_oflag |= ONLRET;
			if (flag&O_NL1)
				tp->t_oflag |= CR1;
			if (flag&O_NL2)
				tp->t_oflag |= CR2;
		}
		if (flag&O_RAW)
		{
			tp->t_cc[VTIME] = 1;
			tp->t_cc[VMIN] = 6;
			tp->t_iflag &= ~(ICRNL|IUCLC);
			tp->t_cflag |= CS8;
		}
		else
		{
			tp->t_cc[VEOF] = CEOF;
			tp->t_cc[VEOL] = 0;
			tp->t_cc[VEOL2] = 0;
			tp->t_iflag |= BRKINT|IGNPAR|ISTRIP|IXON|IXANY;
			tp->t_oflag |= OPOST;
			tp->t_cflag |= CS7|PARENB;
			tp->t_lflag |= ICANON|ISIG;
		}
		tp->t_iflag |= INPCK;
		if (flag&O_ODDP)
			if (flag&O_EVENP)
				tp->t_iflag &= ~INPCK;
			else
				tp->t_cflag |= PARODD;
		if (flag&O_VTDELAY)
			tp->t_oflag |= FFDLY;
		if (flag&O_BSDELAY)
			tp->t_oflag |= BSDLY;
		return(1);

	case TIOCGETP:
		tb.sg_ispeed = tp->t_cflag&CBAUD;
		tb.sg_ospeed = tb.sg_ispeed;
		tb.sg_erase = tp->t_cc[VERASE];
		tb.sg_kill = tp->t_cc[VKILL];
		flag = 0;
		if (tp->t_cflag&HUPCL)
			flag |= O_HUPCL;
		if (!(tp->t_lflag&ICANON))
			flag |= O_RAW;
		if (tp->t_lflag&XCASE)
			flag |= O_LCASE;
		if (tp->t_lflag&ECHO)
			flag |= O_ECHO;
		if (!(tp->t_lflag&ECHOK))
			flag |= O_NOAL;
		if (tp->t_cflag&PARODD)
			flag |= O_ODDP;
		else if (tp->t_iflag&INPCK)
			flag |= O_EVENP;
		else
			flag |= O_ODDP|O_EVENP;
		if (tp->t_oflag&ONLCR)
		{
			flag |= O_CRMOD;
			if (tp->t_oflag&CR1)
				flag |= O_CR1;
			if (tp->t_oflag&CR2)
				flag |= O_CR2;
		}
		else
		{
			if (tp->t_oflag&CR1)
				flag |= O_NL1;
			if (tp->t_oflag&CR2)
				flag |= O_NL2;
		}
		if ((tp->t_oflag&TABDLY)==TAB3)
			flag |= O_XTABS;
		else if (tp->t_oflag&TAB1)
			flag |= O_TBDELAY;
		if (tp->t_oflag&FFDLY)
			flag |= O_VTDELAY;
		if (tp->t_oflag&BSDLY)
			flag |= O_BSDELAY;
		tb.sg_flags = flag;
		if (copyout(&tb, arg, sizeof(tb)))
			u.u_error = EFAULT;
		break;

	default:
		if ((cmd&IOCTYPE) == LDIOC)
			(*linesw[tp->t_line].l_ioctl)(tp, cmd, arg, mode);
		else
			u.u_error = EINVAL;
		break;
	}
	return(0);
}

ttinit(tp)
register struct tty *tp;
{
	tp->t_line = 0;
	tp->t_iflag = 0;
	tp->t_oflag = 0;
	tp->t_cflag = SSPEED|CS8|CREAD|HUPCL;
	tp->t_lflag = 0;
	bcopy(ttcchar, tp->t_cc, NCC);

#ifdef  ISP
	{
		register struct csttinf *csip = getcsip( tp );

		csip->cs_tt = NULL;
		csip->cs_tiopt = 0;
		cselect = 0;

		/*
		 * Clear the state variables
		 */
		CSTATE = CS_2RESET | CS_4RESET;
		csip->cs_1pos = 0;
		csip->cs_1char = -1;
		csip->cs_1len = 0;
		csip->cs_2acc = 0xFFFF;
		csip->cs_2set = 0;
		csip->cs_3set = 0;
		csip->cs_3pos = 0;
		csip->cs_3save = 0;
		csip->cs_4set = 0;
		csip->cs_4acc = 0xFFFF;
	}
#endif  /* ISP */
}

ttywait(tp)
register struct tty *tp;
{
	register int	oldpri;
	static	int	rate[] =
	{
		HZ+1 ,	/* avoid divide-by-zero, as well as unnecessary delay */
		50 ,
		75 ,
		110 ,
		134 ,
		150 ,
		200 ,
		300 ,
		600 ,
		1200 ,
		1800 ,
		2400 ,
		4800 ,
		9600 ,
		19200 ,
		38400 ,
	} ;

	oldpri = spltty();
	while (tp->t_outq.c_cc || (tp->t_state&(BUSY|TIMEOUT))) {
		tp->t_state |= TTIOW;
		sleep((caddr_t)&tp->t_oflag, TTOPRI);
	}
	splx(oldpri);
				/* delay 11 bit times to allow uart to empty */
				/* add one to allow for truncation */
				/* add one to allow for partial clock tick */
	delay(1+1+11*HZ/rate[tp->t_cflag&CBAUD]) ;
}

/*
 * flush TTY queues
 */
ttyflush(tp, cmd)
register struct tty *tp;
{
	register struct cblock *cp;
	register s;
#ifdef  ISP
	register struct csttinf *csip = getcsip( tp );
#endif  /* ISP */
	

	if (cmd&FWRITE)
	{
		while ((cp = getcb(&tp->t_outq)) != NULL)
			putcf(cp);
		(*tp->t_proc)(tp, T_WFLUSH);

#ifdef  ISP
/* Start ISP additions ttyflush-2 */

		/*
		 * Clear output state variables
		 */
		CSTATE &= ~(CS_3INTSO|CS_3DS16|CS_3F377|CS_3FSUB|
			CS_3FXCS|CS_3ACC);
		CSTATE |= CS_4RESET;
		if (csip->cs_tiopt&CS040)
		    csip->cs_3set = CSMOTPRI;
		else
		    csip->cs_3set = 0;
		cselect = 0;
		csip->cs_3pos = 0;
		csip->cs_3save = 0;
		csip->cs_4acc = 0xFFFF;
/* End ISP addition ttyflush-2 */
#endif  /* ISP */

		if (tp->t_state&OASLP)
		{
			tp->t_state &= ~OASLP;
			wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_state&TTIOW)
		{
			tp->t_state &= ~TTIOW;
			wakeup((caddr_t)&tp->t_oflag);
		}
	}
	if (cmd&FREAD)
	{
		while ((cp = getcb(&tp->t_canq)) != NULL)
			putcf(cp);
		s = spltty();

		while ((cp = getcb(&tp->t_rawq)) != NULL)
			putcf(cp);
		tp->t_delct = 0;

#ifdef  ISP
/* Start ISP additions ttyflush-3 */

		/*
		 * Clear input state variables
		 */
		CSTATE &= ~(CS_1SAVE|CS_1XLAT|CS_2INTSO);
		CSTATE |= CS_2RESET;
		csip->cs_1char = -1;
		csip->cs_2acc = 0xFFFF;
		csip->cs_2set = 0;
/* End ISP addition ttyflush-3 */
#endif  /* ISP */

		splx(s);
		(*tp->t_proc)(tp, T_RFLUSH);
		if (tp->t_state&IASLP)
		{
			tp->t_state &= ~IASLP;
			wakeup((caddr_t)&tp->t_rawq);
		}
	}
}

/*
 * Transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 */
/* Pseudocode
	    Prevent interrupts below level 5.
	    If the raw queue is empty, clear the delimiter count too.
	    While the delimiter count is zero:
		If carrier is off or the file mode is no delay
		    Allow all interrupts and return.

		If not doing canonical input and if VMIN count is zero
		    Handle waiting for input.
		If not doing canonical input
		    If doing input translation
			Allow all interrupts.
			Translate raw queue characters into the canonical buffer.
			Translate the canonical buffer into the canonical queue.
		    Else
			Move the raw queue to the canonical queue en masse.
			Allow all interrupts
		    Return.

	    Allow all interrupts.
	    While a translated 16-bit char is available
		If the last char was not an escape
		    Recognize and handle a backslash/erase/kill/eof char.
		Else
		    Handle escaped erase/kill/eof and backslash depending on XCASE.
		Store the char in the canonical buffer.
		Break on a delimiter (newline or alternate newline).
		Avoid buffer overflow by backing up one at end.
	    Decrement the delimiter count.
	    Determine number of chars in canonical buffer and bump statistics.
	    Translate from the canonical buffer into the canonical queue.
	 */
canon(tp)
register struct tty *tp;
{
#ifdef ISP
	register ushort *bp;
#else
	register char *bp;
#endif /* ISP */
	register c, esc;
	register int oldpri;
	register struct cblock *cb;
#ifdef ISP
/* Start ISP additions canon-1 */

	register struct csttinf *csip;
	register unsigned char *cc;
	
	/* get char set translation tty info structure */
	csip = getcsip(tp);

/* End ISP addition canon-1 */
#endif /* ISP */

	oldpri=spltty();


	if (tp->t_rawq.c_cc == 0)
		tp->t_delct = 0;
	while (tp->t_delct == 0)
	{
		if (!(tp->t_state&CARR_ON) || (u.u_fmode&FNDELAY))
		{
			splx(oldpri);
			return;
		}
		if (!(tp->t_lflag&ICANON) && tp->t_cc[VMIN]==0)
		{
			if (tp->t_cc[VTIME]==0)
				break;
			tp->t_state &= ~RTO;
			if (!(tp->t_state&TACT))
				tttimeo(tp);
		}
		tp->t_state |= IASLP;
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	if (!(tp->t_lflag&ICANON))
	{
#ifdef ISP
/* Start ISP additions canon-2 */
		if (!greek && (csip->cs_tiopt&CSTRANS))
		{
			register int ndel = 0;
			splx(oldpri); /*Changed from  splx(s) */
			while ((c = ttre2i(csip)) >= 0)
			{
				if (csip->cs_tiopt&CS_OFF)
				{
					csip->cs_tiopt &= ~CS_OFF;
					if (csip->cs_tiopt&CST16)
					{
					    if (CSTATE&CS_2RESET)
					    {
						CSTATE &= ~CS_2RESET;
						CSTATE &= ~CS_2INTSO;
						putc(0xff, &tp->t_canq);
						putc(0xff, &tp->t_canq);
						putc(0, &tp->t_canq);
					    }
					    putc(CNUL, &tp->t_canq);
					}
					putc(c, &tp->t_canq);
				}
				else
					ttri2xs(csip, c);
				if (c == '\n' || c && (c == tp->t_cc[VEOL] ||
				    c == tp->t_cc[VEOL2]))
					ndel++;
			}
			oldpri=spltty();
			if (tp->t_delct > ndel)
				tp->t_delct -= ndel;
			else
				tp->t_delct = 0;
		}
		else if ((!greek && !(csip->cs_tiopt&CSTRANS)) || (greek))
		{
/* End ISP addition canon-2 */
#endif /* ISP */
			while(cb=getcb(&tp->t_rawq))
				putcb(cb,&tp->t_canq);
			tp->t_delct=0;
#ifdef ISP
		}
#endif /* ISP */
		splx(oldpri);
		return;
	}
	splx(oldpri);
	bp = canonb;
#ifdef ISP
	/* Start ISP addition canon-3 */
	if(!greek)
		cc = csoff;
	/* End ISP addition canon-3 */
#endif /* ISP */
	esc = 0;
	while (1)
	{
		if(!greek && 
		((c=(csip->cs_tiopt&CSTRANS) ? ttre2i(csip) : getc(&tp->t_rawq)) < 0))
			break;
		if(greek && (c=getc(&tp->t_rawq)) < 0)
			break;
		if (!esc)
		{
			if (c == '\\')
			{
				esc++;
			}
			else if (c == tp->t_cc[VERASE])
			{
				if (bp > canonb)
				{
					bp--;
#ifdef ISP
					if(!greek)
						cc--;
#endif /* ISP */
				}
#ifdef ISP
/* Start ISP additions canon-4 */
				/* also back up over an accent character */
				if(!greek)
				{
					while (bp > canonb && CS_ACCENT(bp[-1]))
					{
						bp--;
						cc--;
					}
				}
/* End ISP addition canon-4 */
#endif /* ISP */
				continue;
			}
			else if (c == tp->t_cc[VKILL])
			{
				bp = canonb;
#ifdef ISP
				if(!greek)
					cc = csoff;
#endif /* ISP */
				continue;
			}
			else if (c == tp->t_cc[VEOF])
			{
#ifdef ISP
				if (!greek && (csip->cs_tiopt&CSTRANS) && (csip->cs_tiopt&CS_OFF))
				{
					csip->cs_tiopt &= ~CS_OFF;
					if (csip->cs_tiopt&CST16)
					{
		            			if (CSTATE&CS_2RESET)
			    			{
							CSTATE &= ~CS_2RESET;
							CSTATE &= ~CS_2INTSO;
			    			}
					}
				}
#endif /* ISP */
				break;
			}
		}
		else
		{
			esc = 0;
			if (c == tp->t_cc[VERASE] ||
			    c == tp->t_cc[VKILL] ||
			    c == tp->t_cc[VEOF])
			{
				bp--;
#ifdef ISP
				if(!greek)
					cc--;
#endif /* ISP */
			}
			else if (tp->t_lflag&XCASE)
			{
				if ((c < 0200) && maptab[c])
				{
					bp--;
#ifdef ISP
					if(!greek)
						cc--;
#endif /* ISP */
					c = maptab[c];
				}
				else if (c == '\\')
					continue;
			}
			else if (c == '\\')
				esc++;
		}
#ifdef ISP

/* Start ISP additions canon-8 */
		if (!greek && (csip->cs_tiopt&CS_OFF))
		{
			csip->cs_tiopt &= ~CS_OFF;
			if (csip->cs_tiopt&CST16)
			{
		            if (CSTATE&CS_2RESET)
			    {
				CSTATE &= ~CS_2RESET;
				CSTATE &= ~CS_2INTSO;
				*bp++ = 0xff;
			        (*cc++)++;
				*bp++ = 0xff;
			        (*cc++)++;
				*bp++ = 0;
			        (*cc++)++;
			    }
			    *bp++ = CNUL;
			    (*cc++)++;
			}
			*bp++ = c;
			(*cc++)++;
		}
		else if ((!greek && !(csip->cs_tiopt&CS_OFF)) || (greek))
		{
/* End ISP addition canon-8 */
#endif /* ISP */
			*bp++ = c;
#ifdef ISP
			if(!greek)
				(*cc++) = 0;
		}
#endif /* ISP */
		if (c == '\n' || c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
			break;
		if (bp >= &canonb[CANBSIZ])
		{
			bp--;
#ifdef ISP
			if(!greek)
				cc--;
#endif /* ISP */
		}
	}
	tp->t_delct--;
	c = bp - canonb;
	sysinfo.canch += c;
	bp = canonb;
#ifdef ISP
	/* Start ISP additions canon-11 */
	cc = csoff;
	if(!greek && (csip->cs_tiopt&CSTRANS))
	{
	    while (c--)
	    {
		if (*cc)
		{
			putc(*bp++, &tp->t_canq);
			(*cc++)--;
		}
		else
			ttri2xs(csip, *bp++);
	    }
	}
	else if ((!greek && !(csip->cs_tiopt&CSTRANS)) || (greek))
	{
/* End ISP addition canon-11 */
#endif /* ISP */
		/* faster copy ? */
		while (c--)
			putc(*bp++, &tp->t_canq);
	}
	return;
}

/*
 * Restart typewriter output following a delay timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 */
ttrstrt(tp)
register struct tty *tp;
{

	(*tp->t_proc)(tp, T_TIME);
}
