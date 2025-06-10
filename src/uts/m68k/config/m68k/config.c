/*	@(#)config.c	6.9		*/
static char sccsid[] = "@(#)config.c	6.9	";

#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "sys/types.h"		/* for "sys/param.h" */
#include "sys/param.h"		/* for "sys/sysmacros.h" */
#undef BSIZE			/* conflict */
#undef SSIZE			/* conflict */
#include "sys/sysmacros.h"	/* for ctob() & btoct() */

#define	MAP020	0xff000000	/* 68020 map adjustment/mask */
#define	LOWMAP	0x00800000	/* 68000/68010 lowest memory mapped address */
#define	MAXADD	0x00ffffff	/* 68000/68010 max memory size */

#define	TSIZE	50		/* max configuration table */
#define	DSIZE	150		/* max device table */
#define	ASIZE	20		/* max alias table */
#define	MSIZE	100		/* max address map table */
#define	PSIZE	128		/* max keyword table */
#define	RSIZE	20		/* max ram table */
#define	SYSSIZE	40		/* max system device table */
#define	PRBSIZE	50		/* max probe table */
#define	ALSIZE	50		/* max alien table */
#define	DUPSIZE	50		/* max dup table */
#define	SDSIZE	2000		/* max sysdef buffer size */
#define	FSIZE	20		/* max force table */
#define	FLEN	8		/* max force identifier length */
#define	BSIZE	50		/* max block device table */
#define	CSIZE	50		/* max character device table */
#define	ISIZE	50		/* max other interrupt driven device table */
#define	FSSIZE	50		/* max file system switch size */
#define	SSIZE	50		/* max input string size */
#define	VSIZE	4		/* size of one vector */
#define	STR1	"%30s"		/* one SSIZE input string format */
#define	STR2	"%30s%30s"	/* two SSIZE input string formats */
#define	STR3	"%30s%30s%30s"	/* three SSIZE input string formats */
#define MAXLINE	130		/* max line length allowed in master & dfile*/

/* Field 4 (type & type2) flags */
#define	FSSW	010000		/* File system switch definition */
#define	ADDVEC	04000		/* add _vec definition */
#define	ADDMAJ	02000		/* add _cmaj and _bmaj definition */
#define	ADDLEV	01000		/* add _lev definition */
#define	ONCE	0200		/* allow only one specification of device */
#define	NOCNT	0100		/* suppress device count field */
#define	SUPP	040		/* suppress interrupt vector (on print) */
#define	REQ	020		/* required device */
#define	BLOCK	010		/* block type device */
#define	CHAR	04		/* character type device */
#define	INTR	02		/* not block or char, but interrupt driven */
#define	MADDR	01		/* device has multiple adrs but only 1 vector */

/* Field 3 (mask) flags */
#define	STREAM	01000		/* stream handler exists */
#define	CBSEP	0400		/* character and block open/close separate */
#define	TTYS	0200		/* tty header exists */
#define	INIT	0100		/* initialization routine exists */
#define	PWR	040		/* power fail routine exists */
#define	OPEN	020		/* open routine exists */
#define	CLOSE	010		/* close routine exists */
#define	READ	04		/* read routine exists */
#define	WRITE	02		/* write routine exists */
#define	IOCTL	01		/* ioctl routine exists */

/* Alternate field 3 masks for file system switches. */
#define	IPUT	01	
#define	IREAD	02
#define	IUPDAT	010
#define	READI	020
#define	WRITEI	040
#define	ITRUNC	0200
#define	STATF	0400
#define	NAMEI	01000
#define	MOUNT	02000
#define	UMOUNT	04000
#define	GETINODE 010000
#define	OPENI	020000
#define	CLOSEI	040000
#define	UPDATE	0100000
#define	STATFS	0200000
#define	ACCESS	0400000
#define	GETDENTS 01000000
#define	ALLOCMAP 02000000
#define	FREEMAP	04000000
#define	READMAP	010000000
#define	SETATTR	020000000
#define	NOTIFY	040000000
#define	FCNTL	0100000000
#define	FSINFO	0200000000
#define	FIOCTL	0400000000

/* cache flags  */
#define LOGICAL		01	/* cache is logical addresses*/
#define PHYSICAL	02	/* cache is physical addresses */
#define SUPERVISOR	04	/* cache is supervisor */
#define USER		010	/* cache is user */
#define DATA		020	/* cache contains data */
#define INSTR		040	/* cache contains instructions */

/* ram input flag values */
#define	R_IN_NoInit	01	/* input flag: no initialization required */
#define	R_IN_Single	02	/* input flag: single ram area */
#define	R_IN_Mult	04	/* input flag: multiple rams within area */
#define	R_IN_VALID	07	/* valid bits of r_flag */

/* ram output flag values */
#define	R_OUT_Init	01	/* output flag: initialization required */
#define	R_OUT_Hunt	02	/* output flag: hunt for rams within area */

/* dup flag values */
#define	D_INTRET	01	/* "beq intret" on zero return */
#define	D_ALLTRAPS	02	/* "beq alltraps" on zero return */
#define	D_RETURN	04	/* "beq return" on zero return */
#define	D_BEQ		07	/* one of the above beq's is true */
#define	D_ARG		010	/* dup function needs an argument */
#define	D_VALID		017	/* valid bits of d_flag */

#define	PRTLOW(X)	fprintf(fdlow,"%s\n",X)

struct	t	{
	char	*devname;	/* pointer to device name */
	short	vector;		/* interrupt vector location */
	long	address;	/* device address */
	short	buslevel;	/* bus request level */
	short	addrsize;	/* number of bytes at device address */
	short	vectsize;	/* number of bytes for interrupt vector */
	short	type;		/* ONCE,NOCNT,SUPP,REQ,BLOCK,CHAR,INTR,etc */
	char	*handler;	/* pointer to interrupt handler */
	short	count;		/* sequence number for this device */
	short	blk;		/* major device number if block type device */
	short	chr;		/* major device number if char. type device */
	short	mlt;		/* number of devices on controller */
}	table	[TSIZE];

struct	t2	{
	char	dev[9];		/* device name */
	short	vsize;		/* interrupt vector size */
	short	asize;		/* device address size */
	short	type2;		/* ONCE,NOCNT,SUPP,REQ,BLOCK,CHAR,INTR,etc */
	short	block;		/* major device number if block type device */
	short	charr;		/* major device number if char. type device */
	short	mintr;		/* major device number if other intr device */
	short	mult;		/* max number of devices per controller */
	short	busmax;		/* max allowable bus request level */
	int	mask;		/* device mask indicating existing handlers */
	char	hndlr[5];	/* handler name */
	short	dcount;		/* number of controllers present */
	char	entries[3][9];	/* conf. structure definitions */
	short	acount;		/* number of devices */
}	devinfo	[DSIZE];

struct	t3	{
	char	new[9];		/* alias of device */
	char	old[9];		/* reference name of device */
}	alias	[ASIZE];

struct	map	{
	unsigned long start;	/* starting mapped address */
	unsigned long length;	/* gap length */
}	map[MSIZE] =
{
	LOWMAP, MAXADD - LOWMAP
};

struct	t4	{
	char	indef[SSIZE+1];		/* input parameter keyword */
	char	oudef[SSIZE+1];		/* output definition symbol */
	char	value[SSIZE+1];		/* actual parameter value */
	char	defval[SSIZE+1];	/* default parameter value */
}	parms	[PSIZE];

struct	t5	{
	unsigned short	r_flag;		/* ram flag word */
	unsigned long	r_low;		/* ram low address */
	unsigned long	r_high;		/* ram high address ( + 1 ) */
	unsigned long	r_incr;		/* ram increment */
}	rams	[RSIZE];
struct	t5	*ramptr = rams;		/* ram table pointer */

struct	t6	{
	unsigned long	p_addr;		/* probe address */
	unsigned short	p_byte;		/* probe byte */
}	probes	[PRBSIZE];
struct	t6	*prbptr = probes;	/* probe table pointer */

struct	t7	{
	unsigned short	a_vect;		/* alien vector address */
	unsigned long	a_addr;		/* alien address */
	char	a_hndlr[9];		/* alien handler name. */
}	aliens	[ALSIZE];
struct	t7	*alptr = aliens;	/* alien table pointer */

struct	t8	{
	unsigned short	d_flag;		/* dup flag */
	unsigned short	d_vect;		/* dup vector address */
	char		d_hndlr[9];	/* dup handler */
	unsigned long	d_arg;		/* dup argument */
}	dups	[DUPSIZE];
struct	t8	*dupptr = dups;		/* dup table pointer */

struct	t9	{
	char	s_swap;			/* true for swap system device */
	char	s_count;		/* number of system device minors */
	struct t *s_device;		/* system device's device pointer */
	short	s_minor[SYSSIZE];	/* various minors: one per boot */
	short	s_major[SYSSIZE];	/* various majors: one per boot */
};

struct	t9	root;			/* root system device table */
struct	t9	pipe;			/* pipe system device table */
struct	t9	dump;			/* dump system device table */
struct	t9	swap = {1};		/* swap system device table */
long	swp_lo[SYSSIZE];		/* various swplo's: one per swap minor*/
short	swp_nbr[SYSSIZE];		/* various nswap's: one per swap minor*/
struct	t	*lowptr[256];		/* low core double-word pointers */
struct	t2	*bdevices[BSIZE];	/* pointers to block devices */
struct	t2	*cdevices[CSIZE];	/* pointers to char. devices */
struct	t2	*idevices[ISIZE];	/* pointers to other intr devices */
struct	t2	*fswitch[FSSIZE];	/* pointers to file system switches */
struct	t	*p;			/* configuration table pointer */
struct	t2	*q;			/* master device table pointer */
struct	t3	*r;			/* alias table pointer */
struct	t4	*kwdptr;		/* keyword table pointer */
struct	t	*locate();
struct	t2	*find();
struct	t4	*lookup();
char	*uppermap();
void	exit();
short	dosys();

