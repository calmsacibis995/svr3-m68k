#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/mvme350.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/io.c,v 1.2 87/01/27 16:17:40 mat Exp $";

#define	TRUNC	(O_WRONLY|O_TRUNC)
#define	APPEND	(O_WRONLY|O_APPEND)

static int	sfd = 0;	/* device or special file , file desc */

/*
 *	open special file
 */
opensp(mode)
int	mode;
{
#ifdef	DEBUG
	if( sfd != 0 ) {
		printf("Warning: special file not closed\n");
		close(sfd);
	}
#endif

	if(( sfd = open(Device,(mode==M_TRUNC ? TRUNC : APPEND)))==ERROR) {
		fprintf( stderr, "%s: can't open %s\n", myname, Device );
		return(ERROR);
	}
	return(sfd);
}

/*
 *	close special file
 */
closesp()
{
	close(sfd);
	sfd = 0;
}

/*
 *  rewind streamer tape
 */
rewind_t()
{
	register int	fd;		/* fd for special device */
	register int	errflag = 0;

	if ( (fd=open(Device, (O_NDELAY|O_RDONLY))) == ERROR ||
	     ioctl(fd, M350REWIND, 0) == ERROR ) {
		errflag = ERROR;
	}
	(void)close(fd);
	return(errflag);
}
