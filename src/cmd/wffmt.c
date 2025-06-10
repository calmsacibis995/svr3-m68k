/*  	@(#)wffmt.c	6.2		*/
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	WDSPT		32
#define WDDSKFMT	1

#define FTYPE(x)	(x&0170000)	/* file type from stat(2) 	*/
#define MINOR(x)	(x&0xff)	/* minor device number 		*/
#define CHAR		0020000		/* char (raw) type device 	*/
#define RW		6		/* read/write for access(2) 	*/
#define FLOP		020		/* floppy devices   		*/

struct fmtdes
{
	unsigned char	fd_use;		/* sector use flag: good, bad, alt */
	unsigned char	fd_sn;		/* sector no (00 - 1f ) */
	unsigned long	fd_alsn;	/* alternate sector number */
};

struct wd_ctl
{
	unsigned long	wd_lsn;		/* LSB logical sector number */
	unsigned char	wd_i;		/* interleave value */
	unsigned char	wd_mctl;	/* media control register */
	struct fmtdes   wd_fmt[WDSPT];	/* pointer to format descriptors */
};

struct wd_ctl wcmd = { 0, 1, 0xac };

main(argc, argv)
int  argc;
char  *argv[];
{
	int fdes;
	char *fname;
	int  drive;
	struct stat buf;
	
	if ( argc != 2 )
		{
		fprintf(stderr, "Usage: wffmt <device>\n" );
		exit(1);
		}
	if( stat(fname = argv[1],&buf) == -1 || access(fname,RW) == -1)
		{
		fprintf(stderr,"Cannot stat/access device %s \n",fname);
		exit(1);
		}
	if( FTYPE( buf.st_mode) != CHAR )
		{
		fprintf(stderr, "Must use a raw/character device \n" );
		exit(1);
		}
	if ( (drive = MINOR(buf.st_rdev)) & FLOP )
		{
	    	fprintf(stdout,"Formatting floppy in %s. \n", fname);
	    	if( (fdes = open( fname, O_RDWR) ) <0 )
			{
			fprintf(stderr," Cannot open %s \n", fname);
			exit(-1);
			}
	    	if (ioctl( fdes, WDDSKFMT, &wcmd ))
			{
			fprintf(stderr,"Could not format device: %s\n", fname);
			exit(-1);
			}
 		}
	else    {
		fprintf (stderr, "%s: Not a floppy drive\n", fname);
		exit(-1);
		}

	exit(0);
}
