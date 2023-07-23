/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

char	ident[] = "@(#)mnt.c	1.2		modified:5/1/86";

#include	<stdio.h>
#include	"access.h"

main( argc, argv )
int	argc;
char	*argv[];
{
  struct    usrdev  disk;	/*  Information on the requested disk.       */
  char  mntpt[PATH_SZ];		/*  The directory mount point                */
  int	getperms();		/*  Function to read the PERMS_FILE          */
  int	readonly  = 0;		/*  If set (1) then mount readonly           */

 /*	Check the argument list for valid arguments and determine 
  *	the values of any arguments which have been defaulted.
  */

 /*  If the last argument is a -r then set the read only flag */

  if  ( strcmp( argv[(argc-1)], "-r" ) ==  0 )
   { 
      readonly = 1;
   }

 /*  If to many arguments are given report an error and abort */

  if  ( argc > (3 + readonly)  )
   {
      fprintf( stderr, "Usage:  %s  [ dev  [ dir ] ]  [ -r ]\n", argv[0] );
      exit( 1 );
   }

 /*  If the device was given then use it else the "default" device. */

  if  ( argc > (1 + readonly) )
      strcpy( disk.look_for, argv[1] );
   else
      strcpy( disk.look_for, "default" );



/*	If the requested device is not in the map then the average user
 *	is not allowed to mount that device.
 */

  if  ( getperms( &disk )  )
   {
      fprintf( stderr, "Restricted device or Unknown alias\n" );
      exit( 1 );
   }

#ifdef	EBUG
	fprintf( stderr, "  %s: Found the following device.\n", argv[0] );
	fprintf( stderr, "  real_dev  =  %s\n",  disk.real_dev );
	fprintf( stderr, "  look_for  =  %s\n",  disk.look_for );
	fprintf( stderr, "  mntpt     =  %s\n",  disk.mntpt    );
	fprintf( stderr, "  fsize     =  %s\n",  disk.fsize    );
	fprintf( stderr, "  mkfs_dev  =  %s\n",  disk.mkfs_dev );
	fprintf( stderr, "  modes     =  %s\n",  disk.modes    );
	fprintf( stderr, "  pgm       =  %s\n",  disk.pgm      );
	fprintf( stderr, "  fmt_dev   =  %s\n",  disk.fmt_dev  );
#endif

 /*  If the mount point was given use it else use the default from perms  */

  if  ( argc == (3 + readonly) )
      strcpy( mntpt, argv[2] ); 
   else
      strcpy( mntpt, disk.mntpt );

 /*   
  *   If the user asked for readonly or the device does NOT have write 
  *   permission then mount it readonly  ( -r ), Else mount it r/w
  */

  if  (  readonly  ||  ( ckmode( &disk, 'W' ) && (getuid() != 0))  )
   {
      printf( "Mounting  %s  as  %s  readonly\n", disk.look_for, mntpt );
      if  ( execl( "/etc/mount", "mount", disk.real_dev, mntpt, "-r", 0 )  )
       {
          fprintf( stderr, "%s:  Exec failed\n", argv[0] );
          exit( 1 );
       }
   }
  else
   {
      printf( "Mounting  %s  as  %s\n", disk.look_for, mntpt );
      if  ( execl( "/etc/mount", "mount", disk.real_dev, mntpt, 0 ) )
       {
          fprintf( stderr, "%s:  Exec failed\n", argv[0] );
          exit( 1 );
       }
   }
}
