#ident	"$Header: /grp/mat/53/utils/com/RCS/common.h,v 1.1 87/01/27 16:09:18 mat Exp $"
#include	<sys/dk.h>

#ifdef DEBUG
#define		DEBPRT0(f)		if (Debug) fprintf(stderr, f)
#define		DEBPRT1(f, a)		if (Debug) fprintf(stderr, f, a)
#define		DEBPRT2(f, a, b)	if (Debug) fprintf(stderr, f, a, b)
#define		DEBPRT3(f, a, b, c)	if (Debug) fprintf(stderr, f, a, b, c)
#else
#define		DEBPRT0(f)
#define		DEBPRT1(f, a)
#define		DEBPRT2(f, a, b)
#define		DEBPRT3(f, a, b, c)
#endif /* DEBUG */

#define		EQ(a, b)	(strcmp(a,b)==0)
#define		EQ_N(a, b, n)	(strncmp(a,b,n)==0)
#define		CLEAR_WHITE(p)	while (*p==' '||*p=='\t') p++
#define		ISREG(s)	((s.st_mode&S_IFMT)==S_IFREG)
#define		ISDIR(s)	((s.st_mode&S_IFMT)==S_IFDIR)
#define		VERBOSE(str)	if( Verbose ) printf("%s\n", str )
#define		INT(a)		(*(int *)a)
#define		LONG(a)		(*(long *)a)

#define		TRUE		1
#define		FALSE		0
#define		T_BSIZE		512		/* TAPE block size */
#define		NULL		0
#define		I_BUFSIZ	1024		/* input read buffer size */
#define		DEVSTR		"DEVICE"

/*
 * Some defines use the filler spaces in the "volumeid"
 * structure for local data. Check in igf.h for these.
 */
struct	vidblk {
	struct volumeid vid;
	struct dkconfig dkc;
};
#define		VIDSIZ		sizeof(struct vidblk)

/*
 * Some defines to use the filler spaces in the "volumeid"
 * structure for local data.
 */
#define		vid_ctime	vid_f0
#define		vid_dirents	vid_f1
#define		vid_version	vid_f3
#define		vid_infoflg	vid_f4
#define		vid_dirtype	vid_f5
#define		vid_desc	vid_f6

/*
 * Defines for vid_infoflg.
 */
#define		I_BOOT		0x01
#define		I_SCRIPT	0x02

/*
 * This is the directory that exists on the package media. This
 * structure should remain an even multiple of T_BSIZE, the size
 * of tape blocks.
 *
 * Defines for struct t_dir
 */
/* for "type" field */
#define		D_OS		1
#define		D_RAM		2
#define		D_ARC		3
#define		D_SCR		4
/* for structure size */
#define		D_NAMSZ		15	/* 14=max name, 1=NULL */
#define		D_DSCSZ		26
#define		D_OPTSZ		116
#define		D_PATHSZ	96
struct t_dir {
	unsigned char	number;		/* file number, vid=1 */
	unsigned char	arnum;		/* archive file number 1= 1st archve */
	unsigned char	type;		/* type of file */
	char	name[D_NAMSZ];		/* archiver, os, ramdiks */
	char	a_desc[D_DSCSZ];	/* small archive description */
	char	dest_dir[D_PATHSZ];	/* destination directory or NULL */
	char	i_opt[D_OPTSZ];		/* input options or NULL */
};

/* format of tape directory, 0=struct dirent, 1=struct t_dir, 2=new t_dir */
#define		D_OLDF		0
#define		D_SORTOFNEW	1
#define		D_NEWF		2

/*
 * General purpose return values
 */
#define		ERROR		-1

/*
 * Modes for opening the tape device.
 */
#define		M_TRUNC		1
#define		M_APPEND	2

/*
 * Max string sizes for header information (VID).
 */
#define		VERSIZ		12
#define		DESCSIZ		88
#define		PACKSIZ		14
