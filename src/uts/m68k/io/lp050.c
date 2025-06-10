
/*		Copyright (c) 1985 Motorola Inc.		*/
/*		All Rights Reserved				*/
/*  @(#)lp050.c	8.2        */
/*
 *	MVME050 line printer driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/lprio.h"
#include "sys/lp0.h"
#include "sys/mc68153.h"
#include "sys/sysmacros.h"

#define DEBUG		/* turn on entry printfs */
#undef DEBUG
#define DEBUGINT	/* turn on "unexpected interrupt" printf */
#undef DEBUGINT

extern struct device *l050_addr[];
extern int l050_cnt;
extern int l050_ilev[];
extern int l050_ivec[];



/* The following is description of the device registers in use here */
/*                                                                  */
/* FFXX81  (dreg)  (write D0-D7) printer data register              */
/* FFXX81  (dreg)  (read) - puts FF into data out register          */
/*                                                                  */
/* FFXX83  (ctrlreg) (write D3)  printer strobe register            */
/* FFXX83  (ctrlreg) (read) - clears acknowledge flag and prtr IRQ  */
/*                    				 		    */
/* FFXX85  (statreg) (write) causes hardware buffer conflict        */
/* FFXX85  (statreg) (read) - printer status			    */
/*                     D0 = select from printer                     */
/* 		       D1 = busy from printer                       */
/* 		       D2 = fault from printer			    */
/* 		       D7 = acknowledge from printer                */
/*							            */
/* FFXX87  (prmreg) (write D3) printer input prime                  */
/* FFXX87  (prmreg) (read) - no operation                           */
/*								    */
/********************************************************************/

struct device
{
	unsigned char dreg;	/* output data register */
	char fill0;		/* fill char */
	unsigned char ctrlreg;	/* control register */
	char fill1;		/* fill char */
	unsigned char statreg;	/* status register */
	char fill2;		/* fill char */
	unsigned char prmreg;	/* prime register  */
	char fill3;		/* fill char */
};

#define LPMAX	1	/* will support more later */

struct lp lp_dt[LPMAX];

#define	CNTNUM	10000	/* 150000 chosen because need about 1 second to */
			/* ensure worst case of PACK cycle duration has */
			/* passed before decide somethin wrong with prtr*/
#define YES	1
#define NO	0
#define SEL	1		/* printer is selected */	
#define PBUSY	2		/* printer is busy */
#define FAULT	4		/* printer condition i.e. paper empty, light */
                                /* detect, or deselect condition; NOTE: This */
				/* is actually not faulted because fault is  */
				/* true low.                                 */

#define RESET	0x8		/* an input signal which clears the printer */
				/* buffer and initializes the logic         */

#define PSTB	0xf7		/* toggle bit to send data or reset */
#define PACK	0x80		/* acknowledge from printer. */ 

/*
 *	MVME050 priming
 */

#define	PRIMECHAN	3	/* use BIM channel 3 */

/* initialize the board */
l050open(dev, mode)
{
	register struct lp *lp;
	register struct device *lpdev;
	register unsigned short port = minor(dev);
	extern l050start();


	/* if not a printer we use or another lp file is already open */
	/* generate I/O error      */
#ifdef DEBUG
	printf("LP050: open\n");
#endif

	if (port >= l050_cnt || ((lp = &lp_dt[port])->flag&L_OPEN)) { 
		u.u_error = EIO;
		return;
	}

	lpdev = l050_addr[port];

	/* check for device, use data register to avoid generating
	   a strobe */
	if( bprobe( &lpdev->dreg, 0 )) {
		u.u_error = ENXIO;
		return;
	}
	
	if (lp->lp_routine == 0) {
		lp->lp_routine = l050start;

		if (l050_cnt > LPMAX) {
			l050_cnt = LPMAX;
			if (port >= LPMAX) {
				u.u_error = EIO;
				return;
			}
		}

		/* initialize lp data structures */
		if (l050_ilev[port] > 7) {
			u.u_error = EIO;
			return;
		}
		lp->lev = 0x2000 | (l050_ilev[port] << 8);
		lp->dev = port;
		lp_open(lp);

		lpdev->prmreg = ~RESET; /* need to toggle reset which resets prntr */
		lpdev->prmreg = RESET;  /* which also negates strobe               */

		/* set up hardware */
		m050prime(PRIMECHAN, l050_ivec[0]/4, l050_ilev[0]|IRE);
		l050timer();	/* start timer */
	}

	/* if printer selected and not faulted then OK otherwise generate */
	/* I/O error. Remember FAULT is true low.                         */

	if (((SEL|FAULT)&lpdev->statreg) != (SEL|FAULT)) {
		u.u_error = EIO;
		return;
	}

	lp->flag |= L_OPEN;
	lp_output(lp,'\f');	/* send form feed character to prtr to ensure */
    			        /* that new characters start on new page      */
}

