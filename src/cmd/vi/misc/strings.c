static char *sccsid = "@(#)strings.c	6.1	";
#include <stdio.h>
#include <a.out.h>
#include <ldfcn.h>
#include <ctype.h>

#define  COFF (vax || u3b || m68k)

#if COFF
#define NC	140			/* max string length */
#define DS	(unsigned short) 2	/* data section */
#define MC68MAGIC 0520			/* V/68 magic number */

SCNHDR scnhd;				/* section header buffer */
char bin[ BUFSIZ], bout[ BUFSIZ ];	/* stdio buffers */
char s[ NC ];				/* string buffer */
enum control { white, instring, badstring };

main(argc, argv)
register char **argv;
{
	register LDFILE *ldptr;
	register unsigned short u;
	void pstr();

	setbuf( stdout, bout );
	for( argv++ ; argc > 1 ; --argc )
	{	printf( "%s:", *argv );
		ldptr = NULL;
		ldptr = ldopen( *argv++, ldptr );
		if( ldptr == NULL )
		{	puts( " can't open" );
			continue;
		}
		SETBUF( ldptr, bin );
		pstr( ldptr );
		ldclose( ldptr );
	}
	fclose( stdout );
}

void pstr( lptr )
register LDFILE *lptr;
{
	register long l;
	register int c;
	register char *p;
	register enum control ctrl;

	if( HEADER(lptr).f_magic != VAXWRMAGIC
	&&  HEADER(lptr).f_magic != VAXROMAGIC
	&&  HEADER(lptr).f_magic != MC68MAGIC )
	{	puts( " not object file" );
		return;
	}
	if( ldshread( lptr, DS, &scnhd) == FAILURE )
	{	puts( " can't read data section header" );
		return;
	}
	if( scnhd.s_size == 0 || scnhd.s_scnptr == 0 )
	{	putchar( '\n' );
		return;
	}
	if( ldsseek( lptr, DS ) == FAILURE )
	{	puts(" failed to seek to data");
		return;
	}
	putchar( '\n' );

	ctrl = white;
	for( l = scnhd.s_size ; l > 0 ; l-- )
	{	c = getc( IOPTR( lptr ) );
		if( FEOF( lptr ) || FERROR( lptr ) )
			return;
		switch( (int) ctrl )
		{
		case white:
			if( isprint( c ) )
			{	ctrl = instring;
				p = s;
				*p++ = (char) c;
			}
			break;

		case instring:
			if( c == '\0' )
			{	if( p[ -1 ] == '\n' )
					--p;
				*p = '\0';
				puts( s );
				ctrl = white;
				break;
			}
			if( isascii( c ) && p < &s[ NC ] )
				*p++ = c;
			else
				ctrl = badstring;
			break;

		case badstring:
			if( c == '\0' )
				ctrl = white;
		}
	}
}

#else

long	ftell();

/*
 * strings
 */

struct	exec header;

char	*infile = "Standard input";
int	oflg;
int	asdata;
long	offset;
int	minlength = 4;

main(argc, argv)
	int argc;
	char *argv[];
{

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		register int i;
		if (argv[0][1] == 0)
			asdata++;
		else for (i = 1; argv[0][i] != 0; i++) switch (argv[0][i]) {

		case 'o':
			oflg++;
			break;

		case 'a':
			asdata++;
			break;

		default:
			if (!isdigit(argv[0][i])) {
				fprintf(stderr, "Usage: strings [ -a ] [ -o ] [ -# ] [ file ... ]\n");
				exit(1);
			}
			minlength = argv[0][i] - '0';
			for (i++; isdigit(argv[0][i]); i++)
				minlength = minlength * 10 + argv[0][i] - '0';
			i--;
			break;
		}
		argc--, argv++;
	}
	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			infile = argv[0];
			argc--, argv++;
		}
		fseek(stdin, (long) 0, 0);
		if (asdata ||
		    fread((char *)&header, sizeof header, 1, stdin) != 1 || 
		    N_BADMAG(header)) {
			fseek(stdin, (long) 0, 0);
			find((long) 100000000L);
			continue;
		}
		fseek(stdin, (long) N_TXTOFF(header)+header.a_text, 1);
		find((long) header.a_data);
	} while (argc > 0);
}

find(cnt)
	long cnt;
{
	static char buf[BUFSIZ];
	register char *cp;
	register int c, cc;

	cp = buf, cc = 0;
	for (; cnt != 0; cnt--) {
		c = getc(stdin);
		if (c == '\n' || dirt(c) || cnt == 0) {
			if (cp > buf && cp[-1] == '\n')
				--cp;
			*cp++ = 0;
			if (cp > &buf[minlength]) {
				if (oflg)
					printf("%7D ", ftell(stdin) - cc - 1);
				printf("%s\n", buf);
			}
			cp = buf, cc = 0;
		} else {
			if (cp < &buf[sizeof buf - 2])
				*cp++ = c;
			cc++;
		}
		if (ferror(stdin) || feof(stdin))
			break;
	}
}

dirt(c)
	int c;
{

	switch (c) {

	case '\n':
	case '\f':
		return (0);

	case 0177:
		return (1);

	default:
		return (c > 0200 || c < ' ');
	}
}
#endif
