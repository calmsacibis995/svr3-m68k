/*	@(#)m360fmt.c	7.4	12/13/85	*/
/*
 *	Copyright (c) 1985	Motorola Inc.  
 *		All Rights Reserved.
 */

/*
 *	m360fmt - format disks on the MVME360 disk controller
 *
 * SYNOPSIS
 *	m360fmt  -T  track  special
 *	m360fmt  -M  bad_track  alt_track  special	
 *	m360fmt  -S  track  sector  special	
 *	m360fmt  -R  track  special	
 *	m360fmt  -A  read badtrack table and map tracks.
 *	m360fmt  -F  just overridding the config block.
 *	m360fmt  [ -m ]   [ -s ]   [ -v ]   special
 *		might want format disk to be with -D
 *
 * DESCRIPTION 
 *	m360fmt is used to format disks on the 
 *	MVME360 disk controller.  This causes the MVME360
 *	controller to be busy until a format winchester or a
 *	format tape request has been terminated. Therefore 
 *	it is recommended to invoke m360fmt only on a quiet
 *	system (i.e. single user mode) when formatting those
 *	devices. 
 *
 *
 *	Commands:
 *
 *	-M   <bad_track>  <alt_track>	map bad_track to alt_track.
 *	-R   <track>			Reformat a track.
 *	-S   <track> <sector>		slip at sector on track.
 *	-T   <track>			format a single track.
 *	-A				map bad_tracks listed in bad track table
 *	-F				dinit just overriding config block
 *	default				Format entire disk.
 *
 *	Options:
 * 
 *	-m   format disk with alternate tracks turned on.
 *	-s   format disk with sector slip turned on.
 *	-v   Verify device
 *
 *	Will try to format with sector slip, before using map track.
 */



#include  "stdio.h"
#include  "fcntl.h"
#include  "sys/dk.h"
#include  "sys/types.h"
#include  "sys/mvme360.h"
#include  "sys/stat.h"
#include  "sys/errno.h"

#define	TRUE	1
#define FALSE	0

#define GRACE	10
#define PHYSBLK	512
#define QK	256
#define BUFSIZE	100 * PHYSBLK
#define	BADFILE	"/etc/badlist"
#define COMMENT	'*'
#define	LINELEN	512

#define DISK	0
#define TRACK	1
#define MAP	2
#define SLIP	3
#define REFMT	4
#define ALTS	5

extern	errno;

struct	volumeid  vol;	/*  Volume id block read off the disk */
struct	dkconfig  dkcf;	/*  Config block read off the disk */
struct badent *bttable; /*  Bad track table read off the disk */
int	slip;		/*  Use sector slip if true */
int	map;		/*  Map to alternate track if true */
int	refmt;		/*  Use reformat if true */
int	verify;		/*  Verify disk if true */
int	fd;		/*  File descriptor of device to be formatted */
char	*device;
char	bdev[80];
char	*cmdname;	/*  Name of command invoked */



