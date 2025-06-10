/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)mk68564.c	6.22 MODIFIED		*/
/* Fixed parity selection... was selecting odd for even.. psr 2/28/86 */
/*
 *	Driver for R68564 SIO RS-232C Serial Port Module
 *		on the  or MVME130 board. The current maximum 
 *		number of devices is set for 2.
 *
 */

typedef unsigned char UCHAR;

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/file.h"
#include "sys/signal.h"
#include "sys/tty.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/mk68564.h"

/*
 *	Structure of MK68564 hardware registers.
 */
struct m564_io
{
	unsigned char	cmdreg;	/* Command Register */
	unsigned char	modectl;	/* Mode Control Register */
	unsigned char	intctl;	/* Interrupt Control Register */
	unsigned char 	sync1;	/* Sync Word Register 1 */
	unsigned char	sync2;	/* Sync Word Register 2 */
	unsigned char	rcvctl;	/* Receiver Control Register */
	unsigned char 	xmtctl;	/* Transmitter Control Register */
	unsigned char 	stat0;	/* Status Register 0 */
	unsigned char	stat1;	/* Status Register 1 */
	unsigned char	datarg;	/* Data Register */
	unsigned char	tcreg;	/* Time Constant Register */
	unsigned char	brgctl;	/* Baud Rate Generator Control Reg. */
	unsigned char	vectrg;	/* Interrupt Vector Register */
	unsigned char	fill[3];	/* Not used */

};
/*
	This structure holds the current settings of the
	mk68564 and will be restored when a CONC_OFF
	is received
*/
struct m564save 
{
	unsigned char	intctl;	/* Interrupt Control Register */
	unsigned char	rcvctl;	/* Receiver Control Register */
	unsigned char 	xmtctl;	/* Transmitter Control Register */
	unsigned char	vectrg;	/* Interrupt Vector Register */
} current;

/* Command Register */
#define LOOP		0x1		/* Loop Mode 	  	   */
#define NULL0		0		/* null code 		   */
#define SENDABT		0x8		/* send abort (SDLC) 	   */
#define RESINT		0x10		/* reset ext/status interrupts 	   */
#define RESCHNL		0x18		/* channel reset 	   */
#define NXTCHINT	0x20		/* enable int on next Rx character */
#define RESTXP		0x28		/* reset Tx int pending	   */
#define ERRRES		0x30		/* error reset 		   */
#define NULL1		0x38		/* null code 		   */
#define RESRXCRC	0x40		/* reset Rx CRC checker	   */
#define RESTXCRC	0x80		/* reset Tx CRC checker	   */
#define RESTXUN		0xc0		/* reset Tx underrun/eom latch 	   */


/* Mode Control Register */
#define ENPAR		0x1		/* enable parity 	   */
#define EVENPAR		0x2		/* parity - 1=even 0=odd   */
#define SYNCEN		0		/* sync modes enable	   */
#define ST1BIT		0x4		/* 1 stop bit/character	   */
#define ST1PBIT		0x8		/* 1 1/2 stop bits/character 	   */
#define ST2BIT		0xc		/* 2 stop bits/character   */
#define STBMASK		(ST1BIT|ST1PBIT|ST2BIT)
#define SY8BIT		0		/* 8 bit sync character	   */
#define SY16BIT		0x10		/* 16 bit sync character   */
#define SDLCM		0x20		/* SDLC mode		   */
#define SYEXT		0x30		/* external sync mode	   */
#define X1CLK		0		/* x1 clock mode	   */
#define X16CLK		0x40		/* x16 clock mode	   */
#define X32CLK		0x80		/* x32 clock mode	   */
#define X64CLK		0xc0		/* x64 clock mode	   */

/* Interrupt Control Register */
#define EXINTEN		0x1		/* ext int enable 	   */
#define TXINTEN		0x2		/* Tx int enable 	   */
#define STAFFV		0x4		/* status affects vector   */
#define RXDIS		0		/* Rx int disable	   */
#define RX1ST		0x8		/* Rx int first character  */
#define RXALLP		0x10		/* int on all Rx chars (parity)    */
#define RXALLNOP	0x18		/* " (parity doesn't affect vector)*/
#define RXINTMASK	(RXDIS|RX1ST|RXALLP|RXALLNOP)
#define RXRDYEN		0x20		/* Rx ready enable (for DMA)  */
#define TXRDYEN		0x40		/* Tx ready enable (for DMA)  */
#define CRC16		0x80		/* CRC-16 sdlc-crc polynomial */

/* Receiver Control Register */
#define RXENAB		0x1		/* Rx enable		   */
#define SYCHL		0x2		/* sync character load inhibit     */
#define ADDSRCH		0x4		/* address search mode (sdlc)	   */
#define RXCRCEN		0x8		/* Rx CRC enable	   */
#define HUNTM		0x10		/* enter hunt mode (read as 0)	   */
#define RXAUTO		0x20		/* Rx auto enable	   */
#define RX5BIT		0		/* Rx 5 bits/character	   */
#define RX6BIT		0x40		/* Rx 6 bits/character	   */
#define RX7BIT		0x80		/* Rx 7 bits/character	   */
#define RX8BIT		0xc0		/* Rx 8 bits/character	   */
#define RXMASK		(RX5BIT|RX6BIT|RX7BIT|RX8BIT)

