/*	@(#)mvme331.c	2.1
 *
 *	Driver for the MVME331/332 intelligent communication controller
 */

/*
	available compiler switches:

  	NOT_FINAL_VERSION	print additional messages
  	HOWFAR			print DEBUG messages
*/

/*	available test switch

	HISTORY 		for getting a history table of the last
				256 interrupts in- and output. 
				One interrupt event includes two addresses.
				Address of the packet and then pointer
				to the status- or command queue.
				The left both digits of the pointer are the
				command of the appropriate packet.
				If the next both digits are FF this interrupt
				has been initiated by the ICC module.
				For example:
				02FFbd5e	Command write, 
						Interrupt from ICC module.
				To get this table use the histstat command.
*/


/* SSM 06/13/86 Bugs fixed and cleanups 1.3 -> 2.1
 *
 * 04/24/86 - 1 Errorhandling built in as normal function.
 *              Before it only worked if NOT_FINAL_VERSION has been defined.
 *	        Also the routine ProcessErrors() will now only be called
 *	        if the returncode indicates an error.
 *
 * 04/24/86 - 2 Function ReadCompleteICC() thrown out (has only been one line
 *	        of effective code). Substituted by the actual line.
 *
 * 04/24/86 - 3 Function WriteCompleteICC() thrown out (has only been one line
 *	        of effective code). Substituted by the actual line.
 *
 * 04/24/86 - 4 Drain output before last close.
 *		In m331close() lineswitch close will now only be called, after
 *              it is ensured, that all chars written to the board have already
 *		left the board.
 *              This is done by waiting for the chars to be on board, issuing
 *	        a ConfigPortICC (which will on the board only be performed
 *		when all chars are written out), and issuing a
 *		StartConfigPortICC to start it.
 *
 * 05/20/86 - 1 GetBuffer and FreeBuffer altered, so that there is now one
 *		buffer for each port and not one common buffer for all ports.
 *		Before one port who was sleeping (e.g. if XOFF was pressed)
 *              and who had grabbed the buffer prevented all other ports
 *		to get hold of the buffer and to do anything.
 *
 * 05/20/86 - 2 Softwarehandshake (IXON) for the transmitter isn't any more
 *		dependant on the CLOCAL flag
 *
 * 05/23/86 - 1 The Carrier (DCD) will now only be updated and appropriate
 *		actions taken dependent on the CLOCAL flag
 *
 * 05/27/86 - 1 Three new ioctl cmds introduced: MSETHWHAND, MCLEARHWHAND
 *		MGETHWHAND to implement the correct hardwarehandshake for
 *		the transmitter (CTS), which has to be done by ioctl, because
 *		it's not part of UNIX termio.
 *		Also fixed in m331param, so that hwhandshake is set if
 *		the appropriate flag ICCHWHAND is set, and made independant
 *		of the IXOFF (receiver) flag.
 *
 * 06/02/86 - 1 Check if -CLOCAL and no carrier and if so reset the
 *		state to ~CARR_ON in m331param. If either CLOCAL or
 *		carrier set state to CARR_ON.
 *
 * 06/09/86 - 1 There has been one major problem with mixed configurations,
 *		due to our decoding of the minor device number
 *		(bit 3 to 5 is controller, bit 0 to 2 is the port)
 *		The writei routine in the kernel assumes, that if there 
 * 		is a tty-structure for a characterdevice, the minor device-
 *		number is the number of the ttyport. That is only true
 *		for boards with 8 ports ( on 331 boards minor device-
 *		numbers 6 and 7 respectivly 14 and 15 and so on are
 *		unused). So the tp structure will be addressed wrong
 *		(by the writei routine!) if there is more than one 
 *		331 board, or if there is one 331 board which is not
 *		the last in the configuration.
 *		To come around this problem the driver was changed, so
 *		that for each 331 board there are now two additional 
 *		tp structures, which are unused, and the status for 
 *		the nonexistent ports on this boards is set to ICCCLEAR
 *		Due to this change, the earlier introduced second cross-
 *		reference list setting according to the portscount returned
 *		from the boards was left out again.
 *
 * 06/23/86 - 1 Dependency of call to ReadICC in m331proc, T_INPUT from flag
 *              T_BLOCK eliminated, because we want to continue to read
 *		characters from the board even when rawqueue is nearly
 *		full to be able to flush the rawqueue afterwards.
 *
 * 06/23/86 - 2 In m331proc, T_INPUT computation of variable count 
 *		altered from TTYHOG - 10 to TTYHOG + 1. Same Reason as
 *		above: We want the rawqueue to become full, so it can
 *		be flushed. Therefore we have to continue to read characters 
 *		from the board via ReadICC.
 * 
 * 06/23/86 - 3 Kind of read altered. Before the board returned on least
 *		number of characters read (1), now it returns after timeout
 *		(every 200 ms). Therefore we don't need to abort the pending
 *		read any more, and don't have to take care of that the propper 
 *		restart actions (of writes) will be taken. 
 *		All occurencies of aborts sent to the board thrown out.
 *		Also restart actions.
 *		
 * 07/01/86 - 1 Configuration Buffer replaced by Common Buffer Array.
 *		Before there was only one Conf. Buffer, which could cause 
 *		deadlocks, now there is one for every Port.
 *		Function ConfigPortICC now obsolet and replaced by direct
 *		code.
 * 		In that context removed part of m331param, and placed it
 *		in the new function ConfigRTTTable.
 *		Replaced Function ConfigTableICC.
 *		
 * 07/02/86 - 1 Fixed bug in m331ioctl. Included FreeBuffer before break.
 *		Deadlock (buffer never freed) might have occured.
 *		
 * 07/22/86 - 1 Fixed bug in CCStartCmd. The board could alter the adress
 *		between the interrupt and going to sleep, so the wrong
 *		packet would be awakened. Solved by first saving the
 *		address before interrupting, and going to sleep on the
 *		saved adress afterwards.
 *
 * 07/22/86 - 2 In rawmode the rawq could fill up very quickly. Although
 *		the line was already blocked, reads continued to be sent
 *		to the board, chars were received and passed on to the line-
 *		discipline which caused the rawq to become full and to
 *		be flushed, so chars were lost. Solved by checking wether
 *		the line is blocked in m331intr, and not calling the
 *		linediscipline inputroutine and not calling m331proc
 *		with argument T_INPUT if the line is blocked. Instead
 *		a new flag will be set, which afterwards will be checked
 *		in m331proc, when an T_UNBLOCK occurs. If the flag is set then,
 *		the linediscipline inputroutine will be called before
 *		falling through to T_INPUT, and the flag will be reset.
 *		
 */

#include "sys/param.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/buf.h"
#include "sys/sysmacros.h"
#include "sys/mvme331.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"

/* CONFIGURATIONS */
#define PORTSCOUNT	MAXPORTS	/* maximal count of ports */

#define CNTRLR(x)	((x & 0x00ff) >> 3)

/* DEFINITIONS */
#define FALSE		0


/* EXTERNALS */

/* declared in config.c and space header files */
/* count of MVME 331 controllers */
extern int m331_ctlcnt;

/* count of MVME 331 serial ports */
extern int m331_cnt;

/* interrupt vector array */
extern int m331_ivec[];

/* interrupt level array */
extern int m331_ilev[];

/* Halt Save Tables */
extern char m331_haltsave[][];

/* base addresses of ports */
extern int m331_addr[];

/* tty structures for this driver */
extern struct tty m331_tty[];

/* return variable definition */
extern char *GetBuffer();

/* to get actual process id we have to get access to the 
	p_pid    in struct proc via
	*u_procp in struct user u	*/
extern struct user u;		/* the user structer itself */
extern struct proc proc[];	/* the proc table itself */



/* GLOBALS */

/* dcd event change log */

int dcd_status[MAXICCS*MAXPORTS];
int dcd_event[MAXICCS*MAXPORTS];
int dcd_timeout;
int dcd_tick = 16; /* 16.6 ms * 16 ticks = 250ms */
#define MAXDCDTIME 2

#ifdef PRT
#define PRT1(msg,arg1)	printf(msg,arg1);
#define PRT2(msg,arg1,arg2)	printf(msg,arg1,arg2);
#else
#define PRT1(msg,arg1)
#define PRT2(msg,arg1,arg2)
#endif

/* initializer flag */
int ICCInitFlag;

/* count of present MVME 331 controllers */
int m331_present;

/* description of controllers */
struct icc m331_icc[MAXICCS];

/* cross reference between index of tp structure and cntl / port */
struct tpref tpdev[MAXICCS*MAXPORTS];

/* speed table */
/* 200 bd is not implemented, EXTA=19.2kB, EXTB=38.4 kB */
int m331_speeds[] = { 0, 50, 75, 110, 134, 150, 0, 300, 600,
		   1200, 1800, 2400, 4800, 9600, 19200, 38400 };

/* interrupt flag */
int InM331Intr = 0;

/* dummy for return length, when it is not used */
static int dummy;

#ifdef HOWFAR
    /* flag for printing debug messages */
    int m331Debug = 0;
#endif


/*
 * I N I T
 * =======
 *
 * If not a valid configuration, then
 *	Print a warning
 *	set defaults
 * End if
 * Get configuration items
 * Loop thru all controllers, until
 *	a controller does not acknowledge the hardware probe
 * Print greetings
 *
 * Called by:	initializer device switch
 * Input:	-
 * Return:	-
 * Error:	-
 * Calls to:	bprobe()
 *
 */

m331init()
{
	/* loop count */
	register int i;

	/* check if configuration is okay */
	if( m331_ctlcnt > MAXICCS ) {
		printf( "m331init: Warning: Count of controllers is too big\n" );
		printf( "                   Maximum count is %d\n", MAXICCS );
		/* clean this configuration error */
		m331_ctlcnt = MAXICCS;
	}

	if( m331_cnt > (m331_ctlcnt * MAXPORTS) ) {
		printf( "m331init: Warning: Count of ports is too big\n" );
		printf( "                   Maximum count is %d\n", 
			m331_ctlcnt * MAXPORTS );

		/* clean this configuration error */
		m331_cnt = m331_ctlcnt * MAXPORTS;
	}

	/* set the description of the controllers */

	/* this comes from the configuration file  */
	for( i=0; i<MAXICCS; ++i ) {
	    m331_icc[i].c_intrvector = (unsigned) m331_ivec[i] >> 2;
	    m331_icc[i].c_intrlevel = m331_ilev[i];
	}

	/* check if the hardware acknowledges */
	for( i=0; i<m331_ctlcnt; ++i ) {

		/* try a read to the communication register */
		if( bprobe( m331_addr[i]+COMREGOFFSET, -1 ) ) {

			break;
		}
	}

	/* save the actual count of controllers */
	m331_present = i;

#ifdef NOT_FINAL_VERSION
	printf( "MVME 331/332 DRIVER present\n" );
	printf( "    Configuration: " ); 
	printf( "    Count of controllers: %d", m331_ctlcnt );
	printf( "    Count of ports: %d\n", m331_cnt );
	printf( "    Found: %d controllers\n", m331_present );
#endif /* NOT_FINAL_VERSION */

}	/* end of m331init */


/*
 * O P E N
 * =======
 *
 * If first open of driver, then
 *	Initialize the controllers
 *	Initialize the command channel
 *	Configure controllers
 *	Initialize all ports of a controller
 * End if
 * 
 * Check for valid device
 * If it is not opened by another process, then initialize this port
 * Update carrier flag
 * If carrier is missing and not in NDELAY mode, wait for carrier
 * Call open routine in upper tty driver level via lineswitch
 *
 * Called by:	open device switch
 * Input:	device, flag
 * Return:	ENXIO in u.u_error if device not active
 * Error:	if device not active
 * Calls to:	CCdrvInit(), CCCntlInit(), ConfigICC(), ConfPortsCntICC(),
 *		StatICC(), InitPortICC(), m331param(), GetDCDICC(),
 *		ttinit(), linesw.open()
 *
 */