short	power	= 0;	/* power fail restart flag */
short	eflag	= 0;	/* error in configuration */
short	tflag	= 0;	/* table flag */
short	bmax	= -1;	/* dynamic max. major device number for block device */
short	cmax	= -1;	/* dynamic max. major device number for char. device */
short	fmax	= -1;	/* dynamic max. major device number for char. device */
short	blockex	= 0;	/* dynamic end of block device table */
short	charex	= 0;	/* dynamic end of character device table */
short	intrex	= 0;	/* dynamic end of other interrupt device table */
short	subv	= 0;	/* low core double-word subscript */
short	vsize	= 0;	/* vector allocation */
short	subtmp	= 0;	/* temporary; for subscripting */
short	abound	= -1;	/* current alias table size */
short	dbound	= -1;	/* current device table size */
short	tbound	= -1;	/* current configuration table size */
short	pbound	= -1;	/* current keyword table size */
short	rbound	= -1;	/* current ram table size */
short	prbbound = -1;	/* current probe table size */
short	prbsave = -1;	/* saved probe table size for sysdef(1M) */
short	albound = -1;	/* current alien table size */
short	dupbound = -1;	/* current dup table size */
short	mbound	= 0;	/* current address map table size */
short	rtmaj	= -1;	/* major device number for root device */
short	swpmaj	= -1;	/* major device number for swap device */
short	pipmaj	= -1;	/* major device number for pipe device */
short	dmpmaj	= -1;	/* major device number for dump device */
short	rtmin	= -1;	/* minor device number for root device */
short	swpmin	= -1;	/* minor device number for swap device */
short	pipmin	= -1;	/* minor device number for pipe device */
short	dmpmin	= -1;	/* minor device number for dump device */
long	swplo	= -1;	/* low disc address for swap area */
short	nswap	= -1;	/* number of disc blocks in swap area */
FILE	*confhp;	/* configuration include file pointer */
FILE	*fdconf;	/* configuration file descriptor */
FILE	*fdvec;		/* vector file descriptor */
FILE	*fdlow;		/* low core file descriptor */
struct t *dmppt;	/* pointer to dump entry */
char	*mfile;		/* master device file */
char	*cfile;		/* output configuration table file */
char	*hfile;		/* output configuration define file */
char	*lfile;		/* output low core file */
char	*vfile;		/* output vector file */
char	*infile;	/* input configuration file */
char	line[MAXLINE+1];/* input line buffer area */
char	pwrbf[513];	/* power fail routine buffer */
char	initbf[513];	/* initialization routine buffer */
char	xbf[512];	/* buffer for external symbol definitions */
char	d[9];		/* buffer area */
char	capitals[9];	/* buffer area */
short	vec;		/* specified interrupt vector */
unsigned long	add;	/* specified device address */
short	bl;		/* specified bus level */
short	ml;		/* specified device multiplicity */
long	mpu = 68000;	/* mpu = 68000 || 68010 || 68020 */
long	cache = 0;	/* cache flag;  0 = no cache present on board */
char	force[(FLEN+1)*FSIZE];	/* table for forcing specific drivers */
char	*frcptr = force;	/* force table pointer */
char	sysdef1[SDSIZE];	/* saved dfile part 1 for sysdef(1M) */
char	sysdef2[SDSIZE];	/* saved dfile part 2 for sysdef(1M) */
char	sysdef3[SDSIZE];	/* saved dfile part 3 for sysdef(1M) */
char	indfile;		/* true when dfile read is done */

char	*opterror =	/* option error message */
{
	"Option re-specification\n"
};

char	*low00[] = {				/* M68000 low.s body */
	"nullvect:	global	nullvect",
	"	sub.l	&6,%sp",
	"	movm.l	&0xfffe,-(%sp)",
	"	mov.l	&24,%d0",
	"	bra.b	alltraps",
	"",
	"intret:	global	intret",
	"	clr.b	idleflag",
	"	btst	&5,66(%sp)",
	"	bne.b	return",
	"	tst.b	runrun",
	"	beq.b	return",
	"	mov.w	&0x2000,%sr",
	"	mov.l	&0,%d0",
	"",
	"alltraps:	global	alltraps",
	"	clr.w	64(%sp)",
	"	mov.l	%d0,-(%sp)",
	"	mov.l	%usp,%a0",
	"	mov.l	%a0,-(%sp)",
	"	jsr	trap",
	"	mov.l	(%sp)+,%a0",
	"	mov.l	%a0,%usp",
	"	add.l	&4,%sp",
	"",
	"return:	global	return",
	"	movm.l	(%sp)+,&0x7fff",
	"	add.l	&6,%sp",
	"	rte",
	(char *) 0
};

char	*low10[] = {				/* M68010 low.s body */
	"intret:	global	intret",
	"	clr.b	idleflag",
	"	btst	&5,62(%sp)",
	"	bne 	return",
	"	tst.b	runrun",
	"	beq 	return",
	"	mov.w	&0x2000,%sr",
	"	clr.w	68(%sp)",
	"	bra.b	alltraps",
	"",
	"nullvect:	global	nullvect",
	"	clr.w	-(%sp)",
	"	movm.l	&0xfffe,-(%sp)",
	"",
	"alltraps:	global	alltraps",
	"	mov.l	%usp,%a0",
	"	mov.l	%a0,-(%sp)",
	"	mov.l	saveaddr,%a0",
	"	mov.w	74-2(%sp),%d0",
	"	and.w	&0xf000,%d0",
	"	mov.w	%d0,(%a0)+",
	"	beq.b	L%skip1	",
	"	lea.l	74(%sp),%a1	",
	"	mov.l	%a1,%a2",
	"	mov.w	&50/2-1,%d0",
	"L%dlp1:	mov.w	(%a1)+,(%a0)+",
	"	dbra.w	%d0,L%dlp1",
	"	mov.w	&74/2-1,%d0",
	"L%dlp2:	mov.w	-(%a2),-(%a1)",
	"	dbra.w	%d0,L%dlp2",
	"	add.w	&50,%sp",
	"	and.w	&0xfff,74-2(%sp)",
	"L%skip1:",
	"",
	"	jsr	trap",
	(char *) 0
};
char	*low10a[] = {			/* second part M68010 low.s body */
	"",
	"	mov.l	saveaddr,%a0",
	"	mov.w	(%a0)+,%d1",
	"	beq.b	L%skip2",
	"	mov.l	%sp,%a2	",
	"	sub.w	&50,%sp",
	"	mov.l	%sp,%a1	",
	"	mov.w	&74/2-1,%d0",
	"L%dlp3:	mov.w	(%a2)+,(%a1)+",
	"	dbra.w	%d0,L%dlp3",
	"	lea.l	74(%sp),%a1",
	"	mov.w	&50/2-1,%d0",
	"L%dlp4:	mov.w	(%a0)+,(%a1)+",
	"	dbra.w	%d0,L%dlp4",
	"	or.w	%d1,74-2(%sp)",
	"L%skip2:",
	"",
	"	mov.l	(%sp)+,%a0",
	"	mov.l	%a0,%usp",
	"	tst.w	68(%sp)",
	"	bne.b	return",
	"	bset	&7,69(%sp)",
	"",
	"return:	global	return",
	"	movm.l	(%sp)+,&0x7fff",
	"	add.l	&2,%sp",
	"	rte",
	(char *) 0
};

char	*low20[] = {				/* M68020 low.s body */
	(char *) 0
};

char	*low20a[] = {				/* M68020 low.s body */
	(char *) 0
};

