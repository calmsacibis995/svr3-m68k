/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

char  ident[] = "@(#)fmt.c	1.2		created:9/11/85";

#include	<stdio.h>
#include	"access.h"

main( argc, argv )
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




  while (  (c = getopt( argc, argv, "b;cd;eoft;" ))  !=  EOF )
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

  if  ( optind == (argc-2) ) 
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
	fprintf( stderr, "  fsize       =  %s\n",  disk.fsize    );
	fprintf( stderr, "  modes       =  %s\n",  disk.modes    );
	fprintf( stderr, "  fmt_dev     =  %s\n",  disk.fmt_dev        );
#endif



 /*	The device must have format permissions
  */
  if  ( ckmode( &disk, 'F' ) )
   {
      fprintf( stderr, "%s does not have format permissions\n", disk.look_for );
      exit ( 22 );
   }





 /*		Set up the argt[] array.			*/

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

  printf( "Formatting the disk with %s\n", disk.pgm );
  if  ( execvp( disk.pgm, argt ) ==  -1 )
      fprintf( stderr, "%s:  Exec to %s failed\n", argv[0], argt[0] );
}
