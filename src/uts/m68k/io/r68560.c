/*		Copyright (c) 1985 Motorola Inc.		*/
/*		All Rights Reserved				*/
/*	@(#)r68560.c	8.1		*/
/*
 *	Driver for R68560 MPCC RS-232C Serial Port Module
 *		on MVME050 board
 *
 */

char m5vers[] = {"@(#)r68560.c	8.1	"};

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/file.h"
#include "sys/signal.h"
#include "sys/tty.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/sysmacros.h"
#define M5MSCAN	20	/* scan loops for modem scan */
#define OFF     0
#define ON      1
#define SMAX    22



/* BAUD RATE matrix*/
unsigned char baudrate[]={
B0,B50,B75,B110,B134,B150,B200,B300,B600,
B1200,B1800,B2400,B4800,B9600,EXTA,EXTB};
/*values to vector the interrupt service routines*/
#define PORTOFFSET 1

/* RSR and RIER operations 
	int enabled if bit set in RIER
*/

#define RIDLE		0001
/*receiver idle if set (RSR only)*/
#define RAB		0002
/*break detected if set*/
#define ROVRN		0004
/*data overrun if set*/
#define FRERR		0010
/*frame error if set*/
#define PERR		0020
/*parity error if set*/
#define SEOF		0100	/*not used */
#define RDA		0200	
/*receiver data available*/

/* RCR operations */
#define RRES		0x01
/*reset rcvr if set*/
#define RABTEN		0002
/*set to enable DMA*/

/*TSR and TIER operations*/

#define TDRA		0200
/*set on data register available*/

/*TCR operations*/

#define TRES		0x01
/*set to reset trans*/
#define TLAST		0010
/*set to indicate last character*/
#define TICS		0040
/* set for break*/
#define TEN		0200
/*set to enable trans
  the txFIFO may be loaded with trans disabled*/



/*SISR and SIER  operations*/

/*SISR only*/
/*set if data carrier detect*/
#define DCDLVL		004
/*set if data set ready*/
#define DSRLVL		010
/*set if clear to send*/
#define CTSLVL		020

/*SISR and SIER with SIER enabling interrupt for this condition*/
#define DCD		0040
#define DSR		0100
#define CTS		0200
/*goes high to indicate dcd transition to inactive*/

/*SICR operations*/

#define SIECHO		04
/*set to echo (half duplex)*/
#define DTRLVL		0100
/*set to assert DTR*/
#define RTSLVL		0200
/*set to assert RTS*/


/* global registers
	TRES and RRES must be set and then cleared after
		changes to these registers*/
/*PSR1 operations not used */

/*PSR2 operations*/
/* choose one for protocol mode */
#define ISOC		07
#define ASYNC		06
/* choose one for char length*/
#define CLB		030	/*mask for CL*/
#define CL8		(03<<3)
#define CL7		(02<<3)
#define CL6		(01<<3)
#define CL5		(00<<3)
/*choose one for stop bits*/
#define SBB		0140
#define SB1		(00<<5)
#define SB1P5		(01<<5)
#define SB2		(010<<5)
/*set for word mode*/
#define WDBYT		0200

/*AR1 and AR2 are used in SDLC and BISYNC only*/



/*baud rate selection uses the baud rate divide registers*/
/* for asynch with kvalue set at 2 */
#define BD50L		0x2b
#define BD50H		0x68
/*PSCDIV set*/
#define BD50F		1

#define BD75L		BD50L
#define BD75H		BD50H
/*PSCDIV reset*/
#define BD75F		0

#define BD110L	0x59
#define BD110H	0x2f
/*PSCDIV set*/
#define BD110F		1

#define BD135L	0xdf	
#define BD135H	0x39	
/*PSCDIV reset*/
#define BD135F		0

#define BD150L	0xb9
#define BD150H	0x22
/*PSCDIV set*/
#define BD150F		1

#define BD200L	0x27
#define BD200H	0x10
/*PSCDIV reset*/
#define BD200F		0

#define BD300L	0x0b
#define BD300H	0x1a
/*PSCDIV reset*/
#define BD300F		0

#define BD600L	0x8d
#define BD600H	0x05
/*PSCDIV reset*/
#define BD600F		0

#define BD1200L	0x57
#define BD1200H	0x04
/*PSCDIV set*/
#define BD1200F		1

#define BD1800L	BD1200L
#define BD1800H	BD1200H
/*PSCDIV reset*/
#define BD1800F		0

#define BD2400L	0x41
#define BD2400H	0x03
/*PSCDIV reset*/
#define BD2400F		0