main ( argc, argv )
int	argc;
char	**argv;
{
  register int	c;
  register int	track;		/*  Single track to be formatted */
  register int	alt;		/*  Alternate track number */
  register int	sec;		/*  Sector to be slipped */
  extern char	*optarg;
  extern int	optind;
  int fmttype;			/*  Which type of format to execute */
  int fflag = FALSE;		/*  If set to TRUE, then do not format */

  cmdname = argv[0];

  device = argv[argc-1];
  ckdev(  );		/*  Open the device to be formatted */
  
  fmttype = DISK;	/*  Default to format entire disk */
  while ( (c=getopt(argc, argv, "AFMSTmsv")) != EOF ) {
  	switch (c) {
  
		case 'A':		/*  Format alts in bad track table */
			if ( fmttype )	{
			   fprintf( stderr,
			        "Only one -A, -T, -M, -R, or -S allowed\n" );
			   exit( -1 );
			}
			fmttype = ALTS;
			break;

		case 'F':		/*  dinit just overiding config block */
			fflag = TRUE;
			break;
		case 'M':		/*  Map a single track */
			if ( fmttype )	{
			   fprintf( stderr,
				"Only one -A, -T, -M, -R, or -S allowed\n" );
			   exit( -1 );
			}
  			track = atoi( argv[optind++] );
  			alt   = atoi( argv[optind++] );
			fmttype = MAP;
			break;

		case 'R':		/*  Reformat a single track */
			if ( fmttype )	{
			   fprintf( stderr,
				"Only one -A, -T, -M, -R, or -S allowed\n" );
			   exit( -1 );
			}
			track = atoi( argv[optind++] );
			fmttype = REFMT;
			break;

		case 'S':		/*  Format a track with slip */
			if ( fmttype )	{
			   fprintf( stderr,
				"Only one -A, -T, -M, -R, or -S allowed\n" );
			   exit( -1 );
			}
			track = atoi( argv[optind++] );
			sec   = atoi( argv[optind++] );
			fmttype = SLIP;
			break;

		case 'T':		/*  Format a single track */
			if ( fmttype )	{
			   fprintf( stderr,
				"Only one -A, -T, -M, -R, or -S allowed\n" );
			   exit( -1 );
			}
			track = atoi( argv[optind++] );
			fmttype = TRACK;
			break;

		case 'm':
			map = TRUE;
			break;

		case 's':
			slip = TRUE;
			break;

		case 'v':
			verify = TRUE;	
			break;

		case '?':
			usage ( cmdname );
			exit(2);
			break;
		}
	}

	c = argc - optind - 1;
	if  ( c  !=  0 )  {
	    fprintf( stderr, "%d Extraneous arguement%sstarting with %s\n", 
		   c, c==1?" ":"s ", argv[optind] );
	    exit ( -1 );
	}

	if ( fflag && (fmttype != ALTS) ) {
	   /*
	    *  The -F option was given so do nothing as dinit is just 
	    *  overiding the config block.
	    *  Special case the -A option.  Want to do the alt mapping
	    *  even if not formatting.... MMD - wjc - 9/5/86
            */
	    ;
	}
	else if ( fmttype  ==  DISK ) {
		fmtdsk();
	 }
	else if ( fmttype  ==  ALTS ) {
	        fmtalts();
	     }
	else if ( fmttype  ==  TRACK ) {
	        fmttrk( track );
	     }
	else if ( fmttype  ==  MAP ) {
	        maptrk( track, alt );
	     }
	else if ( fmttype  ==  SLIP ) {
		slptrk( track, sec );
	     }
	else if ( fmttype  ==  REFMT ) {
		rfmttrk( track );
	}
	close(fd);
}



/*	Check the last arguement for allowable device */

ckdev( )	
{
  int	minor;
  struct stat	stbuf;
  if ( (fd = open( device, O_RDWR )) == -1 ) {	
  	fprintf ( stderr, "%s: cannot open %s\n", cmdname, device );
  	perror ( cmdname );
  	exit(2);
  }
	
  if ( stat( device, &stbuf ) == -1 ) {
  	fprintf ( stderr, "%s: cannot stat %s\n", cmdname, device );
  	perror ( cmdname );
  	exit(2);
  }

  if ( ( stbuf.st_mode & S_IFMT ) != S_IFCHR ) {
  	fprintf ( stderr, "%s: raw devices only\n", cmdname );
  	exit(2);
  }
  
  minor = (int)(stbuf.st_rdev & 0xff);
  if ( (minor & 0x07) != 7 ) {
  	fprintf ( stderr, "%s: slice 7 only\n", cmdname );
  	exit(2);
  }
  strcat( bdev, "/dev/" );
  strcat( bdev, device+6 );

}

/*	print out command usage	    */

usage ( name )
char	*name;
{
  fprintf ( stderr, "usage: %s	raw-device [ cmd_type ] [ options ]\n", name );
}



