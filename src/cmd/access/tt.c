/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

char  ident[] = "@(#)tt.c	1.1		created:8/21/85";

#include	<stdio.h>
#include	"access.h"

main(argc, argv)
char **argv;
{
  struct   usrdev   indev;	/*  Information about the disk to be READ    */
  struct   usrdev   outdev;	/*  Information about the disk to be WRITTEN */
  int	getperms();		/*  Function to read the PERMS_FILE          */
  char	*bsize;			/*  The block size of transfer.              */
  char	*bcount;		/*  Number of transfers.                     */
  int	tsize =  1;		/*  Number of 512-byte blocks transferred    */
  int	i;
  


  if ( argc != 5 )
   {
     fprintf( stderr, "Usage  %s if=  of=  [ bs= ]  [ count= ]\n", argv[0] );
     exit ( 1 );
   }

  for ( i = 1; i < argc; i++ )
   {
      if  ( strncmp( argv[i], "if=", 3 )  == 0 )
       {
          strcpy( indev.look_for, (argv[i]+3) );
          getperms( &indev );
          if ( ckmode( &indev, 'R' ) )
           {
             fprintf(  stderr, "No read permission on input file.\n" );
             exit ( 1 );
           }  
          strins( indev.real_dev, "if=", 0 ); /* Make if= a raw dev */
          strins( indev.real_dev, "r", 8 );
#ifdef EBUG
       printf( "input file arg: %s\n", indev.real_dev );
#endif
	  continue;
       }
      if  ( strncmp( argv[i], "of=", 3 )  == 0 )
       {
          strcpy( outdev.look_for, (argv[i]+3) );
          getperms( &outdev );
          if ( ckmode( &outdev, 'W' ) )
           {
             fprintf(  stderr, "No write permission on output file.\n" );
             exit ( 1 );
           }  
          strins( outdev.real_dev, "of=", 0 ); /* Make of= a raw dev */
          strins( outdev.real_dev, "r", 8 );
#ifdef EBUG
       printf( "output file arg: %s\n", outdev.real_dev );
#endif
	  continue;
       }
      if  ( strncmp( argv[i], "bs=", 3 )  == 0 )
       {
	  bsize = argv[i];
	  tsize *= getnum( bsize );    /*  Multiply transfer count by      */
                                       /*  transfer block size and         */
	  tsize /= BS;                 /*  Divide out physical block size  */
                                       /*  gives the number of physical    */
                                       /*  blocks transferred              */
#ifdef EBUG
       printf( "block size arg: %s\n", bsize );
#endif
	  continue;
       }
      if  ( strncmp( argv[i], "count=", 6 )  == 0 )
       {
	  bcount = argv[i];
	  tsize *= getnum( bcount );
#ifdef EBUG
       printf( "count arg: %s\n", bcount );
#endif
	  continue;
       }
      fprintf(stderr,"dd: bad arg %s\n", argv[i]);
      exit(2);
   }

  if (  tsize > getnum( indev.fsize )  )
   {
     if ( tsize > getnum( indev.modes )  )
      {
        fprintf( stderr, "Cannot read this much on input file %s\n", indev.real_dev  );
        exit ( 1 );
      }
     else
      {
        i =  tsize  -  getnum( indev.fsize );
        fprintf( stderr, "WARNING: reading past end of the file system\n" );
        fprintf( stderr, "         by %d    512-byte blocks\n", i );
      }
   }

  if (  tsize > getnum( outdev.fsize )  )
   {
     if ( tsize > getnum( outdev.modes )  )
      {
        fprintf( stderr, "Cannot write this much on output file %s\n", outdev.real_dev );
        exit ( 1 );
      }
     else
      {
        i =  tsize  -  getnum( outdev.fsize );
        fprintf( stderr, "WARNING: writing past end of the file system\n" );
        fprintf( stderr, "         by %d    512-byte blocks\n", i );
      }
   }

  if (  execl( "/bin/dd", "dd", indev.real_dev, outdev.real_dev, bsize, bcount, 0 )  )
   {
     fprintf( stderr, "Failed to exec dd \n" );
     exit ( 1 );
   }
}