l050close(dev)
{
	register unsigned short port = minor(dev);
	register struct lp *lp = &lp_dt[port];

#ifdef DEBUG
	printf("LP050: close\n");
#endif
	lp_output(lp,'\f');
	lp->flag &= ~L_OPEN;
}

l050intr(dev)
{	register struct lp *lp = &lp_dt[dev];
	register x;

	/* check interrupt */
#ifdef DEBUG
	printf("LP050: interrupt\n");
#endif
#ifdef DEBUGINT
	if((lp->flag&L_BUSY) == 0) {
		printf("LP050: unexpected interrupt\n");
		x = l050_addr[dev]->ctrlreg;
		return;
	}
#endif
	x = spl6();			/* lock out timer */
	lp->flag &= ~(L_BUSY|L_TOUT);
	splx(x);

	/* process interrupt */
	l050start(lp);

	/* character count in queue is low and asleep, wakeup process */
	if (lp->l_outq.c_cc <= LPLOWAT && lp->flag&L_ASLP) {
		lp->flag &= ~L_ASLP;
		wakeup(lp);
	}
}

l050write(dev)
{
	register unsigned short port = minor(dev);
	lp_write(&lp_dt[port]);
}

l050start(lp)
register struct lp *lp;
{
	register struct device *lpdev;
	register int c;
	register tmp;
	register cnt;

#ifdef DEBUG
	printf("LP050 start\n");
#endif
	if( lp->flag&L_BUSY )
		return(0);

	lpdev = l050_addr[lp->dev];

	if (((SEL|FAULT)&lpdev->statreg) != (SEL|FAULT)) {
		tmp = lpdev->ctrlreg; 	/* clear PACK flag */
		if (lp->flag&L_FAULT)
			return(0);
		else {
			lp->flag |= L_FAULT;		/* flag the fault */
			printf("LP050: printer offline\n");
			return(0);
		};
	};
	lp->flag&L_FAULT;

#ifdef DEBUG
	printf("LP050 status: %x\n",lpdev->statreg);
#endif

	/* check statreg to see if have PACK  */

	if (lpdev->statreg & PACK) {
		lp->flag &= ~L_WACK;

		/* need this next while loop to make sure PACK is really */
		/* cleared because if I clear it while still low and not */
		/* fully completed cycle then it just immediately resets */
		/* itself.  Could potentially lose characters here.      */
		/* Using cnt as protection from rare case stuck in loop. */
		/* If cnt gets to 0 something is wrong with printer and  */
		/* want to generate error.                               */

		for( cnt = CNTNUM; (lpdev->statreg&PACK) && --cnt;
			tmp = lpdev->ctrlreg /* clear PACK flag */ );
		if (!cnt) {
			printf("LP050: line printer not responding\n");
			return(EIO);
		}
	}

	/* if printer ready and character present strobe character out */

 	/* ((lpdev->statreg&PBUSY) == 0) */

	if (!(lp->flag & L_WACK)) {
	    tmp = lpdev->ctrlreg;	/* clear PACK flag */
	    if((c = getc(&lp->l_outq)) >= 0) {

		lpdev->dreg = c;

#ifdef DEBUG
		printf("LP050: \\%x", c );
#endif

		lp->flag |= L_WACK;	/* note: this instruction delay  meets
					   the data setup time spec. (>1us) */
		/* strobe character out */
		lpdev->ctrlreg = PSTB;  /* assert active low strobe */
		lp->flag |= L_BUSY;	/* note: this instruction delay meets
					   the strobe width spec. (>1us) */
		lpdev->ctrlreg = ~PSTB;  /* negate active low strobe */
	    }
	}

	return(0);
}

l050ioctl(dev, cmd, arg, mode)
{
	register unsigned short port = minor(dev);
	lp_ioctl(cmd,arg,mode,&lp_dt[port]);
}