/*	Format track	*/

fmttrk( track )
register int   track;
{
  register m360fmt arg;

  arg.f_trk = track;
  fprintf( stderr, "Format track %d \n", arg.f_trk );

  if(  ioctl( fd, M360FMTT, &arg )  == -1 )  {
	fprintf( stderr, "ioctl  Failed to format track %d\n", arg.f_trk );
	exit( -1 );
  }

}


/*	Map track to alternate	*/

maptrk( track, alt )
register int   track;
register int   alt;
{
  register m360fmt arg;

  arg.f_trk = track;
  arg.f_alttrk = alt;

  /* Bug in either driver or controller.  Dinit generates (short)(-1)
  ** value for alternate if track is itself an alternate.  When we
  ** map any track to 0xffff, it corrupts track 4095.  So don't bother
  ** sending the directive in this case.  MMD - wjc - 9/4/86
  */
  if (alt == 0xffff)
    return;

  fprintf( stderr, "Map track %d to %d\n", arg.f_trk,
		  arg.f_alttrk );

  if(  ioctl( fd, M360MPT, &arg )  == -1 )  {
	fprintf( stderr, "ioctl  Failed to map track %d to track %d\n", 
		 arg.f_trk, arg.f_alttrk );
	exit( -1 );
  }

}



/*	Format disk	*/

fmtdsk()
{
  register int	track;
  register int	last;
  register m360fmt arg;
  struct m360config  cf;

  if (  ioctl(fd, M360GET, &cf )  == -1 ) {
	fprintf( stderr, "Ioctl failed to get config\n" );
	exit( -1 );
  }

  printf( "Format disk: %d tracks\n", last = cf.cf_cyls * cf.cf_vnh );
  for (  track = 0 ;  track < last ;  track++ )
    {
	  arg.f_trk = track;

	  if(  ioctl( fd, M360FMTT, &arg )  == -1 )  {
		fprintf( stderr, "Failed to format track %d\n", arg.f_trk );
	  }
      
    }
}



/*	Format the alternates */

fmtalts()
{
  register int i;
  register int count;

  readvol();

  count = vol.vid_btl;
  fprintf( stderr, "the volid says there are %d bad tracks\n", count );
  for ( i = 0 ;  i < count ; i++ )  {
      maptrk(  bttable->b_bad, bttable->b_alt );
      bttable++;
  }

}

/*	Read the volume id block and the config block off the disk */
readvol()
{
  int  fid;

  if ( (fid = open( bdev, O_RDONLY )) == -1 ) {	
  	fprintf ( stderr, "%s: cannot open %s\n", cmdname, bdev );
  	perror ( cmdname );
  	exit(2);
  }
	
  if ( lseek(fid, 0L, 0) == -1 || read(fid, &vol,sizeof(struct volumeid)) == -1)
  {
     fprintf( stderr, "%s: Could not read volumeid\n", cmdname );
     exit( 1 );
  }

  if ( lseek(fid, vol.vid_cas*QK, 0) == -1 || read(fid, &dkcf, sizeof(struct dkconfig) ) == -1 )
  {
     fprintf( stderr, "%s: Could not read config block\n", cmdname );
     exit( 1 );
  }
  bttable = (struct badent *)malloc( vol.vid_btl * sizeof(struct badent));
  if ( lseek( fid, (vol.vid_cas+vol.vid_cal+1)*QK, 0) == -1 || read(fid, bttable, vol.vid_btl * sizeof(struct badent) ) == -1 )
  {
     fprintf( stderr, "%s: Could not read bad track table\n", cmdname );
     exit( 1 );
  }

  close( fid );

}



/*	Format with sector slip	*/

slptrk( track, sec )
register int   track;
register int   sec;
{
  printf( "Sector slip not available\n" );
}

/*	Reformat track */

rfmttrk( track )
register int   track;
{
  printf( "Reformat not available\n" );  /* may use fmttrk( flag) */
}