/*Transmitter Control Register */
#define TXENAB		0x1		/* Tx enable		   */
#define RTS		0x2		/* RTS			   */
#define DTR		0x4		/* DTR			   */
#define TXCRCEN		0x8		/* Tx CRC enable	   */
#define SENDBR		0x10		/* send break		   */
#define TXAUTOEN	0x20		/* Tx auto enable	   */
#define TX5BIT		0		/* Tx 5 bits(or less)/character	   */
#define TX6BIT		0x40		/* Tx 6 bits/character	   */
#define TX7BIT		0x80		/* Tx 7 bits/character	   */
#define TX8BIT		0xc0		/* Tx 8 bits/character	   */
#define TXMASK		(TX5BIT|TX6BIT|TX7BIT|TX8BIT)

/* Status Register 0  (read only)  */
#define RXAVAIL		0x1		/* Rx character available  */
#define INTPEND		0x2		/* interrupt pending	   */
#define TXEMPTY		0x4		/* Tx buffer empty	   */
#define DCD		0x8		/* DCD			   */
#define HUNTSYNC	0x10		/* hunt/sync mode	   */
#define CTS		0x20		/* CTS			   */
#define TXUNEOM		0x40		/* Tx underrun/eom	   */
#define BRABRT		0x80		/* break/abort		   */

/* Status Register 1  (read only)  */
#define ALLSENT		0x1		/* all sent		   */
#define PARERR		0x10		/* parity error		   */
#define RXOVER		0x20		/* Rx overrun error	   */
#define FRAM		0x40		/* CRC/framing error	   */
#define EOFRAM		0x80		/* end of frame (sdlc)	   */

/* Baud Rate Generator Control Register */
#define BRGEN		0x1		/* BRG enable		   */
#define DIV64		0x2		/* divide by 64		   */
#define DIV4		0		/* divide by 4		   */
#define TXCLK		0x4		/* Tx internal		   */
#define RXCLK		0x8		/* Rx internal		   */

/* Interrupt Vector Register */
#define PORTMASK	0x4		/* source port of interrupt */
#define TXMTINT		0x0		/* transmit buffer emptry  */
#define EXSTINT		0x1		/* external/status interrupt */
#define RXCHINT		0x2		/* receive character interrupt */
#define SRCINT		0x3		/* special receive condition int. */
#define INTTYPE		(TXMTINT|EXSTINT|RXCHINT|SRCINT)
#define PORTA		0x4
#define PORTB		0x0

/*	The following line should be included only for testing */

unsigned char ebug1,cbug1;
char	Debugm564 = 0;	/* 0 == no debug printouts */
char	Debug1 = 0;	 /* 1 == debug printouts */
char	Debug2 = 0;	 /* 1 == debug printouts */
			 

#ifdef DEBUG1
static int bmsg = 0;
static char msg0[]="No break processing detected\n";
static char msg1[]="Rx error reported by the hardware\n";
static char msg2[]="RX error handled\n";
static char msg3[]="Break/abort processing occurred\n";
static char msg4[]="DCD processing occurred\n";
static char *msgindx[]={msg0,msg1,msg2,msg3,msg4};
#endif

short m564_scan = 0;	/* permits scan to start in first open*/

#define OFF 	0
#define ON	1
#define CR	0x0d
#define LF	0x0a
#define PLUS	0x2B
#define LCP_MODEM	(((LF << 16)|(CR << 8)|PLUS) & MMASK)
#define MMASK 0x00ffffff
#define TRUE (-1)
#define FALSE (NULL)
#define MODEM_QUIT 0x11 /* modem specific sequence to stop modem */
#define R6CSCAN 2	/* clock ticks for character scan */
#define R6_CHAR 01	/* character arrived flag (m5_work) */
#define R6_ACT	02	/* character in buffer flag (m5_work) */

#define	HI_MARK	18	/* clist high-water-mark */

long lcp_char = LCP_MODEM; /* modem is active sequence */

/* baud rate time constants  -  assume using divide by 4 and
   times 16 (sampling rate) with clock of 2.4576 MHz */

#define BD150	0	/* effectively 256 */
#define BD200	192
#define BD300	128
#define BD600	64
#define BD1200	32
#define	BD1800	21
#define BD2400	16
#define BD4800	8
#define BD9600	4
#define BD19k	2
#define BD38k	1

unsigned char speeds[] = {
/* baud rate control values */
	0 	/*    0 */,
	0	/*   50 */,
	0	/*   75 */,
	0	/*  110 */,
	0	/*  134 */,
	BD150	/*  150 */,
	BD200	/*  200 */,
	BD300 	/*  300 */,
	BD600 	/*  600 */,
	BD1200 	/* 1200 */,
	BD1800	/* 1800 */,
	BD2400  /* 2400 */,
	BD4800  /* 4800 */,
	BD9600  /* 9600 */,
	BD19k   /* EXTA */,
	BD38k   /* EXTB */
   /* EXTA = 19.2K  */
   /* EXTB = 38.4K  */
};

/* delay table:
   this table used to delay for TX fifo becomes empty, each clock tick is
   equal to 1/60 second --> 16.6 ms, time delay is atleast long enough
   for 2-character time.
*/
char m564_delay[] = {
	00 /*    0 */,
	20 /*   50 */,
	16 /*   75 */,
	9  /*  110 */,
	8  /*  134 */,
	8  /*  150 */,
	5  /*  200 */,
	4  /*  300 */,
	2  /*  600 */,
	1  /* 1200 */,
	1  /* 1800 */,
	1  /* 2400 */,
	1  /* 4800 */,
	1  /* 9600 */,
	1  /* EXTA */,
	1  /* EXTB */
};