#define BD3600L	0x2c
#define BD3600H	0x02
/*PSCDIV reset*/
#define BD3600F		0

#define BD4800L	0x16
#define BD4800H	0x01
/*PSCDIV set*/
#define BD4800F		1

#define BD7200L	BD4800L
#define BD7200H	BD4800H
/*PSCDIV reset*/
#define BD7200F		0

#define BD9600L	0x8b
#define BD9600H	0x00
/*PSCDIV set*/
#define BD9600F		1

#define BD19kL	0x68
#define BD19kH	0x00
/*PSCDIV reset*/
#define BD19kF		0

#define BD38kL	0x34
#define BD38kH	0x00
/*PSCDIV reset*/
#define BD38kF		0

/*CCR operations*/
/*external clock divider*/

/*choose one*/
#define CLK1		00
#define CLK16		01 /*normal choice for async*/
#define	CLK32		02
#define CLK64		03

#define RCLKIN		04
/*set for internal rcvr clock*/

#define TCLKO		010
/*select for t clock to be an output, else input*/

#define PSCDIV		020
/*prescalar divider, set for 3
	zero for 2 */

/*ECR operations*/
/*select a value for crc polynomial*/
#define CRCV41		00
#define CRC16		01
#define CRCLRC		02

#define	CRCPRE		04
/* zero to preset CRC generator to 0*/

#define CRCCTL		010

/*parity*/
#define ECRODDPAR		0100
#define ECRPAREN		0200

/*
	end global registers
*/

#define RINTTYPE	(ROVRN|RAB|FRERR|PERR|RDA) /*all rcvr interrupts*/



/*	The following line should be included only for testing */

#ifdef	DEBUG
unsigned char ebug1,cbug1;
char	Debugm050 = 0;	/* 0 == no debug printouts
			 * 1 == debug printouts
			 */
#endif
short m5_scan;		/* permits scan to start in first open*/
#define	m050MAX	2

#define R6CSCAN 2	/* clock ticks for character scan */
#define R6_CHAR 01	/* character arrived flag (m5_work) */
#define R6_ACT	02	/* character in buffer flag (m5_work) */
#define	HI_MARK	18	/* local clist high-water-mark */

char	m5_work[m050MAX];	/* activity info for scanner */



/*
 *	Structure of R68560 hardware registers.
 */
struct m5_io
{
	unsigned char	m5_rsr;		/* rec status reg*/
	unsigned char	dummy1;
	unsigned char	m5_rcr;		/* rec cntl reg*/
	unsigned char	dummy2;
	unsigned char	m5_rdr;		/* rec data reg*/
	unsigned char	dummy3[3];
	unsigned char	m5_rivnr;	/* rec int vect reg*/
	unsigned char	dummy4;	/* fill */
	unsigned char	m5_rier;	/* rec int enab reg*/
	unsigned char	dummy5[5];	/* fill */
  	unsigned char	m5_tsr;		/* tran status reg*/
	unsigned char	dummy6;	/* fill */
  	unsigned char	m5_tcr;		/* tran cntl reg*/
	unsigned char	dummy7;	/* fill */
  	unsigned char	m5_tdr;		/* tran data reg*/
	unsigned char	dummy8[3];	/* fill */
  	unsigned char	m5_tivnr;	/* tran int vect reg*/
	unsigned char	dummy9;	/* fill */
  	unsigned char	m5_tier;	/* tran int enab reg*/
	unsigned char	dummy10[5];	/* fill */
  	unsigned char	m5_sisr;	/* ser interface status reg*/
	unsigned char	dummy11;	/* fill */
  	unsigned char	m5_sicr;	/* ser interface cntl reg*/
	unsigned char	dummy12[5];	/* fill */
  	unsigned char	m5_sivnr;	/* ser interrrupt vect reg*/
	unsigned char	dummy13;	/* fill */
  	unsigned char	m5_sier;	/* ser interrupt enab reg*/
	unsigned char	dummy14[5];	/* fill */
  	unsigned char	m5_psr1;	/*prot select reg1*/
	unsigned char	dummy15;	/* fill */
  	unsigned char	m5_psr2;	/*prot select reg2*/
	unsigned char	dummy16;	/* fill */
  	unsigned char	m5_ar1;		/*address reg1*/
	unsigned char	dummy17;	/* fill */
  	unsigned char	m5_ar2;		/*address reg2*/
	unsigned char	dummy18;	/* fill */
  	unsigned char	m5_brdr1;	/*baud rate div reg1*/
	unsigned char	dummy19;	/* fill */
  	unsigned char	m5_brdr2;	/*baud rate div reg2*/
	unsigned char	dummy20;	/* fill */
  	unsigned char	m5_ccr;		/*clock cntl reg*/
	unsigned char	dummy21;	/* fill */
  	unsigned char	m5_ecr;		/*error cntl reg*/
	unsigned char	dummy22;	/* fill */

};

