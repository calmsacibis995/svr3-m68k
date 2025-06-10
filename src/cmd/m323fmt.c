/*	@(#)m323fmt.c	7.4	12/13/86	*/
/*
 *	Copyright (c) 1986	Motorola Inc.  
 *		All Rights Reserved.
 */

/*
 *	m323fmt - format disks on the MVME323 disk controller
 *
 * SYNOPSIS
 *	m323fmt  -T  track  special
 *	m323fmt  -M  bad_track  alt_track  special	
 *	m323fmt  -S  track  sector  special	
 *	m323fmt  -R  track  special	
 *	m323fmt  -A  read badtrack table and map tracks.
 *	m323fmt  -F  just overridding the config block.
 *	m323fmt  [ -m ]   [ -s ]   [ -v ]   special
 *		might want format disk to be with -D
 *
 * DESCRIPTION 
 *	m323fmt is used to format disks on the 
 *	MVME323 disk controller.  This causes the MVME323
 *	controller to be busy until a format winchester or a
 *	format tape request has been terminated. Therefore 
 *	it is recommended to invoke m323fmt only on a quiet
 *	system (i.e. single user mode) when formatting those
 *	devices. 
 *
 *
 *	Commands:
 *
 *	-M   <bad_track>  <alt_track>	map bad_track to alt_track.
 *	-R   <track>			Reformat a track.
 *	-T   <track>			format a single track.
 *	-A				map bad_tracks listed in bad track table
 *	-F				dinit just overriding config block
 *	-S  <track> <sector>NOT SUPPORTED     slip at sector on track.
 *	default				Format entire disk.
 *
 *	Options:
 * 
 *	-m   format disk with alternate tracks turned on.
 *	-s   NOT SUPPORTED    format disk with sector slip turned on.
 *	-v   Verify device
 *
 */



#include  "stdio.h"
#include  "fcntl.h"
#include  "sys/dk.h"
#include  "sys/types.h"
#include  "sys/stat.h"
#include  "sys/errno.h"
#include  "sys/mvme323.h"
/*
#include  "/uts/rdk/io/esdi/constants.h"
#include  "/uts/rdk/io/esdi/misc_defs.h"
#include  "/uts/rdk/io/esdi/structures.h"
#include  "/uts/rdk/io/esdi/ioctl.h"
*/


/* STRUCTURE USED TO BUILD INDIVIDUAL SECTOR ENTRIES FOR TRACK HEADERS */

typedef struct sectorheader HEADER;
typedef struct sectorheader
        {
        UCHAR     cyl_l; 
        UCHAR     cyl_h; 
        UCHAR     head; 
        UCHAR     sector; 
        };

/* VALUES LOADED INTO TRACK HEADERS */

#define BADSECTOR           0xee
#define SPARESECTOR         0xdd
#define ALT_TRACK           0xcc


#define	TRUE	1
#define FALSE	0

#define PHYSBLK	512
#define QK	256

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
UNITINFO       u_table; /* Drive parameters obtained from controller */
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
  rddrvpar();		/* get drive parameters from controller */
  
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
  IOCTL_TABLE 	ioctl_table;
  FORMAT_TABLE	format_table; 

  ioctl_table.ptr_1 = (char *)&format_table; 
  format_table.cylinder = track/u_table.heads;
  format_table.head = track % u_table.heads;
  fprintf( stderr, "Format track %d \n", track );

  if(  ioctl( fd, XYFORMAT, &ioctl_table )  == -1 )  {
	fprintf( stderr, "m323fmt: Failed to format track %d\n", track );
	exit( -1 );
  }

}


/*	Map track to alternate	*/