m331open( dev, flag )
unsigned int dev;
unsigned int flag;
{
	/* decoded cntl number from device number */
	int cntl;

	/* decoded port number from device number */
	int port;

	/* current tty structure */
	register struct tty *tp;

	/* loop count */
	int i, j;

	/* error flag*/
	int errflag;

	/* buffer for ioctl data */
	char *statbuf;
	
	/* pointer to portscount in stat buffer */
	int *countpt;

	/* count of maximum ports */
	int maxcount;

	/* index for tty structures */
	int ttyindex;

	/* save current level */
	int s;

        int retvalue;

	extern m331proc();

#ifdef HOWFAR
	if( m331Debug>3 )
	    printf( "In m331open: dev=0x%x, flag=0x%x\n", dev, flag );
#endif
	
	/* avoid two processes from entering initialisation */
	s = spl6();

	/* if first open of the driver, initialize the controllers */
	if( ICCInitFlag == 0 ) {

	    /* set initialization in progress */
	    ICCInitFlag = 1;

	    /* set level 6 back to the level before */
	    splx(s);

    	    /* set the start of the tty structures */
	    ttyindex = 0;

	    /* start initializing, if controllers are present */
	    if( m331_present > 0 ) {

		/* set max count of ports from config value */
		maxcount = m331_cnt;

	        /* initialize command channel interface */
	        CCDrvInit( m331_present );
    
	        /* initialize command channel of any controller */
	        for( i=0; i<m331_present; ++i ) {

/* SSM 06/09/86 -1 */

	    	    m331_icc[i].c_firsttty = ttyindex;

	           /* set cross reference list */
	           for( j=0; j<MAXPORTS; ++j )
	           {
	   	       tpdev[ ttyindex + j ].t_cntl = i;
	   	       tpdev[ ttyindex + j ].t_port = j;
	    	    }
		    errflag = 0;
		    if( CCCntlInit( i, m331_addr[i],
				m331_icc[i].c_intrvector,
				m331_icc[i].c_intrlevel ) == 0 ) {

			/* configure controller */
		        if( ConfigICC( i,
			       m331_icc[i].c_intrvector,
			       m331_icc[i].c_intrlevel    ) == 0 ) {

			    /* get portscount from controller */
/* SSM 05/20/86 - 1 */
			    statbuf = GetBuffer( 0,0 );
			    if( StatICC( i, statbuf ) == 0 ) {
				/* set pointer to address of portscount
				   in status table */
				countpt = (int *) (statbuf+0x28);
				
				if( maxcount >= *countpt ) {
			            m331_icc[i].c_portscount = *countpt;
				    maxcount -= *countpt;
/* SSM 07/01/86 - 1 */
			            FreeBuffer(0,0 );
				}
				else {
/* SSM 07/01/86 - 1 */
			            FreeBuffer(0,0 );
				    /* set ports count to 
				       remaining count */
			            m331_icc[i].c_portscount = maxcount;
				    maxcount = 0;

				    /* configure controller again */
		        	    if( ConfPortsCntICC( i,
			                m331_icc[i].c_portscount ) != 0 ) {
			    		/* error in ConfPortsCntICC */
			    		errflag = 1;
				    }
				}
/* SSM 06/09/86 - 1 */
	    			m331_icc[i].c_firsttty = ttyindex;

	    		        ttyindex += MAXPORTS;
			    }
			    else {
				/* error in StatICC */
				errflag = 1;
/* SSM 07/01/86 - 1 */
			        FreeBuffer(0,0 );
			    }

					
			    if( !errflag ) {
		                /* initialize the ports */
		                for( j=0;j<m331_icc[i].c_portscount;++j ) {
			            /* initialize the configuration */
			            if( InitPortICC( i, j ) == 0 ) {
			                m331_icc[i].c_stat[j] = ICCINIT;
				    }
				    else {
			                m331_icc[i].c_stat[j] = ICCCLEAR;
				    }
		                }
/* SSM 06/09/86 - 1 */
				for( ; j < MAXPORTS ; ++j)
					m331_icc[i].c_stat[j] = ICCCLEAR;
			    }
			}
			else {
			    /* error in ConfigICC */
			    errflag = 1;
			}
		    }
		    else {
			/* error in CCCntlInit */
			errflag = 1;
		    }

		    if( errflag == 1 ) {
		        /* clear the status */
		        for( j=0; j<m331_icc[i].c_portscount; ++j ) {
			    m331_icc[i].c_stat[j] = ICCCLEAR;
		        }
		    }
	        }
	    }

	    /* set driver initialized */
	    ICCInitFlag = 3;
	    wakeup( &ICCInitFlag );
	}
	else {
	    /* set level 6 back to the level before */
	    splx(s);
	    /* check, if initialization is in progress */
	    if( ICCInitFlag == 1 ) {
		/* wait on completion of initialization */
		while( ICCInitFlag != 3 ) {
		    sleep( &ICCInitFlag, PRIICC );
		}
	    }
	}

	/* decode the device */
	cntl = CNTRLR(dev);
	port  = dev &  0x07;

	/* check if this port is valid and initialized */
	if ( cntl >= MAXICCS ) {
		u.u_error = ENXIO;
		return;
	} else if ( port >= m331_icc[cntl].c_portscount ) {
		u.u_error = ENXIO;
		return;
	} else if ( !(m331_icc[cntl].c_stat[port] & ICCINIT) ) {
		u.u_error = ENXIO;
		return;
	}

	/* get tty pointer */
	tp = &m331_tty[ m331_icc[cntl].c_firsttty + port ];

	/* if tty not opened by another process, init tty */
	if ((tp->t_state & (ISOPEN|WOPEN)) == FALSE)
	{
		/* init tty structure */
		ttinit(tp);
		tp->t_proc = m331proc;
		if((retvalue = CCDoCmd( CMD_RCVFLUSH, cntl, port+1, NULL, 0,
			&dummy, MREAD|MSLEEP )))
			ProcessErrors(retvalue, cntl, port, "FlushReceiveICC" );
		m331param(dev);
	}

	/* add call to new module to assert controls properly 8/26/86 */

	m331modem(cntl, port);	/* assert the DTR and RTS for modem control */

/* SSM 05/23/86 - 1 */
	/* update carrier flag*/
	if ( (tp->t_cflag&CLOCAL) || (GetDCDICC(cntl, port) == 1) ) 
	{
		tp->t_state |= CARR_ON;
		wakeup((caddr_t)&tp->t_canq);
	}
	else
		tp->t_state &= ~CARR_ON;

	/* if O_NDELAY at open call is set, wait for carrier 
	   see OPEN(2) */
	if (!(flag & FNDELAY))
		while ((tp->t_state & CARR_ON) == FALSE)
		{
			/* wait for carrier detect */
			tp->t_state |= WOPEN;
			/* wait for the solicited
			   event for changing the DCD line */
			sleep((caddr_t)&tp->t_canq, TTIPRI);
		}

	/* call open routine in upper tty driver level via lineswitch */
	(*linesw[tp->t_line].l_open)(tp);

}	/* end of m331open */



/*
 * C L O S E
 * =========
 *
 * Call close routine in upper tty driver level via lineswitch
 * If hang up on last close mode, clear DTR and RTS line
 *
 * Called by:	close device switch
 * Input:	device 
 * Return:	-
 * Error:	-
 * Calls to:	StartConfigPortICC(), linesw.close()
 *
 */

m331close( dev )
unsigned int dev;
{
	/* decoded cntl number from device number */
	int cntl;

	/* decoded port number from device number */
	int port;

	/* current tty structure */
	register struct tty *tp;

        /* buffer */
	struct ICCitem *ip;
	struct confbuf *cp;

#ifdef HOWFAR
	if( m331Debug>3 )
	    printf( "In m331close: dev=0x%x\n", dev );
#endif

	/* decode the device */
	cntl = CNTRLR(dev);
	port  = dev &  0x07;

	/* get tty pointer */
	tp = &m331_tty[ m331_icc[cntl].c_firsttty + port ];

	(*linesw[tp->t_line].l_close)(tp);

	/* if hang up on last close */
	if (tp->t_cflag & HUPCL && !(tp->t_state & ISOPEN))
	{
/* SSM 07/01/86 - 1 */
		/* get config table */
		cp = (struct confbuf *) GetBuffer(cntl,port);
		ip = cp->param;

		/* set DTR inactive */
		ip->item  = 5;
		ip++->value = 0;

		/* set RTS inactive */
		ip->item  = 6;
		ip++->value = 0;

		cp->resetflg = 1;
		cp->index = 2;

		/* issue the changes thru a reset command */
		StartConfigPortICC( cntl, port ,cp);

		/* free config table */
		FreeBuffer(cntl,port);

#ifdef NOT_FINAL_VERSION
		if(tp->t_state & BUSY)
		printf("Warning: m331close: Closed dev=%d while busy \n", dev );
#endif
	}

}	/* end of m331close */

/*
 * ASSERT DTR & RTS
 *
 * check if port is valid
 * do following changes
 *	- set DTR and RTS active
 *	- issue the changes thru a reset cmd
 *
 * Called by:	m331open()
 * Input:	device
 * Return:	ENXIO in u.u_error if device not active
 * Error:	if device not active
 * Calls to:	StartConfigPortICC(),
 *
 */

m331modem(cntl, port)
unsigned int cntl, port;
{
        /* buffer */
	struct ICCitem *ip;
	struct confbuf *cp;

	/* get config table */
	cp = (struct confbuf *) GetBuffer(cntl,port);
	ip = cp->param;

	/* set DTR active */
	ip->item  = 5;
	ip++->value = 0xff;

	/* set RTS active */
	ip->item  = 6;
	ip++->value = 0xff;
	cp->resetflg = 1;
	cp->index = 2;

	/* issue the changes thru a reset command */
	StartConfigPortICC( cntl, port, cp );
	FreeBuffer(cntl,port);

}	/* end of m331modem */


/*
 * R E A D
 * =======
 *
 * call read routine in upper tty driver level via lineswitch
 *
 * Called by:	read device switch
 * Input:	device and arguments thru u area
 * Return:	-
 * Error:	-
 * Calls to:	linesw.read()
 *
 */

m331read( dev )
unsigned int dev;
{
	/* decoded cntl number from device number */
	int cntl;

	/* decoded port number from device number */
	int port;

	register struct tty *tp;

#ifdef HOWFAR
	if( m331Debug > 3 ) {
	    printf( "In m331read: dev=0x%x\n", dev );
	    printf( "Addr=0x%x Offset=%d Count=%d\n",
		u.u_base, u.u_offset, u.u_count );
	}
#endif

	/* decode the device */
	cntl = CNTRLR(dev);
	port  = dev &  0x07;

	/* get tty pointer */
	tp = &m331_tty[ m331_icc[cntl].c_firsttty + port ];

	(*linesw[tp->t_line].l_read)(tp);

}	/* end of m331read */


/*
 * W R I T E
 * =========
 *
 * call write routine in upper tty driver level via lineswitch
 *
 * Called by:	write device switch
 * Input:	device and arguments thru u area
 * Return:	-
 * Error:	-
 * Calls to:	linesw.write()
 *
 */

m331write( dev )
unsigned int dev;
{
	/* decoded cntl number from device number */
	int cntl;

	/* decoded port number from device number */
	int port;

	register struct tty *tp;

#ifdef HOWFAR
	if( m331Debug>3 ) {
	    printf( "In m331write: dev=0x%x\n", dev );
	    printf( "Addr=0x%x Offset=%d Count=%d\n",
		u.u_base, u.u_offset, u.u_count );
	}
#endif

	/* decode the device */
	cntl = CNTRLR(dev);
	port  = dev &  0x07;

	/* get tty pointer */
	tp = &m331_tty[ m331_icc[cntl].c_firsttty + port ];

	(*linesw[tp->t_line].l_write)(tp);

}	/* end of m331write */


/*
 * I O C T L
 * =========
 *
 * switch to special ioctls:
 *	MICCSTAT:	call request ICC status
 *	MPORTSTAT:	call port status
 *	MTABLESTAT:	call table status
 *	MHALTSAVE:	get  halt save table
 *	MSETHWHAND:	set hardware handshake mode
 *	MCLEARHWHAND:	clear hardware handshake mode
 *	MGETHWHAND:	get hardware handshake mode
 *
 * or to common tty ioctl and setting parameters
 *
 * Called by:	ioctl device switch
 * Input:	device, command, argument pointer, mode
 * Return:	-
 * Error:	-
 * Calls to:	StatICC(), PortStatICC(), TableStatICC(),
 *		ttiocom(), m331param(), copyin(), copyout(),
 *		GetBuffer(), FreeBuffer()
 *
 */

m331ioctl( dev, cmd, arg, mode )
unsigned int dev, cmd, arg, mode;
{
	/* decoded cntl number from device number */
	int cntl;

	/* decoded port number from device number */
	int port;

	/* buffer for ioctl data */
	char *ioctlbuf;

        /* buffer */
	struct ICCitem *ip;
	struct confbuf *cp;

	/* hardware handshake status */
	int  hwhandstat;

	register struct tty *tp;

#ifdef HOWFAR
	if( m331Debug>3 ) {
	    printf( "In m331ioctl: dev=0x%x, ", dev );
	    printf( "cmd=0x%x arg=0x%x, mode=0x%x\n",
		cmd, arg, mode );
	}
#endif
	/* decode the device */
	cntl = CNTRLR(dev);
	port  = dev &  0x07;

	switch( cmd ) {
	    case MICCSTAT:
/* SSM 05/20/86 - 1 */
		ioctlbuf = GetBuffer( cntl,port );
	        StatICC( cntl, ioctlbuf ) ;
	        if( copyout( ioctlbuf, arg, SIZEOFICCSTAT)) {
			u.u_error = EFAULT;
	        }
/* SSM 05/20/86 - 1 */
		FreeBuffer(cntl,port );
		break;

	    case MPORTSTAT:
/* SSM 05/20/86 - 1 */
		ioctlbuf = GetBuffer( cntl,port );
		PortStatICC( cntl, port, ioctlbuf );
		if( copyout( ioctlbuf, arg, SIZEOFPORTSTAT )) {
			u.u_error = EFAULT;
	        }
/* SSM 05/20/86 - 1 */
		FreeBuffer(cntl,port );
		break;

	    case MTABLESTAT:
/* SSM 05/20/86 - 1 */
		ioctlbuf = GetBuffer( cntl,port );
		/* get table item */
		if( copyin( arg, ioctlbuf, 1 )) {
			u.u_error = EFAULT;
/* SSM 07/02/86 - 1 */
			FreeBuffer(cntl,port );
			break;
	        }
		TableStatICC( cntl, port, ioctlbuf[0], ioctlbuf );
		if( copyout( ioctlbuf, arg, SIZEOFTABLESTAT )) {
			u.u_error = EFAULT;
	        }
/* SSM 05/20/86 - 1 */
		FreeBuffer(cntl,port );
		break;

/* SSM 05/27/86 - 1 */
	    case MSETHWHAND:
/* SSM 07/01/86 - 1 */
		/* get config table */
		cp = (struct confbuf *) GetBuffer(cntl,port);
		ip = cp->param;

	        m331_icc[cntl].c_stat[port] |= ICCHWHAND;
		ip->item  = 11;
		ip->value = 0; /* CTS inhibits the transmitter */
		cp->resetflg = 1;
		cp->index = 1;
		StartConfigPortICC( cntl, port ,cp);
		/* free config table */
		FreeBuffer(cntl,port);
	        break;

	    case MCLEARHWHAND:
/* SSM 07/01/86 - 1 */
		/* get config table */
		cp = (struct confbuf *) GetBuffer(cntl,port);
		ip = cp->param;

	        m331_icc[cntl].c_stat[port] &= ~ICCHWHAND;
		ip->item  = 11;
		ip->value = 0xff; /* CTS notes a solicited event */
		cp->resetflg = 1;
		cp->index = 1;
		StartConfigPortICC( cntl, port ,cp);
		/* free config table */
		FreeBuffer(cntl,port);
	        break;

	    case MGETHWHAND:
	        if ( m331_icc[cntl].c_stat[port] & ICCHWHAND ) {
			hwhandstat = 1;
		}
		else {
			hwhandstat = 0;
		}
		if( copyout( &hwhandstat, arg, sizeof( hwhandstat ) )) {
			u.u_error = EFAULT;
	        }
		break;

	    default:
		/* get tty pointer */
		tp = &m331_tty[ m331_icc[cntl].c_firsttty + port ];

		if (ttiocom(tp, cmd, arg, mode))
		{
			m331param(dev);
		}

	}
}	/* end of m331ioctl */


