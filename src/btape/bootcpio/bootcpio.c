/*
 *		botcpio - bootable tape cpio
 *
 *		reads tape directory and cpios files
 *		on the tape to target directories
 *
 *		Copyright (c) 1986 Motorola Inc.
 *		All Rights Reserved
 *		%W%
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <fcntl.h>

#define BSIZE	512

#define	RDBUFSZ	(BUFSIZ*10)

/*
 *	tape directory buffer
 */

#define	DIRENT	(BSIZE/sizeof(struct direct))

static struct tdir  {
	struct	direct	dbuf[DIRENT];
	struct	tdir	*next;
};

struct tdir tbuf;	/* tape directory buffer */

/*
 *	file types
 */

#define OTHERS	0	
#define	RAMDISK	1
#define	FILESYS	2

#define GTTYPE(x)	((x>>8)&0xff)	/* 256 files */
#define GTFILE(x)	(x&0x00ff)

#define	VERBOSE(str)	if( verbose ) printf("%s\n", str )

/*
 *	
 */

#define OMODES	(O_NDELAY|O_RDONLY)

static	char	*argv0;

static	char	*cpiocmd = "; exec cpio ";
static 	char	*special;		/* special file name */
static	char	*options = NULL;	/* options for cpio */
static	char	*defaultops = "-iBc";	/* default options for cpio */

static char buff[BSIZE];
static int verbose = 0;
static int lastpos = 0;		/* last tape position */

main(argc, argv)
char **argv;
{	register arg;
	register char *bp = buff;
	extern char *optarg;
	extern int optind;

	argv0 = argv[0];

	while(( arg = getopt( argc, argv, "iBcdmrtuvsSb6" )) != EOF ) {
		switch( arg ) {
		case '?':
			fprintf( stderr, "%s: invalid option\n", argv0 );
			usage();
			break;
		case 'v':
			verbose = 1;
		default :
			if(options == NULL)
				options = argv[optind];
			break;
		}
	}
	if( optind >= argc ) {
		fprintf( stderr, "%s: missing special file\n", argv0 );
		usage();
	}
	if( options == NULL)
		options = defaultops;

	special = argv[optind];

	/* seek to directory file */

	VERBOSE("Seeking to tape directory file.");
	dopos(FILESYS);
	VERBOSE("Reading tape directory file.");
	rddir();
	VERBOSE("Seeking to tape file systems.");
	cpfiles();
	VERBOSE("Filesystem copy complete.");
}

/*
 *	rddir() - read tape directory
 */

rddir()
{	register struct tdir *tp = &tbuf;
	register struct tdir *tpt = tp;
	register status;
	register sfd;
	char	*malloc();

	if(( sfd = open(special,OMODES)) == -1) {
		fprintf( stderr, "%s: can't open %s\n", argv0, special );
		exit(-1);
	}

	/* read entire directory */

	while( (status = read(sfd,tp->dbuf,BSIZE)) > 0 ) {
#ifdef	DEBUG
		printf("status: %x\n",status);
#endif
		if(( tp->next = (struct tdir *)malloc(sizeof(struct tdir))) 
				== NULL ){
			fprintf( stderr, "%s: no more space\n", argv0 );
			exit(-1);
		}
		tpt = tp;
		tp = tp->next;
	}

	close(sfd);
	free(tp);
	tpt->next = NULL;
}



/* 
 *	cpfiles() - copy filesystem files
 *
 *	cd to target directory and cpio
 *	for each filesystem entry 
 */

cpfiles()
{	register struct tdir *tp = &tbuf;
	register struct direct *dp;
	register char *bp;
	register last = 0;	/* seek before first cpio */
	register fno;
	register n;

	for(tp = &tbuf; tp ; tp = tp->next) {
		dp = tp->dbuf;
		for( n=0, bp = buff; n < DIRENT; n++, dp++ ) {
			if( (fno = GTFILE(dp->d_ino)) == 0 )
				continue;
#ifdef	DEBUG
			printf("%d ", fno);
#endif
			if( GTTYPE(dp->d_ino) == OTHERS ) 
				continue;
			else if( GTTYPE(dp->d_ino) == RAMDISK ) 
				continue;
			else if( GTTYPE(dp->d_ino) == FILESYS ) {
#ifdef	DEBUG
				printf("%s\n", dp->d_name );
#endif
				dopos(fno);
				strcpy(bp,"cd ");
				strcat(bp,dp->d_name);
				strcat(bp,cpiocmd);
				strcat(bp,options);
				strcat(bp," < ");
				strcat(bp,special);
				last = fno;
				if(verbose)
					printf("%s\n",bp);
				if( system(bp) < 0 ) {
					exit(-1);
				}

			} else {
				fprintf( stderr, "%s: bad tape directory\n"
					,argv0 );
				exit(-1);
			}
		}

	}
}

usage()
{
	fprintf(stderr,"Usage: %s [-iBcdmrtuvfsSb6] special\n",argv0);
	exit(2);
}

/*
 *	dopos: position tape to file 'fnum'
 */

dopos(fnum)
register fnum;
{	register n;
	register sfd;
	register lp = lastpos;
	register char *bp;
	char	*malloc();

#ifdef DEBUG
	printf("dopos: lastpos = %d, fnum = %d\n",lp,fnum);
#endif
	if( fnum < lp) {
		fprintf(stderr,"%s: seeking backwards on %s\n",argv0,special );
		exit(-1);
	}
	lastpos = fnum + 1;
	if( lp == fnum )	/* already at file */
		return;
	if(( sfd = open(special,OMODES)) == -1) {
		fprintf( stderr, "%s: can't open %s\n", argv0, special );
		exit(-1);
	}
	if((bp = malloc(RDBUFSZ)) == NULL ){
		fprintf( stderr, "%s: no more space\n", argv0 );
		exit(-1);
	}
	for ( fnum -= lp; fnum ; fnum-- ) {
		/* read file */
		/* while(( n = read( sfd, bp, RDBUFSZ )) > 0 ); */
		n = read( sfd, bp, RDBUFSZ );
		if( n == -1 ) {
			fprintf(stderr, "%s: read error\n",argv0);
			exit(-1);
		}
		close(sfd);
		if(( sfd = open(special,OMODES)) == -1) {
			fprintf(stderr, "%s: can't open %s\n", argv0, special );
			exit(-1);
		}
	}
	close(sfd);
	free(bp);
}
