
/*	@(#)space.h	6.13		*/
/*
 *	Copyright (c) 1985	AT&T 
 *	All Rights Reserved.
 */
#define	KERNEL 1

#include "sys/tty.h"
struct	cblock	cfree[NCLIST];	/* free list for character block */

#include "sys/buf.h"
struct	buf	buf[NBUF];
char 	buffers [(NBUF+1)*1024];
struct	buf	pbuf[NPBUF];		/* Physical io header pool */

struct	hbuf	hbuf[NHBUF];	/* buffer hash table */

#include "sys/file.h"
struct	file	file[NFILE];	/* file table */

#include	"sys/inode.h"
#include	"sys/fs/s5inode.h"
struct	inode	inode[NINODE];	/* inode table */
struct	s5inode	s5inode[NS5INODE];

#ifdef LAN1_0
#include "sys/space/olanspace.h"
#else
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#endif
struct	proc	proc[NPROC];	/* process table */

reg_t region[NREGION];

#include "sys/getpages.h"
pglst_t	spglst[MAXSWAPLIST];	/* page swap list */
gprgl_t	gprglst[MAXSWAPLIST];	/* region lock list */

#include "sys/map.h"
struct map sptmap[SPTMAP];

char putbuf[PUTBUFSZ];
int putbufsz = {PUTBUFSZ};

#include "sys/callo.h"
struct callo callout[NCALL];	/* Callout table */

#include "sys/mount.h"
struct mount mount[NMOUNT];	/* Mount table */

#include "sys/elog.h"
#include "sys/err.h"
struct	err	err = {		/* Error slots */
	NESLOT,
};

#include "sys/sysinfo.h"
struct sysinfo sysinfo;
struct syswait syswait;
struct syserr syserr;

struct	shlbinfo shlbinfo
	= {SHLBMAX, 0, 0};

#include "sys/utsname.h"

#ifndef	SYS
#define	SYS	"sysV68"
#endif

#ifndef	NODE
#define	NODE	"sysV68"
#endif

#ifndef	REL
#define	REL	"V/68.3"
#endif

#ifndef	VERSION
#define VERSION	"0"
#endif

#ifndef	MACHINE
#define	MACHINE	"M68020"
#endif

struct	utsname utsname = {
	SYS,
	NODE,
	REL,
	VERSION,
	MACHINE
};

#include "sys/var.h"
struct var v = {
	NBUF,
	NCALL,
	NINODE,
	(char *)(&inode[NINODE]),
	NFILE,
	(char *)(&file[NFILE]),
	NMOUNT,
	(char *)(&mount[NMOUNT]),
	NPROC,
	(char *)(&proc[NPROC]),
	NREGION,
	NCLIST,
	MAXUP,
	NHBUF,
	NHBUF-1,
	NPBUF,
	SPTMAP,
	VHNDFRAC,
	MAXPMEM,
	NAUTOUP,
	NOFILES,
	NQUEUE,
	NSTREAM,
	NBLK4096,
	NBLK2048,
	NBLK1024,
	NBLK512,
	NBLK256,
	NBLK128,
	NBLK64,
	NBLK16,
	NBLK4,
	NS5INODE,
	MAXSWAPLIST
};

int	slice_size = MAXSLICE;
int	cdlimit = CDLIMIT;

#include "sys/tuneable.h"
tune_t tune = {
	GPGSLO,
	GPGSHI,
	GPGSMSK,
	VHANDR,
	0,
	MAXUMEM,
	BDFLUSHR,
	MINARMEM,
	MINASMEM,
	MAXRDAHEAD
};

#include "sys/init.h"

/*
#ifndef	PRF_0
prfintr() {}
int	prfstat;
#endif
*/

/*
 * configurable, driver specific, drive independent space declaration
 */
#include "sys/iobuf.h"

#ifdef	VME320_0
#include "sys/space/m320space.h"
#endif

#ifdef	VME360_0
#include "sys/space/m360space.h"
#endif

#ifdef	VME331_0
#include "sys/space/m331space.h"
#endif

#ifdef	VME350_0
#include "sys/space/m350space.h"
#endif

#ifdef	VME323_0
#include "sys/space/m323space.h"
#endif

#if DBUF != 0
#include "sys/space/dbufspace.h"
#endif

#include	"sys/ipc.h"
#include	"sys/msg.h"

char	msglock[MSGMNI];
struct map	msgmap[MSGMAP];
struct msqid_ds	msgque[MSGMNI];
struct msg	msgh[MSGTQL];
struct msginfo	msginfo = {
	MSGMAP,
	MSGMAX,
	MSGMNB,
	MSGMNI,
	MSGSSZ,
	MSGTQL,
	MSGSEG
};