/*
 * I N T E R R U P T
 * =================
 *
 * Check which controller interrupts 
 * Call ISR of command channel
 * Handle completed requests:
 *	If read complete, then
 *		Call the read complete routine
 *		Call upper level of tty input driver
 *		Restart input, if line is still open
 *	If write complete, then
 *		Call the write complete routine
 *		Call mproc with T_OUTPUT to write restart
 *	If solicited event, then
 *		Restart the soliceted event packet
 *		If Break, then
 *			Call upper level of tty driver
 *		If DCD change, then
 *			If DCD lost, then signal it 
 *			If DCD got, then wakeup sleeping processes 
 *
 *
 * Called by:	interrupt handler
 * Input:	first port number of the controller, which interrupts
 * Return:	-
 * Error:	-
 * Calls to:	CCIntrService(), 
 *		SendSolicitedICC(), linesw.input(),
 *		m331proc(), signal(), ttyflush(),
 *
 */

m331intr( intrcntl )
int intrcntl;
{
	/* saved interrupt level */
	int s;

#ifdef NOT_FINAL_VERSION
	/* intermediate save of status code returned by mvme331 f/w */
	int savestat;
#endif /* NOT_FINAL_VERSION */

	/* tty structure */
	struct tty *tp;

	/* completed request packets */
	struct CCpacket *pkcompl[ISRMAXRET];

	/* count of completed requests, which were not wakedup */
	int retcount;

	/* pointer to status of port */
	int *portstat;

	/* loop count */
	register int i;

	/* controller number */
	register int cntl;

	/* character queue description */
	register struct ccblock *tbuf;

	/* returned length by request */
	int retlength;

	extern int dcd_scan();

#ifdef HOWFAR
	if( m331Debug>3 )
	    printf( "In m331intr: 0x%x\n", intrcntl );
#endif

	/* get controller number */
	cntl = tpdev[intrcntl].t_cntl;

	/* only one interrupt is allowed */
	s = splev( m331_ilev[cntl] );

	InM331Intr = 1;

	/* call the command channel ISR */
	do {
	    retcount = CCIntrService( cntl, pkcompl );

#ifdef HOWFAR 
	    if( m331Debug>3 ) {
		printf( "m331intr: retcount=%d\n", retcount);
		for( i=0; i<retcount; ++i ) { 
		    printf( "#%d: pk=0x%x\n", i, pkcompl[i] );
		}
	    }
#endif
	    /* loop thru all completed requests */
	    for( i=0; i<retcount; i++ ) {

#ifdef NOT_FINAL_VERSION

		if( (savestat=pkcompl[i]->pk_stat) == 0xff ) {
			printf( "Internal Error in m331intr: retcount=%d\n",
				retcount );

			printf( "m331intr: savestat=%d\n",savestat);

			PrintPacket( pkcompl[i] );
			return;
		}
#endif /* NOT_FINAL_VERSION */


		/* tty pointer of request */
		tp = &m331_tty[m331_icc[cntl].c_firsttty+(pkcompl[i]->pk_port-1)];
		
		portstat = &m331_icc[cntl].c_stat[pkcompl[i]->pk_port-1];

		retlength = pkcompl[i]->pk_rlen;

		/* READ complete ? */
	        if( pkcompl[i]->pk_cmd == CMD_READ ) {

		    *portstat &= ~ICCREAD;

		    /* set the pointer of the clist buffer */
			    tbuf = &tp->t_rbuf;

			    /* check for parity errors */
			    if( !(tp->t_iflag & IGNPAR ) ) {
				if( (tp->t_iflag & INPCK ) ) {
				    if( pkcompl[i]->pk_stat & 0x80 ) {
					if( tp->t_iflag & PARMRK ) {
					    tbuf->c_ptr[retlength++] = 0xff;
					    tbuf->c_ptr[retlength++] = 0x00;
					    tbuf->c_ptr[retlength++] 
						      = pkcompl[i]->pk_term;
					}
					else {
					    tbuf->c_ptr[retlength++] = 0x00;
					}
				    }
				}
			    }

  		    if(retlength > 0)
		    {
		        tbuf->c_count -= retlength;
			sysinfo.rcvint ++;
/* SSM 07/22/86 - 2 */
			if (!(tp->t_state & TBLOCK))
				(*linesw[tp->t_line].l_input)(tp, L_BUF );
                        else
				tp->t_state |= READBLOCKED;
		    }

/* SSM 07/22/86 - 2 */
		    if( (tp->t_state&ISOPEN) && !(tp->t_state&TBLOCK) ) {
		        m331proc( tp, T_INPUT );
	            }

		}

		/* WRITE completion ? */
	        if( pkcompl[i]->pk_cmd == CMD_WRITE ) {

/* SSM 06/12/86 - 1 */
		    sysinfo.xmtint ++;
		    tp->t_state &= ~BUSY;
		    m331proc( tp, T_OUTPUT );
		}

		/* SEND SOLICITED EVENT completion ? */
	        if( pkcompl[i]->pk_cmd == CMD_SNDSOLEVENT ) {

		    /* restart the solicited event packet */
		    SendSolicitedICC(  cntl, pkcompl[i]->pk_port-1 );

		    /* the solicited event state is the return length */

		    /* BREAK ? */
		    if( retlength & BREAKEVENT ) {
		        if( !(tp->t_iflag&IGNBRK) 
			  && (tp->t_iflag&BRKINT) ) {
			    (*linesw[tp->t_line].l_input)(tp, L_BREAK);
			}
		    }

		    /* DCD change ? */
		    if( retlength & DCDEVENT ) 
		    {
			/* mark the port dcd status */
			dcd_status[tp-m331_tty] = (retlength&DCDSTAT);

			/* reset the counter */
			dcd_event[tp-m331_tty] = 1;

			if(!dcd_timeout)
			{
				/* only start one scan */
				dcd_timeout = 1;
				timeout(dcd_scan, 0, dcd_tick);
			}
/* SSM 05/23/86 - 1 */
		    }

		    /* CTS change ? */
		    if( retlength & CTSEVENT ) {
			/* fall thru, do nothing */
		    }

		}
	    }
	}
	/* repeat if the queue was full */
	while( retcount == ISRMAXRET );


	InM331Intr = 0;

	/* end of critical phase */
	splx(s);

} /* end of m331intr */
/*
	scan all the ports for a change of DCD

	if the port has reached terminal count
	then check for carrier on
	otherwise just increment the counter

	if a change is detected on any port
	the scan is started again
*/
dcd_scan()
{
	register int cnt;
	register struct tty *tp = &m331_tty[0];
	int start_timeout = 0;
	int saveintrflag = InM331Intr;

	InM331Intr = 1;
	for(cnt = 0; cnt < m331_cnt; cnt++, tp++)
	{
		if( dcd_event[cnt] )
		{
			PRT2("port %x DCD changed:%x\n", cnt, dcd_status[cnt]);
			start_timeout = 1;
			if(dcd_event[cnt]++ >= MAXDCDTIME)
			{
				dcd_event[cnt] = 0;

				if(!( dcd_status[cnt]) &&
					!(tp->t_cflag&CLOCAL) ) 
				{
				    if( tp->t_state & CARR_ON ) 
				    {
					/* signal the change to the processes */
					if( ( tp->t_state & ISOPEN ) )
					{
						signal( tp->t_pgrp, SIGHUP );
						ttyflush(tp, (FREAD|FWRITE) );
					}
					tp->t_state &= ~CARR_ON;
				    }
				}
				else 
				{
				    if( !(tp->t_state & CARR_ON )) 
				    {
					wakeup( &tp->t_canq );
					tp->t_state |= CARR_ON;
				    }
				}
			}	
		}
	}
	if(start_timeout)
		timeout(dcd_scan, 0, MAXDCDTIME);
	else
		dcd_timeout = 0;
	InM331Intr = saveintrflag;
}



/*
 * SET PARAMETER OF THE TTY LINE
 *
 * check if port is valid
 * do following changes
 *	- hang up, if necessary
 *	- set baud rate
 *	- set character size
 *	- set number of stop bits
 *	- set type of parity
 *	- set DTR and RTS active
 *	- set receiver state
 *	- set input break state
 *	- set handshake controls
 *	- enable transmitter
 *	- issue the changes thru a reset cmd
 *
 * Called by:	m331open(), m331ioctl(), m331proc()
 * Input:	device
 * Return:	ENXIO in u.u_error if device not active
 * Error:	if device not active
 * Calls to:	StartConfigPortICC(),
 *		SWHandShakeICC()
 *
 */

m331param(dev)
unsigned int dev;
{
	/* decoded cntl number from device number */
	register int cntl;

	/* decoded port number from device number */
	register int port;

	/* current tty structure */
	register struct tty *tp;

	/* current flags of the tty line */
	register int flags;

	/* baud rate from the speed table */
	int baud_rate;

	/* bits per character byte */
	int charsize;

        /* buffer */
	struct ICCitem *ip;
	struct confbuf *cp;


#ifdef HOWFAR
	if( m331Debug>3 )
	    printf( "In m331param: dev=0x%x\n", dev );
#endif
	
	/* decode the device */
	cntl = CNTRLR(dev);
	port  = dev &  0x07;

	/* check if this port is valid and initialized */
	if( (cntl >= MAXICCS) || (port>=MAXPORTS) ) {
	    u.u_error = ENXIO;
	    return;
	}
	else {
	    if( !(m331_icc[cntl].c_stat[port] & ICCINIT ) ) {
		u.u_error = ENXIO;
	    	return;
	    }
	}

	/* get tty pointer */
	tp = &m331_tty[ m331_icc[cntl].c_firsttty + port ];

	/* get current flags */
	flags = tp->t_cflag;

#ifdef NOT_FINAL_VERSION
	if(tp->t_state & BUSY)
	    printf("Warning: m331param: changed dev %d while in state %x\n", 
	    dev, tp->t_state);
#endif

	/* hang up on zero baud rate */
	if ((flags & CBAUD) == B0) {
/* SSM 07/01/86 - 1 */
	    /* get config table */
	    cp = (struct confbuf *) GetBuffer(cntl,port);
	    ip = cp->param;

	    tp->t_state &= ~BUSY;	/* guarantee not busy since XID */
	    /* set DTR inactive */
	    ip->item  = 5;
	    ip++->value = 0;

	    /* set RTS inactive */
	    ip->item  = 6;
	    ip++->value = 0;
	    cp->resetflg = 1;
	    cp->index = 2;

	    /* issue the changes thru a reset command */
	    StartConfigPortICC( cntl, port,cp );
	    FreeBuffer(cntl,port);

	    return;
	}

/* SSM 06/02/86 - 1 */
	if ( !(tp->t_cflag&CLOCAL) && (GetDCDICC(cntl, port) == 0) ) 
		tp->t_state &= ~CARR_ON;
	else
	{
		tp->t_state |= CARR_ON;
		wakeup( &tp->t_canq );
	}

/* SSM 07/01/86 - 1 */
	/* get config table */
	cp = (struct confbuf *) GetBuffer(cntl,port);
	ip = cp->param;
	/* set baud rate */
	baud_rate = m331_speeds[ flags & CBAUD ];

	if( baud_rate ) {		/* skip if not implemented */
		ip->item  = 1;
		ip++->value = baud_rate;
	}
	else /* dummy configure */
	{
		ip->item  = 13;
		ip++->value = 0;
	}	

	/* set character size */
	switch( flags & CSIZE ) {
	    case CS5:	charsize = 5; break;
	    case CS6:	charsize = 6; break;
	    case CS7:	charsize = 7; break;
	    case CS8:	charsize = 8; break;
	    default:	charsize = 8; 
	}
	ip->item  = 2;
	ip++->value = charsize;

	/* set number of stop bits */
	if( flags & CSTOPB ) {		/* 2 stop bits */
		ip->item  = 3;
		ip++->value = 2;
	}
	else {				/* 1 stop bit */
		ip->item  = 3;
		ip++->value = 1;
	}

	/* set type of parity */
	if( flags & PARENB ) {		/* parity enable */
	    if( flags & PARODD ) {	/* odd parity */
		ip->item  = 4;
		ip++->value = 1;
	    }
	    else {			/* even parity */
		ip->item  = 4;
		ip++->value = 2;
	    }
	}
	else {				/* no parity */
	    ip->item  = 4;
	    ip++->value = 0;
	}

/* SSM 05/27/86 - 1 */
	if(m331_icc[cntl].c_stat[port] & ICCHWHAND)
	{
		/* CTS inhibits the transmitter */
		ip->item  = 11;
		ip++->value = 0;
	}
	else
	{
		/* CTS notes a solicited event */
		ip->item  = 11;
		ip++->value = 0xff;
	}
	    
	/* set DTR active */
	ip->item  = 5;
	ip++->value = 0xff;

	/* set RTS active */
	ip->item  = 6;
	ip++->value = 0xff;

	/* set receiver state flag */
	if( flags & CREAD ) {		/* enable */
	    ip->item  = 7;
	    ip++->value = 0xff;
	}
	else {				/* disable */
	    ip->item  = 7;
	    ip++->value = 0;
	}

	/* get current input flags */
	flags = tp->t_iflag;

	/* set break state flag */
	if( flags & IGNBRK ) {		/* disable */
	    ip->item  = 9;
	    ip++->value = 0;
	}
	else {				/* enable */
	    ip->item  = 9;
	    ip++->value = 0xff;
	}
	/* set handshake modes */
	/* output restart at any character ? */
	if( flags & IXANY ) {		/* enable */
	    ip->item  = 33;
	    ip++->value = 0xff;
	}
	else {				/* disable */
	    ip->item  = 33;
	    ip++->value = 0;
	}

/* SSM 05/27/86 - 1 */
	/* input control */
	if( (flags & IXOFF) ) { /* enable */
	    ip->item  = 29;
	    ip++->value = 1;
	}
	else {				/* disable */
	    ip->item  = 29;
	    ip++->value = 0;
	}

	/* enable transmitter */
	ip->item  = 8;
	ip++->value = 0xff;
	cp->resetflg = 1;
	cp->index = 12;

	/* issue the changes thru a reset command */
	StartConfigPortICC( cntl, port, cp );
	FreeBuffer(cntl,port);

	/* output control */
/* SSM 05/20/86 - 2 */
	/* OLD : if( (flags & IXON) && !(tp->t_cflag & CLOCAL) ) { */
	if(flags & IXON)  { /* enable */
	    SWHandShakeICC( cntl, port, 0xff );
	}
	else {				/* disable */
	    SWHandShakeICC( cntl, port, 0x00 );
	}
/* SSM 07/01/86 - 1 */
	ConfigRTTTable(cntl,port,flags);

}	/* end of m331param */


