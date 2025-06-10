/*MH*******************************************************************

MODULE NAME:	bpp_driver.c

DESCRIPTION:	This module contains the functions composing the 
		Unix driver using the BPP and CI protocols.

RESTRICTIONS:	This driver cannot be compiled with the -T (truncate)
		option.  It must be compiled and linked seperately.

FUNCTIONS:	bpp_open
		bpp_clos
		bpp_read
		bpp_writ
		bpp_signal
		bpp_ioct

UPDATE LOG:
     Name	  Date	  Rev	Comments
-------------	--------  ---   ---------------------------------------
Dana Holgate	 3/20/86  1.0   Initial Coding
Ralph Lewis	04/15/86  1.0	Split bpp_driver from CI code
J. Holbrook	06/09/86  1.0	Initilize channel on first open.
B. Volquardsen	06/10/86  1.0	Changed name of bpp_intr to bpp_signal.
B. Volquardsen	06/16/86  1.0	Changed spl4() calls to splx() calls
				 using new "bpp_disable_level".
B. Volquardsen  07/22/86  1.1	Added "bpp_intr_depth" global variable
				 to avoid sleep() calls from interrupt
				 level code.
B. Volquardsen  08/28/86  1.1	Changed ci_trace() parameters to include
				 timestamp via "lbolt".
B. Volquardsen  11/11/86  2.0   Changed "common" include path to "sys/bpp"
				 and added includes "immu.h" & "region.h"
***********************************************************************/

#include "sys/errno.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/dir.h"
#ifndef R2   
#include "sys/immu.h"
#include "sys/region.h"
#endif
#include "sys/proc.h"
#include "sys/user.h"

#include "sys/bpp/types.h"
#include "sys/bpp/ce.h"
#include "sys/bpp/ci_config.h"
#include "sys/bpp/ci_types.h"
#include "sys/bpp/ci_const.h"
#include "sys/bpp/bpp.h"
#ifdef CETRACE
#include "sys/bpp/trace.h"
extern ULONG	lbolt;
extern TRACE_T bpp_trace;
#endif


extern	CHTBL 	lc[];

extern	VOID uwrite();
extern	EPB *uread();
extern	CI_CPUTAB_T	ci_initab[];
extern	int	bpp_disable_level;
int	bpp_intr_depth = 0;		/* global interrupt depth count */

int	bpp_rxint();

static char	devname[] = {"bpp_"};	/* this device name is used when 
					 * printing out debugging and error 
					 * information.  It is used so that 
					 * ifdefs are not required with each 
					 * printf call */

#define	BPP_MAX_SOCKET	32		/* max bpp socket number */

static struct bpp_sot {		/* indexed by minor_device */
	int	flags;		/* the inevitable catch-all */
#define	ST_BLOCK	0x01	/* block socket on read if no data */
#define	ST_SIGNAL	0x02	/* signal process when data arrives */
#define ST_IN_USE	0x04	/* socket is opened */
#define ST_RESERVED	0x08	/* socket is reserved, but not opened */
#define ST_WAIT		0x10	/* user is blocked waiting for input */
#define ST_DATA		0x20	/* flag to eliminate signal window. Means
				   data received, no signal sent. */
	int	signal;		/* signal to be sent to process */
	struct proc *proc_p;	/* pointer to process table */
	int	res_count;	/* number of times found to be reserved */
	int	(*rx_int)();	/* pointer to receive interrupt routine */
	int	ip;		/* socket user's ID */
} bpp_sot[BPP_MAX_SOCKET + 1];

static struct bpp_sot *ip_xref[NUM_REG_ENTRIES + 1];





/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_open

ACTION:

METHOD:

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Dana Holgate	 1/1/86		  1.0		Initial Release
Ralph Lewis	04/15/86	  2.0	    Split bpp_driver from CI code
Jim Holbrook	10/09/86	  2.0	    Set bpp_status at the start of
					    initialization instead of at
					    the end to prevent multiple
					    concurrent opens from starting
					    the initialization twice.