#include	"sys/sem.h"
struct semid_ds	sema[SEMMNI];
struct sem	sem[SEMMNS];
struct map	semmap[SEMMAP];
struct	sem_undo	*sem_undo[NPROC];
#define	SEMUSZ	(sizeof(struct sem_undo)+sizeof(struct undo)*SEMUME)
int	semu[((SEMUSZ*SEMMNU)+NBPW-1)/NBPW];
union {
	short		semvals[SEMMSL];
	struct semid_ds	ds;
	struct sembuf	semops[SEMOPM];
}	semtmp;

struct	seminfo seminfo = {
	SEMMAP,
	SEMMNI,
	SEMMNS,
	SEMMNU,
	SEMMSL,
	SEMOPM,
	SEMUME,
	SEMUSZ,
	SEMVMX,
	SEMAEM
};

#include	"sys/shm.h"
struct shmid_ds	shmem[SHMMNI];
struct	shminfo	shminfo	= {
	SHMMAX,
	SHMMIN,
	SHMMNI,
	SHMSEG,
	SHMALL
};

/* file and record locking */
#include "sys/flock.h"
struct flckinfo flckinfo = {
	FLCKREC
} ;

struct filock flox[FLCKREC];		/* lock structures */

/* parity error check routines table */
extern int m204parity(), cpparity();
int (*chkpartbl[])() = {
#ifdef M204
	m204parity,
#endif
#ifdef CPRAM
	cpparity,
#endif
	NULL
};

/* Streams */
#ifdef STREAMS
#include "sys/stream.h"
queue_t queue[NQUEUE];
struct stdata streams[NSTREAM];
mblk_t mblock[NBLK4096+NBLK2048+NBLK1024+NBLK512+NBLK256+NBLK128+NBLK64+NBLK16+NBLK4 + ((NBLK4096+NBLK2048+NBLK1024+NBLK512+NBLK256+NBLK128+NBLK64+NBLK16+NBLK4)/4)];
dblk_t dblock[NBLK4096+NBLK2048+NBLK1024+NBLK512+NBLK256+NBLK128+NBLK64+NBLK16+NBLK4];
struct linkblk linkblk[NMUXLINK];
int nmuxlink ={NMUXLINK};
int nstrpush ={NSTRPUSH};
struct strevent strevent[NSTREVENT];
int nstrevent ={NSTREVENT};
int maxsepgcnt ={MAXSEPGCNT};
int strmsgsz ={STRMSGSZ};
int strctlsz ={STRCTLSZ};
char strlofrac  ={STRLOFRAC};
char strmedfrac ={STRMEDFRAC};
int nmblock ={NBLK4096+NBLK2048+NBLK1024+NBLK512+NBLK256+NBLK128+NBLK64+NBLK16+NBLK4 + ((NBLK4096+NBLK2048+NBLK1024+NBLK512+NBLK256+NBLK128+NBLK64+NBLK16+NBLK4)/4)};
#endif

/* DU */
#ifdef DU
struct srmnt srmount[NSRMOUNT];
int nsrmount ={ NSRMOUNT };
int nservers ={ 0 };
int idleserver ={ 0 };
int msglistcnt ={ 0 };

#include "sys/adv.h"
#include "sys/sema.h"
#include "sys/comm.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
struct advertise advertise[NADVERTISE];
int nadvertise = NADVERTISE;
struct rcvd rcvd[NRCVD];
int nrcvd = NRCVD;
struct sndd sndd[NSNDD];
int nsndd = NSNDD;
struct gdp gdp[MAXGDP];
int maxgdp = MAXGDP;
int minserve = MINSERVE;
int maxserve = MAXSERVE;
int nrduser = NRDUSER;
struct rd_user rd_user[NRDUSER];
unsigned char rfheap[RFHEAP];
int rfsize = RFHEAP;
int rfs_vhigh = RFS_VHIGH;
int rfs_vlow = RFS_VLOW;
int sck_sanity = SCKSANITY;
#endif

int (*io_start[])()={
	0
	};

short ipid[NINODE];
int	ilogsize=0;
int	ilogs[1];

#include "sys/fsid.h"
struct fsinfo fsinfo[] = {
/*    flags	pipe   name   notify	*/
	0,	NULL,	"",	0,
	0,	NULL,	S51K,	0,
	0,	NULL,	DUFST,	0,
};

#ifdef STREAMS
#include "sys/log.h"
#include "sys/streams.h"
#endif

#include "sys/linesw.h"

#include "sys/space/ispspace.h"

#ifdef GREEK
int greek = GREEK;
#else
int greek = 0;
#endif /* GREEK */