/*
 * PROCESSING OUTPUT DEVICE DEPENDENT
 * ==================================
 *
 * handle the given command
 * at output command give the whole buffer to the controller
 *
 * Called by:	ttout(), m331intr()
 * Input:	tty pointer, command
 * Return:	-
 * Error:	-
 * Calls to:	WriteICC(),SendBreakICC(), linesw.output(), m331param()
 *
 */

m331proc(tp, cmd )
register struct tty *tp;
unsigned int cmd;
{
	/* decoded cntl number from device number */
	register int cntl;

	/* decoded port number from device number */
	register int port;

	/* index in tty structure table */
	unsigned int dev;

        int retvalue;

	/* get cntl and port from tp structure */
	dev = tp - m331_tty;
	cntl = tpdev[dev].t_cntl;
	port = tpdev[dev].t_port;

#ifdef HOWFAR
	if (m331Debug>3)
		printf("In m331proc: tp=0x%x, cntl=%d, port=%d, cmd=%d\n", 
			tp, cntl, port, cmd );
#endif


	switch(cmd)
	{
	case T_TIME:		/* resuming output after delay */
		tp->t_state &= ~TIMEOUT;
		goto start;

	case T_WFLUSH:		/* flush output queue */
		tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
		tp->t_tbuf.c_count = 0;

		/* fall thru */

	case T_RESUME:		/* resume output, XON has been received */
		tp->t_state &= ~TTSTOP;
		/* fall thru */

	case T_OUTPUT:		/* handle first or next character */
	start:
	{
		register struct ccblock *tbuf;

		if (tp->t_state & (BUSY|TTSTOP|TIMEOUT))
		{
			break;
		}

		tbuf = &tp->t_tbuf;
		if (tbuf->c_ptr == NULL || tbuf->c_count == 0)
		{
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
			if (!(CPRES & (*linesw[tp->t_line].l_output)(tp)))
			{
				break;
			}
		}
		tp->t_state |= BUSY;

		/* start output of the whole buffer */
/* SSM 06/12/86 - 1 */
		WriteICC( cntl, port, tbuf->c_ptr, tbuf->c_count );

		tbuf->c_ptr += tbuf->c_count;
		tbuf->c_count = 0;

		break;
	}

	case T_SUSPEND:		/* stop output, XOFF has been received */
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:		/* the intelligent controller should 
				   handle the handshake not the driver.
				   We will not send XOFF  */
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK|TTXOFF;
		break;

	case T_RFLUSH:	/* flush input queue.  if input is blocked, send XON */
		/* flush only, if we are not in an interrupt state,
		   because we will sleep, until the command is completed
		   by the ICC. But sleep is not allowed, when we are in 
		   an interrupt state. */
		if( !InM331Intr )
			if((retvalue = CCDoCmd(CMD_RCVFLUSH,cntl,port+1,NULL,0,
				&dummy, MREAD|MSLEEP )))
				ProcessErrors(retvalue,cntl,port,"FlushReceiveICC");

		if (!(tp->t_state & TBLOCK))
			break;
		/* fall thru */

	case T_UNBLOCK:		/* the intelligent controller should 
				   handle the handshake not the driver
				   we will not send XON  */

		tp->t_state &= ~(TTXOFF | TBLOCK);
		tp->t_state |= TTXON;
/* SSM 07/22/86 - 2 */
                if(tp->t_state & READBLOCKED) {
				(*linesw[tp->t_line].l_input)(tp, L_BUF );
				tp->t_state &= ~READBLOCKED;
		}
		/* fall thru */

	case T_INPUT:		/* start input */
	{
		register struct ccblock *tbuf;
	
		int count;

/* SSM 06/23/86 - 1 */

		/* if a read request is in progress, do nothing */
		if( !(m331_icc[cntl].c_stat[port] & ICCREAD ) ) {

		    tbuf = &tp->t_rbuf;

		    /* request until end of the raw queue */
/* SSM 06/23/86 - 2 */
		    if( (count=(TTYHOG+1)-tp->t_rawq.c_cc) > tbuf->c_count) {

			/* for PARMRK situations there must be space for
			   2 additional characters */
			count = tbuf->c_count - 2;

			/* in PARMRK and !ISTRIP mode the char. 0xff
			   will be doubled by the driver, so there must
			   be space for these expansion. The worst case
			   is that any character is a 0xff, so the space
			   in the buffer is only half of the free size */
			if(  ( tp->t_iflag & PARMRK ) 
			 && !( tp->t_iflag & ISTRIP ) ) {
			    count = count/2;
			}
		    }

		    if( count > 0 )  {
		    	/* start the read request */
		    	m331_icc[cntl].c_stat[port] |= ICCREAD;

		        ReadICC( cntl, port, tbuf->c_ptr, count );
		    }

		}
		break;
	}

	case T_BREAK:		/* send break condition for 1/4 second */
		SendBreakICC( cntl, port );
		break;

	case T_PARM:		/* unused; new for R5.2; issued by sxt-driver*/
		m331param(dev);
		break;
	}
}	/* end of m331proc */



/*------------------------------------------------------------------------*/
/*
 *	S U B R O U T I N E S    F O R   T H E
 *      ======================================
 *	DRIVER FOR MOTOROLA VME BOARD   M V M E  3 3 1 / 3 3 2
 *	======================================================
 *
 *	
 *	Implemented:					checked?
 *	------------
 *	ReadICC		read ICC command start		03/07/85
 *	WriteICC	write ICC command start		03/07/85
 *	SendSolicitedICC send solicited event start	03/07/85
 *	ConfigICC	configure ICC command		03/07/85
 *	ConfPortsCntICC	configure portscount command	10/21/85
 *	ConfigRTTTable	configure table command		03/07/85
 *	StatICC		request ICC status command	03/07/85
 *	PortStatICC	request port status command	03/07/85
 *	TableStatICC	request table status command	03/07/85
 *	ResetPortICC	reset port command		03/07/85
 *	SendBreakICC	send break			03/07/85
 *
 *	InitPortICC	initialize the port config.	03/07/85
 *	GetDCDICC	get state of DCD line		03/07/85
 *	SWHandShakeICC  set software handshake on/off	03/07/85
 *	StartConfigPortICC start configure port command	03/07/85
 *	GetBuffer	get buffer           		03/07/85
 *	FreeBuffer	free buffer           		03/07/85
 *	ProcessErrors	do error processing		03/07/85
 *
 *
 */


/* DEFINITIONS */

/* indices for the icc configure table */
#define	FREQITEM	1	/* processor frequency */
#define	TSLICEITEM	2	/* time slice period */
#define	IVECITEM	3	/* interrupt vector number */
#define	ILEVELITEM	4	/* interrupt level number */
#define	NPORTSITEM	5	/* number of ports used */
#define	HALTITEM	6	/* halt save address */
#define MAXRETITEM	7	/* max retry count for bus errors */

/* indices for the port status table */
#define	DCDITEM	35	/* DCD input state byte */


/* CONFIGURATION */


/* EXTERNALS */

/* tty structures for this driver */
extern struct tty m331_tty[];

struct CCpacket *CCPkGetFree();


/* GLOBALS */

/* array for configure items */
struct ICCitem DoConfigTable[ MAXITEMS ];

/* index in DoConfigTable */
int ConfigIndex;

/* SSM 05/20/86 - 1 */
extern char PortBuf[];

/* SSM 05/20/86 - 1 */
/* busy flag for PortBuf */
int BufBusy[MAXPORTS*MAXICCS];

/* flag, if after port configuring a reset is necessary */
int ResetConfigFlag;

#ifdef HOWFAR
/* flag for printing debug messages */
int msubDebug = 0;
#endif


/* ERROR CODES */
char ErrorStrings[22][60] = {
	"Normal return (no error)",
	"Terminated by character count",
	"Terminated by READ timeout",
	"Terminated by WRITE timeout",
	"Terminated by ABORT command",
	"Insufficient memory available",
	"Invalid ITEM number",
	"Invalid baud rate",
	"Invalid bits per character",
	"Invalid stop bits",
	"Invalid type of parity",
	"Invalid XON/XOFF/RTS/DTR selection",
	"Terminated by RX queue overrun",
	"Invalid delete character string length",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"Terminated by read error",
	"RESERVED",
	"RESERVED",
	"Terminated on at least one character being received",
	"Terminated by buffer length"
};

char IntErrStrings[3][60] = {
	"Driver not initialized",
	"Controller not initialized",
	"Port not initialized",
};


/* standard port configuration */
struct ICCitem PortConfigTable[] = {
	{ 1,	9600	},	/* 9600 bd */
	{ 2,	8	},	/* 8 bits per char. */
	{ 3,	1	},	/* 1 stop bit */
	{ 4,	0	},	/* no parity */
	{ 5,	0xff	},	/* DTR active */
	{ 6,	0xff	},	/* RTS active */
	{ 7,	0xff	},	/* receiver enabled */
	{ 8,	0xff	},	/* transmitter enabled */
	{ 9,	0x00	},	/* BREAK disabled */
	{ 10,	0xff	},	/* DCD change --> event */
	{ 11,	0xff	},	/* CTS change --> event */
	{ 12,	75000	},	/* read timeout */
	{ 13,	0	},	/* write timeout disabled */
	{ 14,	250	},	/* BREAK time is 250 ms */
	{ 15,	0x03082008 },	/* DELETE string is "\b \b" */
	{ 16,	0	},	/* do not echo control char. */
	{ 17,	'^'	},	/* control indicator char. */
	{ 18,	1	},	/* terminate on 1 char. */
	{ 19,	0	},	/* no hardcopy string */
	{ 20,	256	},	/* line width count */
	{ 21,	0	},	/* term. on buffer full */
	{ 22,	0	},	/*  null padding count = 0 */
	{ 23,	0	},	/* disable TRASH */
	{ 24,	0	},	/* do not flush at write start*/
	{ 25,	0	},	/* do not flush at read start */
	{ 26,	0x0d000000 },	/* write term. string = "\r" */
	{ 27,	0x13	},	/* XOFF char is DC3 */
	{ 28,	0x11	},	/* XON  char is DC1 */
	{ 29,	0x01	},	/* use XOFF/XON for handshake */
	{ 30,	0x0d0a0000 },	/* read terminator string */
	{ 31,	0x07	},	/* bell character is BEL */
	{ 32,	0x00	},	/* slash character is NULL */
	{ 33,	0xff	},	/* use any char. to enable TX */
	{ 34,	180	},	/* RX high water mark is 180 */
	{ 35,	80	},	/* RX low water mark is 80 */
};


/*
 * READ ICC COMMAND START
 * ======================
 *
 * if packet not initialized, then do it
 * start command, but do not wait for completion
 *
 * Called by:	m331proc()
 * Input:	controller, port,
 *		pointer to the buffer, count of characters
 * Return:	-
 * Error:	-
 * Calls to:	CCPkGetFree(), CCStartCmd()
 *
 */

