/*
 *	ipl.c: MVME350 IPL main body
 *
 *		Copyright (c) 1986 Motorola Inc.
 *		All Rights Reserved
 *
 *		%W%
 */

#include <sys/types.h>
#include <aouthdr.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <memory.h>
#include "../com/common.h"

#ifdef MVME120
#include <sys/mvme350.h>
#endif

/*
 *	define target MPU board
 */

#ifndef	MVME120
#ifndef	MVME130
#define	MVME130
#endif
#endif


/*
 *	set DEBUG defines
 */


#define	DEBUGG		/* to debug without 130bug */
#undef	DEBUGG		/* not now ! */

/* #define	EXPER		experimental version */

/*
 *	set NULL
 */

#ifndef	NULL
#define	NULL	0
#endif

/*
 *	BSIZE == 1 physical block size on tape
 */

#define	BSIZE	512

/*
 *	RAMADDR == address of beginning of ramdisk
 */

#ifndef	RAMADDR
#ifdef DEBUGR
#define	RAMADDR	(0xc0000)		/* RAMDISK load address	*/
#else
/*
   if this  changes be sure to change cf/dfile (maxpmem) and sys/io/ramdio.h
   (ramd_sizes) in the the special bootable tape kernel
*/
#define	RAMADDR	(0xF3800)		/* RAMDISK load address	*/
#endif
#endif


char	*iplname = {"MVME350 IPL"};
#ifdef	EXPER
char	*iplrel = {"EXPERIMENTAL"};
#else
char	*iplrel = {"1.0"};
#endif

/*
 *	BUG I/O support
 */

#ifdef	MVME120
struct	packet {
	unsigned long	p_blkcnt;	/* block count to transfer	*/
	unsigned long	p_addr;		/* memory address		*/
	unsigned long	p_fileno;	/* file number			*/
	unsigned long	p_status;	/* status long			*/
	struct m350pkt	p_350pkt;	/* 350packet			*/
} packet;

#define F_FMK	0x1			/* filemark 			*/

#endif

#ifdef	MVME130
struct	packet {
	unsigned char	p_clun;		/* cntlr logical unit number	*/
	unsigned char	p_dlun;		/* device logical unit number	*/
	unsigned short	p_status;	/* status word			*/
	unsigned long	p_addr;		/* memory address		*/
	unsigned long	p_fileno;	/* file number			*/
	unsigned short	p_blkcnt;	/* block count to transfer	*/
	unsigned char	p_flags;	/* I/O flags			*/
	unsigned char	p_adrm;		/* address modifier		*/
} packet;

#define	F_EOF	0x01			/* EOF Flag			*/
#define	F_IFN	0x02			/* Ignore File Number Flag	*/
#define F_FMK	0x80			/* File mark seen		*/

#endif

/*
 *	tape directory buffer
 */
#define TDIR	2
#define	DIRSIZ	14
#define	DIRENT	(BSIZE/sizeof(struct t_dir))
struct	t_dir	dbuf[DIRENT];

#define		GTFILE(a)	(a)
#define		GTTYPE(a)	(a)

/*
 *	iregs contains a copy of the original registers
 */

extern	unsigned iregs[];
#define	D0	0
#define	D1	1
#define	D2	2
#define	D3	3
#define	D4	4
#define	D5	5
#define	D6	6
#define	D7	7
#define	A0	8
#define	A1	9
#define	A2	10
#define	A3	11
#define	A4	12
#define	A5	13
#define	A6	14
#define	A7	15

int	entry;

/*
 *	operating system header information
 */

union {
	struct oshdr {
		struct filehdr os_fhdr;
		struct aouthdr os_ahdr;
		struct scnhdr  os_1scn;
	} oshdrblk;
	unsigned char coff_buff[BSIZE];
} oshdrun;

char	namebuf[DIRSIZ+10];

int	osfno = 0;		/* operating system file number */
int	ramfno = 0;		/* ramdisk file number */


main()
{
	printf("%s Version %s\n", iplname, iplrel );

#ifdef	MVME130
	initpkt();		/* initialize packet */
#endif

	getname();

	if(( osfno = rddir()) == 0 ) {	/* go read tape directory */
		printf("%s: ", iplname );
		if( namebuf[0] == '\0' )
			printf("no bootable operating systems found.\n");
		else
			printf("can't find `%s'.\n", namebuf );
		return;
	}

	loados();	/* load operating system */

	if( ramfno == 0 )
		printf("%s: no ramdisk found\n", iplname );
	else
		loadram();	/* load ramdisk */

	jumpos(entry);
	/* should never reach here */
}