main(argc,argv)
int argc;
char *argv[];
{
	register i, l;
	register FILE *fd;
	int number;
	char input[21], bf[9], c, symbol[21];
	char *argv2;
	register struct t *pt;

	argc--;
	argv++;
	argv2 = argv[0];
	argv++;
/*
 * Scan off the 'c', 'l', 'm', and 't' options.
 */
	while ((argc > 1) && (argv2[0] == '-')) {
		for (i=1; (c=argv2[i]) != NULL; i++) switch (c) {
		case 'c':
			if (cfile) {
				printf(opterror);
				exit(1);
			}
			cfile = argv[0];
			argv++;
			argc--;
			break;
		case 'h':
			if (hfile) {
				printf(opterror);
				exit(1);
			}
			hfile = argv[0];
			argv++;
			argc--;
			break;
		case 'l':
			if (lfile) {
				printf(opterror);
				exit(1);
			}
			lfile = argv[0];
			argv++;
			argc--;
			break;
		case 'm':
			if (mfile) {
				printf(opterror);
				exit(1);
			}
			mfile = argv[0];
			argv++;
			argc--;
			break;
		case 't':
			tflag++;
			break;
		case 'v':
			if (vfile) {
				printf(opterror);
				exit(1);
			}
			vfile = argv[0];
			argv++;
			argc--;
			break;

		default:
			printf("Unknown option\n");
			exit(1);
		}
		argc--;
		argv2 = argv[0];
		argv++;
	}
	if (argc != 1) {
		printf("Usage: config [-t][-v file][-l file][-c file][-m file] [-h file] file\n");
		exit(1);
	}
/*
 * Set up defaults for unspecified files.
 */
	if (cfile == 0)
		cfile = "conf.c";
	if (hfile == 0)
		hfile = "config.h";
	if (lfile == 0)
		lfile = "low.s";
	if (mfile == 0)
		mfile = "/etc/master";
	if (vfile == 0)
		vfile = "m68kvec.s";
	infile = argv2;
	fd = fopen (infile,"r");
	if (fd == NULL) {
		printf("Open error for file -- %s\n",infile);
		exit(1);
	}
/*
 * Open configuration file and set modes.
 */
	fdconf = fopen (cfile,"w");
	if (fdconf == NULL) {
		printf("Open error for file -- %s\n",cfile);
		exit(1);
	}
	chmod (cfile,0664);

/*
 * Write configuration define file
 */
	confhp = fopen(hfile,"w");
	if (confhp == NULL) {
		printf("Open error for file -- %s\n",hfile);
		exit(1);
	}
	chmod (hfile,0664);
	fprintf(confhp,"/* Configuration Definition */\n");
/*
 * Print configuration file heading.
 */
	fprintf(fdconf,"/*\n *  Configuration information\n */\n\n");
	p = table;
/*
 * Read in the master device table and the alias table.
 */
	file();
/*
 * Start scanning the input.
 */
	indfile = 1;		/* indicate that dfile is being read */
	prbsave = prbbound;	/* save probe counter for sysdef(1M) */
	while (fgets(line,MAXLINE,fd) != NULL) {
		if (line[0] == '*')
			continue;
		l = sscanf(line,STR1,input);
		if (l == 0) {
			error("Incorrect line format");
			continue;
		}
		if (equal(input,"root")) {
			rtmaj = dosys(&root,fd);
			rtmin = root.s_minor[0];
			continue;
		}
		if (equal(input,"swap")) {
			swpmaj = dosys(&swap,fd);
			swpmin = swap.s_minor[0];
			continue;
		}
		if (equal(input,"pipe")) {
			pipmaj = dosys(&pipe,fd);
			pipmin = pipe.s_minor[0];
			continue;
		}
		if (equal(input,"dump")) {
			dmpmaj = dosys(&dump,fd);
			dmpmin = dump.s_minor[0];
			dmppt  = dump.s_device;
			continue;
		}
		if (equal(input, "cache")) {
			(void) docache();
			continue;
		}
		if (equal(input,"force")) {
			(void) doforce();
			continue;
		}
		if (equal(input,"probe")) {
			(void) doprobe();
			continue;
		}
		if (equal(input,"alien")) {
			(void) doalien();
			continue;
		}
		if (equal(input,"dup")) {
			(void) dodup();
			continue;
		}
		if (equal(input,"ram")) {
			(void) doram();
			continue;
		}
/*
 * Device or parameter specification other than root, swap, pipe, dump,
 * mpu, force, probe, alien, dup or ram.
 * If power fail recovery is specified, remember it.
 */
		kwdptr = lookup(input);
		if (kwdptr) {
			l = sscanf(line,STR2,input,symbol);
			if (l != 2) {
				error("Incorrect line format");
				continue;
			}
			if (strlen(kwdptr->value)) {
				error("Parameter re-specification");
				continue;
			}
			if (equal(input,"power") && equal(symbol,"1"))
				power++;
			strcpy(kwdptr->value,symbol);
			/* if not struct var parameter, save for sysdef(1M) */
			if(equal(input,"buffers") || equal(input,"calls")
			|| equal(input,"inodes")  || equal(input,"files")
			|| equal(input,"mounts")  || equal(input,"procs")
			|| equal(input,"texts")   || equal(input,"clists")
			|| equal(input,"sabufs")  || equal(input,"maxproc")
			|| equal(input,"swapmap") || equal(input,"hashbuf")
			|| equal(input,"physbuf") || equal(input,"power")
			|| equal(input,"mesg")    || equal(input,"sema")
			|| equal(input,"shmem")   || equal(input,"coremap"))
				continue;
			sprintf(line, "%s\t%s\n", input, symbol);
			savedef(sysdef2);
			continue;
		}
		l = sscanf(line,"%8s%hx%lx%hd%hd",d,&vec,&add,&bl,&ml);
		if (l < 1) {
			error("Incorrect line format");
			continue;
		}
/*
 * Does such a device exist, and if so, do we allow its specification?
 */
		if ((q=find(d)) == 0) {
			error("No such device");
			continue;
		}
		if ((pt = locate(d)) && (q->type2 & ONCE)) {
			error("Only one specification allowed");
			continue;
		}
		if (l < 4) {
			error("Incorrect line format");
			continue;
		}
/*
 * Is the address valid?
 */
		if (q->asize) {
			if((mpu == 68020)? add>(MAXADD|MAP020): add>MAXADD) {
				error("Invalid address");
				continue;
			}
			if (addrcheck(add,q->asize)) {
				continue;
			}
		} else {
			if (add) {
				error("Address not null");
				continue;
			}
		}
/*
 * If the vector size field is non-zero, process as a regular device.
 */
		if (q->vsize) {
			if (vec%VSIZE) {
				error("Vector alignment");
				continue;
			}
			if (vec<8 || (vec+q->vsize)>0x400) {	/* m68k */
				error("Vector not in vector table range");
				continue;
			}

			for( subv = vec/4, vsize = q->vsize;
				vsize > 0; vsize -= VSIZE, subv++ ) {

				if (lowptr[subv]) {
					if (q->type2&MADDR) {
						if ( (pt == 0) ||
						     (pt->vector != vec) ||
						     (pt->vectsize != q->vsize)
						     ) {
							error("Vector clash");
							break;
						}
					} else {
						error("Vector allocated");
						continue;
					}
				} else if ((q->type2&MADDR) && pt) {
					error("Vector varies");
					break;
				}
			}
			/* if vsize != 0, then an error occurred */
			if( vsize )
				continue;
		}
/*
 * Device is not interrupt driven, so interrupt vector and
 * bus request levels should be zero.
 */
		else {
			if (vec) {
				error("Interrupt vector not null");
				continue;
			}
			if (bl) {
				error("Bus level not null");
				continue;
			}
			subv = 0;
		}
/*
 * Check to see that bl is less than or equal to the maximum
 * level specified in the master file
 */

		if (bl > q->busmax) {
			error("Bus level too high\n");
			continue;
		}
/*
 * See if the optional device multiplier was specified, and if so, see
 * if it is a valid one.
 */
		if (l == 5) {
			if ((ml > q->mult) || (ml < 1)) {
				error("Invalid device multiplier");
				continue;
			}
		}
/*
 * Multiplier not specified, so take a default value from master device table.
 */
		else
			ml = q->mult;
/*
 * Fill in the contents of the configuration table node for this device.
 */
		enterdev(vec,add,bl,ml,d);
/*
 * Save the device information for sysdef(1M)
 */
		if (l == 4)
			sprintf(line,"%s\t%x\t%x\t%d\n",d,vec,add,bl);
		else
			sprintf(line,"%s\t%x\t%x\t%d\t%d\n",d,vec,add,bl,ml);
		savedef(sysdef1);
	}
/*
 * Make sure that the root, swap, pipe and dump devices were specified.
 * Set up default values for tunable things where applicable.
 */
	if (rtmaj < 0) {
		printf("root device not specified\n");
		eflag++;
	}
	if (swpmaj < 0) {
		printf("swap device not specified\n");
		eflag++;
	}
	if (pipmaj < 0) {
		printf("pipe device not specified\n");
		eflag++;
	}
	if (dmpmaj < 0) {
		printf("dump device not specified\n");
		eflag++;
	}
/*
 * Make sure that the root, swap, pipe and dump devices have same nbr of minors.
 */
	if (root.s_count != swap.s_count || root.s_count != pipe.s_count
	||  root.s_count != dump.s_count) {
		printf("Number of bootable system devices inconsistent");
		eflag++;
	}
/*
 * Set up default values for tunable things where applicable.
 */
	for (kwdptr=parms; kwdptr<= &parms[pbound]; kwdptr++) {
		if (strlen(kwdptr->value) == 0) {
			if (strlen(kwdptr->defval) == 0) {
				printf("%s not specified\n",kwdptr->indef);
				eflag++;
			}
			strcpy(kwdptr->value,kwdptr->defval);
		}
	}
/*
 * See if configuration is to be terminated.
 */
	if (eflag) {
		printf("\nConfiguration aborted.\n");
		exit (1);
	}
/*
 * Configuration is okay, so write the four files and quit.
 */
	for (q=devinfo; q<= &devinfo[dbound]; q++) {
		if (q->type2 & REQ) {
			if (!(locate(q->dev))) {
				enterdev(0,0L,0,q->mult,q->dev);
			}
		}
	}

	prtcdef();
	prtconf();
	prtvec();
	prtlow();
	exit(0);
}

/*
 * This routine writes out the vector.
 */
prtvec()
{
	register int i, j;
	register struct t *ptr;
/*
 * Open vector file and set modes.
 */
	fdvec = fopen (vfile,"w");
	if (fdvec == NULL) {
		printf("Open error for file -- %s\n",vfile);
		exit(1);
	}
	chmod (vfile,0664);
/*
 * Print some headings.
 */
	fprintf(fdvec,"# Motorola M%d vector table\n\n", mpu);
	fprintf(fdvec,"\ttext\n");
	fprintf(fdvec,"\tglobal\tM68Kvec\n");
	fprintf(fdvec,"M68Kvec:\n");
	fprintf(fdvec,"\tnop\n\tjmp\tstart\n\torg\t8\n");	/* reset "sp" & pc */
/*
 * Go through the vector in long increments.
 */
	for (i=2; i<256; i++) {		/* i = vector number */
		j = i * VSIZE;		/* j = vector address */
		ptr = lowptr[i];
/*
 * If the pointer is 0, set vector up for stray interrupt catcher.
 */
		if (ptr == 0) {
		nullvect:
			/* check if vectoring into alien handler */
			for (alptr = &aliens[albound]; alptr >= aliens; alptr--)
				if(alptr->a_vect == j)
					break;
			if (alptr >= aliens){	/* vector into alien */
				if(alptr->a_addr){
					fprintf(fdvec, "\tlong\t0x%x\t# %x <==\n",
						alptr->a_addr, j);
				}
				else{
					fprintf(fdvec, "\tlong\t%s\t# %x <==\n",
						alptr->a_hndlr, j);
				}
			}
			else	/* vector into nullvect() */
				fprintf(fdvec,"\tlong\tnullvect\t# %x\n", j);
			continue;
		}
/*
 * Pointer refers to a device, so set up the handler routine address with
 * the device sequence number, unless SUPP bit is on.
 */
		if (ptr->type & SUPP)
			goto nullvect;
		if (ptr->count == 0 || (ptr->type&MADDR)) {
			fprintf(fdvec, "\tlong\t %4sint\t# %x <==\n",
				ptr->handler, j);
			continue;
		}
		fprintf(fdvec, "\tlong\t%4s%dint\t# %x <==\n",
			ptr->handler, ptr->count, j);
	}
/*
 * Put a `jmp dump%' after the vector.  This provides a constant entry point
 * for operator-initiated dumps.
 */
	fprintf(fdvec,"\n\tjmp\tdump%%\t # goto dump()\n");
/*
 * The rest of this stuff can go in data space.
 * "sysdevs" must go in data space.  It gets written on.
 */
	fprintf(fdvec, "\n\tdata\n");
/*
 * Print out any force entries, so that the proper mmu, cons, clock and putchar
 * routines can be force linked.  These are drivers, but the kernel knows about
 * their entry points.  I.e., they are not table driven.
 */
	if (force[0]) {
		fprintf(fdvec,"\n# force linking of specific drivers\n");
		for (frcptr = force ; *frcptr ; frcptr += strlen(frcptr) + 1 )
			fprintf(fdvec,"\tlong\t%s\n", frcptr);
	}
/*
 * Print out the system devices for booting
 */
	fprintf(fdvec, "\nsysdevs:\tglobal\tsysdevs\n");
	if (root.s_count == 1) {
		/* standard case: only one system device set. can't switch */
		fprintf(fdvec,"\tshort\t-1\n");
	} else {
		/* print the highest valid boot device */
		fprintf(fdvec, "\tshort\t%d\n", root.s_count);
		/* print the data that must be switched at boot time */
		for (i=0 ; i < root.s_count ; i++ ) {
			fprintf(fdvec, "# boot %d system device data\n", i);
			fprintf(fdvec, "\tshort\t0%o\n", 
			root.s_major[i] <<8 | root.s_minor[i]);
			fprintf(fdvec, "\tshort\t0%o\n",
			pipe.s_major[i] <<8 | pipe.s_minor[i]);
			fprintf(fdvec, "\tshort\t0%o\n",
			dump.s_major[i] <<8 | dump.s_minor[i]);
			fprintf(fdvec, "\tshort\t0%o\n",
			swap.s_major[i] <<8 | swap.s_minor[i]);
			fprintf(fdvec, "\tlong\t%d\n", swp_lo[i]);
			fprintf(fdvec, "\tlong\t%d\n", swp_nbr[i]);
		}
	}
/*
 * Print the saved dfile entries for sysdef(1M)
 */
	prdefs(sysdef1, "sysdef1");
	prdefs(sysdef2, "sysdef2");
	prdefs(sysdef3, "sysdef3");
}

/*
 * This routine writes out the low core program.
 */