ReadICC( cntl, port, bufferpt, count )
int cntl;
int port;
char *bufferpt;
int count;
{
	/* command channel data packet */
	register struct CCpacket *pk;

#ifdef HOWFAR
	if( msubDebug > 3 ) {
	    printf( "In ReadICC:" );
	    printf( "cntl=%d, port=%d, bufferpt=0x%x, count=%d\n",
		    cntl, port, bufferpt, count );
	}
#endif

	/* if read packet not initialized, do it */
	if( !(pk=m331_icc[cntl].c_pkread[port]) ) {

	    /* get free packet */
	    pk = CCPkGetFree();
	    m331_icc[cntl].c_pkread[port] = pk;

	    /* initialize parameters */
	    pk->pk_cmd = CMD_READ;
	    pk->pk_port = port+1;

	}

	/* set request parameters */
	pk->pk_len = count;
	pk->pk_stat = 0xff;
	pk->pk_term = 0xff;
	pk->pk_rlen = 0;
	pk->pk_sad = bufferpt;

	/* start the command, do not sleep on completion */
	CCStartCmd( cntl, pk, MREAD );

}	/* end of ReadICC */


/*
 * WRITE ICC COMMAND START
 * =======================
 *
 * if packet not initialized, then do it
 * start command, but do not wait for completion
 *
 * Called by:	m331proc()
 * Input:	controller, port,
 *		pointer to the buffer, count of characters
 * Return:	-
 * Error:	-
 * Calls to:	CCPkGetFree(), CCStartCmd()
 *
 */

WriteICC( cntl, port, bufferpt, count )
int cntl;
int port;
char *bufferpt;
int count;
{
	/* command channel data packet */
	register struct CCpacket *pk;

#ifdef HOWFAR
	if( msubDebug > 3 ) {
	    printf( "In WriteICC:" );
	    printf( "cntl=%d, port=%d, bufferpt=0x%x, count=%d\n",
		    cntl, port, bufferpt, count );
	 }
#endif

	/* if write packet not initialized, do it */
	if( !(pk=m331_icc[cntl].c_pkwrite[port]) ) {

	    /* get free packet */
	    pk = CCPkGetFree();
	    m331_icc[cntl].c_pkwrite[port] = pk;

	    /* initialize parameters */
	    pk->pk_cmd = CMD_WRITE;
	    pk->pk_port = port+1;

	}

	/* set request parameters */
	pk->pk_len = count;
	pk->pk_stat = 0xff;
	pk->pk_term = 0xff;
	pk->pk_rlen = 0;
	pk->pk_sad = bufferpt;

	/* start the command, do not sleep on completion */
	CCStartCmd( cntl, pk, MWRITE);

}	/* end of WriteICC */


/*
 * SEND SOLICITED EVENT
 * ====================
 *
 * if packet not initialized, then do it
 * start command, but do not wait for completion
 *
 * Called by:	InitPortICC(), m331intr()
 * Input:	controller, port
 * Return:	-
 * Error:	-
 * Calls to:	CCPkGetFree(), CCStartCmd()
 *
 */

SendSolicitedICC( cntl, port )
int cntl;
int port;
{
	/* command channel data packet */
	struct CCpacket *pk;

#ifdef HOWFAR
	if( msubDebug > 3 ) {
	    printf( "In SendSolicitedICC: cntl=%d, port=%d\n",
		    cntl, port );
	}
#endif

	/* if solicited event packet not initialized, do it */
	if( !(pk=m331_icc[cntl].c_pksndsol[port]) ) {

	    /* get free packet */
	    pk = CCPkGetFree();
	    m331_icc[cntl].c_pksndsol[port] = pk;

	    /* initialize parameters */
	    pk->pk_cmd  = CMD_SNDSOLEVENT;
	    pk->pk_port = port+1;
	    pk->pk_sad  = 0;

	}

	/* set request parameters */
	pk->pk_len = 0;
	pk->pk_stat = 0xff;
	pk->pk_term = 0xff;
	pk->pk_rlen = 0;

	/* start the command, do not sleep on completion */
	CCStartCmd( cntl, pk, MREAD );

}	/* end of SendSolicitedICC */


/*
 * CONFIGURE ICC COMMAND
 * =====================
 *
 * construct the item table from arguments
 * start command
 * do error processing
 *
 *
 * Called by:	m331open()
 * Input:	controller, 
 *		interrupt vector, interrupt level
 * Return:	error code from command
 * Error:	terminated by abort 
 *		or invalid item number
 * Calls to:	CCDoCmd(), ProcessErrors()
 *
 */

ConfigICC( cntl, intrvector, intrlevel )
int cntl;
int intrvector;
int intrlevel;
{
	/* return status code */
	int retvalue;

        /* buffer */
	struct ICCitem *ip;
	struct confbuf *cp;

#ifdef HOWFAR
	if( msubDebug > 3 ) {
	    printf( "In ConfigICC:" );
	    printf( "cntl=%d, vec=0x%x, lev=0x%x\n",
		    cntl, intrvector, intrlevel );
	}
#endif
/* SSM 07/01/86 - 1 */
	/* get config table */
	cp = (struct confbuf *) GetBuffer(cntl,0);
        ip = cp->param;

	ip->item  = IVECITEM;
	ip++->value = intrvector;

	ip->item  = ILEVELITEM;
	ip++->value = intrlevel;

	/* HALT save address set */
	ip->item  = HALTITEM;
	ip++->value = (int)(m331_haltsave)+cntl*SIZEOFHALTSAVE;	

	ip->item  = MAXRETITEM;
	ip++->value = 1000;	/* set max bus error retry count to 1,000 */

	/* do the command */
	retvalue = CCDoCmd( CMD_CFICC, cntl, 1,
		(caddr_t)cp, 3*sizeof( struct ICCitem ), 
		&dummy, MWRITE|MSLEEP );
	
/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, 1, "ConfigICC" );

	/* free config table */
	FreeBuffer(cntl,0);

	return( retvalue );

}	/* end of ConfigICC */


/*
 * CONFIGURE PORTSCOUNT ICC COMMAND
 * ================================
 *
 * construct the item table from arguments
 * start command
 * do error processing
 *
 *
 * Called by:	m331open()
 * Input:	controller, portscount 
 * Return:	error code from command
 * Error:	terminated by abort 
 *		or invalid item number
 * Calls to:	CCDoCmd(), ProcessErrors()
 *
 */

ConfPortsCntICC( cntl, portscount )
int cntl;
int portscount;
{
	/* return status code */
	int retvalue;

        /* buffer */
	struct ICCitem *ip;
	struct confbuf *cp;

#ifdef HOWFAR
	if( msubDebug > 3 ) {
	    printf( "In ConfPortsCntICC:" );
	    printf( "cntl=%d, cnt=%d\n",
		    cntl, portscount );
	}
#endif
/* SSM 07/01/86 - 1 */
	/* get config table */
	cp = (struct confbuf *) GetBuffer(cntl,0);
        ip = cp->param;

	ip->item  = NPORTSITEM;
	ip->value = portscount;

	/* do the command */
	retvalue = CCDoCmd( CMD_CFICC, cntl, 1,
		(caddr_t)cp, 1*sizeof( struct ICCitem ), 
		&dummy, MWRITE|MSLEEP );
	
/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, 1, "ConfPortsCntICC" );

	/* free config table */
	FreeBuffer(cntl,0);

	return( retvalue );

}	/* end of ConfPortsCntICC */


/*
 * CONFIGURE TABLE
 * ===============
 *
 * start command
 * do error processing
 *
 *
 * Called by:	PortInitICC()
 * Input:	controller, port, table length,
 * 		pointer for the table
 * Return:	error code from command
 * Error:	terminated by abort 
 * Calls to:	CCDoCmd(), ProcessErrors(), 
 *
 */

ConfigRTTTable( cntl, port, flags )
int cntl;
int port;
int flags;
{
	int retvalue,i;
	char *tblbuf;

	tblbuf = GetBuffer( cntl,port);

	/* get RTT table */
	if((retvalue = CCDoCmd( CMD_STTABLE, cntl, port+1, tblbuf + 2, 4,
		&dummy, MREAD|MSLEEP )) != 0) 
		ProcessErrors( retvalue, cntl, port, "TableStatICC" );

	if( flags & ISTRIP ) {		/* do strip */
	    for( i=130; i<258; ++i ) {
		tblbuf[i] &= 0x7f;
	    }
	}
	else {				/* do not strip */
	    for( i=130; i<258; ++i ) {
		tblbuf[i] |= 0x80;
	    }
	}

	/* set the RTT table */
	tblbuf[0] = 0;
	tblbuf[1] = 4;

	if((retvalue = CCDoCmd( CMD_CFTABLE, cntl, port+1, tblbuf,
		SIZEOFTABLESTAT +2, &dummy, MWRITE|MSLEEP )) != 0)
		ProcessErrors( retvalue, cntl, port, "ConfigRTTTable" );

	FreeBuffer(cntl,port );
}	/* end of ConfigRTTTable */


/*
 * REQUEST ICC STATUS
 * ==================
 *
 * start command
 * do error processing
 *
 *
 * Called by:	m331ioctl()
 * Input:	controller, pointer for the status
 * Return:	error code from command
 * Error:	terminated by abort 
 * Calls to:	CCDoCmd(), ProcessErrors(), 
 *
 */

StatICC( cntl, statuspt )
int cntl;
char *statuspt;
{
	/* return status code */
	int retvalue;

#ifdef HOWFAR
	if( msubDebug > 3 )
	    printf( "In StatICC: cntl=%d, statuspt=0x%x\n",
	    	    cntl, statuspt );
#endif


	/* do the command */
	retvalue = CCDoCmd( CMD_STICC, cntl, 1,
		statuspt, SIZEOFICCSTAT,
		&dummy, MREAD|MSLEEP );
	
/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, 0, "StatICC" );

	return( retvalue );

}	/* end of StatICC */


/*
 * REQUEST PORT STATUS
 * ===================
 *
 * start command
 * do error processing
 *
 *
 * Called by:	m331ioctl()
 * Input:	controller, port,
 * 		pointer for the status
 * Return:	error code from command
 * Error:	terminated by abort 
 * Calls to:	CCDoCmd(), ProcessErrors(), 
 *
 */

PortStatICC( cntl, port, statuspt )
int cntl;
int port;
char *statuspt;
{
	/* return status code */
	int retvalue;

#ifdef HOWFAR
	if( msubDebug > 3 )
	    printf( "In PortStatICC: cntl=%d, port=%d, statuspt=0x%x\n",
		    cntl, port, statuspt );
#endif

	/* do the command */
	retvalue = CCDoCmd( CMD_STPORT, cntl, port+1,
		statuspt, SIZEOFPORTSTAT,
		&dummy, MREAD|MSLEEP );
	
/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, port, "PortStatICC" );

	return( retvalue );

}	/* end of PortStatICC */


/*
 * REQUEST TABLE STATUS
 * ====================
 *
 * start command
 * do error processing
 *
 *
 * Called by:	m331ioctl()
 * Input:	controller, port, table item
 * 		pointer for the status
 * Return:	error code from command
 * Error:	terminated by abort 
 * Calls to:	CCDoCmd(), ProcessErrors(), 
 *
 */

TableStatICC( cntl, port, tblitem, statuspt )
int cntl;
int port;
int tblitem;
char *statuspt;
{
	/* return status code */
	int retvalue;

#ifdef HOWFAR
	if( msubDebug > 3 ) {
	    printf( "In TableStatICC:" );
	    printf( "cntl=%d port=%d tblitem=%d statuspt=0x%x\n",
		    cntl, port, tblitem, statuspt );
	}
#endif

	/* do the command */
	retvalue = CCDoCmd( CMD_STTABLE, cntl, port+1,
		statuspt, tblitem,
		&dummy, MREAD|MSLEEP );
	
/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, port, "TableStatICC" );

	return( retvalue );

}	/* end of TableStatICC */


/*
 * RESET PORT
 * ==========
 *
 * start command
 * do error processing
 *
 *
 * Called by:	m331open()
 * Input:	controller, port,
 * Return:	error code from command
 * Error:	terminated by abort,
 *		invalid baud rate,
 *		invalid bits per character,
 *		invalid number of stop bits,
 *		invalid type of parity
 * Calls to:	CCDoCmd(), ProcessErrors(), 
 *
 */

ResetPortICC( cntl, port )
int cntl;
int port;
{
	/* return status code */
	int retvalue;

#ifdef HOWFAR
	if( msubDebug > 3 )
	    printf( "In ResetPortICC: cntl=%d, port=%d\n",
		    cntl, port );
#endif


	/* do the command */
	retvalue = CCDoCmd( CMD_RESET, cntl, port+1,
		NULL, 0,
		&dummy, MREAD|MSLEEP );
	
/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, port, "ResetPortICC" );

	return( retvalue );

}	/* end of ResetPortICC */


/*
 * SEND BREAK
 * ==========
 *
 * start command
 * do error processing
 *
 *
 * Called by:	m331proc()
 * Input:	controller, port,
 * Return:	error code from command
 * Error:	terminated by abort,
 * Calls to:	CCDoCmd(), ProcessErrors()
 *
 */

SendBreakICC( cntl, port )
int cntl;
int port;
{
	/* return status code */
	int retvalue;

#ifdef HOWFAR
	if( msubDebug > 3 )
	    printf( "In SendBreakICC: cntl=%d, port=%d\n",
		    cntl, port );
#endif

	/* do the command */
	retvalue = CCDoCmd( CMD_SNDBREAK, cntl, port+1,
		NULL, 0,
		&dummy, MREAD|MSLEEP );
	
/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, port, "SendBreakICC" );

	return( retvalue );

}	/* end of SendBreakICC */