char	M564con;	/* unique identifier so this putchar can be linked */

#define m564MAX	2

extern int m564_cnt;			/* number of devices - init in conf.c */
extern struct m564_io *m564_addr[];     /* from config.c*/
#define BOARD(x)	(x>>1)
#define PORT1(x)	(x&1)

extern int m564_ivec;		/* interrupt vector for m68564 - from config */
extern struct tty m564_tty[];		/* MK68564 tty structure */
extern ttrstrt();


char	m564_work[m564MAX];	/* activity info for scanner */

short brkflg564[m564MAX];	/* maintain levels to detect which direction */

UCHAR dcdflg564[m564MAX] = 0;	/* of transition occurred		     */
UCHAR dcdscan564[m564MAX] = 0;

UCHAR ctsflg564[m564MAX] = 0;	/* CTS maintain level of transition    */
UCHAR ctsscan564[m564MAX] = 0;

#define MAXDCDTIME 9

UCHAR wclose[m564MAX]; /* waiting for close flag */

/*
	indicates whether some of the 564
	registers have been saved for dual console
	operation
*/
int saved = 0;	


/*
*
*	open entry
*
*/

m564open(dev, flag)
register unsigned int dev, flag;
{
	register struct tty *tp;
	register struct m564_io *m564;
	int lvl;

	int m564proc(),m564scan();
	unsigned sleep();

	dev = minor(dev);
#ifdef DEBUG
	if (Debugm564 )
		printf("m564open:started\n");
#endif
	if (dev >= m564_cnt) {		/* check for valid dev */
		u.u_error = ENXIO;
		return;
	}

	tp = &m564_tty[dev];		/* get tty struct */

	dc_oslp(dev, tp); 		/* sleep if in dual console */

	m564 = m564_addr[BOARD(dev)];

	if(PORT1(dev))
		m564++;

	/* prevent all opens from completeing, until the close is finished */
	if(wclose[dev])
	{
		lvl = spltty();
		while(wclose[dev])
		{
			if(sleep((caddr_t)&tp->t_canq, (TTIPRI|PCATCH)))
			{
				splx(lvl);
				return;
			}
		}
		splx(lvl);
	}
	if (! (tp->t_state & (ISOPEN|WOPEN))) {
		ttinit(tp);		/* init tty structure */
		tp->t_proc = m564proc;
		m564->intctl &= ~RXINTMASK; /* turnoff old Rx int mode */
		m564->intctl |= (TXINTEN|RXALLP|EXINTEN); /* set desired Rx/Tx int */
		brkflg564[dev] = 0;
		dcdflg564[dev] = (m564->stat0&DCD);
		m564param(dev);
	}
	if(tp->t_cflag&CLOCAL || m564modem(dev,ON))
	{
		tp->t_state |= CARR_ON;
		wakeup((caddr_t)&tp->t_canq);
	}
	else
		tp->t_state &= ~CARR_ON;

	if (!(flag & FNDELAY))
		while (! (tp->t_state & CARR_ON)) {
			tp->t_state |= WOPEN;
			sleep(( caddr_t)&tp->t_canq,TTIPRI);
		}

	(*linesw[tp->t_line].l_open)(tp);
	if(m564_scan==0){
		m564_scan++;
		m564scan();
	}
}


/*
*
*	close entry
*
*/

m564close(dev)
unsigned int dev;
{
	register struct tty *tp;
	int cur_lvl;

	dev = minor(dev);
	tp = &m564_tty[dev];

	cur_lvl=spl5();

	/* prevent opens from completing while closing */
	wclose[dev] = TRUE;	

	splx(cur_lvl);

	cur_lvl=spl5();
	(*linesw[tp->t_line].l_close)(tp);
	if ((tp->t_cflag & HUPCL) && !(tp->t_state & ISOPEN))
		m564modem(dev,OFF);
	splx(cur_lvl);
#ifdef DEBUG
	if (Debugm564)
		printf("m564close: closed dev %d\n", dev);
#endif
#ifdef DEBUG1
	if(bmsg>=0 && Debugm564){
		printf("%s",msgindx[bmsg]);
		bmsg= -1;
	}
#endif
	cur_lvl = spltty();

	wclose[dev] = FALSE;

	splx(cur_lvl);

	wakeup((caddr_t)&tp->t_canq);
}


/*
*
*	read entry
*
*/

m564read(dev)
unsigned int dev;
{
	register struct tty *tp;

	dev = minor(dev);
	tp = &m564_tty[dev];
	(*linesw[tp->t_line].l_read)(tp);
}


/*
*
*	write entry
*
*/

m564write(dev)
unsigned int dev;
{
	register struct tty *tp;

	dev = minor(dev);
	tp = &m564_tty[dev];
#ifdef DEBUG
	if (Debugm564)
		printf("m564write: for dev %d\t", dev);
#endif
	(*linesw[tp->t_line].l_write)(tp);
}


/*
*
*	ioctl entry
*
*/

m564ioctl(dev, cmd, arg, mode)
register unsigned int dev, cmd, arg, mode;
{
	register struct tty *tp;
	register struct m564_io	*m564;

	dev = minor(dev);
	tp = &m564_tty[dev];
	m564 = m564_addr[BOARD(dev)];
#ifdef DEBUG
	if (Debugm564)
		printf("m564ioctl: calling m564param \n");
#endif
	if(!dc_ioctl(cmd, arg, dev, tp, m564))
		switch(cmd)
		{
			default:
				if (ttiocom(tp, cmd, arg, mode))
					m564param(dev);
		}
}