#ifdef	MVME130

/*
 *	initpkt: initialize MVME130 packet
 */

initpkt()
{
#ifdef	DEBUGG
	packet.p_clun = 4;		/* 130bug controller # of MVME350 */
	packet.p_dlun = 0;
#else
	packet.p_clun = iregs[D1];
	packet.p_dlun = iregs[D0];
#endif

	packet.p_flags = F_EOF;

#ifdef	DEBUG
	printf("%s: controller lun == %x\n", iplname, packet.p_clun );
	printf("%s: device lun == %x\n", iplname, packet.p_dlun );
	printf("%s: block count == %x\n", iplname, packet.p_blkcnt );
#endif
}

#endif

getname()
{	register char *np = namebuf;
#ifdef	DEBUGG
	register char *sp = NULL;
	register char *ep = NULL;
#else
	register char *sp = (char *)iregs[A5];
	register char *ep = (char *)iregs[A6];
#endif

	while( sp < ep )
		*np++ = *sp++;
	*np ='\0';
}

/*
 *	rddir: read tape directory, looking for passed in name
 */

rddir()
{	register struct t_dir *dp = dbuf;
	register struct packet *pp = &packet;
	register fno = 0;
	register n;

	pp->p_fileno = TDIR;
	pp->p_blkcnt = 1;
	pp->p_addr = (unsigned long)dp;

	while( rdblk(&packet) >= 0 ) {
#ifdef DEBUG
	printf("blkcnt = %x\n",pp->p_blkcnt);
	printf("p_flags = %x\n",(unsigned char)pp->p_flags);
#endif

		for( n=0, dp=dbuf; n < DIRENT; n++, dp++ ) {
			if( GTFILE(dp->number) == 0 )
				continue;
#ifdef	DEBUG
			printf("%d ", GTFILE(dp->number));
			printf(" [%d] ", GTTYPE(dp->type));
#endif
			if( GTTYPE(dp->type) == D_RAM) {
#ifdef	DEBUG
				printf("[RAM] %s\n", dp->name );
#endif
				ramfno = GTFILE(dp->number);

				if( fno )
					break;	/* done */
				continue;
			}
			if( GTTYPE(dp->type) == D_ARC ) {
#ifdef	DEBUG
				printf("[ARC] %s\n", dp->name );
#endif
				continue;
			}
#ifdef	DEBUG
			printf("[OS] %s\n", dp->name);
#endif
			if( fno == 0 ) {
				/*
				 *	if no string given on boot line
				 */

				if( namebuf[0] == '\0' )
					fno = GTFILE(dp->number);

				/*
				 *	now look for string
				 */

				else if(strncmp(dp->name,namebuf,DIRSIZ) == 0)
					/* found it! */
					fno = GTFILE(dp->number);

				if( ramfno )
					break;	/* done */
			}
		}
#ifdef MVME130
		pp->p_flags |= F_IFN;	/* Ignore File Number Flag */
#endif

#ifdef MVME130
		if( pp->p_flags & F_FMK) {	/* filemark seen */
			pp->p_flags |= ~F_FMK;
			break;
		}
#else
		if( pp->p_status & F_FMK) {	/* filemark seen */
			pp->p_status |= ~F_FMK;
			break;
		}
#endif
	}
	return(fno);
}

/*
 *	loadram: read ramdisk into memory
 */

loadram()
{	register struct packet *pp = &packet;
	register count;

#ifdef DEBUG
	printf("loading ramdisk %d at %x\n",ramfno,RAMADDR);
#endif
	pp->p_addr = RAMADDR;
	pp->p_fileno = ramfno;
	pp->p_blkcnt = 0xfffe;	/* block count to transfer	*/
#ifdef MVME130
	pp->p_flags &= ~F_IFN;	/* Ignore File Number Flag off */
#endif
	while(( count = rdblk(pp)) >= 0 ) {
#ifdef MVME130
#ifdef DEBUG
	printf("rdblk = %x, blkcnt = %x\n",count,pp->p_blkcnt);
#endif
		count = pp->p_blkcnt;
		pp->p_blkcnt = 0xfffe;
		pp->p_flags |= F_IFN;	/* Ignore File Number Flag */
		if( pp->p_flags & F_FMK) {	/* filmark seen */
			pp->p_flags |= ~F_FMK;
			break;
		}
#else
		count = pp->p_350pkt.p_termcnt;
		if( pp->p_status & F_FMK) {	/* filemark seen */
			pp->p_status |= ~F_FMK;
			break;
		}
#endif
#ifdef MVME130
		pp->p_addr += count*BSIZE;
#else
		pp->p_addr += count;
#endif
	}
#ifdef DEBUG
	printf("count (%d) %x\n",count,count);
#endif
	if ( count < 0 ) { 
		printf("error in ramdisk read\n");
		gobug();	/* return to bug */
	}
}