/*
 * INITIALIZE A PORT WITH STANDARD UNIX CONFIGURATION
 * ==================================================
 *
 * start command
 * do error processing
 *
 *
 * Called by:	m331open ()
 * Input:	controller, port, 
 * Return:	error code from command
 * Error:	terminated by abort 
 *		or invalid item number
 * Calls to:	CCDoCmd(), ProcessErrors()
 *
 */

InitPortICC( cntl, port )
int cntl, port;
{
	/* return status code */
	int retvalue;

	/* loop counter */
	register int i;

	/* table for initialization */
	char *inittab;


#ifdef HOWFAR
	if( msubDebug > 3 )
	    printf( "In InitPortICC: cntl=%d, port=%d\n",
		    cntl, port );
#endif
	/* do the command */
	retvalue = CCDoCmd( CMD_CFPORT, cntl, port+1,
		(caddr_t)PortConfigTable, sizeof( PortConfigTable ), 
		&dummy, MWRITE|MSLEEP );

/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, port, "PortInitICC" );
	
/* SSM 05/20/86 - 1 */
	inittab = GetBuffer( cntl,port );

	/* initialize RAT */
	inittab[0] = 0;
	inittab[1] = 1;
	for( i=2; i<258; ++i ) {
	    inittab[i] = 0x07;
	}
	if((retvalue = CCDoCmd( CMD_CFTABLE, cntl, port+1, inittab,
		258, &dummy, MWRITE|MSLEEP )) != 0)
		ProcessErrors( retvalue, cntl, port, "ConfigTableICC" );

	/* initialize TAT */
	inittab[0] = 0;
	inittab[1] = 2;
	for( i=2; i<258; ++i ) {
	    inittab[i] = 0;
	}
	if((retvalue = CCDoCmd( CMD_CFTABLE, cntl, port+1, inittab,
		258, &dummy, MWRITE|MSLEEP )) != 0)
		ProcessErrors( retvalue, cntl, port, "ConfigTableICC" );

	/* initialize IRAT */
	inittab[0] = 0;
	inittab[1] = 3;
	for( i=2; i<258; ++i ) {
	    inittab[i] = 0x03; 		/* no echo */
	}
	inittab[ 0x13 ] = 0x01;		/* XON */
	inittab[ 0x15 ] = 0x00;		/* XOFF */
	if((retvalue = CCDoCmd( CMD_CFTABLE, cntl, port+1, inittab,
		258, &dummy, MWRITE|MSLEEP )) != 0)
		ProcessErrors( retvalue, cntl, port, "ConfigTableICC" );

	/* initialize WAT */
	inittab[0] = 0;
	inittab[1] = 6;
	for( i=2; i<258; ++i ) {
	    inittab[i] = 0;
	}
	if((retvalue = CCDoCmd( CMD_CFTABLE, cntl, port+1, inittab,
		258, &dummy, MWRITE|MSLEEP )) != 0)
		ProcessErrors( retvalue, cntl, port, "ConfigTableICC" );

	/* prepare a packet for solicited events ( BREAK, ... )   */
	SendSolicitedICC( cntl, port );

/* SSM 05/20/86 - 1 */
	FreeBuffer(cntl,port );

	return( retvalue );

}	/* end of InitPortICC */


/*
 * GET STATUS OF DCD LINE
 * ======================
 *
 * start request port status command
 * get the item from the table
 *
 *
 * Called by:	m331open()
 * Input:	controller, port,
 * Return:	0 if line is inactive, 1 if line is active, -1 if error
 * Error:	port status command failed
 * Calls to:	PortStatICC()
 *
 */

GetDCDICC( cntl, port )
int cntl;
int port;
{
	/* return status code */
	int retvalue;

	/* status table */
	unsigned int *portstat;


#ifdef HOWFAR
	if( msubDebug > 3 )
	    printf( "In GetDCDICC: cntl=%d, port=%d\n",
		    cntl, port  );
#endif

/* SSM 05/20/86 - 1 */
	portstat = (unsigned int*) GetBuffer( cntl,port );

	/* do the port status command */
	retvalue = PortStatICC( cntl, port, portstat );
	
	if( retvalue != 0 ) {
	    /* error return */
/* SSM 05/20/86 - 1 */
	    FreeBuffer(cntl,port );
	    return( -1 );
	}

	if( portstat[ DCDITEM ] == 0 ) {
	    /* DCD is inactive */
/* SSM 05/20/86 - 1 */
	    FreeBuffer(cntl,port );
	    return( 0 );
	}
	else {
	    /* DCD is active */
/* SSM 05/20/86 - 1 */
	    FreeBuffer(cntl,port );
	    return( 1 );
	}

}	/* end of GetDCDICC */


/*
 * SET SOFTWAREAHANDSHAKE ON/OFF
 * =============================
 *
 * set table
 * start command
 * do error processing
 *
 *
 * Called by:	m331param()
 * Input:	controller, port, flag for on/off
 * Return:	error code from command
 * Error:	terminated by abort,
 * Calls to:	CCDoCmd(), ProcessErrors(), 
 *
 */

SWHandShakeICC( cntl, port, flag )
int cntl;
int port;
int flag;
{
	/* return status code */
	int retvalue;

	/* table pointer for IRAT */
	char *tab;

	/* loop counter */
	register int i;


#ifdef HOWFAR
	if( msubDebug > 3 )
	    printf( "In SWHandShakeICC: cntl=%d, port=%d flag=0x%x\n",
		    cntl, port, flag );
#endif

/* SSM 05/20/86 - 1 */
	tab = GetBuffer( cntl,port );

	/* set IRAT */
	tab[0] = 0;
	tab[1] = 3;
	for( i=2; i<258; ++i ) {
	    tab[i] = 0x03; 		/* no echo */
	}

	if( flag ) {
	    /* handshake on */
	    tab[ 0x13 ] = 0x01;		/* XON */
	    tab[ 0x15 ] = 0x00;		/* XOFF */
	}

	if((retvalue = CCDoCmd( CMD_CFTABLE, cntl, port+1, tab,
		258, &dummy, MWRITE|MSLEEP )) != 0)
		ProcessErrors( retvalue, cntl, port, "ConfigTableICC" );

/* SSM 05/20/86 - 1 */
	FreeBuffer(cntl,port );

	return( retvalue );

}	/* end of SWHandShakeICC */


/*
 * START CONFIGURE PORT COMMAND
 * ============================
 *
 * start request configure port command
 * if Reset is necessary( items 1 - 11 ), issue the reset cmd.
 *
 * Called by:	m331close(), m331param()
 * Input:	controller, port,
 * Return:	0 if line is inactive, 1 if line is active, -1 if error
 * Error:	port status command failed
 * Calls to:	ResetPort(), m331proc()
 *
 */

StartConfigPortICC( cntl, port ,cp)
int cntl;
int port;
struct confbuf *cp;
{
	/* return status code */
	int retvalue;

#ifdef HOWFAR
	if( msubDebug > 3 )
	    printf( "In StartConfigPortICC: cntl=%d, port=%d\n",
		    cntl, port );
#endif
	retvalue = CCDoCmd( CMD_CFPORT, cntl, port+1, (caddr_t)cp,
		sizeof( struct ICCitem ) * cp->index, &dummy, MWRITE|MSLEEP );

/* SSM 04/24/86 - 1 */
	if(retvalue)
		ProcessErrors( retvalue, cntl, port, "StartConfigPortICC" );

	/* if necessary, reset the port */
	if( cp->resetflg ) {
	    ResetPortICC( cntl, port );
	}

	cp->resetflg = 0;

	return( retvalue );

}	/* end of StartConfigPortICC */


/* SSM 05/20/86 - 1 */
/*
 * GET BUFFER
 * ==========
 *
 * If buffer is busy, sleep
 * Return pointer of buffer
 *
 * Called by:	m331ioctl(), InitPortICC(), GetDCDICC()
 * Input:	controller and port the buffer is for
 * Return:	pointer of buffer
 * Error:	-
 * Calls to:	-
 *
 */

caddr_t GetBuffer( cntl,port )
int cntl;
int port;
{
	/* saved interrupt level */
	int s;
	int index;

	index = cntl * MAXPORTS + port;

	s = spl6();

	/* sleep until the buffer is free */
	while( BufBusy[index] ) {
	    sleep( &BufBusy[index], PRIICC );
	}

	/* catch the buffer */
	BufBusy[index] = 1;
	splx( s );

#ifdef HOWFAR
	if( msubDebug > 3 ) 
	    printf("GetBuffer: return 0x%x\n",&PortBuf[index * PORTBUFLENGTH]);
#endif
	return( &PortBuf[index * PORTBUFLENGTH] );

}	/* end of GetBuffer() */


/* SSM 05/20/86 - 1 */
/*
 * FREE BUFFER
 * ===========
 *
 * wakeup who was sleeping of this buffer
 *
 * Called by:	m331ioctl(), InitPortICC(), GetDCDICC()
 * Input:	pointer of buffer
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

FreeBuffer(cntl,port)
int cntl;
int port;
{
        int index;

	index = cntl * MAXPORTS + port;

	/* give the buffer free */
	BufBusy[index] = 0;

	/* wakeup, who is waiting for this buffer */
	wakeup( &BufBusy[index] );

}	/* end of FreeBuffer() */


/* SSM 04/24/86 - 1 */
/*
 * PROCESS ERRORS
 * ==============
 *
 * check if retvalue is okay
 * print error message
 *
 * Called by:	all commands
 * Input:	return status,  name of caller
 * Return:	0 if no error, -1 if error
 * Error:	if return status is not equal 0
 * Calls to:	-
 *
 */

ProcessErrors( errorcode, cntl, port, string )
int errorcode;
int cntl, port;
char *string;
{

    if( errorcode < 0xa0 ) {
    printf( "%s: cntl=%d, port=%d, error=%d, %s\n",
	    string, cntl, port, errorcode, 
	    ErrorStrings[ errorcode ] );
    }
    else {
	printf( "%s: cntl=%d, port=%d, int. error = %d, %s\n",
		string, cntl, port, errorcode,
		IntErrStrings[ errorcode-0xa0 ] );
    }
}	/* end of ProcessErrors */


/*--------------------------------------------------------------------*/
/*
 *	COMMAND CHANNEL INTERFACE FOR MOTOROLA VME BOARDS
 *	=================================================
 *
 *
 *	Following routines are implemented:		checked:
 *	-----------------------------------
 *	CCDrvInit	Driver Initialization		03/21/85
 *	CCCntlInit	Controller Initialization	03/21/85
 *
 *	Pointer Pool Manager:
 *	CCPtInit	Pointer Initialization		03/21/85
 *	CCPtGetFree	Get Free Pointer		03/07/85
 *	CCPtFree	Free Pointer			03/07/85
 *	CCSetCmdSeparator  Set Command Separator	03/07/85
 *	CCSetStatSeparator Set Status Separator		03/07/85
 *
 *	Packet Pool Manager:
 *	CCPkInit	Data Packet Initialization	03/21/85
 *	CCPkGetFree	Get Free Packet			03/07/85
 *	CCPkFree	Free Packet			03/07/85
 *
 *	InitFWCntl	Init Firmware On Controller	03/07/85
 *	SetSeparator	Set Separators			03/07/85
 *	PutComReg	Put Byte to Comm. Register	03/07/85
 *	CCDoCmd		Do Command by CC Interface	03/07/85
 *	CCStartCmd	Start Command on Controller	03/07/85
 *	CCInterrrupt	Interrupt the Controller	03/07/85
 *	CCIntrService	Interrupt Service Routine	03/07/85
 *
 *	COMPILER SWITCHES:
 *	------------------
 *	HISTORY			log all started and completed 
 *				requests in a history ring buffer
 *
 */




/* DEFINITIONS */

#define MAXSETWAIT	50000		/* wait constant for setting the
					   communication register */

#define CCADDRMOD	0x3e		/* address modifier 
					   for set cmd */

#ifdef HISTORY
#define HISTMAX		256		/* maximal number of history */
#endif


/* GLOBALS */

/* pointer in buffer area */
caddr_t CCBufPt;

/* description of the driver */
struct CCdriver CCdrv;

#ifdef HOWFAR
/* flag for printing debug messages */
int CCDebug = 4;
#endif

#ifdef HISTORY
/* History array */
int HistArray[HISTMAX];

/* flag for enabling history of packets */
int HistFlag = 3;
#endif /* HISTORY */


/* GLOBAL FUNCTIONS */

struct CCpointer *CCPtGetFree();
struct CCpacket *CCPkGetFree();


/* EXTERNALS */

/* description of the controller */
extern struct CCcontroller CCcntl[];

/* command channel buffer */
extern char cci_buffer[];

#ifdef NOT_FINAL_VERSION
/* flag, if we are in InterruptServiceRoutine */
extern int InM331Intr;

/* value at end of kernel */
extern int end;
#endif


/*
 * INIT DRIVER FOR COMMAND CHANNEL INTERFACE
 * =========================================
 *
 * get buffer start address
 * init packet pool manager
 * set driverdescription
 *
 * Called by:	open() of driver
 * Input:	count of controllers
 * Return:	-
 * Error:	-
 * Calls to:	CCPkInit()
 *
 */