#define MISSING	((struct m5_io *)0)	/* board not there */
#define COL 8


extern int m5_cnt;			/* number of devices - init in conf.c */
extern int m050_cnt;			/* number of mvme050s -init in conf.c */
extern int m5_ivec[];			/* interrupt vectors, found in conf.c */
extern struct m5_io *m050_addr[]; 	/* from config.c */
struct m5_io *m5_addr[m050MAX];		/* kludge to fool config */
/* this address is also used by the m050 board driver and must be same */
extern struct tty m5_tty[];		/* R68560 tty structure */
extern ttrstrt();

unsigned char m050l_speeds[] = {
/* low byte of baud rate control values */
	 0 /*    0 */,
	 BD50L /*   50 */,
	 BD75L /*   75 */,
	 BD110L /*  110 */,
	 BD135L /*  134 */,
	 BD150L /*  150 */,
	 BD200L /*  200 */,
	 BD300L /*  300 */,
	 BD600L /*  600 */,
	 BD1200L /* 1200 */,
	 BD1800L /* 1800 */,
	BD2400L /* 2400 */,
	BD4800L /* 4800 */,
	BD9600L /* 9600 */,
	BD19kL /* EXTA */,
	BD19kL /* EXTB */
					   /* EXTA & EXTB = 19.2K  */
};
unsigned char m050h_speeds[] = {
/* high byte of baud rate control values */
	 0 /*    0 */,
	 BD50H /*   50 */,
	 BD75H /*   75 */,
	 BD110H /*  110 */,
	 BD135H /*  134 */,
	 BD150H /*  150 */,
	 BD200H /*  200 */,
	 BD300H /*  300 */,
	 BD600H /*  600 */,
	 BD1200H /* 1200 */,
	 BD1800H /* 1800 */,
	BD2400H /* 2400 */,
	BD4800H /* 4800 */,
	BD9600H /* 9600 */,
	BD19kH /* EXTA */,
	BD19kH /* EXTB */
					   /* EXTA & EXTB = 19.2K  */
};
short m050f_speeds[] = {
/* prescalar divide flag for baud rate control values */
	 0 /*    0 */,
	 BD50F /*   50 */,
	 BD75F /*   75 */,
	 BD110F /*  110 */,
	 BD135F /*  134 */,
	 BD150F /*  150 */,
	 BD200F /*  200 */,
	 BD300F /*  300 */,
	 BD600F /*  600 */,
	 BD1200F /* 1200 */,
	 BD1800F /* 1800 */,
	BD2400F /* 2400 */,
	BD4800F /* 4800 */,
	BD9600F /* 9600 */,
	BD19kF /* EXTA */,
	BD19kF /* EXTB */
					   /* EXTA & EXTB = 19.2K  */
};

/* delay table:
   this table used to delay for TX fifo becomes empty, each clock tick is
   equal to 1/60 second --> 16.6 ms, time delay is atleast long enough
   for 8-character time.
*/
char m5_delay[] = {
	00 /*    0 */,
	70 /*   50 */,
	64 /*   75 */,
	50 /*  110 */,
	40 /*  134 */,
	32 /*  150 */,
	24 /*  200 */,
	16 /*  300 */,
	8  /*  600 */,
	4  /* 1200 */,
	3  /* 1800 */,
	2  /* 2400 */,
	1  /* 4800 */,
	1  /* 9600 */,
	1  /* EXTA */,
	1  /* EXTB */
};



/*
*
*	open entry
*
*/

m5open(dev, flag)
unsigned int dev, flag;
{
	register struct tty *tp;
	register struct m5_io *addr;
	int m5proc(),m5scan();
	unsigned int port = minor(dev);
	unsigned sleep();

#ifdef DEBUG
	if (Debugm050 )
		printf("m5open:started\n");
#endif
	/* check for valid dev, and missing device */
	if (port >= m5_cnt || (addr = m5_addr[port]) == MISSING) {
		u.u_error = ENXIO;
		return;
	}
	tp = &m5_tty[port];		/* get tty struct */
	if (! (tp->t_state & (ISOPEN|WOPEN))) {
		ttinit(tp);		/* init tty structure */
		tp->t_proc = m5proc;
		addr->m5_sicr |= RTSLVL|DTRLVL;
		addr->m5_rier |= RINTTYPE;	/* enable Rx interrupt */
		m5param(port);
	}
	if(tp->t_cflag&CLOCAL || m5modem(port,ON)) {
		tp->t_state |= CARR_ON;
		wakeup(( caddr_t)&tp->t_canq);
	}
	else
		tp->t_state &= ~CARR_ON;

	if (!(flag & FNDELAY))
		while (! (tp->t_state & CARR_ON)) {
			tp->t_state |= WOPEN;
			sleep(( caddr_t)&tp->t_canq,TTIPRI);
		}
	(*linesw[tp->t_line].l_open)(tp);
}