/*
*
*	param entry
*
*/

m564param(dev)
unsigned int dev;
{
	register struct tty	*tp;
	register struct m564_io	*m564;
	register ushort		flags;
	register unsigned char  mode, baudrate, charsize;
	register unsigned char	rx_charsize, tx_charsize;

#ifdef DEBUG
	if (Debugm564){
		printf("m564param: started for dev %d.\n",dev);
	}
	/* Note: Since 020Bug uses the same chip port as the console,
        	 during DEBUGging,  a breakpoint can't be set in the
		 block of code between the Rx disable and the Rx
		 re-enable.
	*/
#endif

	m564 = m564_addr[BOARD(dev)];
	if( PORT1(dev) )
		m564++;
	tp = &m564_tty[dev];
	flags = tp->t_cflag;

	if ((flags & CBAUD) == B0)	/* hang up on zero baud rate */
		if(tp->t_state & ISOPEN){
			signal(tp->t_pgrp,SIGHUP);
			ttyflush(tp,(FREAD|FWRITE));
			if(dev != 0)
				m564modem(dev,OFF);
		}

	if (flags & CLOCAL)		/* update the DCD status */
		if ((tp->t_state & CARR_ON) == 0) {
			wakeup(&tp->t_canq);
			tp->t_state |= CARR_ON;
		}

	baudrate = speeds[flags & CBAUD];

	/*  This section modifies the parity checking, number of stop bits
    	    and receiver enable if necessary.
	*/
	mode = m564->modectl & ~STBMASK;	/* get previous setting */
	if (flags & CSTOPB)			/* set # stop bits	  */
		mode |= ST2BIT;
	else
		mode |= ST1BIT;

	if (flags & PARENB) {			/* set up parity */
		mode |= ENPAR;
		if (flags & PARODD)
			mode &= ~EVENPAR;
		else
			mode |= EVENPAR;
	} else
		mode &= ~ENPAR;


	/* RX bit patterns and TX bit patterns are the same */

	switch( flags & CSIZE ) {
	case CS5:	charsize = RX5BIT;	break;
	case CS6:	charsize = RX6BIT;	break;
	case CS7:	charsize = RX7BIT;	break;
	case CS8:	charsize = RX8BIT;	break;
	}

	rx_charsize = (m564->rcvctl & ~RXMASK) | charsize;
	tx_charsize = (m564->xmtctl & ~TXMASK) | charsize;

	/*
	 * Check for any change.
	 */
	if (m564->tcreg != baudrate || m564->modectl != mode ||
	    m564->rcvctl != rx_charsize || m564->xmtctl != tx_charsize) {
		 /*
		  * We need to delay here until all characters
		  * in the transmitter FIFO
		  * are completely sent	out before disabling the transmitter.
		  * Otherwise
		  * the characters will be flushed out when reenabling
		  * the transmitter.
		  */
		if (!(m564->stat1 & ALLSENT)) {	/* any character in fifo ? */
			/*
			 * Set TTSTOP bit to stop the interrupt
			 * handler putting any character while we are
			 * configuring the SIO.
			 */
			tp->t_state |= TTSTOP;
			delay(m564_delay[flags & CBAUD]);
		}
		/*
		 * Disable external/status interrupt so that we don't receive
		 * unexpected interrupt which is caused by changing baud-rate
		 * and character size.
		 */
		m564->intctl &= ~EXINTEN;
		m564->rcvctl &= ~RXENAB;	/* disable receiver */
		m564->xmtctl &= ~TXENAB;	/* disable Tx */

		m564->brgctl &= ~BRGEN;	/* disable baud rate generator */
		m564->tcreg = baudrate;
		m564->modectl = mode;
		m564->rcvctl = rx_charsize;
		m564->xmtctl = tx_charsize;
		m564->brgctl |= BRGEN;	/* re-enable baud rate generator */
		/* re-enable the transmitter */
		tp->t_state &= ~TTSTOP;
		m564->xmtctl |=  TXENAB;	/* re-enable transmit */
		m564->cmdreg = RESINT;
		m564->intctl |=  EXINTEN;	/* re-enable external/status */
		m564proc(tp, T_OUTPUT);		/* emulate an interrupt */
	}

	/* enable receiver? */
	if (flags & CREAD)
		m564->rcvctl |= RXENAB;
	else if (dev != 0) 			/* This may be over-protection*/
		m564->rcvctl &= ~RXENAB;	/* but it keeps the user from */						/* hanging the console        */
}

/*
*
*	proc entry
*
*/

