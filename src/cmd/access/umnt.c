/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

char  ident[] = "@(#)umnt.c	1.1		created:8/21/85";

#include	<stdio.h>
#include	"access.h"

main( argc, argv )
int	argc;
char	*argv[];
{
  struct    usrdev   disk;	/*  Information on the requested disk       */
  int	getperms();		/*  Function to read the PERMS_FILE         */

 /*	Check the argument list for valid arguments and determine 
  *	the values of any arguments which have been defaulted.
  */

 /*  If to many arguments are given report an error and abort */

  if  ( argc > 2  )
   {
      fprintf( stderr, "Usage:  %s  [ dev ]\n", argv[0] );
      exit( 0 );
   }


 /*  If the device was not given then use the default device */

  if  ( argc > 1 )
      strcpy( disk.look_for, argv[1] );
   else
      strcpy( disk.look_for, "default" );


 /*	If the requested device is not in the map then the average user
  *	is not allowed to unmount that device.
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


/*	Perform the actual umount	*/

  if  ( execl( "/etc/umount", "umount", disk.real_dev, 0  ) ==  -1 )
      fprintf( stderr, "%s:  Exec failed\n", argv[0] );
}
