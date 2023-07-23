/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

char  ident[] = "@(#)fs.c	1.2		modified:5/2/86";

#include	<stdio.h>
#include	"access.h"

main( argc, argv )
int	argc;
char	*argv[];
{
  struct  usrdev  disk;		/*  Information on the disk                  */
  int	getperms();		/*  Function to read the PERMS_FILE          */
  char	*inodes;		/*  The number of inodes requested           */
  char	*strchr();



 /*	Check the argument list for valid arguments and determine 
  *	the values of any arguments which have been defaulted.
  */

  if  ( argc > 3  )  /* If to many arguments are given then abort */
   {
      fprintf( stderr, "Usage:  %s  [ dev  [ blocks[:inodes] ] ]\n", argv[0] );
      exit( 1 );
   }

 /*	If the device was given then look for it in the map
  *	else look for the default device.
  */

  if  ( argc > 1 )
      strcpy( disk.look_for, argv[1] );
   else
      strcpy( disk.look_for, "default" );


 /*	If the requested device is not in the map then the average user
  *	is not allowed to make a file system on that device.
  */

  if  ( getperms( &disk )  )
   {
      fprintf( stderr, "Restricted device or Unknown alias\n" );
      exit ( 22 );
   }

#ifdef	EBUG
	fprintf( stderr, "  %s: Found the following device.\n", argv[0] );
	fprintf( stderr, "  real dev    =  %s\n",  disk.real_dev );
	fprintf( stderr, "  look_for    =  %s\n",  disk.look_for );
	fprintf( stderr, "  mntpt       =  %s\n",  disk.mntpt    );
	fprintf( stderr, "  fsize       =  %s\n",  disk.fsize    );
	fprintf( stderr, "  mkfs_dev    =  %s\n",  disk.mkfs_dev );
	fprintf( stderr, "  modes       =  %s\n",  disk.modes    );
	fprintf( stderr, "  pgm         =  %s\n",  disk.pgm      );
	fprintf( stderr, "  format dev  =  %s\n",  disk.fmt_dev  );
#endif

 /*	If the device is not writable then do not allow a file system
  *     to be made.
  */
  if  ( ckmode( &disk, 'W' ) && (getuid() != 0) )
   {
      fprintf( stderr,  "%s is readonly\n", disk.look_for );
      exit ( 1 );
   }

 /*	If the number of blocks are given, determine if this is
  *	greater than the Maximum allowable size. 
  *	If so exit, else use the number asked for.
  */

  if  ( argc == 3 )
   {
      if  (  getnum( argv[2] )  >  getnum( disk.fsize )  )
       {
	  fprintf( stderr, "%s supports a max file system size of %d\n", disk.look_for, getnum(disk.fsize)  );
	  exit( 1 );
       }
      strcpy( disk.fsize, argv[2] );
   }



/*	Perform the actual mkfs	*/

   printf( "Executing  mkfs %s (%s)  %s\n", disk.look_for, disk.mkfs_dev, disk.fsize );
   if  ( execl( "/etc/mkfs", "mkfs", disk.mkfs_dev, disk.fsize, 0  ) ==  -1 )
       fprintf( stderr, "%s:  Exec failed\n", argv[0] );
}