m564proc(tp, cmd)
register struct tty *tp;
unsigned int cmd;
{
	register struct m564_io	*m564;
	int			dev;
	register int cur_lvl;

#ifdef DEBUG
	if (Debugm564){
		printf("m564proc:started.\n");
	}
#endif
	dev = tp - m564_tty;
	m564 = m564_addr[BOARD(dev)];
	if ( PORT1(dev) )
		m564++;
#ifdef DEBUG
	if(Debug1)
	{
		printf("m564proc:cmd: %d,dev = %d, m564 = %x\n",cmd,dev,m564);
	}
#endif

	cur_lvl = spl6();
	switch (cmd) {

	case T_TIME:		/* resuming output after delay */
		tp->t_state &= ~TIMEOUT;
		m564->xmtctl &= ~SENDBR;	/* turn off break*/;
#ifdef DEBUG
		if (Debugm564){
			printf("m564proc:T_TIME.\n");
		}
#endif
		goto start;

	case T_WFLUSH:		/* flush output queue */
		tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
		tp->t_tbuf.c_count = 0;
#ifdef DEBUG
		if (Debugm564){
			printf("m564proc:T_WFLUSH.\n");
		}
#endif
		/* fall thru */

	case T_RESUME:		/* resume output, XON has been received */
		tp->t_state &= ~TTSTOP;
		m564->xmtctl |= TXENAB;		/* re-enable the transmitter */
		if(!(m564->stat0 & TXEMPTY))	/* tx buffer available ? */
			break;
#ifdef DEBUG
		if (Debugm564){
			printf("m564proc:T_RESUME.\n");
		}
#endif
		/* fall thru */

	case T_OUTPUT:		/* handle first or next char */
	start: {
		register struct ccblock *tbuf;

#ifdef DEBUG
		if (Debugm564){
			printf("m564proc:T_OUTPUT started.\n");
		}
#endif
		if (tp->t_state & (BUSY|TTSTOP))
			break;
		tbuf = &tp->t_tbuf;
		if (tbuf->c_ptr == NULL || tbuf->c_count == 0) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))  {
#ifdef DEBUG
				if (Debugm564){
					printf("m564proc:T_OUTPUT reset.\n");
				}
#endif
				break;
				}
		}
		tp->t_state |= BUSY;			/* send char */
		dc_tout(dev, tp, m564);
		tp->t_tbuf.c_count--;	/* update count */
		break;
	}

	case T_SUSPEND:		/* stop output, XOFF has been received */
		/* There may be 3 pending characters in the mk68564 transmitter
		   (2 in FIFO + 1 in shift register), therefore disabling the
		   transmitter is the appropriate way to stop the transmission
		   as soon as posible to prevent overun condition on the other
		   side.
		*/
		m564->xmtctl &= ~TXENAB;
		tp->t_state |= TTSTOP;
#ifdef DEBUG
		if (Debugm564){
			printf("m564proc:T_SUSPEND.\n");
		}
#endif
		break;

	case T_BLOCK:		/* send XOFF */
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK;
#ifdef DEBUG
		if (Debugm564){
			printf("m564proc:T_BLOCK .\n");
		}
#endif
		if (tp->t_state&BUSY)
			tp->t_state |= TTXOFF;
		else {
			tp->t_state |= BUSY;
			m564->datarg = CSTOP;
		}
		break;

	case T_RFLUSH:	/* flush input queue.  if input blocked, send XON */
		if (!(tp->t_state & TBLOCK))
			break;
		/* fall thru */

	case T_UNBLOCK:		/* send XON */
		tp->t_state &= ~(TTXOFF | TBLOCK);
		if (tp->t_state&BUSY)
			tp->t_state |= TTXON;
		else {
			tp->t_state |= BUSY;
			m564->datarg = CSTART;
		}
		break;

	case T_BREAK:		/* send break condition for 1/4 second */
		m564->xmtctl |= SENDBR;	/* send a break*/;
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, HZ/4);
		break;

	case T_PARM:	/* used by sxt driver in R5.2 */
		m564param(dev);
		break;
	}
	splx(cur_lvl);
}



/*
*	init entry
*
*		On the exit of this routine, all devices are initialized to
*		following conditions:
*			1) Async mode, 9600 baud, 8-bit char, and 1 stop-bit.
*			2) Both Transmitter and Receiver are enable, but their
*			   interrupts are disable in order to support polling
*			   mode when the device is used as a system console
*			   during debugging sessions. Also note that most
*			   debuggers use polling mode to display on the console.
*			3) Both DTR and RTS are asserted to allow polling.
*			4) No DCD interrupt or External Interrupt.
*			5) No AUTO_ENABLE mode yet, so just ignore CTS or DSR at
*			   this point until open.
*
*/

m564init()
{
	register unsigned int dev;
	register struct m564_io *m564;
	int i,cur_lvl;

	/* Diagnostic concurrent mode flag is in ml/start.s */
	extern int Dconcurrent; 

#ifdef DEBUG
	if (Debugm564)
	{

		printf("m564init:configured with devcnt= %d\n",
			m564_cnt);
		printf("m564init:Dconcurrent mode is %x\n", Dconcurrent);
	}
#endif

	cur_lvl=spl5();
/* init both ports on each chip HERE */
	for (dev = 0; dev < m564_cnt; dev += 2 ) { 
		m564 = m564_addr[BOARD(dev)];
		for ( i= 0; i < 2; i++ ) {
			if(!dc_init(dev, i, m564))
			{
				m564->cmdreg = RESCHNL;	/* reset channel & all control regs  */
				m564->xmtctl = TXENAB|DTR|RTS|TX8BIT; /* set up a default */
				m564->cmdreg = RESTXUN; /* clear Tx underrun bit */
				m564->modectl = (X16CLK|ST1BIT); /* x16 clock, 1 stop bit */
				m564->tcreg = BD9600;	/* set Time Constant reg. */
				m564->brgctl = (DIV4|TXCLK|RXCLK|BRGEN);	/* set BR control Reg.*/
				m564->vectrg = m564_ivec >> 2; /* not used on vm04 board */
				m564->intctl = (STAFFV|RXDIS);
				/* not yet transmit interrupts,
				   not yet receive interrupts - also set up the
					Status Affects Vector feature		*/
				/* enable receiver for 020bug to use Breakpoint/Abort/Trace */
				m564->rcvctl = RXENAB|RX8BIT;
			}
			m564++;  /*  ports occupy consecutive addresses */
		}

#ifdef DEBUG
		if (Debugm564){
			printf("m564init: initialized dev %d\n",
				dev);
		}
#endif
	}
	splx(cur_lvl);
}


