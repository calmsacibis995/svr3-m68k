/*
** Program to determine terminal type and download if necessary.
**
** Requires that the terminal really be an FT45.  That is, you must
** check with the shell for the proper TERM value *BEFORE* invoking
** this program.
**
** Optionally take a single argument: the file-name to download.
** This is used as the complete name of the file, so be sure you include
** any necessary leading pathname.  The original version took the argument
** as ONLY a pathname to the file "FT45DL", but GLG changed this.
**
** The exit code means:
**	0 = terminal successfully downloaded
**	1 = FT45 was already downloaded
**	2 = catastrophic error
** The distinction between 0 & 1 allows programs to send terminfo "is"
** strings correctly.
**
** original version by:
**  Bob Watson, Motorola Computer Systems, Cupertino, X2495, 07/09/86.
**
** modification history:
**	change so as to return useful exit code:	23 Jul 86 GLG
**	change to work with OfficeLAN remote login:	07 Nov 86 FGC
**
** hacker identities:
**	GLG = Greg Guerin, Motorola Computer Systems, Tempe, x5763
**	FGC = Fred Christiansen, Motorola Microcomputer Division, Tempe
*/

#include <termio.h>
#include <fcntl.h>

#define	ACK	6		/* ASCII ACK (^F) */
#define REG	register	/* for speed */
#define STDIN	0		/* standard input fd */
#define STDOUT	1		/* standard output fd */
 
struct termio	Io_save;	/* original termio struct restored at end */

main(argc, argv)
	int	argc;
	char	**argv;
{
	char		buf[10];
	struct termio	io_data;
REG	struct termio	*ip;		/* io_data pointer */
REG	int		tc;		/* total number of chars read */
REG	int		i;		/* number of chars read */
REG	char		*theFile;	/* file to be downloaded */

	/* Process args */

	if( argc > 1 )
		theFile	= argv[1];	/* use supplied download file-name */
	else
		theFile = "/usr/lib/term/FT45DL";	/* default */

	/* Set raw mode */

	ip	= &io_data;
	ioctl( 0, TCGETA, ip );		/* get current termio */
	Io_save	= io_data;		/* struct copy */

	ip->c_lflag	= ISIG;		/* no ICANON, use VMIN/VTIME */
	ip->c_cc[VMIN]	= 9;		/* need 9 chars or ... */
	ip->c_cc[VTIME]	= 15;		/* timeout on 1.5 secs between chars */

	ioctl( 0, TCSETAW, ip );	/* set new termio */

	/* Send terminal inquiry */

	write( STDOUT, "\033[0c", 4 );
	/* sleep( 1 ); /* V/68 needs sleep; read-delay not working right? */

	/* Read response, which may come one character at a time */

	tc = 0;
	while( tc < 9 )
	{
		if( ( i = read( STDIN, &buf[tc], 10 - tc ) ) < 0 )
		{
			printf("Unknown terminal\n");
			depart( 2 );
		}
		/* else */
		tc += i;
	}

	/* Check for ft45 response */

	if( strncmp( buf, "\033[2", 3 ) == 0 )
		depart( 1 );	/* already downloaded */

	/* else */
	if( strncmp( buf, "\033[1", 3 ) == 0 )
	{
		printf("Downloading ft45...\n");
		download( theFile );
		/* write( STDOUT, "\033P2;114i\134", 9 ); /* set block cursor */
		depart( 0 );
	}

	/* else */
	printf("Unknown terminal\n");
	depart( 2 );
}

/*
** depart:  reset terminal, and exit
*/

depart( result )
	int	result;		/* result code returned to parent process */
{
	ioctl( 0, TCFLSH, 0 );		/* flush input buffer (sent by FT45) */
	ioctl( 0, TCSETAW, &Io_save );	/* reset terminal settings */
	exit( result );
}

/*
** download:  download the terminal with filename's contents
*/

download( filename )
	char	*filename;	/* name of file to send to FT45 */
{
	char	buf[1024];
REG	char	*bp;		/* buf pointer */
REG	int	fp;
REG	int	len;

	if( ( fp = open( filename, O_RDONLY ) ) < 0 )
	{
		printf("Can't open download file %s\n", filename);
		depart( 2 );
	}

	/* else */
	bp = buf;
	while( ( len = read( fp, bp, sizeof( buf ) ) ) > 0 )
		write( STDOUT, bp, len );

	close( fp );

	sleep( 3 );	/* wait for ACK or NAK */

	return;  /* */

	/* !!! never reaches this code !!! */

	/*
	read( STDIN, bp, 10 );
	if ( bp[0] != ACK )
	{
		printf("Failed download\n");
		depart( 2 );
	}
	*/

	/* else return for caller to finish up... */
}
