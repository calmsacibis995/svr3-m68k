
/*
	misc.c -- miscellaneous functions common to both create and upgrade

	Revision history:
	WJC	23-Jun-86	Allow for piped input.
	DS	9-May-86	replace most occurrences of BSIZE with 1024 and
				adjusted algorithms accordingly.
	DS	22-Apr-86	original VME version (David Schwartz)


Includes the following functions:

	char	ask( query, answers,...,0 ) --> first letter of chosen answer
	int		request( prompt, response, maxlen ) --> strlen(response)
	int		is_eov( bufp ) --> TRUE iff buffer at bufp is full of EOF_FLAG
	int		write_blk( fildes, buf, len, crcp ) --> bytes written
	int		read_blk( fildes, buf, len ) --> bytes read
	void	fatal1( msgp )
	void	warn1( msgp )
	int		get_bsize( devname ) --> -1, 512 or 1024, depending on the device
	long	get_media_size( devname_buf_p ) --> byte capacity of the device
*/


#ifndef MOD_misc
#	define MOD_misc
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "common.h"

static	char tape_media[]		= "tape cartridge"; 
static	char bigtape_media[]	= "9-track tape"; 
static	char floppy_media[]		= "floppy disk";
static	char bigfloppy_media[]	= "1.2MB floppy";
static	char cart_media[]		= "cartridge disk"; 

extern	int errno, sys_nerr;
extern	char *sys_errlist[];
extern	char * Whoami;


/*------------------------------------------------------------*/
char 				ask( query, answers )
/*------------------------------------------------------------*/
/*
	This routine displays a query on the terminal, then waits
	for the user to enter a choice.  The first letter of the
	entered choice is compared against the first letters of each
	of the answers, without regard to their case.  If one matches,
	its first letter is returned.

	If a null string is entered, the first letter of the first answer
	is returned.

	If no match is found, an error message is displayed and the 
	user is prompted with the query again.

	At least one answer is assumed to exist.

	Usage:

		resp = ask( "a question?", "yes", "no", 0 )

	where the first letter of each of the answers must be unique!
	You MUST use a literal zero (0) to terminate the answers list.
*/
char *query, *answers;
{
	register char ch, response, *ans, **sp, letters[25], buf[2];
	register int i, n_ans;


	for ( sp = &answers, i = 0; ans = *sp; sp++ )
	{
		*ans = toupper( *ans );
		letters[i++] = tolower( *ans );
		if (sizeof(letters) == i+1)
			break;
	}
	letters[i] = 0;
	n_ans = i;

	/* Prohibit type-ahead for interactive use, but allow for
	** piped input.
	*/
	if ( isatty (fileno(stdin)) )
		fflush( stdin );

	for ( response=0; !response; )
	{
		printf( "\n%s\n{", query );

		for ( sp = &answers, i=0;  ans = *sp; sp++ )
		{
			if (i++ > 0)
				printf( ", " );
			printf( "%s", ans );
		}
		printf( "}\n--> " );

		ch = tolower( (char) getchar() );
		if (ch == '\n')
			response = tolower( answers[0] );
		else
		{
			while (getchar() != '\n') ;
			for ( i=0; !response && i < n_ans; i++ )
				if (ch == tolower( letters[i] ))
					response = ch;
			
			if (!response)
			{
	printf("\007\nYou must enter the first letter/digit (upper or lower case)\n");
	printf("of one of the choices and a RETURN,");
	printf("\nor just a RETURN to select the first choice.\n" );
			}
		}
	}
	return( response );
}


/*----------------------------------------------------------------------*/
int					request( prompt, response, maxlen )
/*----------------------------------------------------------------------*/
/*	returns the length of the response string (0 if null)
/*
/*	if a null response is not desired, use something alont the lines of:
/*		while (!request(...)) ;
*/
char *prompt, *response;
int maxlen;
{
	char lnbuf[128], ch;
	register int i;

	printf( "\n%s", prompt );
	for (i = 0; (ch = getchar()) != '\n';)
		lnbuf[i++] = ch;
	
	i = MIN( i, maxlen - 1 );
	strncpy( response, lnbuf, i );
	response[i] = 0;
	return( i );
}

/*----------------------------------------------------------------------*/
int							is_eov( bufp )
/*----------------------------------------------------------------------*/
unsigned long *bufp;
{
	int i, eov;

	for (	eov = TRUE, i=1;
			i < NELTS(eov_blk) && (eov = eov && (bufp[i] == EOV_FLAG));
			i++)
		;
	return( eov );
}


/*----------------------------------------------------------------------*/
int					write_blk( fildes, buf, len, crcp )
/*----------------------------------------------------------------------*/

	/*	Note that this routine assumes that len never exceeds 1024!!! */