maptrk( track, alt )
register uint track;
register uint alt;
{
	IOCTL_TABLE 	ioctl_table;
	HEADER *header_buff, *alt_buff, *check_buff, *header_ptr;
	uint cylinder, head, alt_cyl, alt_hd, headersize, totalsectors;
	int  i;

	if (alt == 0xffff)
		return;

	cylinder = track/u_table.heads;
	head = track % u_table.heads;
	alt_cyl = alt/u_table.heads;
	alt_hd = alt %  u_table.heads;
	totalsectors = u_table.sectors + u_table.trk_spares;
	headersize = sizeof(HEADER) * totalsectors;

	 /* Allocate the header buffs */
	header_buff = (HEADER *)malloc(headersize);
	alt_buff = (HEADER *)malloc(headersize);
	check_buff = (HEADER *)malloc(headersize);
	if((header_buff == NULL) || (alt_buff == NULL) || (check_buff == NULL)){		fprintf(stderr,"m323fmt: can't allocate space for track headers.\n");
		exit(-1);
	}

	ioctl_table.ptr_1 = (char *)check_buff; 
	ioctl_table.var_1 = alt_cyl;
	ioctl_table.var_2 = alt_hd;

	errno = 0;
	if (ioctl(fd, XYRDTRKHDR, &ioctl_table) == -1 || errno)
	{
		fprintf(stderr,"m323fmt: can't read track header on cyl %d, head %d\n", alt_cyl, alt_hd);
		exit(-1);
	}

	/* If the header disk address of the alternate track does not point */
	/* to itself, we have to assume that it has already been allocated. */
	/* This should be tested by reading the track that it "thinks" that */
	/* it is -- and checking for a "0xcc" in its cyl_h positions.       */

	if ((check_buff->cyl_l | (check_buff->cyl_h << 8)) NOT= alt_cyl)
        {
		fprintf(stderr,"m323fmt: Alternate track is already in use. cyl=%d, head=%d\n", 
			alt_cyl, alt_hd);
		free(check_buff);
		free(header_buff);
		free(alt_buff);
		return;
	}

    
        ioctl_table.ptr_1 = (char *)header_buff; 
        ioctl_table.var_1 = cylinder;
        ioctl_table.var_2 = head;
    
	errno = CLEAR;
	if (ioctl(fd, XYRDTRKHDR, &ioctl_table) == -1 || errno)
        {
		fprintf(stderr,"m323fmt: can't read track header on cyl %d, head %d\n", alt_cyl, alt_hd);
		exit(-1);
	}

	/* Make sure that the selected track is not already mapped. */
	header_ptr = header_buff;
	if (header_ptr->cyl_h == ALT_TRACK)
	{
 		fprintf(stderr,"m323fmt: Cylinder: %d  Head: %d is already mapped.\n",
                        cylinder, head);
		free(check_buff);
		free(header_buff);
		free(alt_buff);
		return;
	}

	/* Stash a copy of the header into the alternate track buff. */
	/* THERE HAD BETTER NOT BE ANY SECTORS MARKED BAD IN THE HEADERS.
	   SECTOR SLIPPING IS NOT SUPPORTED YET. */
	copy(header_buff, alt_buff, headersize);

	for (i = 0; i < totalsectors; i++, header_ptr++)
        {
		if(header_ptr->cyl_l == BADSECTOR)
		{
			fprintf(stderr,"m323fmt error: cant map cylinder %d",cylinder);
			fprintf(stderr," head %d to alternate.\n", head);
			fprintf(stderr,"Reformat the track to remove slipped sector and try again.\n");
			exit(-1);
		}
		header_ptr->sector = (UCHAR)alt_hd;
		header_ptr->head = (UCHAR)(alt_cyl >> 8);
		header_ptr->cyl_l = (UCHAR)alt_cyl;
		header_ptr->cyl_h = ALT_TRACK;
        }


	/* Now write out both track headers. */

	printf("MAPPING  Cylinder: %d Head: %d  TO  Cylinder: %d Head: %d\n",
        cylinder, head, alt_cyl, alt_hd);

	ioctl_table.ptr_1 = (char *)header_buff; 
	ioctl_table.var_1 = cylinder;
	ioctl_table.var_2 = head;

	errno = 0;
	if (ioctl(fd, XYWRTRKHDR, &ioctl_table) == -1 || errno)
        {
          fprintf(stderr, "WRITE TRACK HEADERS ioctl() call failed.\n\n");
	  fprintf( stderr, "ioctl  Failed to map track %d to track %d\n", 
		track, alt);
	  exit(-1);
        }

	ioctl_table.ptr_1 = (char *)alt_buff; 
	ioctl_table.var_1 = alt_cyl;
	ioctl_table.var_2 = alt_hd;
	errno = 0;
	if (ioctl(fd, XYWRTRKHDR, &ioctl_table) == -1 || errno)
        {
           fprintf( stderr, "WRITE TRACK HEADERS ioctl() call failed.\n\n");
	   fprintf( stderr, "ioctl  Failed to map track %d to track %d\n", 
		 track, alt);
           exit(-1);
        }

	free(check_buff);
	free(header_buff);
	free(alt_buff);
}



/*	Format disk	*/
/* 	Assumes that drive parameters, controller parameters, and 
	format parameters are correct and in the controllers memory.  
*/

fmtdsk()
{
  IOCTL_TABLE 	ioctl_table;
  FORMAT_TABLE	format_table; 
  register int	track;
  register int	last;

  ioctl_table.ptr_1 = (char *)&format_table; 

  printf( "Format disk: %d tracks\n", last = u_table.cylinders * u_table.heads );
  for (  track = 0 ;  track < last ;  track++ )
  {
	  format_table.cylinder = track/u_table.heads;
	  format_table.head = track % u_table.heads;
	  if(  ioctl( fd, XYFORMAT, &ioctl_table )  == -1 )  {
		fprintf( stderr, "Failed to format track %d\n", track );
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

rddrvpar()
{
	IOCTL_TABLE    ioctl_table;

	ioctl_table.ptr_1 = (char *)&u_table;

	errno = CLEAR;
	if (ioctl(fd, XYRDDRVPAR, &ioctl_table) == -1 || errno)
	{
		fprintf(stderr,"m323fmt: READ DRIVE PARAMETERS ioctl() call failed.\n");
		exit(-1);
	}
}

copy(full, empty, byte_count)
char     *full, *empty;
unsigned  byte_count;
{
    USHORT  count;

    for (count = 0; count < byte_count; count++)
        empty[count] = full[count];

}
