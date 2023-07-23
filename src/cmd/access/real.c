/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

char	ident[] = "%W%		created: %G%";

#include	<stdio.h>
#include	"access.h"

main( argc, argv )
int	argc;
char	*argv[];
{
  struct    usrdev  disk;	/*  Information on the requested disk.       */
  int	getperms();		/*  Function to read the PERMS_FILE          */
  int	raw = 0;		/*  If 1 then -r given, so return raw device */


 /*  If more than one argument is given report error and abort */

  if  ( argc > 3 )
   {
      fprintf( stderr, "Usage:  %s [ -r ] [ alias ] \n", argv[0] );
      exit( 1 );
   }

  if  (  strcmp( argv[1], "-r" ) == 0  )	 raw = 1;

 /*  If the device was given then use it else the "default" device. */

  if  ( argc > (1+raw)  )
      strcpy( disk.look_for, argv[(1+raw)] );
   else
      strcpy( disk.look_for, "default" );

/*	Print the real device name.		*/

  if  ( getperms( &disk )  )
   {
      fprintf( stderr, "Restricted device or Unknown alias\n" );
      exit( 1 );
   }

  if  (  raw  )		strins( disk.real_dev, "r", 5 );

  printf( "%s\n",  disk.real_dev );
  exit ( 0 );

}
