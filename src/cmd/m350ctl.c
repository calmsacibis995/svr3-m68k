/*
 *	%W%
 *	m350ctl.c: MVME350 control program
 *
 *
 *	m350ctl options:
 *
 *	-r		rewind
 *	-e		erase
 *	-t		retension
 *	-w		open for writing
 *	-s[N]		set DMA buffer size
 *	-g		get (and print) DMA buffer size
 *	-fX		position at beginning of file X
#ifdef	BYTESWAP
 *	-b		set byte swapping
 *	-B		reset byte swapping
#endif
#ifdef	DEBUG
 *	-u		underrun count
#endif
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mvme350.h>

#define	DSIZE	(1024*128)
#define	RDBUFSZ	(BUFSIZ*10)

#define	BYTESWAP

#define	DEBUG
#undef	DEBUG

char	rdbuf[RDBUFSZ];

/*
 *	main: main procedure
 */

main( argc, argv )
register argc;
register char **argv;

{	register char *av;
	register char *fname;
	register arg;
	register omodes = O_NDELAY|O_RDONLY;
	register fd = 0;	/* standard in */
	register c;
	int	sizetype = 1;
	int	fnum = 1;
	struct stat sbuf;

	/*
	 *	first pass over the arguments
	 */

	for( arg=1; arg < argc; arg++ ) {
		av = argv[arg];
		if( *av == '-' ) {
			while(( c = *++av ) != '\0' ) {
				switch(c) {
				case 'w':
					omodes = O_NDELAY|O_WRONLY;
					fd = 1;	/* standard out */
				case 'r':
				case 'e':
				case 't':
					break;
				case 'f':
					if( *++av == '\0' ) {
						av--;
						break;
					}
					while(isdigit(*av))
						av++;
					if( *av != '\0' )
						usage();
					av--;
					break;
				case 's':
					if( *++av == '\0' ) {
						av--;
						break;
					}
					while(isdigit(*av))
						av++;
					if(( *av == 'k' )||( *av == 'K' ))
						sizetype = 1024;
					else if(( *av == 'b' )||( *av == 'B' ))
						sizetype = 512;
					else if( *av != '\0' )
						usage();
					*av-- = '\0';
				case 'g':
					omodes &= ~O_NDELAY;
					break;
#ifdef	BYTESWAP
				case 'b':
				case 'B':
					break;
#endif
#ifdef	DEBUG
				case 'u':
					break;
#endif
				default:
					fprintf( stderr, "m350ctl: illegal option: -%c\n", c );
					usage();
				}
			}

		} else if( arg != argc-1 )
			usage();	/* file can only be last argument */

		else if( stat( av, &sbuf ) == -1 )
			ferr( "Can't stat %s\n", av );

		else if( (sbuf.st_mode&S_IFMT) != S_IFCHR )
			ferr( "%s not a character device.\n", av );

		else if((fd = open(av,omodes)) == -1)
			ferr( "Cannot open %s\n", av );

		else
			fname = av;		/* save file name */
	}

	/*
	 *	second pass
	 */

	for( arg=1; arg < argc; arg++ ) {
		av = argv[arg];
		if( *av != '-' )
			continue;
		while(( c = *++av ) != '\0' ) {
			int	size;

			switch(c) {
#ifdef	DEBUG
			case 'u':	doioctl(fd,M350IOCTL|'U',&size);
					printf("Underruns: %#x(%d)\n",
							size, size );
					break;
#endif
			case 'r':	doioctl(fd,M350REWIND,0);
					break;
			case 't':	doioctl(fd,M350RETENSION,0);
					break;
			case 'e':	doioctl(fd,M350ERASE,0);
					break;
			case 'f':	doioctl(fd,M350REWIND,0);
					if( *++av != '\0' )
						fnum = atoi(av);
					dopos(fname,omodes,fd,fnum);
					break;
#ifdef	BYTESWAP
			case 'b':
			case 'B':	doioctl(fd,M350BYTESWAP,c == 'b');
					break;
#endif
			case 's':	if( *++av == '\0' )
						size = DSIZE;
					else {
						size = atoi(av)*sizetype;
						while( *++av != '\0' );
					}
					--av;
					doioctl(fd,M350SETDMA,size);
			case 'g':	doioctl(fd,M350GETDMA,&size);
					printf("DMA buffer size = %#x(%d)\n",
						size, size );
			default:	break;
			}
		}
	}
}

/*
 *	usage: print out usage message and exit
 */

usage()
{	
#ifdef	BYTESWAP
#ifdef	DEBUG
	fprintf( stderr, "Usage: m350ctl [-retwgbBu] [-fX] [-s[N[kb]]] [special]\n" );
#else
	fprintf( stderr, "Usage: m350ctl [-retwgbB] [-fX] [-s[N[kb]]] [special]\n" );
#endif	DEBUG
#else
#ifdef	DEBUG
	fprintf( stderr, "Usage: m350ctl [-retwgu] [-fX] [-s[N[kb]]] [special]\n" );
#else
	fprintf( stderr, "Usage: m350ctl [-retwg] [-fX] [-s[N[kb]]] [special]\n" );
#endif	DEBUG
#endif	BYTESWAP
	fprintf( stderr, "Default special file: standard input\n");
	fprintf( stderr, "Options:\n");
	fprintf( stderr, "-r:\trewinds tape\n");
	fprintf( stderr, "-e:\terases tape\n");
	fprintf( stderr, "-t:\tretensions tape\n");
	fprintf( stderr, "-w:\topens tape for writing (with filemark)\n");
	fprintf( stderr, "-g:\tprints double-buffer buffer size\n");
#ifdef	BYTESWAP
	fprintf( stderr, "-b:\tsets MVME350 byte swapping\n");
	fprintf( stderr, "-B:\tresets MVME350 byte swapping\n");
#endif
	fprintf( stderr, "-f:\tpositions at file X\n");
#ifdef	DEBUG
	fprintf( stderr, "-u:\tprints number of underruns\n");
#endif
	fprintf( stderr, "-s:\tsets double-buffer buffer size\n");
	fprintf( stderr, "\t\tDefault buffer size (N): %#x(%d) bytes\n",
							DSIZE, DSIZE );
	fprintf( stderr, "\t\tN followed by `b' or `B' is in blocks\n");
	fprintf( stderr, "\t\tN followed by `k' or `K' is in K-bytes\n");
	exit(1);
}

/*
 *	ferr: fatal error
 */

ferr(s,a1)
{	fprintf(stderr, "m350ctl: " );
	fprintf(stderr, s, a1 );
	exit(1);
}

/*
 *	doioctl: perform ioctl
 */

doioctl(fd,cmd,arg)
{	if( ioctl(fd,cmd,arg) != -1 )
		return;
	perror("m350ctl");
	exit(1);
}

/*
 *	dopos: position tape
 */

dopos(fname,omodes,fd,fnum)
register char *fname;
register omodes;
register fd;
register fnum;

{	register n;

	while( --fnum > 0 ) {
		/* read file */
		while(( n = read( fd, rdbuf, RDBUFSZ )) > 0 );
		if( n == -1 ) {
			perror("read error");
			return;
		}
		close(fd);
		if((fd = open(fname,omodes)) == -1)
			ferr( "Cannot open %s\n", fname );
	}
}