prtlow()
{
	register struct t *ptr;
	register int i, j, nbr, arg;
/*
 * Open low core file and set modes.
 */
	fdlow = fopen (lfile,"w");
	if (fdlow == NULL) {
		printf("Open error for file -- %s\n",lfile);
		exit(1);
	}
	chmod (lfile,0664);
/*
 * Print some headings.
 */
	fprintf(fdlow,"# Motorola M%d interrupt/trap handler\n\n", mpu);
	PRTLOW("\ttext");
/*
 * Go through the vector and print put the handler code
 */
	for (i=2; i<256; i++) {		/* i = vector number */
		j = i*VSIZE;		/* j = vector address */
		ptr = lowptr[i];
/*
 * Ignore any vector that has a nullvect() entry.
 */
		if (ptr == 0 || ptr->type & SUPP)
			continue;
/*
 * Look for any dup entry
 */
		for (dupptr = dups; dupptr <= &dups[dupbound]; dupptr++)
			if(dupptr->d_vect == j)
				break;
/*
 * Pointer refers to a device, so set up the handler routine
 * with the device sequence number.
 */
		q = find(ptr->devname);

		if((q->vsize > VSIZE) && (ptr->vector != j))
			continue;

		/* print routine's entry label */

		if (ptr->count == 0 || (ptr->type&MADDR))
			fprintf(fdlow,"%.4sint:	global	%.4sint",
			ptr->handler, ptr->handler);
		else
			fprintf(fdlow,"%.4s%dint:	global	%.4s%dint",
			ptr->handler, ptr->count, ptr->handler, ptr->count);
		fprintf(fdlow,"\t# vector address %x\n",j);

		/* check if the vector is also an alien entry */

		for (alptr = &aliens[albound]; alptr >= aliens; alptr--)
			if(alptr->a_vect == j)
				break;
		if (alptr >= aliens) {
			/* vectoring into alien */
			PRTLOW("	btst	&5,(%sp)");
			/* test if nothing more for this vector */
			if (mpu == 68000
			||  dupptr <= &dups[dupbound]
			||  (ptr->type&(BLOCK|CHAR|INTR))
			    ) {		/* is more */
				fprintf(fdlow,"	bne.b	L%%%d\n",i);
			} else {	/* no more. quit now */
				PRTLOW("	beq	nullvect");
				fprintf(fdlow,"	jmp	0x%x\n\n",
						alptr->a_addr);
				continue;
			}
		}

		/* print stack adjustment code */

		if (mpu == 68000) {
			if (i <= 3)	/* bus or address error */
				PRTLOW("	add.l	&2,%sp");
			else		/* normal stack */
				PRTLOW("	sub.l	&6,%sp");
		} else if ((mpu == 68010) || (mpu == 68020))
			{/*	PRTLOW("	clr.w	-(%sp)");*/}

		/* print register save code */

		PRTLOW("	movm.l	&0xc0c0,-(%sp)");

		/* print any dup entries */

		for (arg = 0; dupptr <= &dups[dupbound]; dupptr++) {
			if(dupptr->d_vect == j) {
				/* if argument, push it */
				if (dupptr->d_flag&D_ARG) {
					pusharg(arg,dupptr->d_arg);
					arg = 1;
				}
				fprintf(fdlow,"	jsr	%s\n", dupptr->d_hndlr);
				if (dupptr->d_flag&D_BEQ) {
					if (dupptr->d_flag&D_ARG) {
						PRTLOW("	add.l	&4,%sp");
						arg = 0;
					}
					PRTLOW("	tst.l	%d0");
					if (dupptr->d_flag&D_INTRET)
						PRTLOW("	beq	intret");
					else if (dupptr->d_flag&D_ALLTRAPS)
						PRTLOW("	beq	alltraps");
					else
						PRTLOW("	beq	return");
				}
			}
		}

		/* determine device/trap number loading */

		nbr = -1;
		if (ptr->type&(BLOCK|CHAR)) {	/* device number */
			if (!(ptr->type&MADDR)) {
				nbr = 0;
				if (ptr->count) {
					for (p=table; p<= &table[tbound]; p++)
						if (ptr->count > p->count
						&&  equal(q->dev,p->devname))
							nbr += p->mlt;
				}
			}
		} else if (mpu == 68000 && !(ptr->type&INTR))
			nbr = i;		/* trap number */

		/* print ending code */

		if (ptr->type&(BLOCK|CHAR|INTR)) {
			/* check for an argument */
			if (nbr >= 0) {
				pusharg(arg, nbr);	/* push the arg */
				/* if multiple devices, go to common jsr */
				if (nbr > 0 ) {
					fprintf(fdlow,"	bra	L%%%s\n",
						ptr->handler);
					goto alienjmp;
				} else {	/* define the common jsr */
					fprintf(fdlow,"L%%%s:", ptr->handler);
				}
			}
			/* make sure "clkintr" not used */
			if (equal(ptr->handler,"clk"))
				PRTLOW("	jsr	clock");
			else
				fprintf(fdlow,"	jsr	%sintr\n",ptr->handler);
			/* stack cleanup */
			if (arg || nbr >= 0)
				PRTLOW("	add.l	&4,%sp");
			PRTLOW("	bra	intret");
		} else {
			/* stack cleanup */
			if (arg)
				PRTLOW("	add.l	&4,%sp");
			if (nbr >= 0)
				fprintf(fdlow,"	mov.l	&%d,%%d0\n", nbr);
			PRTLOW("	bra	alltraps");
		}

		/* if vectoring into alien, put jmp here */

	alienjmp:
		if (alptr >= aliens) {
			fprintf(fdlow,"L%%%d:	jmp	0x%x\n",
					i, alptr->a_addr);
		}
		PRTLOW("");
	}
/*
 * Print out the interrupt return and trap interface
 */
	{	register char **p;

		if (mpu == 68000) {
			p = low00;
			while ( *p )
				PRTLOW(*p++);
		}
		else if (mpu == 68010) {
			p = low10;
			while ( *p )
				PRTLOW(*p++);
			if((cache&(USER | LOGICAL)) ==
					(USER | LOGICAL))
				PRTLOW("	jsr	cacheclr");	
			p = low10a;
			while ( *p )
				PRTLOW(*p++);
		}
		else  {
			p = low20;
			while ( *p )
				PRTLOW(*p++);
			if((cache&(USER | LOGICAL)) ==
					(USER | LOGICAL))
				PRTLOW("	jsr	cacheclr");	
			p = low20a;
			while ( *p )
				PRTLOW(*p++);
		}
	}
}

/*
 * pusharg() - produce low.s code to push an immediate argument onto the stack
 */
pusharg(flag,value)
register value;
{
	static lastval;

	if (flag && lastval == value)
		return;
	lastval = value;
	if (value >= -128 && value < 127) {
		fprintf(fdlow,	"	mov.l	&%d,%%d0\n", value);
		if (flag)
			PRTLOW(	"	mov.l	%d0,(%sp)");
		else
			PRTLOW(	"	mov.l	%d0,-(%sp)");
	} else if (flag)
		fprintf(fdlow,	"	mov.l	&0x%x,(%%sp)\n", value);
	else
		fprintf(fdlow,	"	mov.l	&0x%x,-(%%sp)\n", value);
}

/*
 * This routine is used to read in the master device table and the
 * alias table.  In the master device file, these two tables are separated
 * by a line that contains an asterisk in column 1.
 */
file()
{
	register l;
	register FILE *fd;
	fd = fopen(mfile,"r");
	if (fd == NULL) {
		printf("Open error for file -- %s\n",mfile);
		exit(1);
	}
	q = devinfo;

	while (fgets(line,MAXLINE,fd) != NULL) {
/*
 * Check for the delimiter that indicates the beginning of the
 * alias table entries.
 */
		if (line[0] == '$')
			break;
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		dbound++;
		if (dbound == DSIZE) {
			printf("Device table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%8s%hd%o%ho%4s%hd%hd%hd%hd%hd%8s%8s%8s",
			q->dev,&(q->vsize),&(q->mask),&(q->type2),
			q->hndlr,&(q->asize),&(q->block),
			&(q->charr),&(q->mult),&(q->busmax),
			q->entries[0],q->entries[1],q->entries[2]);
		if (l < 10) {
			printf("%s\nDevice parameter count\n",line);
			exit(1);
		}
		if (   ((q->type2& REQ) && (q->vsize || q->asize || q->busmax))
		    || ((q->type2&INTR) && ((q->mask&(~(PWR|INIT)))
					    || (q->type2&(BLOCK|CHAR))
					    || q->block || q->charr ))
		    ||	(q->vsize && (q->vsize%VSIZE))
		   )	{
			printf("%s\nParameter inconsistency\n",line);
			exit(1);
		}
/*
 * Update the ends of the block and character device tables.
 */
		if (q->type2 & BLOCK)
			if ((blockex = max(blockex,q->block)) >= BSIZE) {
				printf("%s\nBad major device number\n",line);
				exit(1);
			}
		if (q->type2 & CHAR)
			if ((charex = max(charex,q->charr)) >= CSIZE) {
				printf("%s\nBad major device number\n",line);
				exit(1);
			}

		if (q->type2 & INTR) {
			if ( intrex >= ISIZE ) {
				printf("%s\nToo many inter devices\n", line );
				exit(1);
			}
			q->mintr = intrex++;
		}

		q++;
	}
	r = alias;
	while (fgets(line,MAXLINE,fd) != NULL) {
/*
 * Check for the delimiter that indicates the beginning of the
 * keyword table entries.
 */
		if (line[0] == '$')
			break;
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		abound++;
		if (abound == ASIZE) {
			printf("Alias table overflow\n");
			exit(1);
		}
		l = sscanf(line,"%8s%8s",r->new,r->old);
		if (l < 2) {
			printf("%s\nAlias parameter count\n",line);
			exit(1);
		}
		r++;
	}
	kwdptr = parms;
	while (fgets(line,MAXLINE,fd) != NULL) {
/*
 * Check for the delimiter that indicates the beginning of the
 * m68k specific entries.
 */
		if (line[0] == '$')
			break;
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;
		pbound++;
		if (pbound == PSIZE) {
			printf("Keyword table overflow\n");
			exit(1);
		}
		l = sscanf(line,STR3,kwdptr->indef,
			kwdptr->oudef,kwdptr->defval);
		if (l < 2) {
			printf("%s\nTunable parameter count\n",line);
			exit(1);
		}
		if (l == 2)
			*(kwdptr->defval) = NULL;
		kwdptr++;
	}
	while (fgets(line,MAXLINE,fd) != NULL) {
		char sbuf[CSIZE];
/*
 * Check for comment.
 */
		if (line[0] == '*')
			continue;

		l = sscanf(line, STR1, sbuf);
		if (l < 1) {
			printf("%s\nInvalid m68k option\n", line);
			exit(1);
		}
		if (equal(sbuf, "mpu")) {
			if (dompu())
				exit(1);
			continue;
		}
		if (equal(sbuf, "cache")) {
			if (docache())
				exit(1);
			continue;
		}
		if (equal(sbuf, "force")) {
			if (doforce())
				exit(1);
			continue;
		}
		if (equal(sbuf, "probe")) {
			if (doprobe())
				exit(1);
			continue;
		}
		if (equal(sbuf, "alien")) {
			if (doalien())
				exit(1);
			continue;
		}
		if (equal(sbuf, "dup")) {
			if (dodup())
				exit(1);
			continue;
		}
		printf("%s\nInvalid m68k option\n", line);
		exit(1);
	}
}

equal(s1,s2)
register char *s1, *s2;
{
	while (*s1++ == *s2) {
		if (*s2++ == 0)
			return(1);
	}
	return(0);
}

/*
 * This routine is used to print a configuration time error message
 * and then set a flag indicating a contaminated configuration.
 */
error(message)
char *message;
{
	printf("%s%s\n\n",line,message);
	eflag++;
}

/*
 * This routine is used to search through the master device table for
 * some specified device.  If the device is found, we return a pointer to
 * the device.  If the device is not found, we search the alias table for
 * this device.  If the device is not found in the alias table, we return a
 * zero.  If the device is found, we change its name to the reference name of
 * the device and re-initiate the search for this new name in the master
 * device table.
 */
struct t2 *
find(device)
char *device;
{
	register struct t2 *q;
	register struct t3 *r;
	for (;;) {
		for (q=devinfo; q<= &devinfo[dbound]; q++) {
			if (equal(device,q->dev))
				return(q);
		}
		for (r=alias; r<= &alias[abound]; r++) {
			if (equal(device,r->new)) {
				device = r->old;
				break;
			}
		}
		if (r > &alias[abound])
			return(0);
	}
}

/*
 * This routine is used to set the character and/or block table
 * or other interrupt pointers to point to an entry of the
 * master device table.
 */
setq()
{
	register struct t2 *ptr;

	switch (q->type2 & (BLOCK|CHAR|INTR|FSSW)) {
	default:
	case 0:
		break;
	case INTR:
		idevices[q->mintr] = q;
		break;
	case FSSW:
		subtmp = q->charr;
		ptr = fswitch[subtmp];
		fswitch[subtmp] = q;
		fmax = max (fmax,subtmp);
		break;
	case CHAR:
		subtmp = q->charr;
		ptr = cdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				charex++;
				if (charex == CSIZE) {
					printf("Character table overflow\n");
					exit(1);
				}
				q->charr = subtmp = charex;
			}
		}
		cdevices[subtmp] = q;
		cmax = max (cmax,subtmp);
		break;
	case BLOCK:
		subtmp = q->block;
		ptr = bdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				blockex++;
				if (blockex == BSIZE) {
					printf("Block table overflow\n");
					exit(1);
				}
				q->block = subtmp = blockex;
			}
		}
		bdevices[subtmp] = q;
		bmax = max (bmax,subtmp);
		break;
	case BLOCK|CHAR:
		subtmp = q->charr;
		ptr = cdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				charex++;
				if (charex == CSIZE) {
					printf("Character table overflow\n");
					exit(1);
				}
				q->charr = subtmp = charex;
			}
		}
		cdevices[subtmp] = q;
		cmax = max (cmax,subtmp);
		subtmp = q->block;
		ptr = bdevices[subtmp];
		if (ptr) {
			if (!equal(ptr->dev,q->dev)) {
				blockex++;
				if (blockex == BSIZE) {
					printf("Block table overflow\n");
					exit(1);
				}
				q->block = subtmp = blockex;
			}
		}
		bdevices[subtmp] = q;
		bmax = max (bmax,subtmp);
		break;
	}
}