*************************************************************************/
bpp_open(dev)
dev_t dev;
{
	extern	VOID init_cpu();
	extern	USHORT bpp_status;

	register int cpu;
	short	min_dev;

#ifdef DEBUG
	printf("%sopen: opening minor device %d\n",devname,minor(dev));
#endif
	u.u_error = 0;
	min_dev = minor(dev);

	if (min_dev == 0) 
	{
	/* 
	**  If open is for device 0, we initialize the buffered pipe 
	**   if it's the first open;  otherwise, there is nothing to
	**   do until the user issues ioctl()
	*/

/*  
**  Initialization of channels to the configured cpu's is performed on
**  the first attempt to open the BPP driver.  If bpp_status is zero,
**  each cpu in the config table is initialized.
*/
	   if (bpp_status == 0)
	   {
	      bpp_status = -1;
	      for(cpu = 1; cpu <= CI_MAX_CPU; cpu++)
	      {
		   if (ci_initab[cpu].cpu_alive == TRUE)
		   {
			DEBUG2("%sinit: init cpu: %d\n", devname,cpu);
			init_cpu(cpu);
		   }
	      }
	   }
	   return;
	}

	u.u_error = bpp_kopen(min_dev, bpp_rxint);
	if (u.u_error)
		u.u_error = -u.u_error;
	return;
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_clos

ACTION:

METHOD:

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Dana Holgate	 1/1/86		  1.0		Initial Release
Ralph Lewis	04/15/86	  2.0	    Split bpp_driver from CI code
*************************************************************************/
#ifdef R2
bpp_clos(dev)
#else
bpp_close(dev)
#endif
dev_t		 dev;
{
	int		min_dev;

	min_dev = minor(dev);
#ifdef DEBUG
	printf("%sclos: closing minor device %d\n",devname,min_dev);
#endif
	u.u_error = 0;
	if (min_dev == 0)
		return;
	u.u_error = bpp_kclose(min_dev);
	if (u.u_error)
		u.u_error = -u.u_error;
	return;
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_read

ACTION:

METHOD:

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Dana Holgate	 1/1/86		  1.0		Initial Release
Ralph Lewis	04/15/86	  2.0	    Split bpp_driver from CI code
*************************************************************************/
bpp_read(dev)

dev_t		dev;

{
	int		total_length;
	int		move_length;
	int 		count;
	struct 		{
				struct bpp_ios	h;
				int		more[4];
			} io;
	int		min_dev;
	int		pri;
	register struct bpp_sot	*bso_p;
	register struct blk_descr *bd_p;
	
	min_dev = minor(dev);
	bso_p = bpp_sot + min_dev;
#ifdef DEBUG
	printf("%sread: reading minor device %d\n",devname,min_dev);
#endif
	u.u_error = 0;
	if (u.u_count > (MAX_EPB_SIZE + MAX_BUF_SIZE))
	{
		u.u_error = EINVAL;
		return;
	}

	if (bso_p->ip < 0)
	{
		u.u_error = EIDRM;
		return;
	}
	
	/* see if something is waiting */
	io.h.count = 3;
	pri = splx(bpp_disable_level);
	while ((total_length = bpp_kpoint(min_dev, &io)) == 0) {

		/* nothing waiting - see if we should block */
		if (bso_p->flags & ST_BLOCK) {

			/* block until something comes in */
#ifdef DEBUG
	printf("%sread: no data - sleeping\n",devname);
#endif
			bso_p->flags |= ST_WAIT;
			sleep(bso_p, PZERO+1);
		} else {

			/* user doesn't want to block */
#ifdef DEBUG
	printf("%sread: no data - no block - returning\n",devname);
#endif
			splx(pri);
			return;
		}

		/* we're awake again, set up to retry bpp_kpoint() */
		io.h.count = 3;
	}
	splx(pri);

#ifdef DEBUG
	printf("%sread: got data - epb = 0x%x, length = %d\n",
	devname, io.h.data_array.data_p, total_length);
#endif
	/* if we do not have good data, return to user */
	if (total_length < 1) {
		u.u_error = -total_length;
		return;
	}

	count = io.h.count;
	bd_p = &io.h.data_array;
	if (total_length > u.u_count)
		total_length = u.u_count;
	move_length = bd_p->data_len;
	if ( move_length > total_length ) {
		move_length = total_length;
	}

	/* move the epb */
	if (copyout(bd_p->data_p,u.u_base,move_length) != 0)
	{
		bpp_krel(min_dev, &io);
		u.u_error = EFAULT;
		return;
	}
#ifdef CETRACE
	ci_trace(&bpp_trace, 0x42524401,bd_p->data_p);		/* BRD */
	ci_trace(&bpp_trace,(((EPB *) bd_p->data_p)->type << 16) 
		| ((EPB *) bd_p->data_p)->status, lbolt);
#endif

	/* move buf(s) */
	total_length -= move_length;
	u.u_count -= move_length;
	while (--count ) {
		if (total_length  <= 0) 
			break;
		u.u_base += move_length;
		bd_p++;
		if ((move_length = bd_p->data_len) > total_length)
			move_length = total_length;
		if (copyout(bd_p->data_p, u.u_base, move_length) != 0)
		{
			bpp_krel(min_dev, &io);
			u.u_error = EFAULT;
			return;
		}
		total_length -= move_length;
		u.u_count -= move_length;
	}
	bpp_krel(min_dev, &io);
	return;
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_writ

ACTION:

METHOD:

UPDATE LOG:
     Name	  Date		Rev	Comments
-------------	--------	---    ----------------------
Dana Holgate	 1/1/86		1.0	Initial Release
Ralph Lewis	04/15/86	1.0	Split bpp_driver from CI code
Jim Holbrook	05/13/86	1.0	Modified for multiple cpus
Jim Holbrook	05/22/86	1.0	Copyin buf length field only
					    to preserve buf pool ID.
Jim Holbrook	06/12/86	1.0	Added return to error path
					    on check for cpu_running.
*************************************************************************/
#ifdef R2
bpp_writ(dev)
#else
bpp_write(dev)
#endif
dev_t		dev;
{
	VOID release_sepb();
	BOOLEAN dst_ok();
	BUF *get_sbuf();
	EPB *get_sepb();

	register EPB *epb;
	register BUF *buf = (BUF *)NULL;
	register int	min_dev;
	register ULONG  dest_cpu;	/* destination cpu */
	EPB	 local_epb_copy;	/* local copy of header -- used
					   to look at destination address
					   before allocating a buffer. */
	POOL_T	pool;			/* temporary hold location for
					   pool id during copy of EPB */

	ULONG	buf_length;		/* temporary variable to copy in
					   buf data length */
	min_dev = minor(dev);
#ifdef DEBUG
	printf("%swrit: writing minor device %d\n", devname, min_dev);
#endif
	u.u_error = 0;
	if (u.u_count > (MAX_EPB_SIZE + MAX_BUF_SIZE))
	{
		u.u_error = EINVAL;
		return;
	}
/* get the header portion of EPB */
	if (copyin(u.u_base, &local_epb_copy, sizeof(EPB)) != 0)
	{
		u.u_error = EFAULT;
		return;
	}
	dest_cpu = GET_CPU(local_epb_copy.dst_id);
	if (dest_cpu > CI_MAX_CPU+1)
	{
		u.u_error = EINVAL;
		return;
	}
	if (dest_cpu == CI_MAX_CPU+1)
	{ /* map local cpu to 0 for config table tests */
		dest_cpu = 0;
	}
	if (ci_initab[dest_cpu].cpu_alive != TRUE ||
	    ci_initab[dest_cpu].cpu_running != TRUE)
	{  /* destination cpu is not available */
		u.u_error = EINVAL;
		return;
	}

/* get the shared epb */
	if ((epb = get_sepb(dest_cpu)) == (EPB *)NULL)
	{
		u.u_error = ENOMEM;
		return;
	}
	pool = epb->pool;	/* save buffer pool id */
	copy(&local_epb_copy,epb,sizeof(EPB));
	epb->pool = pool;

	if (epb->epb_size > u.u_count || (ULONG)epb->buf > u.u_count)
	{
		release_sepb(epb);
		u.u_error = ENOTBLK;
		return;
	}
/* get user data portion of EPB */
	if (epb->epb_size > sizeof(EPB))
	{
		if (copyin(u.u_base + sizeof(EPB), (BYTE *)epb + sizeof(EPB), epb->epb_size - sizeof(EPB)) != 0)
		{
			release_sepb(epb);
			u.u_error = EFAULT;
			return;
		}
	}
/* get buffer if attached */
	if (epb->buf != (BUF *)NULL)
	{
		if ((ULONG)epb->buf < epb->epb_size)
		{
			release_sepb(epb);
			u.u_error = ENOTBLK;
			return;
		}
		if ((buf = get_sbuf(dest_cpu)) == (BUF *)NULL)
		{
			release_sepb(epb);
			u.u_error = ENOMEM;
#ifdef DEBUG
	printf("%swrit: can't get buf\n", devname);
#endif
			return;
		}
/* get the header portion of BUF */
		if (copyin(u.u_base + (ULONG)epb->buf + BUF_HDR_SIZE - 4, 
			     (char *) &buf_length, 4) != 0)
		{
			release_sbuf(dest_cpu, buf);
			release_sepb(epb);
			u.u_error = EFAULT;
			return;
		}
		buf->length = buf_length;
		if ((ULONG)epb->buf + buf->length > u.u_count)
		{
			release_sbuf(dest_cpu, buf);
			release_sepb(epb);
			u.u_error = ENOTBLK;
			return;
		}
/* get the user data portion of BUF */
		if (buf->length > 0)
		{
			if (copyin(u.u_base + (ULONG)epb->buf + BUF_HDR_SIZE, buf->data, u.u_count - (ULONG)epb->buf - BUF_HDR_SIZE) != 0)
			{
				release_sbuf(dest_cpu, buf);
				release_sepb(epb);
				u.u_error = EFAULT;
				return;
			}
#ifdef DEBUG
	printf("%swrite: buf copied - buf = 0x%x, length = %d\n",
		devname, epb->buf, u.u_count - (ULONG) epb->buf);
#endif
		}
		epb->buf = buf;
	}

	/* send it */
	u.u_error = bpp_sendit(min_dev, epb, buf);
	u.u_count = 0;
	if (u.u_error)
		u.u_error = -u.u_error;
	return;
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_ioct

ACTION:

METHOD:

UPDATE LOG:
     Name	  Date		Rev	Comments
-------------	--------	---    ----------------------
Dana Holgate	 4/1/86		1.0	Initial Coding
Ralph Lewis	04/15/86 	1.0 	Split bpp_driver from CI code
Jim Holbrook	06/23/86	1.0	Added BPP_RESET_CPU command. 
*************************************************************************/

#ifdef R2
bpp_ioct(dev, command, arg, mode)
#else
bpp_ioctl(dev, command, arg, mode)
#endif
dev_t		dev;
int		command;
char		*arg;
int		mode;
{
	extern REG_ENT rt[];
	extern VOID init_cpu();

	int		min_dev;
	int		i;
	register struct bpp_sot	*bso_p;
	struct bpp_ios	io;
	int			pri;	/* save interrupt level */
	register REG_ENT	*rgptr;	/* pointer to registration table */

	u.u_error = 0;
	min_dev = minor(dev);
	bso_p = bpp_sot + min_dev;

#ifdef CETRACE
	ci_trace(&bpp_trace, 0x42494f01,min_dev);		/* BIO */
	ci_trace(&bpp_trace,command,(ULONG) arg);
#endif
#ifdef DEBUG
	printf("%sioct: ioctl on minor device %d, cmd = %d\n",
		devname,min_dev, command);
#endif

	switch (command) {

	/* get a socket number */	
	case BPP_NEXT_SOCKET:	
		if ((i = bpp_knxt()) > 0) {
			copyout((char *)&i, arg, sizeof(int));
		} else
			u.u_error = -i;
		break;

	/* return the CPU configuration table */
	case BPP_CONFIG:
		i = CI_MAX_CPU;
		if (copyout((char *)&i,arg, sizeof(int)) != 0)
		{
		    u.u_error = EFAULT;
		}

		if (copyout((char *)&ci_initab[1],arg+4,
		         sizeof(CI_CPUTAB_T)*CI_MAX_CPU) != 0)
		{
		    u.u_error = EFAULT;
		}
		break;

	/* register a signal to be sent when data arrives */	
	case BPP_SIGNAL:	
		if (min_dev == 0) {
			u.u_error = EINVAL;
			break;
		}

		if (! dst_ok(bso_p->ip))
		{
			u.u_error = EINVAL;
			break;
		}
		pri = splx(bpp_disable_level);
		bso_p->signal = (int)arg;
		bso_p->proc_p = u.u_procp;
		bso_p->flags |= ST_SIGNAL;

/*  The ST_DATA flag is used to close a window during which data could
    arrive but a sginal not be sent.  The ST_DATA flag is set during
    processing of the pipe interrupt when data is received, but no signal
    is sent.  If the flag is set when we arrive here, a signal needs to be
    sent. */
		if (bso_p->flags & ST_DATA)
		{
			psignal(bso_p->proc_p, bso_p->signal);
			bso_p->signal = 0;
			bso_p->flags &= ~ST_SIGNAL;
			bso_p->flags &= ~ST_DATA;
		}
		splx(pri);
		break;

	/* cancel a previous signal registration */
	case BPP_NSIGNAL:
		if (min_dev == 0) {
			u.u_error = EINVAL;
			break;
		}
		bso_p->signal = 0;
		bso_p->proc_p = NULL;
		bso_p->flags &= ~ST_SIGNAL;
		break;

	/* set socket to block on read if no data is found */
	case BPP_BLOCK:
		bso_p->flags |= ST_BLOCK;
		break;

	/* set socket to not block on read if no data is found */
	case BPP_NBLOCK:
		bso_p->flags &= ~ST_BLOCK;
		break;

	/* reset the cpu indicated by the CPU number in arg */
	case BPP_RESET_CPU:
		if ((ULONG) arg > CI_MAX_CPU || (ULONG) arg < 1)
		{
		    u.u_error = EINVAL;
		    break;
		}
		ci_initab[(ULONG) arg].cpu_running = FALSE;

		(*ci_initab[(ULONG) arg].rst_write)(ci_initab[(ULONG) arg].icc_rst_mask);
		init_cpu((ULONG) arg);
/******************************************************
		if (init_cpu((ULONG) arg) != TRUE)
		{
		    u.u_error = EINVAL;	
		}
		else
		    ci_initab[(ULONG) arg].cpu_running = FALSE;
		break;
********************************************************/

	/* this is a no-no */
	default:
		u.u_error = EINVAL;
		break;
	}
	return;
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_rxint

ACTION:	Process interrupt for incoming packet.

METHOD:	If the socket related to the incoming transaction wants a wakeup()
	or a signal(), give it to him.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
*************************************************************************/

int
bpp_rxint(minor_device, epb_p)

int		minor_device;
EPB		*epb_p;

{
	register struct bpp_sot	*bso_p;

	bso_p = bpp_sot + minor_device;

	/* if user wants signal, give it to him */
	if (bso_p->flags & ST_SIGNAL) {
#ifdef CETRACE
	ci_trace(&bpp_trace, 0x52584901,bso_p->signal);		/* RXI */
	ci_trace(&bpp_trace, bso_p->flags, lbolt);
#endif
		psignal(bso_p->proc_p, bso_p->signal);
		bso_p->flags &= ~ST_SIGNAL;
		bso_p->flags &= ~ST_DATA;
		bso_p->signal = 0;
	}
	else
	{  /* close with ioctl signal arming.  ST_DATA set means
	      that we've received data and not sent a signal.  The 
	      ioctl routine will send a signal if this flag is set
	      and then reset the flag. */

		bso_p->flags |= ST_DATA;
	}

	/* if the user wants to wake up, now is the time */

	if ((bso_p->flags & ST_BLOCK) && (bso_p->flags &ST_WAIT)) {
		wakeup(bso_p);
		bso_p->flags &= ~ST_WAIT;
	}

	return;

}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_kcnfg;

ACTION:	Return a CPU table to the caller.

METHOD:	Extract the CPU number and type from the CPU table.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
*************************************************************************/
bpp_kcnfg(minor_device, cputab_p)

int		minor_device;
struct cpu_tab	*cputab_p;

{
	
	CI_CPUTAB_T		*citab_p;
	struct cpu_descr	*cpudescr_p;
	int			i;

	citab_p = ci_initab;
	citab_p++;
	cpudescr_p = cputab_p->cpu_descr;
	i = CI_MAX_CPU;
	if ( i > cputab_p->count)
		i = cputab_p->count;
	
	while(i-- > 0) {
		cpudescr_p->cpu_type = citab_p->cpu_type;
		cpudescr_p->cpu_id = citab_p->cpu_number;
		cpudescr_p++;
		citab_p++;
	}

	return(0);
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_knxt

ACTION:	Find an unused socket and return the socket number to the caller.

METHOD:	Search the socket table for an entry that is not in use and is not
	reserved.  If no elegible entry is found, search the table again
	for an entry that was reserved but never claimed.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
*************************************************************************/
bpp_knxt()
{
	int i;
	register struct bpp_sot *bso_p;

	bso_p = bpp_sot;
	for (i = 1; i <= BPP_MAX_SOCKET ; i++) {
		bso_p++;
		/* if socket is in use or reserved, pass on by */
		if (bso_p->flags & ST_IN_USE )
			continue;
		
		if (bso_p->flags & ST_RESERVED) {
			bso_p->res_count++;
			continue;
		}
		
		/* socket is available, reserve it */
		bso_p->flags |= ST_RESERVED;
		bso_p->proc_p = u.u_procp;
		bso_p->res_count = 0;
		return(i);
	}

	/* No socket was available.  Make a second pass and see if
	 * a socket was reserved and never claimed */
	for ( i = 1; i <= BPP_MAX_SOCKET ; i++ ) {
		if ((bso_p->flags & ST_RESERVED) && (bso_p->flags > 10)) {
			bso_p->proc_p = u.u_procp;
			bso_p->res_count = 0;
			return(i);
		}
	}

	return(-ENOSPC);
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_kopen

ACTION:	Open a socket.  Register an interrupt routine.

METHOD:	Open a socket, if the request is legitimate.
	Register an interrupt routine, if it was supplied.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
B. Volquardsen  08/13/86	  2.0	    Changed bso_p->flags init.
					    to clear any flags from 
					    previous use of socket.
*************************************************************************/

bpp_kopen(minor_device, rx_int)

int	minor_device;
int	(*rx_int)();
{
	register struct bpp_sot	*bso_p;

	bso_p = bpp_sot + minor_device;

	/* User can not open a socket that is in use */
	if (bso_p->flags & ST_IN_USE) {
		return(-EBUSY);
	}

	/* The socket must be specifically reserved for this user */
	if ((bso_p->flags & ST_RESERVED)  && (bso_p->proc_p == u.u_procp)) {
		bso_p->flags = ST_IN_USE;
		bso_p->rx_int = rx_int;
		bso_p->ip = -1;
		return(0);
	}

	return(- EINVAL);
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_kpoint

ACTION:	Point to a transaction

METHOD:	Return a structure containing pointers to an epb and (possibly) a
	buffer.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
*************************************************************************/

bpp_kpoint(minor_device, bio_p)

int		minor_device;
struct bpp_ios	*bio_p;

{
	register struct bpp_sot		*bso_p;
	register struct blk_descr	*bd_p;
	register EPB			*epb_p;
	BUF			*buf_p;
	register int			count, length;
	short 			ip;


	bso_p = bpp_sot + minor_device;
	ip = bso_p->ip;
	length = 0;

	if ((count = bio_p->count) < 1)	/* max number of blocks to user */
		return(-ENOSPC);		
	bio_p->count = 0;		/* actual number of blocks to user */

	bd_p = &bio_p->data_array;

	if ((epb_p = uread(ip)) == NULL) 
		return(0);

	bso_p->flags &= ~ST_DATA;	/* reset ioctl signal flag */

	/* save epb pointer and epb length */
	bd_p->data_p = (char *)epb_p;
	length = epb_p->epb_size;
	bd_p->data_len = length;
	bio_p->count++;

	bd_p++;
	if (--count) {
		if ((buf_p = epb_p->buf) != (BUF *)NULL) {
			bd_p->data_p = (char *)buf_p->data;
			bd_p->data_len = buf_p->length;
			length += buf_p->length;
			bio_p->count++;
		} else
			bd_p->data_len = 0;
	}

	return(length);

}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_krel

ACTION:	Release epb and buffers.

METHOD:	Go thru the list from the caller and release the epb and buffers.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
*************************************************************************/

bpp_krel(minor_device, bio_p)

    register int	minor_device;
    register struct bpp_ios	*bio_p;

{
	register ULONG	cpu;
	register int	count;
	register EPB	*epb_p;
	register BUF	*buf_p;
	register struct blk_descr *bd_p;

	count = bio_p->count;
	bd_p = &bio_p->data_array;
	epb_p = (EPB *)bd_p->data_p;
	buf_p = epb_p->buf;
	cpu = epb_p->pool.cpu;

	release_sepb(epb_p);	 /* release the epb */
	
	/* if any buffers exist, release them */

	if (--count)
		release_sbuf(cpu,buf_p);

	return(0);

}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_kwrite

ACTION:	Prepare epb and buffer(s) for transmission.

METHOD:	Go thru the list from the caller.  The first item is an epb.  The
	remaining items are data items to be collected into a buffer.

UPDATE LOG:
     Name	  Date		Rev	Comments
-------------	--------	---    ----------------------------------
Ralph Lewis	04/15/86	1.0	Initial Code
B. Volquardsen	06/25/86	1.0	Fixed cpu_running checks to 
					 return error code -EINVAL to
					 caller if not alive & running.
*************************************************************************/

bpp_kwrite(minor_device, bio_p)

int		minor_device;
struct bpp_ios	*bio_p;

{
	extern BUF	*get_sbuf();
	extern EPB	*get_sepb();

	register int	count;
	register int	length;
	int		err;
	register char	*cin_p, *cout_p;
	register EPB	*epb_p;
	register BUF	*buf_p;
	register ULONG	dest_cpu;	/* destination cpu from dest_id --
					   needed to get epb from the
					   correct pool */
	POOL_T	pool;			/* temporary save location for epb
					   pool id during copy to epb */
	struct blk_descr	*bd_p;

	if ((count = bio_p->count) < 1)
		return(-EINVAL);
	
	/* make sure the epb and data are within legal size */
	bd_p = &bio_p->data_array;
	if (bd_p->data_len > MAX_EPB_SIZE)
		return(-EINVAL);
	
	/* get the destination from the EPB header */
	epb_p = (EPB *) bd_p->data_p;
	dest_cpu = GET_CPU(epb_p->dst_id);

	if (dest_cpu > CI_MAX_CPU+1)
	{
		return(-EINVAL);
	}
	if (dest_cpu == CI_MAX_CPU+1)
	{ /* map local cpu to 0 for config table tests */
		dest_cpu = 0;
	}
	if (ci_initab[dest_cpu].cpu_alive != TRUE ||
	    ci_initab[dest_cpu].cpu_running != TRUE)
	{  /* destination cpu is not available */
		return(-EINVAL);
	}


	/* accumulate data block lengths */
	length = 0;
	while (--count) {
		bd_p++;
		length += bd_p->data_len;
	}

	if (length > MAX_BUF_SIZE)
		return(-EINVAL);

	/* get an epb and (if needed) a buf */
	if ((epb_p = get_sepb(dest_cpu)) == (EPB *)NULL)
		return(-ENOMEM);

	pool = epb_p->pool;	/* save epb pool id */
	if (length) {
		if ((buf_p = get_sbuf(dest_cpu)) == (BUF *)NULL) {
			release_sepb(epb_p);
			return(-ENOMEM);
		}
		buf_p->next = NULL;
		buf_p->length = length;
	} else
		buf_p = NULL;

	/* copy data to epb */
	bd_p = &bio_p->data_array;
	cin_p = bd_p->data_p;
	cout_p = (char *)epb_p;
	for (length = bd_p->data_len; length > 0; length--)
		*cout_p++ = *cin_p++;

	epb_p->pool = pool;	/* restore epb pool id after copy */
	epb_p->buf = buf_p;

	/* if there is data, copy data to buf */
	count = bio_p->count;
	cout_p = (char *)buf_p;
	cout_p += BUF_HDR_SIZE;
	while (--count) {
		bd_p++;
		cin_p = bd_p->data_p;
		for (length = bd_p->data_len; length > 0; length--)
			*cout_p++ = *cin_p++;
	}

	/* send it */
	err = bpp_sendit(minor_device, epb_p, buf_p);
	if (err)
		return(err);
	else
		return(0);

}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_sendit

ACTION:	Write epb and buffer to a pipe.

METHOD:	If this is a registration or de-registration, call registration.
	Otherwise, send as requested.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
*************************************************************************/
bpp_sendit(minor_device, epb_p, buf_p)

int		minor_device;
EPB		*epb_p;
BUF		*buf_p;

{
	int		err;
	register struct bpp_sot	*bso_p;
	DEFAULT ip;

	bso_p = bpp_sot + minor_device;
	epb_p->buf = buf_p;
	ip = GET_IP(epb_p->src_id);

	/* handle registration & deregistration */
	switch (epb_p->type) {
	case CE_REGISTRATION_REQUEST:

		/* allow only one registration per socket */
		if (bso_p->ip != -1) {
			if (buf_p != NULL)
				release_sbuf(GET_CPU(epb_p->src_id), buf_p);
			release_sepb(epb_p);
			return(-EINVAL);
		}

		if ((err = registration(epb_p, minor_device)) != NO_ERROR)
		{
			if (buf_p != (BUF *)NULL)
				release_sbuf(GET_CPU(epb_p->src_id), buf_p);
			release_sepb(epb_p);
			return(-err);
		}
#ifdef DEBUG
		dump_epb("after registration:\n",epb_p);
#endif
		ip = GET_IP(epb_p->dst_id);
		ip_xref[ip] = bso_p;		/* save socket xref pointer */
		bso_p->ip = ip;			/* save ip xref */
		return(0);

	case CE_DEREGISTRATION_REQUEST:
		if ((err = registration(epb_p, minor_device)) != NO_ERROR)
		{
			if (buf_p != (BUF *)NULL)
				release_sbuf(GET_CPU(epb_p->src_id), buf_p);
			release_sepb(epb_p);
			return(-err);
		}
		ip_xref[ip] = NULL;		/* zero out socket xref */
		bso_p->ip = -1;			/* indicate no registration */
		return(0);


	default:
		/* is the user registered */
		if (bso_p->ip <= 0)
		{
			if (buf_p != (BUF *)NULL)
				release_sbuf(GET_CPU(epb_p->src_id), buf_p);
			release_sepb(epb_p);
			return(-EIDRM);
		}
		break;
	}

	/* update the source cpu and destination cpu */
	/* we don't allow lying about the src_id */
	epb_p->src_id = PUT_ID(lc[0].local_cpu,bso_p->ip);
	
	if (GET_CPU(epb_p->dst_id) == LOCAL_CPU)
		epb_p->dst_id = PUT_ID(lc[0].local_cpu,GET_IP(epb_p->dst_id));
	
	/* is it a local destination */
	if (GET_CPU(epb_p->dst_id) == lc[0].local_cpu)
	{
	/* if it is a local destination it must be registered */
		if (!dst_ok(GET_IP(epb_p->dst_id)))
		{
			if (buf_p != (BUF *)NULL)
				release_sbuf(GET_CPU(epb_p->src_id), buf_p);
			release_sepb(epb_p);
			return(-ESRCH);
		}
		uwrite(GET_IP(epb_p->dst_id),epb_p);
	}
	/* it is a remote destination */
	else
		if (pwrite(epb_p) != NO_ERROR)
		{
			if (buf_p != (BUF *)NULL)
				release_sbuf(GET_CPU(epb_p->src_id), buf_p);
#ifdef DEBUG
	printf("%ssendit: pwrite failed\n");
#endif
			release_sepb(epb_p);
			return(-ENOMEM);
		}
	return(0);
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_kclose

ACTION:	Close a socket.

METHOD:	Clean up the socket table & make ready for re-use.  If any
	registrations exist for this socket, de-register them cuz this
	is an abnormal close - possibly the process has crashed.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
*************************************************************************/

bpp_kclose(minor_device)

int		minor_device;

{
	register int	ip;
	register struct bpp_sot		*bso_p, **ixr_p;
	register EPB	*epb_p;

	bso_p = bpp_sot + minor_device;

	/* if socket isn't open, don't close it */
	if ( !(bso_p->flags & ST_IN_USE))
		return(-EINVAL);

	/* check for any registrations for this socket */
	ixr_p = ip_xref;
	for (ip = 1; ip <= NUM_REG_ENTRIES; ip++) {
		ixr_p++;

		/* if xref points to the socket we are closing, 
		 * de-register the ip */
		if (*ixr_p == bso_p) {
			/* get an epb */
			if ((epb_p = get_sepb(lc[0].local_cpu)) == NULL)
				return;
	
			/* fill in the parts de-registration looks at */
			epb_p->buf = NULL;
			epb_p->type = CE_DEREGISTRATION_REQUEST;
			epb_p->src_id = PUT_ID(LOCAL_CPU,ip);
			if (registration(epb_p, minor_device))
				release_sepb(epb_p);
		}
	}
	bso_p->flags &= ~(ST_RESERVED | ST_IN_USE);
	bso_p->rx_int = NULL;
	bso_p->proc_p = NULL;

	return(0);
}




/*FH************************* FUNCTION HEADER ****************************

FUNCTION NAME:	bpp_signal

ACTION:	Handle interrupt

METHOD:	A packet has arrived for this CPU.  Notify users who want to know
	about this.

UPDATE LOG:
     Name	  Date		Vers/Rev	  Comments
-------------	--------	--------    ----------------------
Ralph Lewis	04/15/86	  2.0	    Initial Code
*************************************************************************/

void
bpp_signal(ip, epb_p)

register int	ip;
register EPB	*epb_p;

{
	int minor_device;
	register struct bpp_sot	*bso_p, **ixr_p;

	/* verify legal addressee */
	if ( ip > NUM_REG_ENTRIES)
		return;

	/* relate addressee to a socket */
	ixr_p = ip_xref + ip;
	bso_p = *ixr_p;

	/* calculate socket number */
	minor_device = bso_p - bpp_sot;

	/* indicate to bpp that we are in interrupt level code */
	bpp_intr_depth++;

	/* if user wants notification, call user now */
	if (bso_p->rx_int != NULL)
		(*bso_p->rx_int)(minor_device,epb_p);

	/* reset bpp interrupt level flag */
	bpp_intr_depth--;
	return;
}