/*
 *	loados: load operating system
 */

loados()
{	register struct packet *pp = &packet;
	register struct oshdr *op = &oshdrun.oshdrblk;
	register struct aouthdr *ap = (struct aouthdr *)&op->os_ahdr;
	register struct filehdr *fp = (struct filehdr *)&op->os_fhdr;
	register struct scnhdr  *sp = (struct scnhdr *)&op->os_1scn;
	register count;
	register text;
	register data;
	register i;

#ifdef DEBUG
	register status;

	printf("loading os %d \n",osfno);
#endif
	pp->p_addr = (unsigned long)op;
#ifdef MVME130
	pp->p_flags &= ~F_IFN;	/* Ignore File Number Flag off */
#endif
	pp->p_fileno = osfno;
	pp->p_blkcnt = 1;	/* block count to transfer	*/

	if( rdblk(pp) < 0) {
		printf("%s: error in reading operating system\n", iplname );
		gobug();	/* return to bug */
	}
	if( fp->f_magic != MC68MAGIC) {	
		printf("%s: operating system bad magic %s\n",iplname,namebuf);
		gobug();	/* return to bug */
	}
#ifdef DEBUG
	printf("text_start %x \n",ap->text_start);
	printf("tsize  %x \n",ap->tsize);
	printf("data_start %x \n",ap->data_start);
	printf("dsize  %x \n",ap->dsize);
	printf("entry %x \n",ap->entry);
#endif
	/*
	 * loop through section headers looking for .text
	 * section for the section offset (in file).
	 */
	for ( i=fp->f_nscns; i; sp++, i-- ) {
		if ( sp->s_flags & STYP_TEXT ) {
			break;
		}
	}
	if ( i == 0 ) {
		printf("%s: no text section found in operating system\n",
		    iplname);
		gobug();	/* return to bug */
	}
	/*
	 *	move first part of vectors already read
	 *	this will start at "scnptr" from the beginning of
	 *	the buffer, pp->p_addr
	 */
#ifdef DEBUG
	printf("text start = 0x%x, ", ap->text_start);
	printf("beg in buf = 0x%x, ", (pp->p_addr+sp->s_scnptr));
	printf("byte count = 0x%x\n", (BSIZE - sp->s_scnptr));
#endif
	memcpy(ap->text_start,(pp->p_addr+sp->s_scnptr),(BSIZE - sp->s_scnptr));

	/*
	 *	move rest of text from tape
	 */

#ifdef MVME130
	pp->p_flags |= F_IFN;	/* Ignore File Number Flag */
#endif
	pp->p_addr = (unsigned long) (ap->text_start + (BSIZE-sp->s_scnptr));
	pp->p_blkcnt = 0xfffe;	/* block count to transfer	*/
	while(( count = rdblk(pp)) >= 0 ) {
#ifdef MVME130
		count = pp->p_blkcnt;
#ifdef DEBUG
	printf("rdblk = %x, blkcnt = %x\n",count,pp->p_blkcnt);
#endif
		pp->p_blkcnt = 0xfffe;
		if( pp->p_flags & F_FMK) {	/* filmark seen */
			pp->p_flags |= ~F_FMK;
			break;
		}
#else
		count = pp->p_350pkt.p_termcnt;
		if( pp->p_status & F_FMK) {	/* filemark seen */
			pp->p_status |= ~F_FMK;
			break;
		}
#endif
#ifdef MVME130
		pp->p_addr += count*BSIZE;
#else
		pp->p_addr += count;
#endif
	}
	if ( count < 0 ) { 
#ifdef DEBUG
		printf("%s: error in text section read\n", iplname );
#else
		printf("%s: error in reading operating system\n", iplname );
#endif
		gobug();	/* return to bug */
	}

	/*
	 *	move data in reverse order
	 *		if text_start + tsize != data_start
	 */

	if((text = ap->text_start + ap->tsize ) != ap->data_start) {
		text += ap->dsize - 1;
		data = ap->data_start + ap->dsize - 1;
		for( i = ap->dsize; i ; i--)
			*(char *) data-- = *(char *) text--;
	}
	/* save entry point */
	entry = ap->entry;
}
