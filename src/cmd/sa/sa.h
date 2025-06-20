/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sa:sa.h	1.16"
/*	sa.h 1.16 of 5/23/85 */
/*	sa.h contains struct sa and defines variable used 
		in sadc.c and sar.c.
	The nlist setup table in sadc.c is arranged as follows:
	For VAX and PDP11 machines,
		- device status symbol names for controllers with 8 drives ,
			headed by _hpstat. 
			(not include the _gdstat)
		- device status symbol names for controllers with 4 drives,
			headed by _rlstat. 
		- device status symbol names for controllers with 1 drive,
			headed by _tsstat. 
		- general disk driver status system name.
		- symbol name for _sysinfo.
		- symbol name for _minfo. (not on pdp11)
		- symbol names for system tables: inode, file,
			process and record locking (not on PDP11).
		- symbol name for _var.
		- symbol name for _rl_cnt.
		- symbol name for _gd_cnt.
		- symbol name for system error table:

	For 3b20S system,
		- symbol name of dskinfo table
		- symbol name of MTC tape drive.
		- symbol name for _sysinfo.
		- symbol name for minfo. (also after sysinfo for u3b2 and u3b5)
		- symbol names for system tables: inode, file,
			process and record locking.
		- symbol name for _var.
		- symbol name for dsk_cnt.
		- symbol name for mtc_cnt.
		- symbol name for ap_state.
		- symbol name for system error table:

	For IBM 370 system,
		- symbol name for sysinfo.
		- symbol names for system tables: inode, file,
			text and process.
		- symbol name for var.
		- symbol name for system error table:
			Note that this is always the last entry in setup
			table, since the number of entries of setup table
			is used in sadc.c.
	For M68K systems,
		- device status symbol names for controllers with 12 drives ,
			headed by m320stat.
		- device status symbol names for controllers with 8 drives ,
			headed by udstat.
		- device status symbol names for controllers with 4 drives,
			headed by wdstat.
		- symbol name for sysinfo.
		- symbol names for system tables: inode, file,
				text and process.
		- symbol name for var.
		- device count variable symbol name for each device stat symbol
			ahead of sysinfo
		- symbol name for system error table:
			Note that this is always the last entry in setup
			table, since the number of entries of setup table
			is used in sadc.c.
*/
 
 
#ifdef u3b2
#undef	u3b5	/* A kludge to get around 3b2cc defining u3b5 */
#endif 
 
#include <a.out.h>
/*	The following variables define the positions of symbol
	table names in setup table:
*/
 
#if vax || pdp11
#define HPS	0
#define HMS	1
#define HSS	2
#define RFS	3
#define RKS	4
#define RPS	5
#define RLS	6
#define GTS	7
#define HTS	8
#define TMS	9
#define TSS	10
#define GDS	11
#define SINFO	12
#ifdef vax
#define	MINFO	13
#define INO	14
#define FLE	15
#define PRO	16
#define FLCK	17
#define V	18
#define RLCNT	19
#define GDCNT	20
#define SERR	21
#endif
#ifdef pdp11
#define INO	13
#define FLE	14
#define PRO	15
#define V	16
#define RLCNT	17
#define GDCNT	18
#define SERR	19
#endif
#endif
 
#ifdef u3b2
#define ID	0
#define IF	1
#define SINFO	2
#define	MINFO	3
#define INO	4
#define FLE	5
#define TXT	6
#define PRO	7
#define FLCK	8
#define V	9
#define IDEQ	10
#define SERR	11
#define DINFO	12
#define	MINSERVE	13
#define	MAXSERVE	14
#endif

#ifdef u3b
#define DSKINFO 0
#define MTC	1
#define SINFO	2
#define	MINFO	3
#define INO	4
#define FLE	5
#define PRO	6
#define FLCK	7
#define V	8
#define DSKCNT  9
#define MTCCNT  10
#define APSTATE 11
#define SERR	12
#define	DINFO	13
#define	MINSERVE	14
#define	MAXSERVE	15
#endif

