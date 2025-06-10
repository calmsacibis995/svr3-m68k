/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/user.h	10.15"

/*
 * The user structure.  One allocated per process.  Contains all per process
 * data that doesn't need to be referenced while the process is swapped.
 * Currently, user blocks are NOT swapped with the process.  This may change
 * in the future.  The user block is USIZE*click bytes long and resides at 
 * kernel virtual address 0x1000000.  The user structure starts at the beginning
 * and the kernel system stack starts at the back-end of the block and grows
 * forward.  There is one system stack per user and a separate interrupt stack
 * for the entire system.  The interrupt stack is located outside the user 
 * block.  The user structure is cross referenced with the proc structure
 * for the same process.
 */
 
#define	PSARGSZ	80		/* Space in u-block for exec arguments.	*/
				/* Used by ps command.			*/

#define	PSCOMSIZ	DIRSIZ	/* For the time being set PSCOMSIZ	*/
				/* to DIRSIZ until we can get rid of	*/
				/* struct direct u_dent			*/

#include "fpu.h"
typedef	struct	user
{
	label_t	u_rsav;		/* save info when exchanging stacks	*/
	label_t u_qsav;		/* label variable for quits and interrupts */
	
	int	u_caddrflt;	/* Ptr to function to handle		*/
				/* user space external memory		*/
				/* faults encountered in the		*/
				/* kernel.				*/

	char	u_filler1[40];	/* DON'T TOUCH--this is used by cond-	*/
				/* itionally-compiled code in iget.c	*/
				/* which checks consistency of inode 	*/
				/* locking and unlocking.  Name change 	*/
				/* to follow in a later release.	*/
	
	char	u_nshmseg;	/* Nbr of shared memory			*/
				/* currently attached to the		*/
				/* process.				*/

 	struct rem_ids {		/* for exec'ing REMOTE text	*/
 		ushort	ux_uid;		/* uid of exec'd file		*/
 		ushort	ux_gid;		/* group of exec'd file		*/
 		ushort	ux_mode;	/* file mode (set uid, etc.	*/
 	} u_exfile;

	char	*u_comp;		/* pointer to current component */
	char	*u_nextcp;		/* pointer to beginning of next */
					/* following for Distributed UNIX */
	ushort 		u_rflags;	/* flags for distripution	*/
	struct cookie	u_rrcookie;	/* pointer to remote root inode */
	int 		u_syscall;	/* system call number		*/
	int		u_mntindx;	/* mount index from sysid	*/
	struct sndd	*u_gift;	/* gift from message     	*/
	struct cookie	 u_newgift;	/* cookie to be returned to client*/
	struct response	*u_copymsg;	/* copyout unfinished business	*/
	struct msgb	*u_copybp;	/* copyin premeditated send	*/
	char 		*u_msgend;	/* last byte of copymsg + 1	*/
					/* end of Distributed UNIX	*/
	
	long	u_bsize;		/* block size of device		*/
	char	u_psargs[PSARGSZ];	/* args from exec system call	*/
	char	*u_tracepc;		/* Return PC if tracing enabled	*/

	void(*u_signal[MAXSIG])();	/* disposition of signals	*/

	int	u_shmcnt;		/* no. of shared segments	*/
					/* attached to this process.    */

	char	u_segflg;		/* IO flag: 0:user D; 1:system;	*/
					/*          2:user I		*/

	char	u_error;		/* return error code		*/

	ushort	u_uid;			/* effective user id		*/
	ushort	u_gid;			/* effective group id		*/
	ushort	u_ruid;			/* real user id			*/
	ushort	u_rgid;			/* real group id		*/

	struct proc *u_procp;		/* pointer to proc structure	*/

	int	*u_ap;			/* pointer to arglist		*/

	union {				/* syscall return values 	*/
		struct	{
			int	r_val1;
			int	r_val2;
		}r_reg;
		off_t	r_off;
		time_t	r_time;
	} u_r;

	caddr_t	u_base;		/* base address for IO		*/
	unsigned u_count;	/* bytes remaining for IO	*/
	off_t	u_offset;	/* offset in file for IO	*/

	short	u_fmode;	/* file mode for IO		*/
	ushort	u_pbsize;	/* Bytes in block for IO	*/
	ushort	u_pboff;	/* offset in block for IO	*/
	short	u_magic;	/* magic number of process	*/
	short	u_errcnt;	/* syscall error count		*/
	dev_t	u_pbdev;	/* real device for IO		*/

	daddr_t	u_rablock;	/* read ahead block address	*/

	struct inode *u_cdir;	/* current directory		*/

	struct inode *u_rdir;	/* root directory		*/
	caddr_t	u_dirp;		/* pathname pointer		*/
	struct direct u_dent;	/* current directory entry	*/
	struct inode *u_pdir;	/* inode of parent directory	*/
				/* of dirp			*/


	int	*u_stack;	/* Ptr to start of kernel stack.*/
	char	*u_pofile;	/* Ptr to open file flag array.	*/

	int	u_arg[6];	/* arguments to current system call */

	unsigned u_tsize;	/* text size (clicks)		*/
	unsigned u_dsize;	/* data size (clicks)		*/
	unsigned u_ssize;	/* stack size (clicks)		*/

	time_t	u_utime;	/* this process user time	*/
	time_t	u_stime;	/* this process system time	*/
	time_t	u_cutime;	/* sum of childs' utimes	*/
	time_t	u_cstime;	/* sum of childs' stimes	*/

	int	*u_ar0;		/* address of users saved R0	*/

	struct {			/* profile arguments	*/
		short	*pr_base;	/* buffer base		*/
		unsigned pr_size;	/* buffer size		*/
		unsigned pr_off;	/* pc offset		*/
		unsigned pr_scale;	/* pc scaling		*/
	} u_prof;

	short  *u_ttyp;		/* pointer to pgrp in "tty" struct	*/
	dev_t	u_ttyd;		/* controlling tty dev			*/
	struct inode *u_ttyip;	/* inode of controlling tty (streams)	*/

	long   u_execsz;

	struct exdata { 		/* Executable file info.	*/
		struct    inode  *ip;
		long      ux_tsize;	/* text size			*/
		long      ux_dsize;	/* data size			*/
		long      ux_bsize;	/* bss size			*/
		long      ux_lsize;  	/* lib size			*/
		long      ux_nshlibs; 	/* number of shared libs needed */
		short     ux_mag;   	/* magic number MUST be here	*/
		long      ux_toffset;	/* file offset to raw text      */
		long      ux_doffset;	/* file offset to raw data      */
		long      ux_loffset;	/* file offset to lib sctn      */
		long      ux_txtorg;	/* start addr. of text in mem   */
		long      ux_datorg;	/* start addr. of data in mem   */
		long      ux_entloc;	/* entry location               */
	} u_exdata;

	char	u_comm[PSCOMSIZ];

	time_t	u_start;
	time_t	u_ticks;
	long	u_mem;
	long	u_ior;
	long	u_iow;
	long	u_iosw;
	long	u_ioch;
	char	u_acflag;

	char	u_traptype;		/* records the type of last trap      */
#define TRAPNORM	0		/* "normal" trap		      */
#define TRAPBUS		1		/* bus error trap		      */
#define TRAPADDR	2		/* address error trap		      */
#define TSAVSIZE	22		/* size of long fault frame save area */
	char	u_trapflag;		/* flag that trapsave has valid info  */
	long	u_trapsave[TSAVSIZE];	/* long fault frame save area	      */

	fpu_info u_fpu;		

	short	u_cmask;		/* mask for file creation	*/
	daddr_t	u_limit;		/* maximum write address	*/

	short	u_lock;			/* process/text locking flags	*/

	struct file	*u_ofile[1];	/* Start of array of pointers	*/
					/* to file table entries for	*/
					/* open files.			*/
} user_t;

extern struct user u;

#define u_exuid u_exfile.ux_uid
#define u_exgid u_exfile.ux_gid
#define u_exmode u_exfile.ux_mode

#define	u_rval1	u_r.r_reg.r_val1
#define	u_rval2	u_r.r_reg.r_val2
#define	u_roff	u_r.r_off
#define	u_rtime	u_r.r_time

/* ioflag values: Read/Write, User/Kernel, Ins/Data */

#define	U_WUD	0
#define	U_RUD	1
#define	U_WKD	2
#define	U_RKD	3
#define	U_WUI	4
#define	U_RUI	5

#define	EXCLOSE	01

/* distribution: values for u_rflags */
#define FREMOTE	0x0002	/* file is remote  */
#define RFLOCK	0x0004	/* for remote record locking*/

#define	U_RCDIR		0x0001	/* remote current directory */
#define	U_RRDIR		0x0002	/* remote root directory    */
#define	U_RSYS		0x0004	/* system call has gone remote */
#define	U_LBIN		0x0100	/* dotdot at lbin mount */
#define	U_DOTDOT	0x0200
#define U_RCOPY		0x0400	/* used by copyout for non-delay copy */
