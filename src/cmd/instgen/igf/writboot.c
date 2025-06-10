/*
 *	wboot- strips and writes bootloader 
 */
#include	<stdio.h>
#include	<filehdr.h>
#include	<scnhdr.h>
#include	<ldfcn.h>
#include	"igf.h"
#include	"../com/common.h"

#ident	"$Header: /grp/mat/53/utils/igf/RCS/writboot.c,v 1.2 87/01/27 16:17:51 mat Exp $";

wr_boot( bootfile )
char	*bootfile;
{
	LDFILE *ldptr;
	SCNHDR text, data;
	register int l, s;
	int	fd;

	VERBOSE("Writing bootloader.");

	ldptr = NULL;
	if(( ldptr = ldopen(bootfile, ldptr)) == NULL ) {
		fprintf(stderr, "%s: can't open bootloader file %s\n",
			myname, bootfile);
		return(ERROR);
	}

	if( ldnshread(ldptr, ".text", &text) == FAILURE ) {
		fprintf(stderr,"%s: can't find text header on %s\n",
			myname, bootfile );
		ldclose(ldptr);
		return(ERROR);
	}

	if( ldnshread(ldptr, ".data", &data) == FAILURE ) {
		fprintf(stderr,"%s: can't find data header on %s\n",
			myname, bootfile );
		ldclose(ldptr);
		return(ERROR);
	}

	/*
	 * seek to text section in a.out file
	 */
	ldnsseek( ldptr, ".text");

	/*
	 *	open tape for append
	 */
	DEBPRT1("wboot: opening %s\n", Device );

	if ( (fd=opensp(M_APPEND)) == ERROR ) {
		return(ERROR);
	}

	/*
	 *	now copy bootloader text to tape
	 */
	DEBPRT1("wboot: writing %s\n", Device );
	DEBPRT1("wboot: text.s_size: %d\n", text.s_size);

	for( s = text.s_size; s > 0; s -= l ) {
		l = (s > T_BSIZE) ? T_BSIZE : s;
		if( FREAD(Io_buf, l, 1, ldptr) != 1 ) {
			fprintf( stderr, "%s: bootloader read error\n", myname);
			ldclose(ldptr);
			return(ERROR);
		}
		if( write( fd, Io_buf, l) != l ) {
			fprintf( stderr, "%s: bootloader copy error\n", myname);
			ldclose(ldptr);
			return(ERROR);
		}
	}

	/*
	 *	now copy bootloader data to tape
	 */
	DEBPRT1("wboot: data.s_size: %d\n", data.s_size);

	if( ldnsseek( ldptr, ".data" ) == SUCCESS ) {
		for( s = data.s_size; s > 0; s -= l) {
			l = (s > T_BSIZE) ? T_BSIZE : s;
			if( FREAD(Io_buf, l, 1, ldptr) != 1 ) {
				fprintf( stderr, "%s: bootloader read error\n",
				    myname );
				ldclose(ldptr);
				return(ERROR);
			}
			if( write( fd, Io_buf, l) != l ) {
				fprintf( stderr, "%s: bootloader copy error\n",
				    myname );
				ldclose(ldptr);
				return(ERROR);
			}
		}
	}
	else return(ERROR);

	/* close the bootloader file */
	ldclose(ldptr);

	/* now close the tape */

	DEBPRT1("wboot: closing %s\n", Device );

	closesp();
	return(0);
}
