/*	@(#)udfmt.c	6.3		*/
/*
 *	udfmt.c: EXORmacs Universal Disk Controller disk formatter
 */

#define	UDFMT	1

#define	SPT	64	/* Sectors per track */
#define	SPB	2	/* Sectors per block */

#define MAXERRS	100	/* default maximum errors allowed in format */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 *	main: udfmt special [blocks]
 */

main( argc, argv )
int	argc;
char	**argv;

{	register int	maxsec;
	register int	curblk;
	register int	cursec;
	register int	fd;
	register int	spt;
	register char	*estr;
	register int 	i;
	char		*getenv();
	struct	stat	stbuf;
	int	maxerrs;
	int	toterrs = 0;

	if(( estr = getenv("SPT")) != NULL )
		spt = atoi( estr );
	else if(( estr = getenv("BPT")) != NULL )
		spt = atoi( estr ) * SPB;
	else 
		spt = SPT;

	if(( estr = getenv("MAXERRS")) != NULL )
		maxerrs = atoi( estr );
	else
		maxerrs = MAXERRS;

	if( argc > 3 || argc < 2)
	{	fprintf( stderr, "Usage: udfmt special [blocks]\n");
		exit(1);
	}

	for ( i=1; i < argc; i++ )
		if (strcmp(argv[i] ,"-F" ) == 0)
			exit(0);

	if( stat( argv[1], &stbuf ) == -1 )
	{	fprintf( stderr, "%s: Cannot stat %s\n", argv[0], argv[1] );
		exit(1);
	}

	if(( stbuf.st_mode & S_IFMT ) != S_IFCHR )
	{	fprintf( stderr, "%s: raw devices only.\n", argv[0] );
		exit(1);
	}
	
	if(( fd = open( argv[1], 2 )) == -1 )
	{	fprintf( stderr, "%s: Cannot open %s\n", argv[0], argv[1] );
		exit(1);
	}

	if( argc == 3 )
	{	maxsec = atoi( argv[2] ) * SPB;
		
		for( cursec=0; cursec < maxsec; cursec += spt )
			if( ioctl( fd, UDFMT, (curblk = (cursec + SPB - 1) 
				/ SPB)) == -1 )
			{	fprintf( stderr,
					"%s: error on block %d (track %d)\n",
					argv[0], curblk, cursec / spt );
				if (++toterrs >= maxerrs)
					break;
			}
	}
	else
		for( cursec=0; ioctl(fd,UDFMT,(cursec + SPB - 1) / SPB) != -1; 
			cursec += spt );

	printf( "%s: formatted %d blocks\n", argv[0], cursec / SPB );
	exit (0);
}
