
/*		Copyright (c) 1985 Motorola, Inc.   */
/*		All Rights Reserved		*/

char  ident[] = "@(#)chk.c	1.0		created:5/2/86";

#include	<stdio.h>
#include	"access.h"

int Readonly, Yesflg;

main( argc, argv )
int	argc;
char	*argv[];
{
  struct   usrdev  disk;	/*  Information on the disk to format        */
  int	getperms();		/*  Function to read the PERMS_FILE          */
  int   flag = 1;		/*  if format device is not given set to 0   */
  int	c;			/*  A scan char for getopt	             */
  int	getopt();		/*  Need to sift options for device          */
  extern  int	optind;
  extern  char	*optarg;
  register  int   i;		/*  Loop counter                    */
  register  int   end;		/*  End of the given argument list  */




  while (  (c = getopt( argc, argv, "yn" ))  !=  EOF )
        switch ( c )  {
	case 'y':
		if (Readonly)  {
		    fprintf (stderr, "%s:  -y not valid with -n\n", argv[0]);
		    exit (1);
		}
		else Yesflg = 1;
		break;
	case 'n':
		if (Yesflg)  {
		    fprintf (stderr, "%s:  -y not valid with -n", argv[0]);
		    exit (1);
		}
		else Readonly = 1;
		break;
	case '?':
		fprintf( stderr,
		    "Usage:  %0  [ -y | -n ] [ dev ... dev ]", argv[0]);
		exit (1);
	}
		   
 /*	If devices were given 
  *	then fsck them, else fsck the "default" device.
  */

  if  ( optind < argc ) 
   {
      for ( ; optind < argc; optind++)
	check (argv[optind], argv[0]);
   }
  else
   {
      check ("default", argv[0]);
   }
}



 check (dev, whoami)
 char *dev;
 char *whoami;
 {

 struct usrdev disk;
 char	*argt[ARG_COUNT];	/*  Need an array of pointers with one more  */
				/*  entries than argv.                       */
 register int end;
 register int i;
 register int readonly = Readonly;
 register int yesflg   = Yesflg;

 for ( i = 1 ; i < ARG_COUNT ; i++ )   /* init argt to empty state */
   argt[i] = 0;

 /*	If the requested device is not in the map then the average user
  *	is not allowed to format that device.
  */

  strcpy (disk.look_for, dev);

  if  ( getperms( &disk )  )
   {
      fprintf( stderr,
      "%s: %s - Restricted device or Unknown alias\n", whoami, dev );
      return(0);
   }

#ifdef	EBUG
	fprintf( stderr, "  %s: Found the following device\n", whoami);
	fprintf( stderr, "  real dev    =  %s\n",  disk.real_dev );
	fprintf( stderr, "  look_for    =  %s\n",  disk.look_for    );
	fprintf( stderr, "  mntpt       =  %s\n",  disk.mntpt    );
	fprintf( stderr, "  fsize       =  %s\n",  disk.fsize    );
	fprintf( stderr, "  mkfs_dev    =  %s\n",  disk.mkfs_dev );
	fprintf( stderr, "  modes       =  %s\n",  disk.modes    );
	fprintf( stderr, "  fmt_dev     =  %s\n",  disk.fmt_dev        );
#endif


 /*	If the device doesn't have write permissions, we only do
  *	'fsck -n'.
  */

  if  ( readonly || ( ckmode( &disk, 'W' ) && (getuid() != 0) ) ) {
      argt[1] = "-n";
      readonly = 1; yesflg = 0;
  }
  else if (yesflg)
      argt[1] = "-y";

 /*		set up the argt[] array.			*/

 end = 1 + readonly + yesflg;

 argt[0] = "/etc/fsck";
 argt[1 + readonly + yesflg] = disk.mkfs_dev;
 argt[end+1] = 0;

/* echo the actual command */

    putchar ('\n');
    for ( i = 0 ;  i <= end   ;  i++ )
      printf("%s  ", argt[i] );
    printf ("  (%s)\n", disk.look_for);
 

 /*	Perform the actual fsck */

  if ((i = fork()) == -1)  {
	perror ("Can't fork: ");
	exit(1);
  } else if (i == 0)  {			/* Child */
	if  ( execvp( argt[0], argt ) ==  -1 )
	    fprintf( stderr, "%s:  Exec to %s failed\n", whoami, argt[0] );
	
  } else 				/* Parent */
	(void) wait();

}