CCDrvInit( count )
int count;
{
	
	/* set start of global memory area for controller and
	   driver */
	CCBufPt = cci_buffer;

	/* initialize packet pool for cmd. channel interface */
	CCPkInit( (struct CCpacket *)CCBufPt, PKMAX*count );

	/* increment to next area */
	CCBufPt = (caddr_t) ((int)CCBufPt) 
			  + PKMAX * count * sizeof( struct CCpacket );

	/* set count of controllers */
	CCdrv.cd_cntlcount = count;

	/* initialization complete */
	CCdrv.cd_stat = CDINIT;

}	/* end of CCDrvInit */


/*
 * INIT CONTROLLER FOR COMMAND CHANNEL INTERFACE
 * =============================================
 *
 * set IO addresses
 * set interrupt information
 * init pointer pool manager
 * initialize firmware
 *
 * Called by:	open() of driver
 * Input:	controller number, base address,
 *		interrupt vector, interrupt level
 * Return:	0 if ok, -1 if error
 * Error:	InitFWCntl() fails
 * Calls to:	CCPtInit(), InitFWCntl() 
 *
 */

CCCntlInit( cntl, baseaddress, vector, level )
int cntl, vector, level;
int baseaddress;
{

	/* clear the status of this controller */
	CCcntl[cntl].cc_stat = 0;

	/* set communication register address */
	CCcntl[cntl].cc_comreg = (char *) baseaddress + COMREGOFFSET;

	/* set attention register address */
	CCcntl[cntl].cc_attreg = (char *) baseaddress + ATTREGOFFSET;

	/* set interrupt vector */
	CCcntl[cntl].cc_intrvector = vector;

	/* set interrupt level */
	CCcntl[cntl].cc_intrlevel = level;

	/* initialize pointer pool for cmd. channel interface */
	CCPtInit( cntl, (struct CCpointer *)CCBufPt, PTMAX );

	/* increment to next pool */
	CCBufPt = (caddr_t) ((int)CCBufPt) 
			  + PTMAX * sizeof( struct CCpointer );

	/* initialize the firmware */
	if( InitFWCntl( cntl ) == 0 ) {
		    CCcntl[cntl].cc_stat |= CCINIT;
		    return( 0 );
	}
	else {
		    CCcntl[cntl].cc_stat = CCCLEAR;
		    return( -1 );
	}

}	/* end of CCCntlInit */

/*
 * FREE POINTER POOL INITIALIZATION
 * ================================
 *
 * there is one pool for every controller
 *
 * initialize global values
 * initialize queue
 * initialize command separator
 * initialize status separator
 *
 * Called by:	CCCntlInit()
 * Input:	controller number,
 *		base address for pointer queue
 *		count of pointers
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

CCPtInit( cntl, firstpt, count )
int cntl;
struct CCpointer *firstpt;
int count;
{
	/* next pointer in queue */
	struct CCpointer *nextpt;

	/* loop count */
	int i;


	/* initailize global values */
	CCcntl[cntl].cc_ptcount = count;
	CCcntl[cntl].cc_ptnextfree = firstpt;

	/* set nextpt to firstpointer + 1 */
	nextpt = ++firstpt;
	--firstpt;

	/* initialize pointer queue */
	for( i=0; i<count; ++i ) {
	    firstpt->pt_next = nextpt;
	    firstpt->pt_datapt = 0;
	    ++firstpt;
	    ++nextpt;

	}

	/* set command separator */
	CCSetCmdSeparator( cntl, CCPtGetFree( cntl ) );

	/* set status separator */
	CCSetStatSeparator( cntl, CCPtGetFree( cntl ) );


}	/* End of CCPtInit */

	

/*
 * GET FREE COMMAND CHANNEL POINTER
 * ================================
 *
 * sleep if no pointer is available
 * get free pointer from queue
 * set next free pointer
 *
 * Called by:	-
 * Input:	controller number
 * Return:	free pointer
 * Error:	-
 * Calls to:	-
 *
 */

struct CCpointer *
CCPtGetFree( cntl )
int cntl;
{
	/* saved interrupt level */
	int s;

	/* got free pointer from queue */
	struct CCpointer *gotpt;


	/* start of critical phase */
	/* do not allow to interrupt by other processes */
	s = spl7();

	/* check if all pointers are busy */
	while( CCcntl[cntl].cc_ptcount == 0 ) {
	    sleep( &CCcntl[cntl].cc_ptcount, PRICCI );
	}

	/* a pointer is available */
	CCcntl[cntl].cc_ptcount--;

	gotpt = CCcntl[cntl].cc_ptnextfree;

	/* set next free pointer */
	CCcntl[cntl].cc_ptnextfree = 
				CCcntl[cntl].cc_ptnextfree->pt_next;

	/* end of critical phase */
	splx( s );

	return( gotpt );

}	/* end of CCPtGetFree */


/*
 * FREE A COMMAND CHANNEL POINTER
 * ==============================
 *
 * set the pointer to the head of the queue
 * increment available count
 * wakeup who was sleeping for a pointer
 *
 * Called by:	-
 * Input:	controller number,
 *		pointer which sould be freed
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

CCPtFree( cntl, freept )
int cntl;
struct CCpointer *freept;
{
	/* saved interrupt level */
	int s;


	/* start of critical phase */
	/* do not allow to interrupt by other processes */
	s = spl7();

	/* set pointer to head of queue */
	freept->pt_next = CCcntl[cntl].cc_ptnextfree;

	CCcntl[cntl].cc_ptnextfree = freept;

	/* increment available count */
	++CCcntl[cntl].cc_ptcount;

	/* end of critical phase */
	splx( s );

	wakeup( &CCcntl[cntl].cc_ptcount);

}	/* end of CCPtFree */


/*
 * SET COMMAND SEPARATOR
 * =====================
 *
 * Called by:	CCPtInit
 * Input:	controller number, pointer	
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

CCSetCmdSeparator( cntl, pointer)
int cntl;
struct CCpointer *pointer;
{

	/* set separator */
	CCcntl[cntl].cc_cmdsep = pointer;

	/* mark this pointer to a cmd separator */
	/* set the next pointer address to an odd address */
	CCcntl[cntl].cc_cmdsep->pt_next = (struct CCpointer *)
			( (int)CCcntl[cntl].cc_cmdsep->pt_next + 1 );

}	/* end of CCSetCmdSeparator */


/*
 * SET STATUS SEPARATOR
 * ====================
 *
 * Called by:	CCPtInit
 * Input:	controller number, pointer	
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

CCSetStatSeparator( cntl, pointer)
int cntl;
struct CCpointer *pointer;
{

	/* set separator */
	CCcntl[cntl].cc_statsep = pointer;

	/* mark this pointer to a stat separator */
	/* set the next pointer address to an odd address */
	CCcntl[cntl].cc_statsep->pt_next = (struct CCpointer *)
			( (int)CCcntl[cntl].cc_statsep->pt_next + 1 );

}	/* end of CCSetStatSeparator */


/*
 * DATA PACKET POOL INITIALIZATION
 * ===============================
 *
 * there is only one packet pool for all controllers
 *
 * initialize global values
 * initialize queue
 *
 * Called by:	-
 * Input:	base address for packet queue
 *		count of packets
 * Input:	-
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

CCPkInit( firstpk, count )
struct CCpacket *firstpk;
int count;
{
	/* next packet in queue */
	struct CCpacket *nextpk;

	/* loop count */
	int i;


	/* initailize global values */
	CCdrv.cd_pkcount = count;

	CCdrv.cd_pknextfree = firstpk;

	/* set nextpk to firstpacket + 1 */
	nextpk = ++firstpk;
	--firstpk;


	/* initialize packet queue */
	for( i=0; i<count; ++i ) {
	    firstpk->pk_next = nextpk;
	    firstpk->pk_flag = 0;
	    firstpk->pk_fln = 0;
	    firstpk->pk_cmd = 0;
	    firstpk->pk_port = 0;
	    firstpk->pk_sad = 0;
	    firstpk->pk_len = 0;
	    firstpk->pk_stat = 0xff;
	    firstpk->pk_term = 0xff;
	    firstpk->pk_rlen = 0;
	    ++firstpk;
	    ++nextpk;
	}

}	/* End of CCPkInit */


/*
 * GET FREE COMMAND CHANNEL PACKET
 * ===============================
 *
 * sleep if no packet is available
 * get free packet from queue
 * set next free packet
 *
 * Called by:	-
 * Input:	-
 * Return:	free packet
 * Error:	-
 * Calls to:	spl7, splx
 *
 */

struct CCpacket *
CCPkGetFree()
{
	/* saved interrupt level */
	int s;

	/* got free packet from queue */
	struct CCpacket *gotpk;


	/* start of critical phase */
	/* do not allow to interrupt by other processes */
	s = spl7();

	/* check if all packets are busy */
	while( CCdrv.cd_pkcount == 0 ) {
	    sleep( &CCdrv.cd_pkcount, PRICCI);
	}

	/* a packet is available */
	CCdrv.cd_pkcount--;

	gotpk = CCdrv.cd_pknextfree;

	/* set next free packet */
	CCdrv.cd_pknextfree = CCdrv.cd_pknextfree->pk_next;

	/* end of critical phase */
	splx( s );

	return( gotpk );

}	/* end of CCPkGetFree */


/*
 * FREE A COMMAND CHANNEL POINTER
 * ==============================
 *
 * set the packet to the head of the queue
 * increment available count
 * wakeup who was sleeping for a packet
 *
 * Called by:	-
 * Input:	packet which sould be freed
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

CCPkFree( freepk )
struct CCpacket *freepk;
{
	/* saved interrupt level */
	int s;


	/* start of critical phase */
	/* do not allow to interrupt by other processes */
	s = spl7();

	/* clear allflags */
	freepk->pk_flag = 0;

	/* set packet to head of queue */
	freepk->pk_next = CCdrv.cd_pknextfree;

	CCdrv.cd_pknextfree = freepk;

	/* increment available count */
	++CCdrv.cd_pkcount;

	wakeup( &CCdrv.cd_pkcount );

	/* end of critical phase */
	splx( s );

}	/* end of CCPkFree */




/*
 * INIT FIRMWARE ON CONTROLLER
 * ===========================
 *
 * check selftest flag
 * give Command and Status Separator via S cmd to controller
 *
 * Called by:	CCCntlInit()
 * Input:	controller number
 * Return:	0 if okay, -1 if error
 * Error:	Selftest or SetSeparator() fails
 * Calls to:	SetSeparator()
 *
 */

InitFWCntl( cntl )
int cntl;
{
	/* selftest status */
	int selfstat;

	/* get status of selftest */
	selfstat = *CCcntl[cntl].cc_comreg & 0x7;

	if( selfstat != 0x00 ) {
#ifdef NOT_FINAL_VERSION
	    printf( "MVME 331: Selftest failed: Error Code = %d\n",
		    selfstat );
#endif
	    return( -1 );
	}
	    
	if( SetSeparator( cntl, CCcntl[cntl].cc_cmdsep,
			        CCcntl[cntl].cc_statsep ) == -1 ) {
	    return( -1 );
	}
	
	return( 0 );
}	/* End of InitFWCntl */


/*
 * SET SEPARATORS
 * ==============
 *
 * split separators in bytes and
 * give them via S command to communication register
 *
 * Called by:	InitFWCntl()
 * Input:	controller number,
 *		command separator
 *		status separator
 * Return:	0 if okay, -1 if error
 * Error:	PutComReg() fails
 * Calls to:	PutComReg()
 *
 */

SetSeparator( cntl, cmdseparator, statseparator )
int cntl;
struct CCpointer *cmdseparator, *statseparator;
{

#ifdef HOWFAR
	if( CCDebug > 3 ) {
	    printf( "In SetSeparator: cntl=%d, ", cntl );
	    printf( "cmdseparator=0x%x, ", cmdseparator );
	    printf( "statseparator=0x%x\n", statseparator );
	}
#endif

	if( ( PutComReg( cntl, 'S' ) ) == -1 ) {
	    return( -1 );
	}

	if( ( PutComReg( cntl, CCADDRMOD ) ) == -1 ) {
	    return( -1 );
	}

	if( ( PutComReg( cntl,
		( (int)cmdseparator & 0xff000000 ) >> 24 ) ) == -1 ) {
	    return( -1 );
	}
	if( ( PutComReg( cntl,
		( (int)cmdseparator & 0x00ff0000 ) >> 16 ) ) == -1 ) {
	    return( -1 );
	}
	if( ( PutComReg( cntl,
		( (int)cmdseparator & 0x0000ff00 ) >> 8 ) ) == -1 ) {
	    return( -1 );
	}
	if( ( PutComReg( cntl,
		( (int)cmdseparator & 0x000000ff ) ) ) == -1 ) {
	    return( -1 );
	}

	if( ( PutComReg( cntl,
		( (int)statseparator & 0xff000000 ) >> 24 ) ) == -1 ) {
	    return( -1 );
	}
	if( ( PutComReg( cntl,
		( (int)statseparator & 0x00ff0000 ) >> 16 ) ) == -1 ) {
	    return( -1 );
	}
	if( ( PutComReg( cntl,
		( (int)statseparator & 0x0000ff00 ) >> 8 ) ) == -1 ) {
	    return( -1 );
	}
	if( ( PutComReg( cntl,
		( (int)statseparator & 0x000000ff ) ) ) == -1 ) {
	    return( -1 );
	}

	return( 0 );

}	/* end of SetSeparator */



/*
 *
 * PUT A BYTE TO THE COMMUNICATION REGISTER
 * ========================================
 *
 * write byte to communication register
 * wait for acknowledge
 * write 0x00 to communication register
 * wait for synchronization
 *
 * Called by:	SetSeparator()
 * Input:	controller number, value
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

PutComReg( cntl, value )
int cntl;
char value;
{

	/* loop count */
	register int i;