int fildes;
char *buf;
unsigned len;
unsigned long *crcp;
/* returns number of characters actually written -- should always be 1024 */
{
	char blk[1024];
	int i;

	for (i = 0; i < MIN( len, sizeof(blk) ); i++)		blk[i] = buf[i];
	for (i = len; i < sizeof(blk); i++)	blk[i] = 0;

	i = write( fildes, blk, sizeof(blk) );

	if (crcp)
		*crcp = checksum( crctable, blk, sizeof(blk), *crcp );

	return( i );
}


/*----------------------------------------------------------------------*/
int						read_blk( fildes, buf, len )
/*----------------------------------------------------------------------*/
int fildes;
char *buf;
unsigned len;
/* returns number of characters actually read in */
{
	char blk[1024];
	int i,j;

	i = read( fildes, blk, sizeof(blk) );
	if (i > 0)
		for (j=0; j < MIN( len, sizeof(blk) ); j++)
			buf[j] = blk[j];

	return( i );
}

/*----------------------------------------------------------------------*/
void						fatal1( msg )
/*----------------------------------------------------------------------*/
char *msg;
{
	fprintf( stderr, "\n%s: %s", Whoami, msg );
	if (errno)
	{
		fprintf( stderr, "\n  (errno = %d)", errno );
		if (errno <= sys_nerr)
			fprintf( stderr, ":  %s", sys_errlist[errno] );
	}
	fprintf( stderr, "\n" );
	close( media );
	pclose( in_stream );
	exit(1);
}



/*----------------------------------------------------------------------*/
void						warn1( msg )
/*----------------------------------------------------------------------*/
char *msg;
{
	fprintf( stderr, "\n%s: warning -- %s", Whoami, msg );
	if (errno)
	{
		fprintf( stderr, "\n  (errno = %d)", errno );
		if (errno <= sys_nerr)
			fprintf( stderr, ":  %s", sys_errlist[errno] );
	}
	fprintf( stderr, "\n" );
}

/*----------------------------------------------------------------------*/
int						get_bsize( devname )
/*----------------------------------------------------------------------*/
char *devname;	/* the name of the device in question (or a pathname on it) */
/* returns -1, 512 or 1024, depending on the inherent block size of device. */
{
	FILE *pipe;
	int fp, i;
	char dev[32], junk[1024], *errp;

	errno = 0;
	dev[0] = '/';
	for (i=1; i < sizeof(dev) && devname[i] && devname[i] != '/'; i++)
		dev[i] = devname[i];
	dev[i] = 0;
	strcat( dev, "/__tmp__" );

	sprintf( junk, "du -a %s", dev );

	errp = "creat";
	if ((fp = creat( dev, 777 )) == -1)
		goto failure;
	
	errp = "write";
	if (write_blk( fp, junk, 1024, 0 ) != 1024)
		goto failure;

	close( fp );

	errp = "popen";
	if (!(pipe = popen( junk, "r" )))
		goto failure;

	errp = "fscanf";
	if (fscanf( pipe, "%d", &i ) == EOF)
		goto failure;

	unlink( dev );

	if (i == 1)
		return( 1024 );
	else if (i == 2)
		return( 512 );

failure:
	sprintf( junk, "error in get_bsize: %s", errp );
	warn1( junk );
	return( -1 );
}

#define FLOPPY 631808		/* 617 * 1024 blks */
#define BIGFLOP 1200128		/* 1172 * 1024 blks */
#define CARTRIDGE 4950016	/* 4834 * 1024 blks */
#define STREAMTAPE 59499520	/* 58105 * 1024 blks */
#define BIGTAPE 59499520	/* Assume the same as a streamer tape for now! */

/*----------------------------------------------------------------------*/
unsigned long			get_media_size( devname_buf_p )
/*----------------------------------------------------------------------*/
char **devname_buf_p;	/* ptr to str buf in which a device name is returned */
{
	long msize;

	media_type = ask( "\nWhat type of media is this product distributed on?",
				floppy_media,
				bigfloppy_media,
				tape_media, 
				cart_media, 
				bigtape_media, 
				0 );
	switch (media_type)
	{
	case 'f':
		msize = FLOPPY;
		media_name = floppy_media;
		*devname_buf_p = "/dev/FLOPPY"; 
		break; 

	case '1':
		msize = BIGFLOP;
		media_name = bigfloppy_media;
		*devname_buf_p = "/dev/FLOPPY.MB";
		break;

	case 'c': 
		msize = CARTRIDGE;
		media_name = cart_media;
		*devname_buf_p = "/dev/IOMEGA";
		break; 

	case 't':
		msize = STREAMTAPE;
		media_name = tape_media;
		*devname_buf_p = "/dev/TAPE.CART";
		break;

	case '9':
		msize = BIGTAPE;
		media_name = bigtape_media;
		*devname_buf_p = "/dev/TAPE.9TRK";
		break;
	}

	blocks_per_volume = msize / 1024;

	can_format_media = (media_type == 'f' 
						|| media_type == '1' 
						|| media_type == 'c');

	return( msize );
}