/*
 * This routine writes out the configuration define file (include file.)
 */
prtcdef()
{
	for (kwdptr=parms; kwdptr <= &parms[pbound]; kwdptr++) {
		fprintf(confhp,"#define\t%s\t%s\n",
			kwdptr->oudef, kwdptr->value);
	}
	fclose(confhp);
	return;
}

/*
 * This routine writes out the configuration file (C program.)
 */
prtconf()
{
	register i, j, counter,k;
	register struct t2 *ptr;
/*
 * Print some headings.
 */
	fprintf(fdconf,"\n");
	fprintf(fdconf,"#include\t\"%s\"\n",hfile);
	fprintf(fdconf,"#include\t\"sys/conf.h\"\n");
	fprintf(fdconf,"#include\t\"sys/param.h\"\n");
	fprintf(fdconf,"#include\t\"sys/types.h\"\n");
	fprintf(fdconf,"#include\t\"sys/sysmacros.h\"\n");
	fprintf(fdconf,"#include\t\"sys/space.h\"\n");
	fprintf(fdconf,"#include\t\"sys/io.h\"\n\n\n");
	fprintf(fdconf,"extern nodev(), nulldev();\n");
	fprintf(fdconf,"extern int fsstray(),fsnull(),*fsistray();\n");
	fprintf(fdconf,"extern struct inode *fsinstray();\n");
	fprintf(fdconf,"#define\tnotty\t(struct tty *)(0)\n");
	fprintf(fdconf,"#define\tnostr\t(struct streamtab *)(0)\n\n");

/*
 * Search the configuration table and generate an extern statement for
 * any routines that are needed.
 */
	for (p=table; p<= &table[tbound]; p++) {
		if (p->count)
			continue;
		switch (p->type & (BLOCK|CHAR|FSSW|INTR)) {
		case INTR:
			q = find(p->devname);
			if (q->mask & INIT)
				fprintf(fdconf,"extern %sinit();\n",q->hndlr);
			break;
		case FSSW:
			q = find(p->devname);
			sprintf(xbf,"extern ");
			if (q->mask & INIT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"init(), ");
			}
			if (q->mask & IPUT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"iput(), ");
			}
			if (q->mask & IUPDAT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"iupdat(), ");
			}
			if (q->mask & READI) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"readi(), ");
			}
			if (q->mask & WRITEI) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"writei(), ");
			}
			if (q->mask & ITRUNC) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"itrunc(), ");
			}
			strcat(xbf,"\n");
			if (q->mask & STATF) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"statf(), ");
			}
			if (q->mask & NAMEI) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"namei(), ");
			}
			if (q->mask & MOUNT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"mount(), ");
			}
			if (q->mask & UMOUNT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"umount(), ");
			}
			if (q->mask & OPENI) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"openi(), ");
			}
			if (q->mask & CLOSEI) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"closei(), ");
			}
			if (q->mask & UPDATE) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"update(), ");
			}
			strcat(xbf,"\n");
			if (q->mask & STATFS) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"statfs(), ");
			}
			if (q->mask & ACCESS) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"access(), ");
			}
			if (q->mask & GETDENTS) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"getdents(), ");
			}
			if (q->mask & ALLOCMAP) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"allocmap(), ");
			}
			if (q->mask & FREEMAP) {
				strcat (xbf,"*");
				strcat (xbf,q->hndlr);
				strcat (xbf,"freemap(), ");
			}
			if (q->mask & READMAP) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"readmap(), ");
			}
			if (q->mask & SETATTR) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"setattr(), ");
			}
			if (q->mask & NOTIFY) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"notify(), ");
			}
			strcat(xbf,"\n");
			if (q->mask & FCNTL) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"fcntl(), ");
			}
			if (q->mask & FSINFO) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"fsinfo(), ");
			}
			if (q->mask & FIOCTL) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"ioctl(), ");
			}
			xbf[strlen(xbf)-2] = ';';
			xbf[strlen(xbf)-1] = NULL;
			fprintf(fdconf,"%s\n",xbf);
			sprintf(xbf,"extern struct inode ");
			if (q->mask & IREAD) {
				strcat (xbf,"*");
				strcat (xbf,q->hndlr);
				strcat (xbf,"iread(), ");
			}
			if (q->mask & GETINODE) {
				strcat (xbf,"*");
				strcat (xbf,q->hndlr);
				strcat (xbf,"getinode(), ");
			}
			xbf[strlen(xbf)-2] = ';';
			xbf[strlen(xbf)-1] = NULL;
			fprintf(fdconf,"%s\n",xbf);
			break;
			
		case CHAR:
			q = find(p->devname);
			sprintf(xbf,"extern ");
			if (q->mask & OPEN) {
				strcat (xbf,q->hndlr);
				if (q->mask & CBSEP)
					strcat (xbf,"c");
				strcat (xbf,"open(), ");
			}
			if (q->mask & CLOSE) {
				strcat (xbf,q->hndlr);
				if (q->mask & CBSEP)
					strcat (xbf,"c");
				strcat (xbf,"close(), ");
			}
			if (q->mask & READ) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"read(), ");
			}
			if (q->mask & WRITE) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"write(), ");
			}
			if (q->mask & IOCTL) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"ioctl(), ");
			}
			if (q->mask & PWR)
				if (power) {
					strcat (xbf,q->hndlr);
					strcat (xbf,"clr(), ");
				}
			if (q->mask & INIT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"init(), ");
			}
			if (q->mask & STREAM) {
				strcat(xbf," struct streamtab ");
				strcat(xbf,q->hndlr);
				strcat(xbf,"info;\n");
			}
			xbf[strlen(xbf)-2] = ';';
			if (q->mask & TTYS) {
				strcat(xbf,"extern struct tty ");
				strcat(xbf,q->hndlr);
				strcat(xbf,"_tty[];\n");
			}
			xbf[strlen(xbf)-1] = NULL;
			fprintf(fdconf,"%s\n",xbf);
			break;
		case BLOCK:
			fprintf(fdconf, "extern %s", p->handler);
			if (q->mask & CBSEP)
				fprintf(fdconf, "b");
			fprintf(fdconf, "open(), %s", p->handler);
			if (q->mask & CBSEP)
				fprintf(fdconf, "b");
			fprintf(fdconf, "close(), %sstrategy(), %sprint();\n",
				p->handler,p->handler,p->handler);
			break;
		case BLOCK|CHAR:
			q = find(p->devname);
			if(q->mask & CBSEP) {
				strcpy (xbf,"extern ");
				strcat (xbf,q->hndlr);
				strcat (xbf,"bopen(), ");
				strcat (xbf,q->hndlr);
				strcat (xbf,"copen(), ");
				strcat (xbf,q->hndlr);
				strcat (xbf,"bclose(), ");
				strcat (xbf,q->hndlr);
				strcat (xbf,"cclose(), ");
			} else
				sprintf(xbf,"extern %sopen(), %sclose(), ",
					q->hndlr,q->hndlr);
			if (q->mask & READ) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"read(), ");
			}
			if (q->mask & WRITE) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"write(), ");
			}
			if (q->mask & IOCTL) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"ioctl(), ");
			}
			if (q->mask & PWR)
				if (power) {
					strcat (xbf,q->hndlr);
					strcat (xbf,"clr(), ");
				}
			if (q->mask & INIT) {
				strcat (xbf,q->hndlr);
				strcat (xbf,"init(), ");
			}
			strcat (xbf,q->hndlr);
			strcat (xbf,"strategy(), ");
			strcat (xbf,q->hndlr);
			strcat (xbf,"print();");
			fprintf(fdconf,"%s\n",xbf);
			break;
		}
	}

	fprintf(fdconf,"\nstruct bdevsw bdevsw[] = {\n");