/*
*
*	interrupt entry
*
*/

m564intr()
{
	register struct tty	*tp;
	register struct m564_io	*m564;
	register unsigned char	c, ctmp, count, status;
	unsigned int port,dev;
	register int		flg;
	char			discard, lbuf[3];
	int			cur_lvl, lcnt;
	unsigned char	stat1;
	/* modem character */
	static long cur_char = 0;

	/* all chips on one vector */
	for ( dev = 0; dev < m564_cnt; dev +=2 )  {
		m564=m564_addr[BOARD(dev)];
		port = dev;

		if (!(m564->vectrg&PORTMASK)) {
			/* we already know that its a port 1 device */
			if(!Dconcurrent)
				port++;
			m564++;
		}

		for ( ;; ) {
			/* no more interrupts pending */
			if ( ! ( m564->stat0 & INTPEND ))	
				break;

			tp= &m564_tty[port];
		
			switch ( m564->vectrg & INTTYPE ) {

			/* Transmit Buffer Empty Interrupt  */
			case TXMTINT:
				sysinfo.xmtint++;
				m564->cmdreg = RESTXP;	/* reset */
				if(PORT1(dev) && Dconcurrent)
					continue;
				if(tp->t_state & TTXOFF){
					m564->datarg = CSTOP;
					tp->t_state &= ~TTXOFF;
				} else if(tp->t_state & TTXON){
					m564->datarg = CSTART;
					tp->t_state &= ~TTXON;
				} else {
					tp->t_state &= ~BUSY;
					m564proc(tp,T_OUTPUT);
				}
				continue;

			case EXSTINT:		/* External/Status Interrupt  */
				status = m564->stat0 & (DCD|TXUNEOM|BRABRT);
				m564->cmdreg = RESINT|RESTXUN;

				/* break processing */
#			ifdef DEBUG1
			bmsg=3;
#			endif
				if ( brkflg564[port] )  {
					if ( !(status & BRABRT)) {
						brkflg564[port] = 0;
						discard = m564->datarg;
					}
				} else if ( status & BRABRT ) {
					brkflg564[port]++;
					flg = tp->t_iflag;
					if ( !(flg & IGNBRK) && (flg & BRKINT) )
						(*linesw[tp->t_line].l_input)(tp, L_BREAK );
					}

				/* carrier processing */
#			ifdef DEBUG1
			bmsg=4;
#			endif

			if(dcdflg564[port] != (status & DCD))
				dcdscan564[port] =  1;
			if(ctsflg564[port] != (status & CTS))
				ctsscan564[port] = 1;
		
			continue;

			case RXCHINT:		/* Receive character interrupt */
				sysinfo.rcvint++;

				c = m564->datarg;

				if(PORT1(dev) && Dconcurrent) 
				{
					cur_char = (((cur_char<<8)|c)&MMASK);
					if(cur_char == lcp_char)
						putchar(MODEM_QUIT);
				}

				lcnt= 1;
				if(tp->t_iflag & IXON){
					ctmp= c & 0x7f;
					if(tp->t_state & TTSTOP){
						if((ctmp==CSTART) || 
						(tp->t_iflag & IXANY))
							m564proc(tp,T_RESUME);
				}else{
					if(ctmp == CSTOP)
						m564proc(tp,T_SUSPEND);
				}
				if(ctmp == CSTART || ctmp == CSTOP){
					continue;
				}
			}
				if(tp->t_iflag & ISTRIP)
					c &= 0x7f;
				else if(c == 0xff && tp->t_iflag & PARMRK){
					lbuf[1]= 0xff;
					lcnt= 2;
				}
				break;

			/* Special Receive Condition Interrupt */
			case SRCINT:		
#			ifdef DEBUG1
			bmsg=1;
#			endif
				/* get the status and character */
				stat1 = m564->stat1;	
				c = m564->datarg;
				lcnt = 1;

				m564->cmdreg = ERRRES;/*clear status*/

				flg=tp->t_iflag;


				if (!(flg&IGNPAR)) {	/* no checking at all */
					if (!(flg&INPCK))	/* no parity check */
						stat1 &= ~PARERR;
					if (stat1 & (PARERR|RXOVER)) {
						if (flg & PARMRK) {
							lbuf[2] = 0xFF;
							lbuf[1] = 0;
							lcnt = 3;
							sysinfo.rawch += 2;
						} else
							c = 0;
#				ifdef DEBUG1
				bmsg=2;
#				endif
					}
				}
				break;
			}	/* end of switch */
#			ifdef DEBUG
			if (Debugm564 && stat1&RXOVER)
				printf("m564intr: rx overrun on dev %d\n",
						tp-m564_tty);
#			endif

			/* stash character in r_buf */
			lbuf[0] = c;
			if (tp->t_rbuf.c_ptr == NULL) {
				if(Debugm564)
					printf("rbuf is null: %x\n", tp->t_state);
			}
			if (tp->t_rbuf.c_ptr != NULL) {
				while (lcnt) {
					cur_lvl = spl6(); /*to block m564scan() from
							    running at clock priority*/
					*tp->t_rbuf.c_ptr++ = lbuf[--lcnt];
					if ((count = --tp->t_rbuf.c_count) < HI_MARK) {
						if(!(tp->t_state & TBLOCK) && tp->t_iflag & IXOFF)
						    m564proc(tp, T_BLOCK);
						if(!(m564->stat0 & RXAVAIL) || count < 1) {
							m564_work[port] = 0;
							tp->t_rbuf.c_ptr -= tp->t_rbuf.c_size - count;
							(*linesw[tp->t_line].l_input)(tp,L_BUF);
							if(tp->t_rawq.c_cc <= TTXOHI && tp->t_iflag & IXOFF)
								m564proc(tp, T_UNBLOCK);
							}
						}
						m564_work[port] = R6_ACT;
					/* note R6_CHAR bit cleared */
					splx(cur_lvl);
				} 
			} 
		}	/* end of "inside for" loop  */
	}	/* end of " outside for" loop */

}