/*
*
*	close entry
*
*/

m5close(dev)
unsigned int dev;
{
	register struct tty *tp;
	unsigned int port = minor(dev);
	int cur_lvl;

	tp = &m5_tty[port];
	(*linesw[tp->t_line].l_close)(tp);
	cur_lvl=spl5();
	if ((tp->t_cflag & HUPCL) && !(tp->t_state & ISOPEN))
		delay(m5_delay[tp->t_cflag&CBAUD]); /* delay for fifo empty */
		m5modem(port,OFF);
	splx(cur_lvl);
#ifdef DEBUG
	if (Debugm050)
		printf("m5close: closed dev %d\n", port);
#endif
}




/*
*
*	read entry
*
*/

m5read(dev)
unsigned int dev;
{
	register struct tty *tp;
	unsigned int port = minor(dev);

	tp = &m5_tty[port];
	(*linesw[tp->t_line].l_read)(tp);
}


/*
*
*	write entry
*
*/

m5write(dev)
unsigned int dev;
{
	register struct tty *tp;
	unsigned int port = minor(dev);

	tp = &m5_tty[port];
#ifdef DEBUG
	if (Debugm050)
		printf("m5write: for dev %d\t", port);
#endif
	(*linesw[tp->t_line].l_write)(tp);
}




/*
*
*	ioctl entry
*
*/

m5ioctl(dev, cmd, arg, mode)
unsigned int dev, cmd, arg, mode;
{
	register struct tty *tp;
	unsigned int port = minor(dev);

	tp = &m5_tty[port];
#ifdef DEBUG
	if (Debugm050)
		printf("m5ioctl: calling m5param \n");
#endif
	if (ttiocom(tp, cmd, arg, mode))
		m5param(port);
}




/*
*
*	param entry
*
*/

m5param(dev)
unsigned int dev;
{
	register struct tty	*tp;
	register struct m5_io	*addr;
	register		flags;
	register int indx;
	unsigned char   charsize;

	if(m5_scan==0){
		m5_scan++;
		m5scan();
	}
	addr = m5_addr[dev];
	if (addr == MISSING) {
		u.u_error = ENXIO;
	}
#ifdef DEBUG
	if (Debugm050){
		printf("m5param: started for dev %d.\n",dev);
	}
#endif
	tp = &m5_tty[dev];
	flags = tp->t_cflag;

	/* Reset to make global changes*/
	delay(m5_delay[tp->t_cflag&CBAUD]); /* delay for fifo empty */
	addr->m5_tcr |=  TRES ;		/* disable Tx */
	addr->m5_rcr |=  RRES ;		/* disable Rx */

	if ((flags & CBAUD) == B0)	/* hang up on zero baud rate */
		if(tp->t_state & ISOPEN){
			signal(tp->t_pgrp,SIGHUP);
			ttyflush(tp,(FREAD|FWRITE));
			m5modem(dev, OFF);
		}

	if (flags & CLOCAL)		/* update the DCD status */
		if ((tp->t_state&CARR_ON) == 0) {
			wakeup(&tp->t_canq);
			tp->t_state |= CARR_ON;
		}
	indx=(flags&CBAUD);
	addr->m5_brdr1= m050l_speeds[indx];
	addr->m5_brdr2= m050h_speeds[indx];
	if(m050f_speeds[indx])
		addr->m5_ccr |= PSCDIV;
	else
		addr->m5_ccr &= ~PSCDIV;

	switch( flags & CSIZE) {
		case CS8:	charsize = CL8; break;
		case CS7:	charsize = CL7; break;
		case CS6:	charsize = CL6; break;
		case CS5:	charsize = CL5; break;
	}
	addr->m5_psr2 &= ~CLB;		/* clear the preivous setting */
	addr->m5_psr2 |= charsize;

	addr->m5_psr2 &= ~SBB;
	if ( flags & CSTOPB)
		addr->m5_psr2 |= SB2;
	else
		addr->m5_psr2 |= SB1;

	if (flags & PARENB) {			/* set up parity */
		addr->m5_ecr |= ECRPAREN;
		if (flags & PARODD)
			addr->m5_ecr |= ECRODDPAR;
		else
			addr->m5_ecr &= ~ECRODDPAR;
	} else
		addr->m5_ecr &= ~ECRPAREN;

	if ( flags & CREAD ) 			/* enable receiver? */
		addr->m5_rcr &= ~RRES;

	/* re-enable the transmitter */
	addr->m5_tcr &= ~TRES;
}