#ifdef u3b5
#define DFDFC	0
#define SINFO	1
#define	MINFO	2
#define INO	3
#define FLE	4
#define TXT	5
#define PRO	6
#define FLCK	7
#define V	8
#define DFCNT	9
#define SERR	10
#define	DINFO	11
#define	MINSERVE	12
#define	MAXSERVE	13
#endif
 
#ifdef u370
#define	SINFO	0
#define	INO	1
#define	FLE	2
#define	TXT	3
#define	PRO	4
#define	V	5
#define	SERR	6
#endif

#ifdef m68k
#define	M320S	0
#define	UDS	1
#define	WDS	2
#define	SINFO	3
#define	MINFO	4
#define	INO	5
#define	FLE	6
#define	TXT	7
#define	PRO	8
#define	FLCK	9
#define	V	10
#define DEVCNT	11	/* always the same as the first xxCNT index */
#define	M320CNT	11
#define	UDCNT	12
#define	WDCNT	13
#define	SERR	14
#define	DINFO	15
#define	MINSERVE	16
#define	MAXSERVE	17
#endif

#if vax || pdp11
struct nlist setup[] = {
	{"_hpstat"},
	{"_hmstat"},
	{"_hsstat"},
	{"_rfstat"},
	{"_rkstat"},
	{"_rpstat"},
	{"_rlstat"},
	{"_gtstat"},
	{"_htstat"},
	{"_tmstat"},
	{"_tsstat"},
	{"_gdstat"},
	{"_sysinfo"},
#ifdef	vax
	{"_minfo"},
#endif
	{"_inode"},
	{"_file"},
	{"_proc"},
#ifdef vax
	{"_flckinfo"},
#endif
	{"_v"},
	{"_rl_cnt"},
	{"_gd_cnt"},
	{"_syserr"},
	{0},
};
#endif

#ifdef u3b
struct nlist setup[] = {
	{"dskinfo"},
	{"mtc_mtc"},
	{"sysinfo"},
	{"minfo"},
	{"inode"},
	{"file"},
	{"proc"},
	{"flckinfo"},
	{"v"},
	{"dsk_cnt"},
	{"mtc_cnt"},
	{"ap_state"},
	{"syserr"},
	{"dinfo"},
	{"minserve"},
	{"maxserve"},
	{0},
};
#endif

#ifdef u3b2
struct nlist setup[] = {
	{"idtime"},
	{"ifstat"},
	{"sysinfo"},
	{"minfo"},
	{"inode"},
	{"file"},
	{"text"},
	{"proc"},
	{"flckinfo"},
	{"v"},
	{"idstatus"},
	{"syserr"},
	{"dinfo"},
	{"minserve"},
	{"maxserve"},
	{0},
};
#endif

#ifdef u3b5
struct nlist setup[] = {
	{"df_dfc"},
	{"sysinfo"},
	{"minfo"},
	{"inode"},
	{"file"},
	{"text"},
	{"proc"},
	{"flckinfo"},
	{"v"},
	{"df_cnt"},
	{"syserr"},
	{"dinfo"},
	{"minserve"},
	{"maxserve"},
	{0},
};
#endif

#ifdef u370
struct nlist setup[] = {
	{"sysinfo"},
	{"inode"},
	{"file"},
	{"text"},
	{"proc"},
	{"v"},
	{"syserr"},
	{0},
};
#endif

#ifdef m68k
struct nlist setup[] = {
	{"m320stat"},
	{"m360stat"},
	{"wdstat"},
	{"sysinfo"},
	{"minfo"},
	{"inode"},
	{"file"},
	{"text"},
	{"proc"},
	{"flckinfo"},
	{"v"},
	{"m320_cnt"},
	{"m360_cnt"},
	{"wd_cnt"},
	{"syserr"},
	{"dinfo"},
	{"minserve"},
	{"maxserve"},
	{(char*)0}
};
#endif

#if vax || pdp11
#define NCTRA	6  /* number of 8-drive disk controllers  */
#define NCTRB	4  /* number of 4-drive disk and tape controllers  */
#define NCTRC	1  /* number of general disk controllers  */
		   /* and number of ts11 tape controller	*/