/*
*
*	scan entry
*
*/

m564scan()
{
	register struct tty	*tp;
	register int		dev;
	register char		*workp;

	for (dev = 0, workp = m564_work; dev < m564_cnt; dev++, workp++) {
		if (*workp & R6_CHAR) {
			tp = &m564_tty[dev];
			*workp = 0;
			if (tp->t_rbuf.c_ptr) {
				tp->t_rbuf.c_ptr -= tp->t_rbuf.c_size
						- tp->t_rbuf.c_count;
				(*linesw[tp->t_line].l_input)(tp, L_BUF);
			}
		}
		if (*workp & R6_ACT)
			*workp |= R6_CHAR;
		if((dcdscan564[dev]) && (dcdscan564[dev]++ >= MAXDCDTIME))
		{
			m564_chkdcd(dev);
			dcdscan564[dev] = FALSE;
		}
	}
	timeout(m564scan,(struct tty *) 0, R6CSCAN);
}

/*
*
*	modem entry
*
*/

m564modem(dev,flag)
register int dev;
int flag;
{
	register struct m564_io  *m564;
	int cur_lvl;
	register struct tty	*tp;
	int s;

	tp = &m564_tty[dev];
	m564 = m564_addr[BOARD(dev)];
	if( PORT1(dev) )
		m564++;
	
	cur_lvl=spl5();
	if(flag==OFF){
		m564->xmtctl &= ~(DTR|RTS);
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
	} else{
		m564->xmtctl |= DTR|RTS;
	}
	s = m564->stat0 & DCD;
#ifdef DEBUG
	if (Debugm564)
		printf("m564modem: DCD = %d\n", s);
#endif
	splx(cur_lvl);
	return( s );
}


putchar(c)
register char c;
{
	register struct tty	*tp;
        register struct m564_io *m564;
        register char intstate,xmtstate;
	register cur_lvl;

        m564 = m564_addr[0];		 	/* MK68564 address  */
	cur_lvl = splhi();

        intstate = m564->intctl;		/* save current int mask */
	xmtstate = m564->xmtctl;
	
        m564->intctl &= ~(RXALLNOP|TXINTEN|EXINTEN); /* disable interrupts */
	m564->xmtctl |= TXENAB;

        for (;;)
        {
                while (!(m564->stat0 & TXEMPTY)); /* wait for tx ready 	 */
                m564->datarg = c; 		  /* xmit char 	 */
		dc_putc(c, m564);
                if (c != LF)
                        break;
                c = CR;			/* send CR if previous c==LF */
        }
        while (!(m564->stat1 & ALLSENT));	/* wait for tx empty 	 */

	m564->cmdreg = RESINT;
        m564->intctl = intstate;		/* restore previous state */
	m564->xmtctl = xmtstate;

	/* simulate any missing interrupt */
	tp = &m564_tty[0];
	if(tp->t_state & TTXOFF){
		m564->datarg = CSTOP;
		tp->t_state &= ~TTXOFF;
	} else if(tp->t_state & TTXON){
		m564->datarg = CSTART;
		tp->t_state &= ~TTXON;
	} else if ( tp->t_state & BUSY ){
		tp->t_state &= ~BUSY;
		m564proc(tp,T_OUTPUT);
	}
	
	splx(cur_lvl);
}

/*
	dual console - open routine
	sleep and wait for access to this port 
*/
dc_oslp(dev, tp)
int dev;
register struct tty *tp;
{
	extern int Dconcurrent;

	if(PORT1(dev) && Dconcurrent)
		sleep(( caddr_t)&tp->t_canq,TTIPRI);
}

