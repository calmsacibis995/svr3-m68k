/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/flock.h	10.5"

/* file segment locking set data type - information passed to system by user */
/* it is also found in fcntl.h */
#ifndef	F_RDLCK

/* file segment locking types */
#define	F_RDLCK	01	/* Read lock */
#define	F_WRLCK	02	/* Write lock */
#define	F_UNLCK	03	/* Remove lock(s) */

struct	flock	{
	short	l_type;
	short	l_whence;
	long	l_start;
	long	l_len;		/* len = 0 means until end of file */
	short	l_pid;
	short	l_sysid;
};
#endif

#define INOFLCK		1	/* Inode is locked when reclock() is called. */
#define SETFLCK		2	/* Set a file lock. */
#define SLPFLCK		4	/* Wait if blocked. */

#define USE_PID		1	/* use epid when cleaning locks		*/
#define IGN_PID		2	/* ignore epid when cleaning locks	*/

/* file locking structure (connected to inode) */

#define l_end 		l_len
#define MAXEND  	017777777777

struct	filock	{
	struct	flock set;	/* contains type, start, and end */
	union	{
		int wakeflg;	/* for locks sleeping on this one */
		struct {
			short pid;
			short sysid;
		} blk;			/* for sleeping locks only */
	}	stat;
	struct	filock *prev;
	struct	filock *next;
};

/* file and record locking configuration structure */
/* record use total may overflow */
struct flckinfo {
	long recs;	/* number of records configured on system */
	long reccnt;	/* number of records currently in use */
	long recovf;	/* number of times system ran out of record locks. */
	long rectot;	/* number of records used since system boot */
};

extern struct flckinfo	flckinfo;
extern struct filock	flox[];