/*
*
*	proc entry
*
*/

m5proc(tp, cmd)
register struct tty *tp;
unsigned int cmd;
{
	register struct m5_io	*addr;
	int			dev;
	register int piplev;

#ifdef DEBUG
	if (Debugm050){
		printf("m5proc:started.\n");
	}
#endif
	piplev = spl6();
	dev = tp - m5_tty;
	addr = m5_addr[dev];

	switch (cmd) {

	case T_TIME:		/* resuming output after delay */
		tp->t_state &= ~TIMEOUT;
		addr->m5_tcr &= ~TICS; /* end break*/;
#ifdef DEBUG
		if (Debugm050){
			printf("m5proc:T_TIME.\n");
		}
#endif
		goto start;

	case T_WFLUSH:		/* flush output queue */
		tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
		tp->t_tbuf.c_count = 0;
#ifdef DEBUG
		if (Debugm050){
			printf("m5proc:T_WFLUSH.\n");
		}
#endif
		/* fall thru */

	case T_RESUME:		/* resume output, XON has been received */
		tp->t_state &= ~TTSTOP;
		addr->m5_tcr |= TEN;		/* reanable transmitter */
		if(!(addr->m5_tsr & TDRA))	/* transmitter buffer available ? */
			break;
#ifdef DEBUG
		if (Debugm050){
			printf("m5proc:T_RESUME.\n");
		}
#endif
		/* fall thru */

	case T_OUTPUT:		/* handle first or next char */
	start: {
		register struct ccblock *tbuf;

#ifdef DEBUG
		if (Debugm050){
			printf("m5proc:T_OUTPUT started.\n");
		}
#endif
		if (tp->t_state & (BUSY|TTSTOP))
			break;
		tbuf = &tp->t_tbuf;
		if (tbuf->c_ptr == NULL || tbuf->c_count == 0) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp))) 
				break;
		}
		tp->t_state |= BUSY;			/* send char */
		addr->m5_tdr = *tp->t_tbuf.c_ptr++; /* put char to trans data reg*/
		tp->t_tbuf.c_count--;	/* update count */
		addr->m5_tier |= TDRA;
		break;
	}

	case T_SUSPEND:	   /* stop output, XOFF has been received */
		/* There may be 9 pending characters in the r68560 transmitter
		   (8 in FIFO + 1 in shift register), therefore disabling the
		   transmitter is the appropriate way to stop the transmission
		   as soon as posible to prevent overun condition on the other
		   side.
		*/
		addr->m5_tcr &= ~TEN;		/* disable transmitter */
		tp->t_state |= TTSTOP;
#ifdef DEBUG
		if (Debugm050){
			printf("m5proc:T_SUSPEND.\n");
		}
#endif
		break;

	case T_BLOCK:		/* send XOFF */
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK;
#ifdef DEBUG
		if (Debugm050){
			printf("m5proc:T_BLOCK .\n");
		}
#endif
		if (tp->t_state&BUSY)	/* If BUSY, tell intr() to send CSTOP */
			tp->t_state |= TTXOFF;
		else
			addr->m5_tdr = CSTOP;
		break;

	case T_RFLUSH:	/* flush input queue.  if input blocked, send XON */
		if (!(tp->t_state & TBLOCK))
			break;
		/* fall thru */

	case T_UNBLOCK:		/* send XON */
		tp->t_state &= ~(TTXOFF | TBLOCK);
		if (tp->t_state&BUSY) /* If BUSY, tell intr() to send CSTART */
			tp->t_state |= TTXON;
		else
			addr->m5_tdr = CSTART;
		break;

	case T_BREAK:		/* send break condition for 1/4 second */
		addr->m5_tcr |= TICS;		/* send a break*/;
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, HZ/4);
		break;

	case T_PARM:	/* used by sxt driver in R5.2 */
		m5param(dev);
		break;
	}
	splx(piplev);
}




