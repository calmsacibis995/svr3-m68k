/*
 *	wr_ram - copies image (only filesystem size) 
 */

#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/filsys.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/writram.c,v 1.2 87/01/27 16:17:56 mat Exp $";

wr_ram()
{
	register int	rfd;		/* ramdisk file descriptor */
	register int	sfd;		/* special file fd */
	register int	n;
	register char *sname = Ram;
	struct filsys fbuf;

	if( Ram == NULL)	/* no ramdisk specified */
		return;

	VERBOSE("Loading ramdisk image.");

	/*
	 *	open ramdisk filesystem
	 */
	if(( rfd = open( sname, O_RDONLY )) == -1 ) {
		fprintf( stderr, "%s: can't open %s\n", myname, sname );
		return(ERROR);
	}

	/*
	 *	seek to super block
	 */
	if( lseek( rfd, BSIZE, 0 ) == -1 ) {
		fprintf( stderr, "%s: seek error on %s\n", myname, sname );
		close(rfd);
		return(ERROR);
	}

	/*
	 *	read super block
	 */
#ifdef	DEBUG
	if( sizeof(fbuf) != BSIZE )
		printf("superblock size: %d\n", sizeof(fbuf) );
#endif
	if( read( rfd, &fbuf, sizeof(fbuf)) != sizeof(fbuf) ) {
		fprintf( stderr, "%s: read error on %s\n", myname, sname );
		close(rfd);
		return(ERROR);
	}

	/*
	 *	check super block
	 */
	if( fbuf.s_magic != FsMAGIC ) {
		fprintf( stderr, "%s: ramdisk filesystem %s corrupted.\n",
		    myname, sname );
		close(rfd);
		return(ERROR);
	}

	/*
	 *	open tape for append
	 */
	DEBPRT1("wr_ram: opening %s\n", Device );
	if ( (sfd=opensp(M_APPEND)) == ERROR) {
		close(rfd);
		return(ERROR);
	}

	/*
	 *	copy filesystem to tape
	 */
	if( lseek( rfd, 0, 0 ) == -1 ) {
		fprintf( stderr, "%s: seek error on %s\n", myname, sname );
		close(rfd);
		closesp();
		return(ERROR);
	}

	DEBPRT1("wr_ram: copying %d blocks\n", (int)fbuf.s_fsize*fbuf.s_type );
	DEBPRT1("wr_ram: filesystem type: %d byte\n",fbuf.s_type == 1?512:1024);

	/*
	 * This will not work if a mkfs(1M) is done into a file within a
	 * filesystem with or without a proto file because the file made
	 * will be quite a bit smaller than the requested number of blocks.
	 */
	for( n = (int)fbuf.s_fsize*fbuf.s_type; n > 0; n-- ) {
		if( read( rfd, Io_buf, BSIZE ) != BSIZE ) {
			fprintf( stderr,"%s: read error on %s\n", myname,sname);
			close(rfd);
			closesp();
			return(ERROR);
		}
		if( write( sfd, Io_buf, BSIZE ) != BSIZE ) {
			fprintf( stderr, "%s: ramdisk copy error\n", myname );
			close(rfd);
			closesp();
			return(ERROR);
		}
	}

	/*
	 *	close tape and ramdisk
	 */
	closesp();
	close(rfd);
	return(0);
}
