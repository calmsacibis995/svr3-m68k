/*
** Command to select most recently modified file from set
** if equal Mtimes are found on several files, the first file with 
** that time is kept as the 'newer'.
** The name of the file selected as newest is written to stdout.
*/

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<stdio.h>

#define	TIME_0	0L		/* earliest possible date */

main( argc, argv )
int	argc;
char	**argv;
{
	char	*newest;	/* name of newest file, NULL if none */
	long	theTime;	/* Mtime of newest file */
	struct stat curStat;	/* status of current file */

	newest = NULL;
	theTime = TIME_0;

	for ( ; ++argv, --argc; )	/* thru argv[1..argc-1] */
	{
		if ( stat( *argv, &curStat ) == 0 )
		{
		    /* File was found */
		    if ( curStat.st_mtime > theTime )
		    {
			newest = *argv;			/* keep name */
			theTime = curStat.st_mtime;	/* keep Mtime */
		    }
		}

		/* If stat failed, just ignore file */
	}

	/* Print file-name, if non-NULL */
	if ( newest != NULL )
	{
	    puts( newest );		/* emits trailing \n */
	}

	exit( 0 );
}
