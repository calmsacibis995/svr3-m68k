
/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

#include	<stdio.h>
#include	"access.h"
#include	<string.h>

main(argc, argv)
int argc;
char *argv[];
{

	if (strcmp (argv[0], "real") == 0)
		real(argc, argv);
	else if (strcmp (argv[0], "fmt") == 0)
		fmt(argc, argv);
	else
	    fprintf (stderr, "%s:  Don't know how to do this....\n", argv[0]);
	
	exit (1);
}

real( argc, argv )
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


fmt( argc, argv )
int	argc;
char	*argv[];
{
  struct   usrdev  disk;	/*  Information on the disk to format        */
  int	getperms();		/*  Function to read the PERMS_FILE          */
  int   flag = 1;		/*  if format device is not given set to 0   */
  char	*argt[ARG_COUNT];	/*  Need an array of pointers with one more  */
				/*  entries than argv.                       */
  int	c;			/*  A scan char for getopt	             */
  int	getopt();		/*  Need to sift options for device          */
  extern  int	optind;
  extern  char	*optarg;
  register  int   i;		/*  Loop counter                    */
  register  int   end;		/*  End of the given argument list  */




  while (  (c = getopt( argc, argv, "b:cd:eoft:" ))  !=  EOF )
        switch ( c )  {
        case 'b':   printf( "Using %s as the bootloader\n", optarg );
		    break;
	case 'd':   printf( "Set description string to: %s\n", optarg );
		    break;
	case 't':   printf( "Reading bad tracks from file: %s\n", optarg );
		    break;
	case 'c':
	case 'e':
	case 'o':
	case 'f':
		   break;
	case '?':  fprintf( stderr, "I hope dinit understands all these options cause I do NOT\n" );
	}
		   
 /*	If the device was given 
  *	then format it, else format the "default" device.
  */

  if  ( optind == (argc-1) ) 
   {
      strcpy( disk.look_for, argv[(argc-1)] );
   }
  else
   {
      strcpy( disk.look_for, "default" );
      flag=0;
   }



 /*	If the requested device is not in the map then the average user
  *	is not allowed to format that device.
  */

  if  ( getperms( &disk )  )
   {
      fprintf( stderr, "Restricted device or Unknown alias\n" );
      exit ( 22 );
   }

#ifdef	EBUG
	fprintf( stderr, "  %s: Found the following device\n", argv[0] );
	fprintf( stderr, "  real dev    =  %s\n",  disk.real_dev );
	fprintf( stderr, "  look_for    =  %s\n",  disk.look_for    );
	fprintf( stderr, "  mntpt       =  %s\n",  disk.mntpt    );
	fprintf( stderr, "  fsize       =  %s\n",  disk.fsize    );
	fprintf( stderr, "  mkfs_dev    =  %s\n",  disk.mkfs_dev );
	fprintf( stderr, "  modes       =  %s\n",  disk.modes    );
	fprintf( stderr, "  fmt_dev     =  %s\n",  disk.fmt_dev        );
#endif



 /*	The device must have format permissions
  */
  if  ( ckmode( &disk, 'F' ) && access (disk.fmt_dev, 02) )
   {
      fprintf( stderr, "%s does not have format permissions\n", disk.look_for );
      exit ( 22 );
   }





 /*		set up the argt[] array.			*/

 end = argc - flag ;

 argt[0] = disk.pgm;		/*  Set the fmt pgm into argt[0]    */

 for ( i = 1 ; i < end ; i++ )  
   argt[i] = argv[i];	/* copy the args which were given   */

   argt[end]  =  disk.fmt_dev;	/* Put the fmt_dev at the   */
   argt[(end+1)] = NULL;             /* end of the args list.    */

#ifdef  EBUG
    fprintf( stderr, "Arguments for exec to %s are:\n", argt[0] );
    for ( i = 0 ;  i <= end   ;  i++ )
      fprintf( stderr,  " argt[%d]  =  %s\n", i, argt[i] );
#endif
 

 /*	Set the real user id to root.  This is needed as the
  *	dinit command executes the access(2) routine several
  *	times which uses the real user id.
  */

  if  (  setuid( ROOT )  == -1 )
   {
      fprintf( stderr, "Could not set real user id to root\n" );
      exit( 2 );
   }


 /*	Perform the actual format */

  printf( "Formatting  %s  (%s) with %s\n",
			disk.look_for, disk.fmt_dev, disk.pgm );
  printf ( "Is this really what you want? <y or n>: ");
  if ( getchar () != 'y' )
      exit (3);

  if  ( execvp( disk.pgm, argt ) ==  -1 )
      fprintf( stderr, "%s:  Exec to %s failed\n", argv[0], argt[0] );
}