/*
 * Go through block device table and indicate addresses of required routines.
 * If a particular device is not present, fill in "nodev" entries.
 */
	for (i=0; i<=bmax; i++) {
		ptr = bdevices[i];
		fprintf(fdconf,"/*%2d*/\t",i);
		if (ptr) {
			fprintf(fdconf,"%s%sopen,\t%s%sclose,\t%sstrategy,\t%sprint,\n",
				ptr->hndlr, (ptr->mask & CBSEP)? "b": "",
				ptr->hndlr, (ptr->mask & CBSEP)? "b": "",
				ptr->hndlr,
				ptr->hndlr);
		} else {
			fprintf(fdconf,"nodev, \tnodev, \tnodev, \tnodev,\n");
		}
	}
	fprintf(fdconf,"};\n\n");
	fprintf(fdconf,"struct cdevsw cdevsw[] = {\n");
/*
 * Go through character device table and indicate addresses of required
 * routines, or indicate "nulldev" if routine is not present.  If a
 * particular device is not present, fill in "nodev" entries.
 */
	for (j=0; j<=cmax; j++) {
		ptr = cdevices[j];
		fprintf(fdconf,"/*%2d*/",j);
		if (ptr) {
			if (ptr->mask & OPEN)
				fprintf(fdconf,"\t%s%sopen,",ptr->hndlr,
					(ptr->mask & CBSEP)? "c": "");
			else
				fprintf(fdconf,"\tnulldev,");
			if (ptr->mask & CLOSE)
				fprintf(fdconf,"\t%s%sclose,",ptr->hndlr,
					(ptr->mask & CBSEP)? "c": "");
			else
				fprintf(fdconf,"\tnulldev,");
			if (ptr->mask & READ)
				fprintf(fdconf,"\t%sread,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (ptr->mask & WRITE)
				fprintf(fdconf,"\t%swrite,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (ptr->mask & IOCTL)
				fprintf(fdconf,"\t%sioctl,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnodev, ");
			if (ptr->mask & TTYS)
				fprintf(fdconf,"\t%s_tty,",ptr->hndlr);
			else
				fprintf(fdconf,"\tnotty,");
			if (ptr->mask & STREAM)
				fprintf(fdconf,"\t&%sinfo,\n",ptr->hndlr);
			else
				fprintf(fdconf,"\tnostr,\n");
		}
		else
			fprintf(fdconf,"\tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \tnotty,\tnostr,\n");
	}
	fprintf(fdconf,"};\n\n");
	fprintf(fdconf,"struct fstypsw fstypsw[] = {\n");
/* Print out the file system switching table. */
	for (k=0; k<=fmax; k++) {
		ptr = fswitch[k];
		if (ptr) {
			fprintf(fdconf,"\n");
			if (ptr->mask & INIT) {
				fprintf(fdconf,"\t%sinit, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & IPUT) {
				fprintf(fdconf,"%siput, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & IREAD) {
				fprintf(fdconf,"%siread, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"\t,fsinstray,");
			fprintf(fdconf,"fsstray,");
			if (ptr->mask & IUPDAT) {
				fprintf(fdconf,"%siupdat, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & READI) {
				fprintf(fdconf,"%sreadi, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & WRITEI) {
				fprintf(fdconf,"%swritei, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & ITRUNC) {
				fprintf(fdconf,"%sitrunc,\n ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,\n");
			if (ptr->mask & STATF) {
				fprintf(fdconf,"\t%sstatf, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & NAMEI) {
				fprintf(fdconf,"%snamei, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & MOUNT) {
				fprintf(fdconf,"%smount, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & UMOUNT) {
				fprintf(fdconf,"%sumount, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & GETINODE) {
				fprintf(fdconf,"%sgetinode, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsinstray,");
			if (ptr->mask & OPENI) {
				fprintf(fdconf,"%sopeni, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & CLOSEI) {
				fprintf(fdconf,"%sclosei, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & UPDATE) {
				fprintf(fdconf,"%supdate,\n ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,\n");
			if (ptr->mask & STATFS) {
				fprintf(fdconf,"\t%sstatfs, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & ACCESS) {
				fprintf(fdconf,"%saccess, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & GETDENTS) {
				fprintf(fdconf,"%sgetdents, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & ALLOCMAP) {
				fprintf(fdconf,"%sallocmap, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & FREEMAP) {
				fprintf(fdconf,"%sfreemap, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & READMAP) {
				fprintf(fdconf,"%sreadmap, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & SETATTR) {
				fprintf(fdconf,"%ssetattr, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & NOTIFY) {
				fprintf(fdconf,"%snotify,\n ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,\n");
			if (ptr->mask & FCNTL) {
				fprintf(fdconf,"\t%sfcntl, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"\tfsstray,");
			if (ptr->mask & FSINFO) {
				fprintf(fdconf,"%sfsinfo, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			if (ptr->mask & FIOCTL) {
				fprintf(fdconf,"%sioctl, ",ptr->hndlr);
			}
			else
				 fprintf(fdconf,"fsstray,");
			fprintf(fdconf,"fsstray,fsstray,fsstray,fsstray,fsstray,\n");
		}
		else{
			fprintf(fdconf,"\tfsstray,fsstray,fsinstray,fsstray,fsstray,fsstray,fsstray,fsstray,\n");
			fprintf(fdconf,"\tfsstray,fsstray,fsstray,fsstray,fsinstray,fsstray,fsstray,fsstray,\n");
			fprintf(fdconf,"\tfsstray,fsstray,fsstray,fsstray,fsistray,fsstray,fsstray,fsstray,\n");
			fprintf(fdconf,"\tfsstray,fsstray,fsstray,fsstray,fsstray,fsstray,fsstray,fsstray,\n");
		}
	}
/*
 * Print out block and character device counts, root, swap, and dump device
 * information, and the swplo, and nswap values.
 */
	fprintf(fdconf,"};\n\n");
	fprintf(fdconf,"#define\tNFSTYPE\t%d\n",k);
	fprintf(fdconf,"short\t nfstyp = {NFSTYPE};\n");
	fprintf(fdconf,"int\tbdevcnt = %d;\nint\tcdevcnt = %d;\n\n",i,j);
	fprintf(fdconf,"char MAJOR[%d] = {",j);
	for(k=0; k !=j; k++){
		fprintf(fdconf,"%d,",k);
	}
	fprintf(fdconf,"};\n");
	fprintf(fdconf,"char MINOR[%d] = {",j);
	for(k=0; k !=j; k++){
		fprintf(fdconf,"0,",k);
	}
	fprintf(fdconf,"};");
		
	fprintf(fdconf,"\n\n");
	fprintf(fdconf,"dev_t\trootdev = makedev(%d, %d);\n",rtmaj,rtmin);
	fprintf(fdconf,"dev_t\tpipedev = makedev(%d, %d);\n",pipmaj,pipmin);
	fprintf(fdconf,"dev_t\tdumpdev = makedev(%d, %d);\n",dmpmaj,dmpmin);
	fprintf(fdconf,"extern %sdump();\n", dmppt->handler);
	fprintf(fdconf, "int\t(*dump)() = %sdump;\n",dmppt->handler);
	fprintf(fdconf, "int\tdump_addr = %#8x;\n",dmppt->address);
	fprintf(fdconf,"dev_t\tswapdev = makedev(%d, %d);\n",swpmaj,swpmin);
	fprintf(fdconf,"daddr_t\tswplo = %lu;\nint\tnswap = %d;\n\n",swplo,nswap);
/*
 * Initialize the power fail and init handler buffers.
 */
	sprintf(pwrbf,"\nint\t(*pwr_clr[])() = \n{\n");
	sprintf(initbf,"\nint\t(*io_init[])() = \n{\n");
/*
 * Go through the block device table, ignoring any zero entries.
 * Add power fail and init handler entries to the buffers as appropriate.
 */
	if (tflag)
		printf("Block Devices\nmajor\tdevice\thandler\tcount\n");
	for (i=0; i<=bmax; i++) {
		if ((q=bdevices[i]) == 0)
			continue;
		if (power)
			if (q->mask & PWR)
				sprintf(&pwrbf[strlen(pwrbf)],"\t%sclr,\n",q->hndlr);
		if (q->mask & INIT)
				sprintf(&initbf[strlen(initbf)],"\t%sinit,\n",q->hndlr);
		counter = 0;
/*
 * For each of these devices, go through the configuration table and
 * look for matches. For each match, print the device address, and keep
 * a count of the number of matches.
 */
		fprintf(fdconf,"\n");
		if (q->asize)
			fprintf(fdconf,"int\t%s_addr[] = {\n",q->hndlr);
		for (p=table; p<= &table[tbound]; p++) {
			if (equal(q->dev,p->devname)) {
				if (q->asize)
					fprintf(fdconf,"\t0x%x,\n",p->address);
				counter += p->mlt;
			}
		}
		if (q->asize)
			fprintf(fdconf,"};\n");
		if (!(q->type2 & NOCNT))
			fprintf(fdconf,"int\t%s_cnt = %d;\n",
				q->hndlr,counter);
		if ((q->type2&ADDVEC) && q->vsize) {
			fprintf(fdconf,"int\t%s_ivec[] = {\n",q->hndlr);
			for (p=table; p<= &table[tbound]; p++) {
				if (equal(q->dev,p->devname))
					fprintf(fdconf,"\t0x%x,\n",p->vector);
			}
			fprintf(fdconf,"};\n");
		}
		if (q->type2&ADDLEV) {
			fprintf(fdconf,"int\t%s_ilev[] = {\n",q->hndlr);
			for (p=table; p<= &table[tbound]; p++) {
				if (equal(q->dev,p->devname))
					fprintf(fdconf,"\t%d,\n",p->buslevel);
			}
			fprintf(fdconf,"};\n");
		}
		if (q->type2&ADDMAJ) {
			fprintf(fdconf,"int\t%s_bmaj = %d;\n",
				q->hndlr,q->block);
			if (q->type2&CHAR)
				fprintf(fdconf,"int\t%s_cmaj = %d;\n",
					q->hndlr,q->charr);
		}
		if (tflag)
			printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,counter);
		q->acount = counter;
/*
 * Print any required structure definitions.
 */
		for (j=0; j<3; j++) {
			if (*(q->entries[j]) != NULL)
				fprintf(fdconf,"struct\t%s\t%s_%s[%d];\n",
					q->entries[j],q->hndlr,
					q->entries[j],counter);
		}
	}
/*
 * Go through the character device table, ignoring any zero entries,
 * as well as those that are not strictly character devices.
 * Add power fail and init handler entries to the buffers as appropriate.
 */
	if (tflag)
		printf("Character Devices\nmajor\tdevice\thandler\tcount\n");
	for (i=0; i<=cmax; i++) {
		if ((q=cdevices[i]) == 0)
			continue;
		if ((q->type2 & (BLOCK|CHAR)) != CHAR) {
			if (tflag)
				printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,q->acount);
			continue;
		}
		if (power)
			if (q->mask & PWR)
				sprintf(&pwrbf[strlen(pwrbf)],"\t%sclr,\n",q->hndlr);
		if (q->mask & INIT)
				sprintf(&initbf[strlen(initbf)],"\t%sinit,\n",q->hndlr);
		counter = 0;
/*
 * For each of these devices, go through the configuration table and
 * look for matches. For each match, print the device address, and keep
 * a count of the number of matches.
 */
		fprintf(fdconf,"\n");
		if (q->asize)
			fprintf(fdconf,"int\t%s_addr[] = {\n",q->hndlr);
		for (p=table; p<= &table[tbound]; p++) {
			if (equal(q->dev,p->devname)) {
				if (q->asize)
					fprintf(fdconf,"\t0x%lx,\n",p->address);
				counter += p->mlt;
			}
		}
		if (q->asize)
			fprintf(fdconf,"};\n");
		if (!(q->type2 & NOCNT))
			fprintf(fdconf,"int\t%s_cnt = %d;\n",
				q->hndlr,counter);
		if ((q->type2&ADDVEC) && q->vsize) {
			fprintf(fdconf,"int\t%s_ivec[] = {\n",q->hndlr);
			for (p=table; p<= &table[tbound]; p++) {
				if (equal(q->dev,p->devname))
					fprintf(fdconf,"\t0x%x,\n",p->vector);
			}
			fprintf(fdconf,"};\n");
		}
		if (q->type2&ADDLEV) {
			fprintf(fdconf,"int\t%s_ilev[] = {\n",q->hndlr);
			for (p=table; p<= &table[tbound]; p++) {
				if (equal(q->dev,p->devname))
					fprintf(fdconf,"\t%d,\n",p->buslevel);
			}
			fprintf(fdconf,"};\n");
		}
		if (q->type2&ADDMAJ)
			fprintf(fdconf,"int\t%s_cmaj = %d;\n",
				q->hndlr,q->charr);
		if (tflag)
			printf("%2d\t%s\t%s\t%2d\n",i,q->dev,q->hndlr,counter);
/*
 * Print any required structure definitions.
 */
		for (j=0; j<3; j++) {
			if (*(q->entries[j]) != NULL)
				fprintf(fdconf,"struct\t%s\t%s_%s[%d];\n",
					q->entries[j],q->hndlr,
					q->entries[j],counter);
		}
	}

/*
 * Go through the other interrupt table to print out addr table
 *	if defined.
 */

	for(i=0; i<=intrex; i++) {
		counter = 0;
		if((q=idevices[i]) == 0 )
			continue;
		if (power && (q->mask & PWR))
			sprintf(&pwrbf[strlen(pwrbf)],"\t%sclr,\n",q->hndlr);
		if (q->mask & INIT)
			sprintf(&initbf[strlen(initbf)],"\t%sinit,\n",q->hndlr);
		if (q->asize) {
			fprintf(fdconf,"int\t%s_addr[] = {\n",q->hndlr);
			for (p=table; p<= &table[tbound]; p++)
				if (equal(q->dev,p->devname)) {
					fprintf(fdconf,"\t0x%lx,\n",p->address);
					counter += p->mlt;
				}
			fprintf (fdconf, "};\n");
		}

		if (!(q->type2 & NOCNT))
			fprintf(fdconf,"int\t%s_cnt = %d;\n",q->hndlr,counter );
		if ((q->type2&ADDVEC) && q->vsize) {
			fprintf(fdconf,"int\t%s_ivec[] = {\n",q->hndlr);
			for (p=table; p<= &table[tbound]; p++) {
				if (equal(q->dev,p->devname))
					fprintf(fdconf,"\t0x%x,\n",p->vector);
			}
			fprintf(fdconf,"};\n");
		}
		if (q->type2&ADDLEV) {
			fprintf(fdconf,"int\t%s_ilev[] = {\n",q->hndlr);
			for (p=table; p<= &table[tbound]; p++) {
				if (equal(q->dev,p->devname))
					fprintf(fdconf,"\t%d,\n",p->buslevel);
			}
			fprintf(fdconf,"};\n");
		}
	}
					


/*
 * Write out NULL entry into power fail and init buffers.
 * Then write the buffers out into the configuration file.
 */
	sprintf(&pwrbf[strlen(pwrbf)],"\t(int (*)())0\n};\n");
	sprintf(&initbf[strlen(initbf)],"\t(int (*)())0\n};\n");
	fprintf(fdconf,"%s",pwrbf);
	fprintf(fdconf,"%s",initbf);
/*
 * Write out the ram table
 */
	fprintf(fdconf, "\n#include \"sys/ram.h\"\n\n");
	ramptr = rams;
	if (rbound < 0) {
		/* use default ram table */
		rbound = 0;
		ramptr->r_high = LOWMAP;
	} else {
		/* let sysdef(1M) know ram_tbl came from dfile */
		fprintf(fdconf,"char sysdefr;\n");
	}
	fprintf(fdconf, "struct ram ram_tbl[] = {\n");
	for (i = 0; i <= rbound; i++, ramptr++) {
		j = (ramptr->r_flag&R_IN_NoInit) == 0;	/* R_OUT_Init or 0 */
		if  (ramptr->r_flag&R_IN_Mult)
			j |= R_OUT_Hunt;
		if (ramptr->r_incr == 0)
			ramptr->r_incr = ramptr->r_high - ramptr->r_low;
		fprintf(fdconf, "\t{0x%x, 0x%x, 0x%x, 0x%x}",
			btoct(ramptr->r_low), btoct(ramptr->r_high),
			btoct(ramptr->r_incr), j);
		if (i == rbound)
			fprintf(fdconf, "\n");
		else
			fprintf(fdconf, ",\n");
	}
	fprintf(fdconf, "};\n\n");
	fprintf(fdconf, "unsigned ram_nbr = sizeof(ram_tbl)/sizeof(struct ram);\n");

/*
 * Write out the probe table
 */
	fprintf(fdconf, "\n\nstruct probe probe_tbl[] = {\n");
	for (prbptr = probes, i = 0; i <= prbbound; i++, prbptr++)
		fprintf(fdconf, "\t{(char*)0x%x, 0x%x},\n",
			prbptr->p_addr, prbptr->p_byte);
	fprintf(fdconf, "\t{(char*)0, 0}\n");
	fprintf(fdconf, "};\n");
	/* if probes were in the dfile,
	 * sysdef(1M) needs the index of where they start
	 */
	if (prbsave != prbbound)
		fprintf(fdconf, "\nshort sysdefp = %d;\n", prbsave + 1);
}

max(a,b)
int a, b;
{
	return(a>b ? a:b);
}

/*
 * This routine is used to search the configuration table for some
 * specified device.  If the device is found we return a pointer to
 * that device.  If the device is not found, we search the alias
 * table for this device.  If the device is not found in the alias table
 * we return a zero.  If the device is found, we change its name to
 * the reference name of the device and re-initiate the search for this
 * new name in the configuration table.
 */

struct t *
locate(device)
char *device;
{
	register struct t *p;
	register struct t3 *r;
	for (;;) {
		for (p=table; p<= &table[tbound]; p++) {
			if (equal(device,p->devname))
				return(p);
		}
		for (r=alias; r<= &alias[abound]; r++) {
			if (equal(device,r->new)) {
				device = r->old;
				break;
			}
		}
		if (r > &alias[abound])
			return(0);
	}
}

/*
 * This routine is used to search the parameter table
 * for the keyword that was specified in the configuration.  If the
 * keyword cannot be found in this table, a value of zero is returned.
 * If the keyword is found, a pointer to that entry is returned.
 */
struct t4 *
lookup(keyword)
char *keyword;
{
	register struct t4 *kwdptr;
	for (kwdptr=parms; kwdptr<= &parms[pbound]; kwdptr++) {
		if (equal(keyword,kwdptr->indef))
			return (kwdptr);
	}
	return(0);
}

/*
 * This routine is used to map lower case alphabetics into upper case.
 */
char *
uppermap(device,caps)
char *device;
char *caps;
{
	register char *ptr;
	register char *ptr2;
	ptr2 = caps;
	for (ptr=device; *ptr!=NULL; ptr++) {
		if ('a' <= *ptr && *ptr <= 'z')
			*ptr2++ = *ptr + 'A' - 'a';
		else
			*ptr2++ = *ptr;
	}
	*ptr2 = NULL;
	return (caps);
}
/*
 * This routine enters the device in the configuration table.
 */
enterdev(vec,add,bl,ml,name)
int	vec, bl, ml;
unsigned long add;
char	*name;
{
	tbound++;
	if (tbound == TSIZE) {
		printf("Configuration table overflow\n");
		exit(1);
	}
/*
 * Write upper case define statement and sequence number for device.
 */
	fprintf(confhp,"#define\t%s_%d %d\n",
		uppermap (name,capitals),
		q->dcount, ml);
	if (q->vsize)
		setlow(vec,q->vsize);
	setq();
	p->devname = q->dev;
	p->vector = vec;
	p->address = add;
	p->buslevel = bl;
	p->addrsize = q->asize;
	p->vectsize = q->vsize;
	p->type = q->type2;
	p->handler = q->hndlr;
	p->count = q->dcount;
	p->mlt = ml;
	p->blk = q->block;
	p->chr = q->charr;
	p++;
	q->dcount++;
}

/*
 * This routine enters the appropriate pointer for the device
 * into low core.
 */
setlow(vec,vsz)
int vec;
int vsz;
{
	for( subv = vec/4; vsz > 0; vsz -= VSIZE, subv++ )
		lowptr[subv] = p;
}

addrcheck(add,addsize)
register unsigned long add;
short addsize;
{
	register struct map *mp;
	register struct map *hold;
	register unsigned long size = (unsigned long) addsize;

	for (mp= &map[mbound]; mp >= map; mp--) if (add >= mp->start) {
		if ((add+size) > (mp->start+mp->length)) {
			error("Address + size too large");
			return(1);
		}
		if ((add == mp->start) &&
			((size+add)==(mp->start+mp->length))) {
			mbound--;
			while (mp <= &map[mbound]) {
				mp->start = (mp+1)->start;
				mp->length = (mp+1)->length;
				mp++;
			}
			return(0);
		}
		if (add == mp->start) {
			mp->start += size;
			mp->length -= size;
			return(0);
		}
		if ((size + add) == (mp->start + mp->length)) {
			mp->length -= size;
			return(0);
		}
		hold = mp;
		if ((mbound + 1) == MSIZE) {
			printf("Address map table overflow\n");
			exit(1);
		}
		for (mp= &map[mbound]; mp >= hold; mp--) {
			(mp+1)->start = mp->start;
			(mp+1)->length = mp->length;
		}
		mp++;
		mp->length = add - mp->start;
		(mp+1)->start = size + add;
		(mp+1)->length = (mp+1)->length-(size+add)+mp->start;
		mbound++;
		return(0);
	}
	error("Address collision");
	return(1);
}

/*
 * System device specification
 */
short
dosys(sysdev,fd)
register struct t9 *sysdev;
register FILE *fd;
{
	register struct t *pt;
	register char *p;
	register char *tmpp;
	register l, n;
	unsigned int number;
	char bf[9];
	long sl;
	short ns;

	if (sysdev->s_swap) {
		l = sscanf(line,"%*8s%8s%o%ld%hd", bf, &number, &swplo, &nswap);
		if (l != 4) {
			error("Incorrect line format");
			return(-1);
		}
		if (nswap < 1) {
			error("Invalid nswap");
			return(-1);
		}
		if (swplo <= 0) {
			error("Invalid swplo");
			return(-1);
		}
		swp_lo[0] = swplo;
		swp_nbr[0] = nswap;
	} else {
		l = sscanf(line,"%*8s%8s%o",bf,&number);
		if (l != 2) {
			error("Incorrect line format");
			return(-1);
		}
	}
	if ((pt=locate(bf)) == 0) {
		error("No such device");
		return(-1);
	}
	if ((pt->type & BLOCK) == 0) {
		error("Not a block device");
		return(-1);
	}
	if (sysdev->s_count) {
		error("Invalid re-specification");
		return(-1);
	}
	sysdev->s_device = pt;
	sysdev->s_minor[0] = number;
	sysdev->s_major[0] = pt->blk;
	/*
	 * Now scan for a comma, indicating multiple system devices
	 */
	for (n = 1, p = line ; p = strchr(p,',') ; n++) {
		p++;
		for(tmpp = p; *tmpp; tmpp++){
			if(*tmpp== ' ' || *tmpp == '\t')
				continue;
			else
				if(*tmpp == '\n'){
					fgets(line,MAXLINE,fd);
					p = line;
					break;
				}
				else
					break;
		}
		
		if (n == SYSSIZE) {
			error("System device table exceeded");
			exit(1);
		}
		if (sysdev->s_swap) {
			l = sscanf(p,"%8s%o%ld%hd",bf, &number, &sl, &ns);
			if (l != 4) {
				error("Incorrect line format");
				return(-1);
			}
			if (ns < 1) {
				error("Invalid nswap");
				return(-1);
			}
			if (sl <= 0) {
				error("Invalid swplo");
				return(-1);
			}
			swp_lo[n] = sl;
			swp_nbr[n] = ns;
		} else {
			l = sscanf(p,"%8s%o",bf, &number);
			if (l != 2) {
				error("Incorrect line format");
				return(-1);
			}
		}
		if ((pt=locate(bf)) == 0) {
			error("No such device");
			return(-1);
		}
		if ((pt->type & BLOCK) == 0) {
			error("Not a block device");
			return(-1);
		}
		sysdev->s_minor[n] = number;
		sysdev->s_major[n] = pt->blk;
	}
	sysdev->s_count = n;
	return(pt->blk);
}

/*
 * MPU specification
 */
dompu()
{
	register l;

	l = sscanf(line, "%*8s%d", &mpu);
	if (l<1 || !(mpu==68000 || mpu==68010 || mpu==68020)) {
		error("Invalid mpu");
		return(1);
	}
	if (indfile) {
		sprintf(line, "mpu\t%d\n", mpu);
		savedef(sysdef3);
	}
	if( mpu == 68020 ) {
		map[0].start |= MAP020;
	}
	return(0);
}
/*
 * cache specification
 */
docache()
{
	register l;

	l = sscanf(line, "%*8s%o", &cache);
	if (l<1 || (cache&(~(LOGICAL|PHYSICAL|SUPERVISOR|USER|DATA|INSTR)))) {
		error("Invalid cache entry");
		return(1);
	}
	if (indfile) {
		sprintf(line, "cache\t%o\n", cache);
		savedef(sysdef3);
	}
	return(0);

}
/*
 * Force specification
 */
doforce()
{
	char dummy[CSIZE+1], fbuf[CSIZE+1];
	register l;

	l = sscanf(line, STR2, dummy, fbuf);
	if (l<2 || (l=strlen(fbuf)) == 0 || l > FLEN) {
		error("Invalid \"force\" identifier");
		return(1);
	}
	if (frcptr + l + 1 >= &force[sizeof force]) {
		error("Force table exceeded");
		return(1);
	}
	strcpy(frcptr, fbuf);
	frcptr += l + 1;
	if (indfile) {
		sprintf(line, "force\t%s\n", fbuf);
		savedef(sysdef3);
	}
	return(0);
}

/*
 * Probe specification
 */
doprobe()
{
	unsigned long addr;
	short byte;
	register l;

	l = sscanf(line, "%*s%x%hx", &addr, &byte);
	if (l == 1 )
		l = sscanf(line, "%*s%x%hd", &addr, &byte);
	if (l < 2 || byte < -1 || byte > 255) {
		error("Invalid probe input");
		return(1);
	}
	if (prbbound == PRBSIZE) {
		error("Probe table exceeded");
		return(1);
	}
	prbbound++;
	prbptr->p_addr = addr;
	prbptr->p_byte = (unsigned short) byte;
	prbptr++;
	return(0);
}

/*
 * Alien specification
 */
doalien()
{
	unsigned short vect;
	unsigned long addr;
	char	hndlr[9];
	
	register l;

	l = sscanf(line, "%*s%hx%x", &vect, &addr);
	if(addr == 0){
		l = sscanf(line, "%*s%hx%x%8s", &vect, &addr,hndlr);
	}
	if (l < 2 || vect < 8 || vect > 255*4 || (vect&3) || (addr&1)) {
		printf("\nl=%d,vect=%x,addr=%x",l,vect,addr);
		error("Invalid alien input");
		return(1);
	}
	if (albound == ALSIZE) {
		error("Alien table exceeded");
		return(1);
	}
	albound++;
	alptr->a_vect = vect;
	alptr->a_addr = addr;
	strcpy(alptr->a_hndlr, hndlr);
	
	alptr++;
	if (indfile) {
		if(addr)
			sprintf(line, "alien\t%x\t%x\n", vect, addr);
		else
			sprintf(line, "alien\t%x\t%x%s\n", vect, addr,hndlr);
		savedef(sysdef3);
	}
	return(0);
}

/*
 * Dup specification
 */
dodup()
{
	unsigned short flag;
	unsigned short vect;
	char hndlr[9];
	unsigned long arg;
	register i, l;

	l = sscanf(line, "%*s%ho%hx%8s%x", &flag, &vect, hndlr, &arg);
	if (l < 3
	   || (flag & ~D_VALID)
	   || vect < 8
	   || vect > 255*4
	   || (vect&3)
	   || (l==3 &&  (flag&D_ARG))
	   || (l==4 && !(flag&D_ARG))
	   || (i=(flag&D_BEQ)) && i!=D_INTRET && i!=D_ALLTRAPS && i!=D_RETURN
	   ) {
		error("Invalid dup input");
		return(1);
	}
	if (dupbound == DUPSIZE) {
		error("Dup table exceeded");
		return(1);
	}
	dupbound++;
	dupptr->d_flag = flag;
	dupptr->d_vect = vect;
	strcpy(dupptr->d_hndlr,hndlr);
	dupptr->d_arg = arg;
	dupptr++;
	if (indfile) {
		if (l==3)
			sprintf(line, "dup\t%o\t%x\t%s\n", flag, vect, hndlr);
		else
			sprintf(line, "dup\t%o\t%x\t%s\t%x\n",
						flag, vect, hndlr, arg);
		savedef(sysdef3);
	}
	return(0);
}

/*
 * Ram specification
 */
doram()
{
	register l;

	rbound++;
	if (rbound == RSIZE) {
		printf("Ram table overflow\n");
		exit(1);
	}
	l = sscanf(line, "%*8s%ho%x%x%x", &ramptr->r_flag,
		&ramptr->r_low, &ramptr->r_high, &ramptr->r_incr);
	if (l < 3
	||  ramptr->r_low >= ramptr->r_high
	||  ramptr->r_low != ctob(btoct(ramptr->r_low))
	||  ramptr->r_high != ctob(btoct(ramptr->r_high))
	||  ramptr->r_incr != ctob(btoct(ramptr->r_incr))
	||  (l= ramptr->r_high - ramptr->r_low) < ramptr->r_incr
	||  ramptr->r_flag == 0
	||  (ramptr->r_flag & ~R_IN_VALID)
	||  ( (ramptr->r_flag&R_IN_Single)
	    && ramptr->r_incr != 0
	    && ramptr->r_incr != l
	    )
	||  (  (ramptr->r_flag&R_IN_Mult)
	    && (  ramptr->r_flag&R_IN_Single
	       || ramptr->r_incr == 0
	       || (l % ramptr->r_incr) != 0
	       )
	    )
	   ) {
		error("Invalid ram data");
		return(1);
	}
	ramptr++;
	return(0);
}

/*
 * save dfile data line for sysdef(1M)
 */
savedef(buf)
char *buf;
{
	if (strlen(line) + strlen(buf) >= SDSIZE - 1) {
		fprintf(stderr, "sysdef buffer overflow\n");
		exit(1);
	}
	strcat(buf,line);
}

/*
 * Print the saved dfile entries for sysdef(1M)
 */
prdefs(buf, name)
register char *buf;
char *name;
{
	register i, c;

	if (strlen(buf) == 0)
		return;

	fprintf(fdvec, "\n%s:\tglobal\t%s\n", name, name);

	i = 0;
	for ( ; ; ) {
		if (i == 0)
			fprintf(fdvec, "\tbyte\t");
		c = *buf++;
		if (isprint(c))
			fprintf(fdvec, "'%c", c);
		else if (!isascii(c)) {
			error("error: non-ascii characters");
			return;
		} else switch (c) {
			case 0:
				fprintf(fdvec, "0\n");
				return;
			case 1: case 2: case 3: case 4: case 5: case 6: case 7:
				fprintf(fdvec, "%o", c);
				break;
			case '\b':	/* 0x08 */
				fprintf(fdvec, "'\\b");
				break;
			case '\t':	/* 0x09 */
				fprintf(fdvec, "'\\t");
				break;
			case '\n':	/* 0x0a */
				fprintf(fdvec, "'\\n");
				break;
			case '\v':	/* 0x0b */
				fprintf(fdvec, "'\\v");
				break;
			case '\f':	/* 0x0c */
				fprintf(fdvec, "'\\f");
				break;
			case '\r':	/* 0x0d */
				fprintf(fdvec, "'\\r");
				break;
			case '\\':	/* 0x5c */
				fprintf(fdvec, "'\\\\");
				break;
			default:
				fprintf(fdvec, "0x%x", c);
		}
		if (i==7) {
			i = 0;
			fprintf(fdvec, "\n");
			continue;
		}
		i++;
		fprintf(fdvec, ",");
	}
}
