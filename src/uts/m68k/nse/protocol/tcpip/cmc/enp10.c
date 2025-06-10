/*	Copyright (c) 1984 by Communication Machinery Corporation 
 *
 *	This file contains material which is proprietary to
 *	Communication Machinery Corporation (CMC) and which
 *	may not be divulged without the written permission
 *	of CMC.
 */

/*
 *	File:	 enp10.c	Module(s): HOST Resident
 *	ENP:	 10/VMEbus
 *	Unix:	 System V
 *
 *	This file implements the support procedures utilized by
 *	sock_driver.c which may change on a per-system basis.
 *	In particular, details of moving data between the user
 *	address space and the kernel and/or ENP are handled here;
 *	also included are those routine which may vary for different
 *	models of ENP.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/fs/s5dir.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/CMC/errno.h>
#include <sys/CMC/signal.h>
#include <sys/CMC/enp_addr.h>
#include <sys/CMC/netbfr.h>
#include <sys/CMC/ring.h>
#include <sys/CMC/CMC_proto.h>
#include <sys/CMC/CMC_socket.h>
#include <sys/CMC/enp.h>
#include <sys/CMC/copyin.h>

/*
 *	Usefull constants
 */

# define K *1024


/*
 *	Fundamental attributes of the implementation
 */

# define INTENP 0xE01			/* Poke to interrupt ENP */
# define ENPVAL 0xFF			/* Value to poke */

# define ENPINTBASE (ENPBASE + 124 K)
# define ENPINT (caddr_t)(ENPINTBASE + INTENP)		/* Intr. addr */

# define ENPGOADDR	0x1000				/* Start addr */
# define GOCMD		0x8080				/* Start command */

# define ENPRSADDR	((124 K) + 1)			/* Reset addr */
# define RSCMD		0x01				/* Reset command */

struct startup {
	short	g_cmd;
	short	g_dummy;
	long	g_addr;
};

/*	global location for enp board in system */
int	enp_in_system = 0;		/* 0=board not present */

/*
 *	ENP model dependent routines
 *
 *	intenp()		Interrupt the ENP
 *	enpintr()		ENP interrupt handler
 */

intenp()
{
	*ENPINT = ENPVAL;
}

enpintr()
{
	enpservice();
}

enpgo(enpaddr, arg)
{
	struct startup *enpgo;

	enpgo = (struct startup *) (enpaddr + ENPGOADDR);
	enp_assign(enpgo -> g_addr, arg);
	enpgo -> g_cmd = GOCMD;
}

resetenp(enpaddr)
{
	/*
	 * Rev A & B ENP-10's can't be reset.
	 */
	short *reset;
	reset = (short *)(enpaddr + 0x1fff0);
	if(bprobe(reset,-1)) {
		enp_in_system = 0;
		return(1);
	}
	else
		enp_in_system = 1;
	*reset = (short)(0xffff);
	return(0);
}


/*
 *	Unix dependent routines
 *
 *	kern_to_user()		Move data from kernel to user space
 *	user_to_kern()		Move data from user to kernel space
 *	splnet()		Set processor level for ENP
 *	device()		Convert user fid to major,minor
 */

user_to_kern(from, to , count)
{
	if (count > 0) {
		if (short_copyin (from, to, count)) {
			u.u_error = EFAULT;
			return (EFAULT);
		}
	}
	return (0);
}

kern_to_user(from, to, count)
{
	if (count > 0) {
		if (short_copyout (from, to , count)) {
			u.u_error = EFAULT;
			return (EFAULT);
		}
	}
	return (0);
}

splnet()
{
	return (spl6());
}

device(fid)
{
	register struct file *fp;

	fp = (struct file *) getf (fid);	/* convert fid to file * */
	if (fp == 0)				/* failed, so fid is bogus */
		return (-1);

	return ((int) fp->f_inode->i_rdev);
}