#define NDRA	8  /* number of data units for a 8-drive disk controller  */
#define NDRB	4  /* number of data units for a 4-drive disk controller  */
#define NDRC	32 /* number of data units for a general disk controller  */
#define NDRD	1  /* number of data units for a ts11 tape controller  */
/*	this is for gd device	*/
/*	NDEVS defines number of total data units 
*/
#define NDEVS NCTRA *NDRA +NCTRB * NDRB + NCTRC * NDRC + NCTRC * NDRD 
/*	iotbsz, devnm tables define the number of drives,
	controller name  of devices
	hpstat, hmstat, hsstat, rfstat, rkstat, rpstat, rlstat,
	gtstst, htstat, tmstat, tsstat.
	Note that the ordering of them is consistent with the ordering 
	of device status symbol names in setup table.
*/
 
int iotbsz[SINFO] = {
	NDRA,NDRA,NDRA,NDRA,NDRA,NDRA,NDRB,NDRB,NDRB,NDRB,NDRD,NDRC
};
 
char devnm[SINFO][6] ={
	"rp06-",
	"rm05-",
	"rs04-",
	"rf11-",
	"rk05-",
	"rp03-",
	"rl02-",
	"tape-",
	"tm03-",
	"tm11-",
	"ts11-",
	"dsk-"
};
#endif

#ifdef m68k
/* 
   NCTRx values are used differently here than they are in the vax case.  
   Here they represent a maximum number of controllers expected on a 
   single machine of the given type.  The value is used to define
   storage in sadc.c.  The actual number of devices for a given
   driver is derived from the xx_cnt variable associated with the driver.
*/
#define NCTRA	2  /* number of 12-drive disk controllers  */
#define NDRA	12 /* number of data units for a 12-drive disk controller  */
#define NCTRB	4  /* number of 8-drive disk controllers  */
#define NDRB	8  /* number of data units for a 8-drive disk controller  */
#define NCTRC	2  /* number of 4-drive disk controllers  */
#define NDRC	4  /* number of data units for a 4-drive disk controller  */
#define	NDEVS	NCTRA*NDRA + NCTRB*NDRB + NCTRC*NDRC
/*	
	iotbsz defines the number of drives associated with one of each
	controller type identified in the devnm table.
	This is the default number of devices expected and recorded.
	The xx_cnt value associated with a driver can override the default.
	Note that the ordering is consistent with the ordering
	of device status symbol names in setup table.
*/
int iotbsz[SINFO] = {
	NDRA,
	NDRB, 
	NDRC,
};
#define IOSZ(X)		(sizeof(struct iotime)*(X) )

/* Each count variable xx_cnt is the number of logical devices configured
   into the driver. Currently all drivers have eight logical devices (slices)
   per physical device */
#define CNTtoPHYS(X)	( (X) >> 3 )

char devnm[SINFO][6] = {
	"m320-",
	"m360-",
	"wd-",
};
#endif

#ifdef u3b
#define NDEVS 100
/*	iotbsz, devnm tables define the initial value of number of drives
	and name of devices.
*/
int iotbsz[SINFO] = {
	0,0
};
char devnm[SINFO][6] ={
	"dsk-",
	"tape-"
};
#endif

#ifdef u3b2
#define NDEVS 3
/*	iotbsz, devnm tables define the initial value of number of drives
	and name of devices.
*/
int iotbsz[SINFO] = {
	0,
	1		/* a 3B 2 must have and has at most one floppy drive */
};
char devnm[SINFO][6] ={
	"hdsk-",		/* integral hard disk */
	"fdsk-"		/* integral floppy disk */
};
#endif

#ifdef u3b5
#define NDEVS 100
/*	iotbsz, devnm tables define the initial value of number of drives
	and name of devices.
*/
int iotbsz[SINFO] = {
	0
};
char devnm[SINFO][6] ={
	"dsk-"
};
#endif

