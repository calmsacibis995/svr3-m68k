/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/sysinfo.h	10.4"

/*
 *	System Information.
 */

struct sysinfo {
	time_t	cpu[5];
#define	CPU_IDLE	0
#define	CPU_USER	1
#define	CPU_KERNEL	2
#define	CPU_WAIT	3
#define CPU_SXBRK	4
	time_t	wait[3];
#define	W_IO	0
#define	W_SWAP	1
#define	W_PIO	2
	long	bread;
	long	bwrite;
	long	lread;
	long	lwrite;
	long	phread;
	long	phwrite;
	long	swapin;
	long	swapout;
	long	bswapin;
	long	bswapout;
	long	pswitch;
	long	syscall;
	long	trap;
	long	resched;
	long 	remcall;	/* REMOTE: remote call */
	long    fileop;		/* REMOTE: remote fileop */
	long    serve;		/* REMOTE: remote server */
	long	sysread;
	long	syswrite;
	long	sysfork;
	long	sysexec;
	long	runque;
	long	runocc;
	long	swpque;
	long	swpocc;
	long	iget;
	long	namei;
	long	dirblk;
	long	readch;
	long	writech;
	long	rcvint;
	long	xmtint;
	long	mdmint;
	long	rawch;
	long	canch;
	long	outch;
	long	msg;
	long	sema;
	long	pnpfault;
	long	wrtfault;
};

extern struct sysinfo sysinfo;

struct syswait {
	short	iowait;
	short	swap;
	short	physio;
};

struct dinfo {
			/* ELEMENT FOR sar -Du */
	time_t	serve;		/* ticks in serve() since boot */
			/* ELEMENTS FOR sar -Dc */
	long	isyscall;	/* # of incoming syscalls since boot */
	long	isysread;	/* # of incoming read's since boot */
	long	ireadch;	/* char's read by incoming read's */
	long	isyswrite;	/* # of incoming write's since boot */
	long	iwritech;	/* char's written by incoming writes */
	long	isysexec;	/* # of incoming exec's since boot */
	long	osyscall;	/* # of outgoing syscalls since boot */
	long	osysread;	/* # of outgoing read's since boot */
	long	oreadch;	/* char's read by outgoing read's */
	long	osyswrite;	/* # of outgoing write's since boot */
	long	owritech;	/* char's written by outgoing writes */
	long	osysexec;	/* # of outgoing exec's since boot */
			/* ELEMENTS FOR sar -S*/
	long	nservers;	/* sum of all servers since boot */
	long	srv_que;	/* sum of server Q lgth since boot */
	long	srv_occ;	/* ticks server Q found occupied */
	long	rcv_que;	/* sum of receive Q lgth since boot */
	long	rcv_occ;	/* clock ticks receive Q occupied */
};
extern struct dinfo dinfo;
extern	int minserve;		/* DU tunable: sever low water mark */
extern	int maxserve;		/* DU tunable: sever high water mark */
extern	int nservers;		/* total servers in system */
extern	int idleserver;		/* idle servers in system */
extern	int msglistcnt;		/* receive descriptors in msg queue */


struct minfo {
				/* following counts from file clock.c	*/
	unsigned long 	freemem[2]; 	/* freemem in pages		*/
					/* "double" long format		*/
					/* freemem[0] least significant */
	long	freeswap;	/* free swap space			*/

				/* following found in file trap.c	*/
	long	buserr;		/* count of all processor bus errors	*/
				/*  -all faults kernel, user, and parity */

				/* following counts from file fault.c	*/
	long	fault;		/*   count of all user faults		*/
	long    pfault;		/*    protection fault			*/
	long    cw;		/*     copy on write			*/
	long    steal;		/*     steal the page			*/
	long    vfault;  	/*    translation fault			*/
	long    demand;		/*     demand zero and demand fill pages */
	long    cache;		/*     pages in cache			*/
	long    swap;		/*     pages on swap			*/
	long    file;		/*     pages on file			*/

				/* following counts from file getpages.c      */
	long	getpages;	/* number of times getpages was called        */
	long	recent;		/* getpages finds a recently accessed page    */
	long	none;		/* getpages finds DBD_NONE page		      */
	long    cwsw;		/* getpages finds DBD_NONE page hashed to swap*/
	long    unmodsw;	/* getpages finds unmodified pages on swap    */
	long    modsw;		/* getpages finds modified pages on swap      */
	long	unmodfl;	/* getpages finds unmodified pages in file    */

	long	swappage;	/* number of times addspg was called	*/
	long	freepage;	/* number of times addfpg was called	*/
	long	swapchunk;	/* number of times swapchunk was called */
	long	chunksize;	/* total number of pages written by swapchunk */
	long    freedpgs;	/* total pages freed by getpages	*/

	long	vhandwake;	/* number of times vhand woke up	*/
	long	vhandage;	/* number of regions vhand aged		*/
	long	vhandout;	/* number of regions vhand getpage'd	*/

				/* following counts found in file sched.c */
	long	schedwake;	/* number of times sched woke up	*/
	long	schedrun;	/* number of times sched ran		*/
	long	schedout;	/* number of times sched swapped out a proc */
};

extern struct minfo minfo;
extern struct syswait syswait;

struct syserr {
	long	inodeovf;
	long	fileovf;
	long	textovf;
	long	procovf;
};

extern struct syserr syserr;

struct shlbinfo {
	long	shlbs;		/* Max # of libs a process can link in	*/
				/*   at one time.			*/
	long	shlblnks;	/* # of times processes that have used	*/
				/*   shared libraries.			*/
	long	shlbovf;	/* # of processes needed more shlibs	*/
				/*   than the system imposed limit.	*/
};

extern struct shlbinfo shlbinfo;