/*
*
*	init entry
*		On the exit of this routine, all devices are initialized to
*		following conditions:
*			1) Async mode, 9600 baud, 8-bit char, and 1 stop-bit.
*			2) Both Transmitter and Receiver are enable, but thier
*			   interrupts are disable in order to support polling
*			   mode when the device is used as a system console
*			   during debugging sessions. Also note that most
*			   debuggers use polling mode to display on the console.
*			3) Both DTR and RTS are asserted to allow polling.
*			4) No DCD interrupt.
*
*/

m5init()
{
	register unsigned int dev;
	register struct m5_io *addr;
	int i, port, cur_lvl;

/* two devices per mvme050 */

	cur_lvl=spl5();
	for (i=0; i<m050_cnt; i++) {
		dev = 2*i;
		if ((m050_addr[i] == 0) ||	/* no mvme050? */
		   (bprobe(&((struct m5_io *)m050_addr[i])->m5_rsr,0))) {
			m5_addr[dev] = MISSING;
			m5_addr[dev+1] = MISSING;
#ifdef DEBUG
		if (Debugm050) {
		   printf("m5init:configured with m050_cnt= %d\n", m050_cnt);
		   printf("m5init: missing mvme050 board %d \n", i);
		}
#endif
			continue;
	        }

		/* card exists, init both ports */

		m5_addr[dev] = m050_addr[i];
		m5_addr[dev+1] = m050_addr[i] + PORTOFFSET;
		for(port=0; port<2; port++){
			addr = m5_addr[dev + port];
			/* reset Tx and Rx sections, required by r68560 prior
			   to setting up its global parameters */
			addr->m5_tcr = TRES|TEN;
			addr->m5_rcr = RRES;

	 		addr->m5_psr2 = ASYNC|CL8|SB1;
	 		addr->m5_psr1 = 0;
	 		addr->m5_ar1  = 0;
	 		addr->m5_ar2  = 0;
	 		addr->m5_brdr1= BD9600L;
	 		addr->m5_brdr2= BD9600H;
	 		addr->m5_ccr  = CLK16|RCLKIN|TCLKO|PSCDIV;
	 		addr->m5_ecr  = 0;
	 		addr->m5_rier = 0;	/* no Rx interrupt yet */
	 		addr->m5_tier = 0;	/* no Tx interrupt yet */
	 		addr->m5_sier = 0;	/* no DCD interrupt yet */
	 		addr->m5_sicr = DTRLVL|RTSLVL;
			addr->m5_rivnr= addr->m5_tivnr =
	 		addr->m5_sivnr=(unsigned char)((m5_ivec[dev + port])>>2)
					+ dev;
			addr->m5_tcr &= ~TRES;	/* enable Transmitter */
			addr->m5_rcr &= ~RRES;	/* enable Receiver */

#ifdef DEBUG
    	if (Debugm050){
	printf("rsr at %x value= %x\t",&(m5_addr[dev + port]->m5_rsr),m5_addr[dev + port]->m5_rsr);
	printf("rcr at %x value= %x\n",&(m5_addr[dev + port]->m5_rcr),m5_addr[dev + port]->m5_rcr);
	printf("rdr at %x value= %x\t",&(m5_addr[dev + port]->m5_rdr),m5_addr[dev + port]->m5_rdr);
	printf("rivnr at %x value= %x\n",&(m5_addr[dev + port]->m5_rivnr),m5_addr[dev + port]->m5_rivnr);
	printf("rier at %x value= %x\t",&(m5_addr[dev + port]->m5_rier),m5_addr[dev + port]->m5_rier);
	printf("tsr at %x value= %x\n",&(m5_addr[dev + port]->m5_tsr),m5_addr[dev + port]->m5_tsr);
	printf("tcr at %x value= %x\t",&(m5_addr[dev + port]->m5_tcr),m5_addr[dev + port]->m5_tcr);
	printf("tdr at %x value= %x\n",&(m5_addr[dev + port]->m5_tdr),m5_addr[dev + port]->m5_tdr);
	printf("tivnr at %x value= %x\t",&(m5_addr[dev + port]->m5_tivnr),m5_addr[dev + port]->m5_tivnr);
	printf("tier at %x value= %x\n",&(m5_addr[dev + port]->m5_tier),m5_addr[dev + port]->m5_tier);
	printf("sisr at %x value= %x\t",&(m5_addr[dev + port]->m5_sisr),m5_addr[dev + port]->m5_sisr);
	printf("sicr at %x value= %x\n",&(m5_addr[dev + port]->m5_sicr),m5_addr[dev + port]->m5_sicr);
	printf("sivnr at %x value= %x\t",&(m5_addr[dev + port]->m5_sivnr),m5_addr[dev + port]->m5_sivnr);
	printf("sier at %x value= %x\n",&(m5_addr[dev + port]->m5_sier),m5_addr[dev + port]->m5_sier);
	printf("psr1 at %x value= %x\t",&(m5_addr[dev + port]->m5_psr1),m5_addr[dev + port]->m5_psr1);
	printf("psr2 at %x value= %x\n",&(m5_addr[dev + port]->m5_psr2),m5_addr[dev + port]->m5_psr2);
	printf("ar1 at %x value= %x\t",&(m5_addr[dev + port]->m5_ar1),m5_addr[dev + port]->m5_ar1);
	printf("ar2 at %x value= %x\n",&(m5_addr[dev + port]->m5_ar2),m5_addr[dev + port]->m5_ar2);
	printf("brdr1 at %x value= %x\t",&(m5_addr[dev + port]->m5_brdr1),m5_addr[dev + port]->m5_brdr1);
	printf("brdr2 at %x value= %x\n",&(m5_addr[dev + port]->m5_brdr2),m5_addr[dev + port]->m5_brdr2);
	printf("ccr at %x value= %x\t",&(m5_addr[dev + port]->m5_ccr),m5_addr[dev + port]->m5_ccr);
	printf("ecr at %x value= %x\n",&(m5_addr[dev + port]->m5_ecr),m5_addr[dev + port]->m5_ecr);
	}
#endif
		}
		
        }
	splx(cur_lvl);
}