/*
	dual console - ioctl routine
*/
dc_ioctl(cmd, arg, dev, tp, m564)
register int cmd;
register int arg;
register int dev;
register struct tty *tp;
register struct m564_io *m564;
{
	int status = TRUE;
	extern int Dconcurrent;

	switch(cmd)
	{
		/*
			The utility 'dcon' must never be changed
			to open port1. This is only important in the
			case of concurrent mode (CONC_ON and CONC_OFF) 
		*/
		case CONC_ON: 
			if(PORT1(dev))
			{
				u.u_error = ENXIO;
				return;
			}
			/* we want to make changes to the modem port */
			m564++; /* point to the modem 564 structure */

			/* turn off concurrent mode */
			Dconcurrent = 1;

			/* save all current settings */
			if(!saved)
			{
				saved = 1;
				current.vectrg = m564->vectrg;
				current.intctl = m564->intctl;
				current.xmtctl = m564->xmtctl;
				current.rcvctl = m564->rcvctl;
			}


			/* init interrupt vector */
			m564->vectrg = m564_ivec >> 2;
			/* turnoff old Rx int mode */
			m564->intctl &= ~RXINTMASK;
			/* set desired Rx/Tx int */
			m564->intctl |= TXINTEN|RXALLP;
			/* enable receiver */
			m564->cmdreg = RESINT;
			/* re-enable external/status */
			m564->intctl |=  EXINTEN;
			m564->xmtctl |= (DTR|RTS);
			m564->rcvctl |= RXENAB;
			putchar(MODEM_QUIT);
			break;

		case CONC_OFF: 
			if(PORT1(dev))
			{
				u.u_error = ENXIO;
				return;
			}
			/* we want to make changes to the modem port */
			m564++;

			/* make sure the modem is turned off */
			putchar(MODEM_QUIT);

			/* turn off concurrent mode */
			Dconcurrent = 0;


			/*
				we only want to restore them
				if they have been saved
			*/
			if(saved)
			{
				/* reset the previous settings */
				m564->vectrg = current.vectrg;
				m564->intctl = current.intctl;
				m564->xmtctl = current.xmtctl;
				m564->rcvctl = current.rcvctl;
			}

			/* we want to get rid of the old changes */
			saved = 0;

			/* disconnect the port from the modem */
			m564modem(dev+1, OFF);

			/*
				remove any pending I/O flags
				from the modem port, so a
				getty can continue
				This is about the only disorderly
				thing that dual console does.
			*/
			(++tp)->t_state &= ~(TTIOW|BUSY);

			/* wait for at least 250 mills */
			/* 16.6 ms * 16 = 265.6 ms */
			/* for the drop of DCD to take effect */
			/* restore all previous settings */
			delay(16);

			break;

		case CONC_DEBUG: 
			if(arg == '1')
				Debug1 = ~Debug1;
			else if(arg == '2')
				Debug2 = ~Debug2;
			else
				Debugm564 = ~Debugm564;
			break;
		default:
			status = FALSE;
			break;
	}
	return(status);
}

/* 
	dual console
	T_OUTPUT routine - transmit character to modem port
*/
dc_tout(dev, tp, m564)
int dev;
register struct tty *tp;
register struct m564_io *m564;
{
	register int cur_lvl;

	if(Dconcurrent)
	{
		/* put char to trans data reg of console port*/
		m564->datarg = *tp->t_tbuf.c_ptr;
		/* put char to trans data reg of modem port*/
		if(!(PORT1(dev)))
			m564++;
		if((!PORT1(dev)) && (Debug2))
			printf("T_OUT: m564: %x\n", m564);
		m564->datarg = *tp->t_tbuf.c_ptr++;
		cur_lvl = spl6();
		wait_for_done(m564);
		splx(cur_lvl);
	}
	else
		/* put char to trans data reg */
		m564->datarg = *tp->t_tbuf.c_ptr++;
}
/*
	wait for done
	wait for output on the modem port to complete
	before going onto the next character
*/
wait_for_done(m564)
register struct m564_io *m564;
{
	/* wait for tx ready 	 */
	while (!(m564->stat0 & TXEMPTY));
	
}
/*
	dual console
	init routine
*/
dc_init(dev, i, m564)
int dev;
int i;
register struct m564_io *m564;
{
	/* 
	   if we are in concurrent mode then don't initialize
	   the integral modem port
	*/
	if(PORT1(i) && (dev == 0) && Dconcurrent)
	{
		m564->vectrg = m564_ivec >> 2;
		/* turnoff old Rx int mode */
		m564->intctl &= ~RXINTMASK;
		/* set desired Rx/Tx int */
		m564->intctl |= TXINTEN|RXALLP;
		m564->cmdreg = RESINT;
		/* re-enable external/status */
		m564->intctl |=  EXINTEN;
		m564->rcvctl |= RXENAB;
		return(TRUE);
	}
	return(FALSE);
}
/*
	dual console 
	putchar routine for modem port
*/
dc_putc(c, m564)
register struct m564_io *m564;
{
	extern int Dconcurrent;

	if(Dconcurrent)
	{
		/*
			send data to the modem port
			if connected in dual console mode
		*/
		m564++;
		while (!(m564->stat0 & TXEMPTY));
		m564->datarg = c; 		  /* xmit char 	 */
	}
}







m564_chkdcd(dev)
int dev;
{
	register struct m564_io	*m564;
	register struct tty	*tp;
	int status;

	m564=m564_addr[BOARD(dev)];

	if (PORT1(dev))
		m564++;

	tp= &m564_tty[dev];
	
	status = m564->stat0;
	if ( dcdflg564[dev] ) { 
		if ( !(status & DCD) ) {
			dcdflg564[dev] = 0;
			if ( !(tp->t_cflag & CLOCAL) ) 
				if(tp->t_state & CARR_ON ) {
					if (tp->t_state & ISOPEN) {
						signal(tp->t_pgrp,SIGHUP);
						ttyflush(tp,(FREAD|FWRITE));
					}
					tp->t_state &= ~CARR_ON;
				}
			}
	} else if (status & DCD) {
		dcdflg564[dev]++;
		if ((tp->t_state & (CARR_ON|WOPEN)) == WOPEN) {
			wakeup(&tp->t_canq);
			tp->t_state |= CARR_ON;
		}
	}
}

