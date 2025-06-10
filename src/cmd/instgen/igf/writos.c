#include	<stdio.h>
#include	<fcntl.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/writos.c,v 1.2 87/01/27 16:17:54 mat Exp $";

wr_os()
{
	register struct os_files *osp;
	register int	cnt = 0,
			sfd,		/* file des for special file */
			ofd;		/* operating system file des */

	if( Verbose )
		printf("Copying intermediate operating system(s).\n");
	
	for ( osp=Os_head; osp; osp=osp->next ) {
		/* open a.out file */
		if ( (ofd=open(osp->os_path, O_RDONLY)) == ERROR ) {
			fprintf(stderr,
			    "%s: cannot open operating system \"%s\"\n",
			    myname, osp->os_path);
			return(ERROR);
		}
		/* open special file */
		if ( (sfd=opensp(M_APPEND)) == ERROR ) {
			close(ofd);
			return(ERROR);
		}
		DEBPRT2("wr_os: reading %s --> writing %s\n",
		    osp->os_path,Device);
		while ( (cnt=read(ofd, Io_buf, T_BSIZE)) > 0 ) {
			if ( cnt == ERROR ) {
				fprintf(stderr,
				     "%s: read error on operating system %s\n",
				     myname, osp->os_path);
				close(ofd);
				closesp();
				return(ERROR);
			}
			if ( write(sfd, Io_buf, cnt) != cnt ) {
				fprintf(stderr,
				     "%s: write error of operating system %s on %s\n",
				     myname, osp->os_path, Device);
				close(ofd);
				closesp();
				return(ERROR);
			}
		}
		close(ofd);
		closesp();
	}
	return(0);

}