/*
*
*	interrupt entry
*
*/

m5intr(dev)
unsigned int dev;
{
	register struct tty	*tp;
	register struct m5_io	*addr;
	register unsigned char	c, ctmp;
	register int		flg, count;
	char			lbuf[3];
	int			lcnt;
	unsigned int		rdat;
	int cur_lvl;

	if ((addr = m5_addr[dev]) == MISSING)
		return;
	tp= &m5_tty[dev];

	/* Serial Interface Interrupt */
	/* Note : r68560 only generates the interrupt when DCD
		  changing from active to inactive. */
	if(addr->m5_sisr & DCD) {
		if(!(addr->m5_sisr & DCDLVL)) {
			if (tp->t_state&CARR_ON) {
				if (tp->t_state&ISOPEN) {
				    signal(tp->t_pgrp, SIGHUP);
				    addr->m5_sicr &= ~(DTRLVL|RTSLVL);
				    ttyflush(tp, (FREAD|FWRITE));
				}
				tp->t_state &= ~CARR_ON;
			}
		}
		addr->m5_sisr = DCD;	/* clear DCD sense */
	}

	/* XMT interrupt handler */
	if((addr->m5_tier & TDRA) && (addr->m5_tsr & TDRA))
	{
		sysinfo.xmtint++;
		if(tp->t_state & TTXOFF){
			addr->m5_tdr= CSTOP;
			tp->t_state &= ~TTXOFF;
		} else if(tp->t_state & TTXON){
			addr->m5_tdr= CSTART;
			tp->t_state &= ~TTXON;
		} else {
			addr->m5_tier &= ~TDRA; /* disable Tx intr, m5proc */
			tp->t_state &= ~BUSY;	/* will re-anable if there */
			m5proc(tp,T_OUTPUT);	/* is a character to send. */
		}
		addr->m5_tsr |= TDRA;		/* reset the status bit */
	}

	/* RCV interrupt handler */
	if(rdat = addr->m5_rsr & RINTTYPE) 	/*receive interrupts*/
	{
		c = addr->m5_rdr;		/* get character from device */
		lcnt = 1;
		addr->m5_rsr |= RINTTYPE;	/* clear all status bits */
		flg = tp->t_iflag;
		if(rdat == RDA){		/* good data available */
			sysinfo.rcvint++;
			if(flg & IXON){
				ctmp= c & 0x7f;
				if(tp->t_state & TTSTOP){
					if((ctmp==CSTART) || 
					(flg & IXANY))
						m5proc(tp,T_RESUME);
				} else {
					if(ctmp == CSTOP)
						m5proc(tp,T_SUSPEND);
				  }
				if(ctmp == CSTART || ctmp == CSTOP)
					return; /* ignore these characters */
			}
			if(flg & ISTRIP)
				c &= 0x7f;
			else
			    	if(c == 0xff && flg & PARMRK){
					lbuf[1]= 0xff;
					lcnt= 2;
			    	}
		}
		else {		/* RX error handler */
			if(rdat&RAB){	/* if break detected */
				if(!(flg&IGNBRK) && (flg&BRKINT))
					(*linesw[tp->t_line].l_input)
						        (tp, L_BREAK);
				return;	/* ignore the current character */
			}
			if (!(flg&IGNPAR)) {	/* no checking at all */
				if (!(flg&INPCK))	/* no parity check */
					rdat &= ~PERR;
				if (rdat & (PERR|ROVRN)) {
					if (flg & PARMRK) {
						lbuf[2] = 0xFF;
						lbuf[1] = 0;
						lcnt = 3;
						sysinfo.rawch += 2;
					} else
						c = 0;
				}
			}
		}
#ifdef DEBUG
		if (Debugm050 && rdat&ROVRN)
			printf("m5intr: rx overrun on dev %d\n", tp-m5_tty);
#endif
		/* stash character in r_buf */
		lbuf[0] = c;
		if (tp->t_rbuf.c_ptr != NULL) {
			while (lcnt) {
				cur_lvl = spl6();   /*to block m5scan() from
						    running at clock priority*/
				*tp->t_rbuf.c_ptr++ = lbuf[--lcnt];
				if ((count = --tp->t_rbuf.c_count) < HI_MARK) {
					if(!(tp->t_state & TBLOCK))
					    m5proc(tp, T_BLOCK);
					if(!(addr->m5_rsr & RDA) || count <= 1) {
			    			m5_work[dev] = 0;
			    			tp->t_rbuf.c_ptr -= tp->t_rbuf.c_size - count;
			    			(*linesw[tp->t_line].l_input)(tp,L_BUF);
						if(tp->t_rawq.c_cc <= TTXOHI)
							m5proc(tp, T_UNBLOCK);
					}
				}
			    	m5_work[dev] = R6_ACT;
			    	/* note R6_CHAR bit cleared */
				splx(cur_lvl);
			} 
		} 
	}
}




