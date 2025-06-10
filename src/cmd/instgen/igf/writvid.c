#include	<stdio.h>
#include	<a.out.h>
#include	<fcntl.h>
#include	<ldfcn.h>
#include	<time.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/writvid.c,v 1.2 87/01/27 16:17:58 mat Exp $";

static struct vidblk vidblk;

extern long	time();

wr_vid()
{
	static	AOUTHDR	ahdr;
	register AOUTHDR *ap = &ahdr;
	register struct volumeid *vp = &vidblk.vid;
	register struct dkconfig *cp = &vidblk.dkc;
	register struct os_files	*osp;
	register struct ar_files	*arp;
	int	bootlen;		/* length of text and data sec's */
	int	fd;			/* file des for output device */

	VERBOSE("Creating volume ID.");

	/*
	 *	set up volume ID block
	 */
	memcpy(&vp->vid_vol,"UNIX",4);
	*vp->vid_dirtype = D_NEWF;	/* new directory type */

	if( Version != NULL ) {
		/* ASCII Version */
		strncpy(vp->vid_version,Version,VERSIZ);
	}
	if( Desc != NULL ) {
		/* ASCII Volume description */
		strncpy(vp->vid_desc,Desc,DESCSIZ);
	}
	if( Package != NULL ) {
		/* package name */
		strncpy(vp->vid_vd,Package,PACKSIZ);
	}
	memcpy(vp->vid_mac,"MOTOROLA",8);	

	/* put in creation time of media */
	LONG(vp->vid_ctime) = time((char *)0);

	/*
	 * Calculate the number of the tape directory entries, rounding
	 * up to the next T_BSIZE boundary.
	 */
	Dirents = 0;
	
	if ( Bootable ) {
		/* mark this media as bootable */
		*vp->vid_infoflg |= I_BOOT;

		/* Operating Systems */
		for ( osp=Os_head; osp; osp=osp->next ) Dirents++;

		/* Ramdisks */
		Dirents++;	/* must be one and only one ramdisk */
	}
	/* Scripts */
	if ( Scripts ) {
		Dirents++;
		*vp->vid_infoflg |= I_SCRIPT;	/* scripts entry on media */
	}

	/* Archives */
	for ( arp=Files_head; arp; arp=arp->next ) Dirents++;

	if ( Dirents & 1 ) {
		Dirents++;
	}

	INT(vp->vid_dirents) = Dirents;	/* number of directory entries*/

	if ( Bootable ) {
		/* get information about bootloader ready if bootable media */
		if( getaout(Bootloader, &ahdr) != 0 ) {
			fprintf(stderr,
			    "%s: unable to read a.out header of %s\n",
			    myname, Bootloader);
			return (ERROR);
		}
		vp->vid_oss= 1;			/* Bootloader file number */

		if ( (bootlen=chkboot(Bootloader)) == ERROR ) {
			return(ERROR);
		}

		/* Bootloader length in tape 512 byte blocks */
		vp->vid_osl = bootlen/T_BSIZE;

		DEBPRT1("vp->vid_osl: %d\n", vp->vid_osl);

		if(bootlen & 0x1ff)	/* if an odd byte length */
			vp->vid_osl++;

		if( vp->vid_osl&1 )	/* if an odd block number?	*/
			vp->vid_osl++;

		DEBPRT1("odd: vp->vid_osl: %d\n", vp->vid_osl);

		vp->vid_ose = ap->entry;/* Bootloader execution address */
		vp->vid_osa = ap->text_start;	/* Bootloader load address */
		vp->vid_cas = 1;	/* Starting block of config block */
		vp->vid_cal = 1;	/* config area length in 256 blocks */

		/*
		 * Set up config block
		 */
		/* to help change the log. block size */
		*(short *)(cp->ios_f1) = 0x01;
#ifdef VME131
		cp->ios_rec = 0x200;		/* logical block size to use */
#else
		cp->ios_rec = 0x100;		
		cp->ios_psm = 0x200;		/* logical block size to use */
#endif
	}

	/*
	 *	open special file for truncate
	 */
	DEBPRT1("wr_vid: opening %s\n", Device );

	if ( (fd=opensp(M_TRUNC)) == ERROR ) {
		return(ERROR);
	}

	/*
	 *	write volume ID block
	 */
	DEBPRT0("wr_vid: writing the volume id block\n" );

	if( write( fd, &vidblk, VIDSIZ ) != VIDSIZ ) {
		fprintf( stderr, "%s: tape write error\n", myname );
		closesp();
		return(ERROR);
	}

	/*
	 *	close special file
	 */
	DEBPRT1("wr_vid: closing %s\n", Device );

	closesp();
	return(0);
}

/*
 *	getaout: get a.out header
 */
static
getaout(f, ap)
char	*f;
AOUTHDR	*ap;
{
	register od;
	register ret = 0;

	if(( od = open( f, O_RDONLY )) == -1 ) {
		return(ERROR);
	}

	if( lseek( od, sizeof(struct filehdr), 0 ) == -1 ||
	    (ret=read( od, ap, sizeof(AOUTHDR) ) != sizeof(AOUTHDR))) {
		ret = ERROR;
	}
	else {
		DEBPRT0("wr_vid: file header info\n" );
		DEBPRT1("\tmagic=%x\n", ap->magic );
		DEBPRT1("\tvstamp=%x\n", ap->vstamp );
		DEBPRT1("\ttsize=%x\n", ap->tsize );
		DEBPRT1("\tdsize=%x\n", ap->dsize );
		DEBPRT1("\tbsize=%x\n", ap->bsize );
		DEBPRT1("\tentry=%x\n", ap->entry );
		DEBPRT1("\ttext_start=%x\n", ap->text_start );
		DEBPRT1("\tdata_start=%x\n", ap->data_start );
	}

	close(od);
	return(ret);
}

chkboot(nam)
char	*nam;
{
	LDFILE *ldptr;
	SCNHDR text, data;

	ldptr = NULL;
	if(( ldptr = ldopen(nam, ldptr)) == NULL ) {
		fprintf(stderr, "%s: can't open bootloader file %s\n",
			myname, nam);
		return(ERROR);
	}

	if( ldnshread(ldptr, ".text", &text) == FAILURE ) {
		fprintf(stderr,"%s: can't find text header on %s\n",
			myname, nam);
		ldclose(ldptr);
		return(ERROR);
	}

	if( ldnshread(ldptr, ".data", &data) == FAILURE ) {
		fprintf(stderr,"%s: can't find data header on %s\n",
			myname, nam);
		ldclose(ldptr);
		return(ERROR);
	}

	DEBPRT1("chkboot: text.s_size: %d\n", text.s_size);
	DEBPRT1("chkboot: data.s_size: %d\n", data.s_size);

	ldclose(ldptr);

	return( text.s_size + data.s_size );
}
