/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

char  ident[] = "@(#)dcpy.c	1.1		created:10/3/85";

#include	<stdio.h>
#include	"access.h"

main(argc, argv)
char **argv;
{
  struct   usrdev   indev;	/*  Information about the disk to be READ    */
  struct   usrdev   outdev;	/*  Information about the disk to be WRITTEN */
  int	getperms();		/*  Function to read the PERMS_FILE          */
  int	Fsize;			/*  Size of output file sys in 1k blocks     */
  


 /*
  *	If the wrong number of arguments is given then
  *	report the error and exit.
  */

  if (  argc  != 3 )
   {
     fprintf( stderr, "Usage:  %s  from  to\n", argv[0]);
     exit ( 1 );
   }


 /*
  *	Determine the input device, and see if it is readable
  */

  strcpy( indev.look_for, argv[1] );
  getperms( &indev );
  if ( ckmode( &indev, 'R' ) )
   {
     fprintf(  stderr, "No read permission on input file.\n" );
     exit ( 1 );
   }  
  strins( indev.real_dev, "r", 5 );	/* Make input a raw device */
#ifdef EBUG
       printf( "input file arg: %s\n", indev.real_dev );
#endif


 /*
  *	Determine the output device, and see if it is writable
  */

  strcpy( outdev.look_for, argv[2] );
  getperms( &outdev );
  if ( ckmode( &outdev, 'W' ) )
   {
     fprintf(  stderr, "No write permission on output file.\n" );
     exit ( 1 );
   }  
#ifdef EBUG
       printf( "output file arg: %s\n", outdev.real_dev );
#endif

 /*
  *	Make the output filesystem the same size as that given
  *	by the fsize parameter.  Remember dcopy uses 1k blocks.
  */

  Fsize = getnum( outdev.fsize ) /  2;
  sprintf( outdev.fsize, "-f%d", Fsize );

  if (  execl( "/etc/dcopy", "dcopy", outdev.fsize, indev.real_dev, outdev.real_dev, 0 )  )
   {
     fprintf( stderr, "Failed to exec dcopy \n" );
     exit ( 1 );
   }
}