/*
 *
 *	scan entry
 *
 */

m5scan()
{
	register struct tty	*tp;
	register int		dev;
	register char		*workp;
	register struct m5_io	*addr;
	static	scancnt;

	for (dev = 0, workp = m5_work; dev < m5_cnt; dev++, workp++) {
		if (*workp & R6_CHAR) {
			tp = &m5_tty[dev];
			*workp = 0;
			if (tp->t_rbuf.c_ptr) {
				tp->t_rbuf.c_ptr -= tp->t_rbuf.c_size
						- tp->t_rbuf.c_count;
				(*linesw[tp->t_line].l_input)(tp, L_BUF);
			}
		}
		if (*workp & R6_ACT)
			*workp |= R6_CHAR;
	}

	/* r68560 does not allow interrupt when DCD changes from inactive state
           to active state, therefore scanning is needed to wakeup whoever is
	   openning the line, and waiting for DCD to become active. */

	if (!(scancnt++ % M5MSCAN)) {
		for (dev = 0; dev < m5_cnt; dev++) {
			tp = &m5_tty[dev];
			if(!(tp->t_state & (ISOPEN|WOPEN)))
				continue;
			addr = m5_addr[dev];
			if(tp->t_cflag&CLOCAL || addr->m5_sisr&DCDLVL) {
				if ((tp->t_state&CARR_ON) == 0) {
					wakeup(&tp->t_canq);
					tp->t_state |= CARR_ON;
				}
			}
		}
	}
	timeout(m5scan, (struct tty *) 0, R6CSCAN);
}



/*
*
*	modem entry
*
*/

m5modem(dev,flag)
register int dev;
int flag;
{
	register struct m5_io   *addr;
	int cur_lvl;
	register struct tty	*tp;
	int s;

	tp = &m5_tty[dev];
	addr = m5_addr[dev];
	cur_lvl=spl5();
	if(flag==OFF){
		addr->m5_sier &= ~DCD;		/* disable DCD interrupt */
		addr->m5_sicr &= ~(DTRLVL|RTSLVL);
		if(!(tp->t_cflag & CLOCAL))
		{
			if(tp->t_state & CARR_ON){
				if(tp->t_state & ISOPEN){
					signal(tp->t_pgrp,SIGHUP);
					ttyflush(tp,(FREAD|FWRITE));
				}
				tp->t_state &= ~CARR_ON;
			}
		}
		else if(!(tp->t_state & CARR_ON)){
			wakeup(&tp->t_canq);
			tp->t_state |= CARR_ON;
		}
	} else {
		addr->m5_sicr |= DTRLVL|RTSLVL; /* assert DTR and RTS */
		addr->m5_sier |= DCD;		/* enable DCD interrupt */
	  }

	s = addr->m5_sisr & DCDLVL;
#ifdef DEBUG
	if (Debugm050)
		printf("m5modem: DCDLVL = %d\n", s);
#endif
	splx(cur_lvl);
	return( s );
}

