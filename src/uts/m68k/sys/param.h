/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/param.h	10.10"
#include "sys/fs/s5param.h"
/*
 * fundamental variables
 * don't change too often
 */

#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	1000		/* max links */

#define	SSIZE	2		/* initial stack size (*1024 bytes) */
#define	SINCR	4		/* increment of stack (*1024 bytes) */
#define	USIZE	4		/* size of user block (*1024 bytes) */

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	HZ	60		/* 100 ticks/second of the clock */

#define	NCARGS	(8*1024)	/* # characters in exec arglist */
				/*   must be multiple of NBPW.  */

/*	The following define is here for temporary compatibility
**	and should be removed in the next release.  It gives a
**	value for the maximum number of open files per process.
**	However, this value is no longer a constant.  It is a
**	configurable parameter, NOFILES, specified in the kernel
**	master file and available in v.v_nofiles.  Programs which
**	include this header file and use the following value may
**	not operate correctly if the system has been configured
**	to a different value.
*/

#define	NOFILE	100		/* was 20 -mat */

/*	The following represent the minimum and maximum values to
**	which the paramater NOFILES in the kernel master file may
**	be set.
*/

#define	NOFILES_MIN	 20
#define	NOFILES_MAX	100

#define	USRSTACK (32*1024*1024)	/* User's stack, see immu.h */

/*	The following represent the minimum and maximum values to
**	which the paramater NOFILES in the kernel master file may
**	be set.
*/

#define	NOFILES_MIN	 20
#define	NOFILES_MAX	100

/*	The following define builds the file system
**	to use the file system switch and enables profiling
*/

#define	FSPTR		1
#define PRFMAX		2048	/* maximum number of text addresses */
#define PRF_ON  	1	/* profiler collecting samples */
#define PRF_VAL		2	/* profiler contains valid text symbols */
#define SPILLSIZE	64	/* no of bytes copied from stack overflow */

extern unsigned  prfstat;	/* state of profiler */

/*
 * priorities
 * should not be altered too much
 */

#define	PMASK	0177
#define	PCATCH	0400
#define	PSWP	0
#define	PINOD	10
#define	PRIBIO	20
#define	PZERO	25
#define PMEM	0
#define	NZERO	20
#define	PPIPE	26
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127

/*
 * fundamental constants of the implementation--
 * cannot be changed easily
 */

#define NBPS	0x40000		/* Number of bytes per segment */
#define	NBPW	sizeof(int)	/* number of bytes in an integer */
#define LOGNBPW	2		/* log2(NBPW) */
#define	NCPS	256		/* Number of clicks per segment */
#define	NBPC	1024		/* Number of bytes per click */
#define	BPCSHIFT	10	/* LOG2(NBPC) if exact */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	NODEV	(dev_t)(-1)
#define	NBPSCTR		512	/* Bytes per disk sector.	*/
#define SCTRSHFT	9	/* Shift for BPSECT.		*/

#define	UMODE	PS_CUR		/* current Xlevel == user */
#define	USERMODE(psw)	((psw & UMODE) == 0)
#define	BASEPRI(psw)	((psw & PS_IPL) != 0)

#define	lobyte(X)	(((unsigned char *)&X)[1])
#define	hibyte(X)	(((unsigned char *)&X)[0])
#define	loword(X)	(((ushort *)&X)[1])
#define	hiword(X)	(((ushort *)&X)[0])

#define	MAXSUSE	255

/* REMOTE -- whether machine is primary, secondary, or regular */
#define SYSNAME 9		/* # chars in system name */
#define PREMOTE 39

#define	NUBLK		31	/* No. of Ublock windows [1..31] */
#define	UB_WANTED	(1<<NUBLK)