#ifdef HOWFAR
	if( CCDebug > 3 )
	    printf( "%x ", value );
#endif /* HOWFAR */

	*CCcntl[cntl].cc_comreg = value;

	/* give interrupt to controller */
	*CCcntl[cntl].cc_attreg = ATTINTR;

	i = 0;
	while( ( *CCcntl[cntl].cc_comreg & 0x7 ) != 0x01 ) {
	    if( i++ > MAXSETWAIT ) {
#ifdef NOT_FINAL_VERSION
		printf(
		    " PutComReg failed, no response of controller\n" );
#endif 
		return( -1 );
	    }
	}

	/* send synchronization byte */
	*CCcntl[cntl].cc_comreg = 0;

	/* give interrupt to controller */
	*CCcntl[cntl].cc_attreg = ATTINTR;

	i = 0;
	while( ( *CCcntl[cntl].cc_comreg & 0x7 ) != 0x00 ) {
	    if( i++ > MAXSETWAIT ) {
#ifdef NOT_FINAL_VERSION
		printf(
		    " PutComReg failed, no response of controller " );
		printf( "at synchronization byte\n" );
#endif 
		return( -1 );
	    }
	}
	return( 0 );

}	/* End of PutComReg */


/*
 * DO COMMAND BY COMMAND CHANNEL INTERFACE
 * =======================================
 *
 * get data packet
 * set parameters into data packet
 * if write move data to shared memory
 * start command
 * if read move data from shared memory
 * set return length
 *
 * Called by:	all commands
 * Input:	command code, controller, port,
 *		address of buffer, length of buffer,
 *		pointer to return length, mode for direction
 * Return:	return length in pointer, 
 *		status code from controller
 * Error:	error from controller
 * Calls to:	CCPkGetFree(), MoveData(), CCStartCmd(),
 *		CCPkFree()
 *
 */

CCDoCmd( command, cntl, port, address, length, retlength, mode )
int command, cntl, port, length, mode;
char *address;
int *retlength;
{
	/* pointer to data packet */
	struct CCpacket *pk;

	/* return status */
	int retstat;

	/* check if controller is initialized */
	if( !( CCcntl[cntl].cc_stat & CCINIT ) ) {
	    return( 0xa1 );
	}

	/* get a free packet */
	pk = CCPkGetFree();

	pk->pk_cmd = command;
	pk->pk_port = port;
	pk->pk_len = length;
	pk->pk_stat = 0xff;
	pk->pk_term = 0xff;
	pk->pk_rlen = 0;
	pk->pk_sad = address;

	/* start the command */
	CCStartCmd( cntl, pk, mode );

	/* set return length */
	*retlength = pk->pk_rlen;

	/* save status */
	retstat = pk->pk_stat;

	/* free packet */
	CCPkFree( pk );

	return( retstat );

}	/* end of CCDoCmd */



/*
 * START CONTROLLER
 * ================
 *
 * get a free pointer
 * link free pointer to command separator
 * set command separator at free pointer
 * clear command separator at old pointer
 * interrupt controller
 * sleep until completion
 *
 * Called by:	CCDoCmd()
 * Input:	controller, data packet pointer, mode
 * Return:	-
 * Error:	-
 * Calls to:	CCPtGetFree(), CCSetCmdSeparator(), CCInterrupt(),
 *		CCPtFree()
 *
 */

CCStartCmd( cntl, packet, mode )
int cntl, mode ;
struct CCpacket *packet;
{
	/* free pointer from queue */
	struct CCpointer *pt;

	/* last command separator */
	struct CCpointer *lastcmdsep;

	/* saved interrupt level */
	int s;

/* SSM 07/22/86 - 1 */
	/* saved packet pointer that this process sleeps on it */
	register int spk;

#ifdef NOT_FINAL_VERSION
	if( (int)packet->pk_sad > (int)&end ) {
	    printf( "CCStartCmd: StartAddress=0x%x is out of range\n",
			packet->pk_sad );
	}
#endif

	/* get a free pointer */
	pt = CCPtGetFree( cntl );

	/* start of critical phase */
	/* do not allow to interrupt by other processes */
	s = spl7();

	/* save the last command separator */
	lastcmdsep = CCcntl[cntl].cc_cmdsep;

	/* set the command separator into the free pointer */
	CCSetCmdSeparator( cntl, pt );

	/* link the data packet to the last command separator 
	   skip the first members of the structure, because they
	   are private for the driver */
	lastcmdsep->pt_datapt = (char *) &packet->pk_fln;

/* SSM 07/22/86 - 1 */
/* df  10/16/86 must save sleep address before putting command on queue */
	spk = (int) lastcmdsep->pt_datapt;

	/* link the last separator pointer to the new */
	lastcmdsep->pt_next = pt;

#ifdef HOWFAR
	if( CCDebug ) {
	    printf( "CCStart: pt=%x ", lastcmdsep );
	    PrintPacket( packet );
	}
#endif
#ifdef HISTORY
	if( HistFlag ) {
	    PutHistory( (int)pt + (packet->pk_cmd<<24) );
	    PutHistory( (int)packet );
	}
#endif
	/* interrupt the controller */
	CCInterrupt( cntl );


	if( mode & MSLEEP ) {

#ifdef NOT_FINAL_VERSION
	    if( InM331Intr ) {
		printf( "\b\bWARNING: " );
		printf( "CCStartCmd: go sleeping in M331 Interrupt routine\n" );
	    }
#endif
	    /* wait until this command is completed */
	    packet->pk_flag |= PKSLEEP;

/* SSM 07/22/86 - 1 */
/*	    sleep( lastcmdsep->pt_datapt, PRICCI ); */
	    sleep( spk, PRICCI );

#ifdef HOWFAR
	    if( CCDebug > 3 ) {
	        printf( "CCCompl: pt=%x ", lastcmdsep );
	        PrintPacket( packet );
	    }
#endif

	}

	/* end of critical phase */
	splx( s );

}	/* end of CCStartCmd */


/*
 * INTERRUPT THE CONTROLLER
 * ========================
 *
 * set bit 0 in attention register
 *
 * Called by:	CCStartCmd()
 * Input:	controller
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */
 
CCInterrupt( cntl )
int cntl;
{
#ifdef HOWFAR
	if( CCDebug > 3 ) {
	    printf( "In CCInterrupt: cntl=%d, attreg=0x%x\n",
		cntl, CCcntl[cntl].cc_attreg );
	}
#endif
	*CCcntl[cntl].cc_attreg = ATTINTR;

}	/* End of CCInterrupt */


/*
 * INTERRUPT SERVICE ROUTINE
 * =========================
 *
 * loop thru status queue
 * wakeup sleeping data packets
 *
 * Called by:	intr() of driver
 * Input:	controller, pointer to return packets
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

CCIntrService( cntl, pkreturn )
int cntl;
struct CCpacket *pkreturn[];
{
	/* last status separator */
	register struct CCpointer *laststatsep;

	/* completed status pointer */
	register struct CCpointer *complstat;

	/* completed packet pointer */
	register struct CCpacket *pk;

	/* count of completed packets, which are not waked up and
	   will be returned by the pkreturn pointer */
	register int retcount = 0;


#ifdef HOWFAR
	if( CCDebug > 3 ) {
	    printf( "In CCIntrService: cntl=%d, pkreturn=0x%x\n",
		cntl, pkreturn );
	}
	if( CCDebug > 1) {
	    printf( "In CCIntrService: cmdsep=0x%x statsep=0x%x\n",
		CCcntl[cntl].cc_cmdsep,
		CCcntl[cntl].cc_statsep );
	}
#endif

	/* get last status separator */
	laststatsep = CCcntl[cntl].cc_statsep;

	/* loop thru status queue until tail is found */
	while( !((int)laststatsep->pt_next & 1 ) ) {
	    /* found a pointer who is not the separator */
	    complstat = laststatsep;

	    /* point to next in queue */
	    laststatsep = complstat->pt_next;

#ifdef NOT_FINAL_VERSION
	    if( (int)complstat->pt_datapt == 0 ) {
		printf( "CCIntrService: datapointer == 0\n" );
		printf( "complstat+0x%x\n", complstat );
		printf( "CPU halted !\n" );
		while( 1 );
	    }
#endif

	    /* here should be the offset between start of
	       packet structure and fln member = 8 !!!!!! */
	    pk = (struct CCpacket *)( (int)complstat->pt_datapt -
		  (int) 8);
#ifdef HOWFAR
	    if( CCDebug > 3 ) {
	        PrintPacket( pk );
	    }
#endif
#ifdef HISTORY
	    if( HistFlag > 1 ) {
		PutHistory( ((int)complstat&0xffff) + 0x00ff0000
					     + (pk->pk_cmd<<24) );
		PutHistory( (int)pk );
	    }
#endif


	    if( pk->pk_flag & PKSLEEP ) {
	        /* wakeup who was sleeping on this datapacket */
	        wakeup( complstat->pt_datapt );
	        pk->pk_flag &= ~PKSLEEP;
	    }
	    else {
		/* set the packet on the complete queue */
		pkreturn[retcount++] = pk;

		/* stop, if the queue is full */
		if( retcount >= ISRMAXRET ) {
	        
		    /* free pointer */
	            CCPtFree( cntl, complstat );

		    break;
		}
	    }
	    /* free pointer */
	    CCPtFree( cntl, complstat );


	}

	/* save the new status separator */
	CCcntl[cntl].cc_statsep = laststatsep;

#ifdef HOWFAR
	if( CCDebug > 3 ) {
	    printf( "CCIntrService ok: retcount=%d pkreturn=0x%x\n",
		retcount, pkreturn );
	}
	if( CCDebug > 1 ) {
	    printf( "CCIntrService ok: cmdsep=0x%x statsep=0x%x\n",
		CCcntl[cntl].cc_cmdsep,
		CCcntl[cntl].cc_statsep );
	}
#endif
	/* return the count of completed, not waked up packets */
	return( retcount );

}	/* end of CCIntrService */


#ifdef NOT_FINAL_VERSION
/*
 * PRINT INITIALIZATION MESSAGES
 * =============================
 *
 * print driver description
 * print controller description
 *
 * Called by:	init() of driver
 * Input:	count of controllers
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

CCPrintInit( count )
int count;
{

	/* loop count */
	register int i;

	printf( "    Command Channel Interface:\n" );
	printf( "    count of controllers:   %d\n", CCdrv.cd_cntlcount);
	printf( "    count of data packets:  %d\n", CCdrv.cd_pkcount );
	printf( "    first free data packet: 0x%x\n",
		     CCdrv.cd_pknextfree );
	printf( "    start of buffer: 0x%x\n", CCBufPt );

	for( i=0; i<count; ++i ) {
	    printf( "    Controller #%d:\n", i );
	    printf( "        count of pointer packets: %d\n",
			     CCcntl[i].cc_ptcount );
	    printf( "        communication register address: 0x%x\n",
			     CCcntl[i].cc_comreg );
	    printf( "        attention register address:     0x%x\n",
			     CCcntl[i].cc_attreg );
	    printf( "        command separator:              0x%x\n",
			     CCcntl[i].cc_cmdsep );
	    printf( "        status separator:               0x%x\n",
			     CCcntl[i].cc_statsep );
	    printf( "        first free pointer:             0x%x\n",
			     CCcntl[i].cc_ptnextfree );
	}

}	/* end of CCPrintInit */



/*
 * PRINT DATA PACKET
 * =================
 *
 * print driver description
 * print controller description
 *
 * Called by:	some debug routines
 * Input:	pointer to data packet
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

PrintPacket( packet)
struct CCpacket *packet;
{
	printf( "pk=%x flag=%x cmd=%d port=%d a=%x l=%d s=%d t=%x rl=%d\n",
		packet,
		packet->pk_flag,
		packet->pk_cmd,
		packet->pk_port,
		packet->pk_sad,
		packet->pk_len,
		packet->pk_stat,
		packet->pk_term,
		packet->pk_rlen );

}	/* end of PrintPacket */
#endif /* NOT_FINAL_VERSION */


#ifdef HISTORY
/*
 * PUT VALUE IN HISTORY RINGBUFFER
 * ===============================
 *
 * If first call, then initialize the ringbuffer
 * Set value in ringbuffer
 * Set current pointer in first element of array
 * If end of ringbuffer reached, then set pointer to start of buffer
 *
 * Called by:	some debug routines
 * Input:	value to store in history buffer
 * Return:	-
 * Error:	-
 * Calls to:	-
 *
 */

PutHistory( value )
int value;
{
	int s;
	int i;
	static HistIndex =-1;

	if( HistIndex == -1 ) {
	    for( i=0; i<HISTMAX; ++i )
		HistArray[i] = 0;
	    HistIndex = 1;
	    HistArray[0] = (int) &HistArray[HistIndex];
	}

	s =spl6();

	HistArray[ HistIndex++ ]  = value;
	HistArray[0] = (int) &HistArray[HistIndex];

	if( HistIndex == HISTMAX ) {
	    HistIndex = 1;
	    HistArray[0] = (int) &HistArray[HistIndex];
	}
	splx( s );
}	/* End of PutHistory */
#endif


/* Splev exists to isolate the ugliness necessary to get the level in the  */
/* right location for the status register.  Splx() should actually be      */
/* doing this work.  Hopefully this problem will be remedied at a later    */
/* date and this function can be removed.                                  */

static int splev(level)
{
#define SR	0x2000		/* status register with supervisor bit set */
	return( splx(SR | (level<<8)) );
}