#ifdef u370
#define NDRUM 1
#define NDISK 22
#define NDEV NDRUM + NDISK

/*	The structure procinfo contains data about process table */ 
struct	procinfo { 
	int	sz;	/* number of processes */ 
	int	run;	/* number that are running	*/ 
	int	wtsem;	/* number that are not process 	*/  
			/* group leaders that are 	*/ 
			/* waiting on semaphores	*/ 
	time_t  wtsemtm;/*acc wait time	*/
	int	wtio;	/* number that are not process 	*/ 
			/* group leaders that are 	*/ 
			/* waiting on i/o		*/ 
	time_t wtiotm;	/*acc wait time		*/
	}; 
/* The following structure has information */ 
/*	about drums and disks.			*/ 
struct	iodev { 
	long	io_sread; 
	long	io_pread; 
	long	io_swrite; 
	long	io_pwrite; 
	long	io_total;
}; 
#endif 
 
/*	structure sa defines the data structure of system activity data file
*/
 
struct sa {
	struct	sysinfo si;	/* defined in /usr/include/sys/sysinfo.h  */
#if	vax || u3b || u3b5 || u3b2 || m68k
	struct	minfo	mi;	/* defined in /usr/include/sys/sysinfo.h */
#endif
	struct	dinfo	di;	/* defined in /usr/include/sys/sysinfo.h */
	int	minserve;
	int	maxserve;
	int	szinode;	/* current size of inode table  */
	int	szfile;		/* current size of file table  */
	int	szproc;		/* current size of proc table  */
	int	szlckf;		/* current size of file record header table */
	int	szlckr;		/* current size of file record lock table */
	int	mszinode;	/* maximum size of inode table  */
	int	mszfile;	/* maximum size of file table  */
	int	mszproc;	/* maximum size of proc table  */
	int	mszlckf;	/* maximum size of file record header table */
	int	mszlckr;	/* maximum size of file record lock table */
	long	inodeovf;	/* cumulative overflows of inode table
					since boot  */
	long	fileovf;	/* cumulative overflows of file table
					since boot  */
	long	procovf;	/* cumulative overflows of proc table
					since boot  */
	time_t	ts;		/* time stamp  */

	int apstate;

#ifdef u370
	time_t	elpstm;		/* elapsed time - normally  */
 			/* gotten from sysinfo structure */	/*370*/
	time_t	curtm;		/* time since 1/1/1970 - for Q option */
	double	tmelps;		/* elapsed time in micro secs  */
				/* obtained from tss table	*/
	short	nap;		/* the number of processors	*/
	int	lines;		/* the number of lines - for q option */
/* The following represent times obtained from the tss 	*/ 
/* system statistics table. They are in micro secs.	*/ 
	double	vmtm;		/* cumulative vm time since ipl	*/ 
	double	usrtm;		/* cumulative user (vm) time since ipl	*/
	double	usuptm;		/* cumulative unix superviser (vm) time 
				since ipl */
	double	idletm;		/* cumulative idle time since ipl	*/ 
	double	ccv;		/* current clock value	*/ 
/* The following are from the tss table and give info	*/ 
/* about scheduling and dispatching			*/ 
	int	intsched;	/*no. of times the internal scheduler */ 
				/* was entered since ipl	*/ 
	int	tsend;		/* no. of time slice ends since ipl	*/ 
	int	mkdisp;		/* no. of tasks which entered 	*/ 
				/* dispatchable list since ipl	*/ 
	struct procinfo pi;	/* process table info	*/ 
	struct iodev io[NDEV];	/*drum - disk info	*/ 
#endif

#ifndef u370
	long	devio[NDEVS][4]; /* device unit information  */
#endif 

#define	IO_OPS	0  /* number of I /O requests since boot  */
#define	IO_BCNT	1  /* number of blocks transferred since boot */
#define	IO_ACT	2  /* cumulative time in ticks when drive is active  */
#define	IO_RESP	3  /* cumulative I/O response time in ticks since boot  */
};
extern struct sa sa;