l050timer()
{	register struct lp *lp;
	register struct device *lpdev;
	register dev;

	for (lp = &lp_dt[0], dev=0; dev<l050_cnt; dev++, lp++) {
#ifdef DEBUG
		printf("LP050: timer flag = %x\n",lp->flag);
#endif
		/* L_BUSY on iff waiting for an interrupt */
		if ((lp->flag&L_BUSY) == 0)
			continue;
		/* L_TOUT on iff no interrupt occurred in last 60 secs */
		if ((lp->flag&L_TOUT) == 0) {
			lp->flag |= L_TOUT;
			continue;
		}
		/* no interrupt in last 60 seconds */
		lpdev = l050_addr[dev];
		lpdev->prmreg = ~RESET; /* resets printer */
		lpdev->prmreg = RESET;	/* negates strobe */

		/* fake an interrupt condition */
		lp->flag &= ~(L_BUSY|L_TOUT|L_WACK);
		l050start(lp);		/* continue */
		/* wakeup any waiting processes */
		if (lp->l_outq.c_cc <= LPLOWAT && lp->flag&L_ASLP) {
#ifdef DEBUG
			printf("LP050: wakeup\n");
#endif
			lp->flag &= ~L_ASLP;
			wakeup(lp);
		}
	}

	timeout(l050timer,0,60*HZ);
}

lp_open(lp)
register struct lp *lp;
{
	if (lp->col == 0) {		/* first time only */
		lp->ind = 4;		/* set indentation */
		lp->col = 132;		/* set number of columns */
		lp->line = 66;		/* set number of lines/page */
	}
}

lp_output(lp,c)
register struct lp *lp;
register c;
{

	/* ccc = current character count (logical printer head placement) */
	/* mcc = maximum character count (actual physical prtr head placement */
	/* mlc = maximum line count */

	switch(c) {
	case '\t':
		lp->ccc = ((lp->ccc+8-lp->ind) & ~7) + lp->ind;
		return;
	case '\n':
		lp->mlc++;
		if(lp->mlc >= lp->line )
			c = '\f';
	case '\f':
		putc('\r', &lp->l_outq);
		lp->mcc = 0;
		if (lp->mlc) {
			putc(c, &lp->l_outq);
			if(c == '\f')
				lp->mlc = 0;
		}
		goto strtlp;
	case '\r':
		putc('\r', &lp->l_outq);
	   strtlp:
		lp->ccc = lp->ind;
		splx(lp->lev);
		(*lp->lp_routine)(lp);
		spl0();
		return;
	case '\b':
		if(lp->ccc > lp->ind)
			lp->ccc--;
		return;
	case ' ':
		lp->ccc++;
		return;
	default:
		if(lp->ccc < lp->mcc) {
			if (lp->flag&L_NOCR) {
				lp->ccc++;
				return;
			}
			putc('\r', &lp->l_outq);
			lp->mcc = 0;
		}
		if(lp->ccc < lp->col) {
			while(lp->ccc > lp->mcc) {
				putc(' ', &lp->l_outq);
				lp->mcc++;
			}
			putc(c, &lp->l_outq);
			lp->mcc++;
		}
		lp->ccc++;
	}
}

lp_write(lp)
register struct lp *lp;
{	register c;

	if (u.u_error)
		return;

	while (u.u_count) {		/* while bytes remaining for output */

		/* while charctr count in outpt queue is already large enough */
		/* "sleep" for a while so as not to flood the character buffr */
		/* pool.                                                      */

		while (lp->l_outq.c_cc > LPHIWAT) {
			splx(lp->lev);	/* inhibit interrupts from lp */
			/* check for interrupts in progress */
			if (u.u_error = (*lp->lp_routine)(lp))
				return;
			if (lp->l_outq.c_cc > LPHIWAT) {
#ifdef DEBUG
				printf("lp0: going to sleep \n");
#endif
				lp->flag |= L_ASLP;
				sleep(lp, LPPRI);
			}
			spl0();  /* start interrupts from lp */
		}
		c = fubyte(u.u_base++);  /* fetch a byte from user space */
		if (c < 0) {  /* if bad return error */
			u.u_error = EFAULT;
			break;
		}
		u.u_count--;
		lp_output(lp,c);  /* store character in queue */
	}
	splx(lp->lev);
	u.u_error = (*lp->lp_routine)(lp);
	spl0();
}

lp_ioctl(cmd, arg, mode, lp)
register struct lp *lp;
{
	struct lprio lpr;

	switch (cmd) {
	default:
		u.u_error = EINVAL;
		return;

	case LPRGET:
		lpr.ind = lp->ind;
		lpr.col = lp->col;
		lpr.line = lp->line;
		if (copyout(&lpr, arg, sizeof lpr))
			u.u_error = EFAULT;
		return;

	case LPRSET:
		if (copyin(arg, &lpr, sizeof lpr)) {
			u.u_error = EFAULT;
			return;
		}
		lp->ind = lpr.ind;
		lp->col = lpr.col;
		lp->line = lpr.line;
	}
}
