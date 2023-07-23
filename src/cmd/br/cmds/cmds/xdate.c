/*	@(#)date.c	1.5	*/
/*
** Modified by GLG to look at any file and apply formatting abilities
** to the modification-time of the file
** Used as 'xdate' in backup system
*/

static char sccsid[] = "@(#)date.c	1.3";
/*
**	xdate - with format and arbitrary-file capabilities
*/

#include	<sys/types.h>
#include	"utmp.h"
#include        <stdio.h>
#include	<time.h>
#include	<sys/stat.h>

#define	YEAR	itoa(tim->tm_year,cp,2)
#define	MONTH	itoa(tim->tm_mon+1,cp,2)
#define	DAY	itoa(tim->tm_mday,cp,2)
#define	HOUR	itoa(tim->tm_hour,cp,2)
#define	MINUTE	itoa(tim->tm_min,cp,2)
#define	SECOND	itoa(tim->tm_sec,cp,2)
#define	JULIAN	itoa(tim->tm_yday+1,cp,3)
#define	WEEKDAY	itoa(tim->tm_wday,cp,1)
#define	MODHOUR	itoa(h,cp,2)
#define	ZONE	timeZone(tim->tm_isdst,cp)
#define	dysize(A) (((A)%4)? 365: 366)

int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char	month[12][3] = {
	"Jan","Feb","Mar","Apr",
	"May","Jun","Jul","Aug",
	"Sep","Oct","Nov","Dec"
};

char	days[7][3] = {
	"Sun","Mon","Tue","Wed",
	"Thu","Fri","Sat"
};

int	fillChar = '0';			/* fill numbers with... */

char	*itoa();
char	*timeZone();
char	*Format = "%a %h %b%d %T %z 19%y";	/* default format */
char	*theFile = NULL;		/* file-name to use, or NULL */


main(argc, argv)
int	argc;
char	**argv;
{
	long	theTime;

	theFile = NULL;
	theTime = 0L;

/* 	printf( "argc = %d\n", argc ); */

	while ( --argc )
	{
		++argv;
		if ( **argv == '+'  ||  **argv == '-' )
			Format = *argv + 1;
		else
			theFile = *argv;
	}

	GetTime( theFile, &theTime );

	ShowTime( Format, theTime );

	exit( 0 );
}


ShowTime( format, theTime )
char	*format;
long	theTime;
{
	int	h, hflag, i;
	char	buf[200];
	register char *cp, *aptr, c;
	struct tm *tim;


	hflag = 0;
	for(cp=buf;cp< &buf[200];)
		*cp++ = '\0';

	tim = localtime( &theTime );

	aptr = format;
	cp = buf;

	while(c = *aptr++) {
		if(c == '%')
			switch(*aptr++) {
			case '%':
				*cp++ = '%';
				continue;
			case 'b':	/* blank-fill next number-item only */
				fillChar = ' ';
				continue;
			case 'f':	/* file-name string */
				if ( theFile != NULL )
				{
					strcpy( cp, theFile );
					cp += strlen( theFile );
				}
				continue;
			case 'n':
				*cp++ = '\n';
				continue;
			case 't':
				*cp++ = '\t';
				continue;
			case 'm' :
				cp = MONTH;
				fillChar = '0';
				continue;
			case 'd':
				cp = DAY;
				fillChar = '0';
				continue;
			case 'y' :
				cp = YEAR;
				fillChar = '0';
				continue;
			case 'z' :	/* time-zone string */
				cp = ZONE;
				fillChar = '0';
				continue;
			case 'D':
				cp = MONTH;
				*cp++ = '/';
				cp = DAY;
				*cp++ = '/';
				cp = YEAR;
				fillChar = '0';
				continue;
			case 'H':
				cp = HOUR;
				fillChar = '0';
				continue;
			case 'M':
				cp = MINUTE;
				fillChar = '0';
				continue;
			case 'S':
				cp = SECOND;
				fillChar = '0';
				continue;
			case 'T':
				cp = HOUR;
				*cp++ = ':';
				cp = MINUTE;
				*cp++ = ':';
				cp = SECOND;
				fillChar = '0';
				continue;
			case 'j':
				cp = JULIAN;
				fillChar = '0';
				continue;
			case 'w':
				cp = WEEKDAY;
				continue;
			case 'r':
				if((h = tim->tm_hour) >= 12)
					hflag++;
				if((h %= 12) == 0)
					h = 12;
				cp = MODHOUR;
				*cp++ = ':';
				cp = MINUTE;
				*cp++ = ':';
				cp = SECOND;
				*cp++ = ' ';
				if(hflag)
					*cp++ = 'P';
				else *cp++ = 'A';
				*cp++ = 'M';
				fillChar = '0';
				continue;
			case 'h':
				for(i=0; i<3; i++)
					*cp++ = month[tim->tm_mon][i];
				continue;
			case 'a':
				for(i=0; i<3; i++)
					*cp++ = days[tim->tm_wday][i];
				continue;
			default:
				(void) fprintf(stderr,
				  "xdate: bad format - %c\n", *--aptr);
				exit(2);
			}	/* endsw */
		*cp++ = c;
	}	/* endwh */

	*cp = '\n';
	(void) write( 1, buf, (cp - buf) + 1 );
}


char *
itoa(i,ptr,dig)			/* return new position of buffer end */
register  int	i;
register  char	*ptr;
register  int	dig;
{
	register int counter;

	/* Start at field end, work backwards */
	for ( ptr += dig, counter = dig; counter--; i /= 10 )
	{
		*--ptr = ( (i)	? i % 10 + '0'		/* convert digit... */
				: fillChar );		/* or use fillChar */
	}

	return ( ptr + dig );

/*	switch(dig)	{
		case 3:
			*ptr++ = i/100 + '0';
			i = i - i / 100 * 100;
		case 2:
			*ptr++ = i / 10 + '0';
		case 1:	
			*ptr++ = i % 10 + '0';
	}
	return(ptr);
*/	

}

char *
timeZone( isDST, cp )
int	isDST;		/* non-zero if DST in effect */
char	*cp;		/* where to put time-zone string */
{
	/*
	* Get TZ environment variable, then copy into buffer
	* Return new location of buffer pointer
	*/
	register char *p;	/* temp */

	tzset();			/* set tzname[] from env-var TZ */

/*	printf( "%d %s %s\n", isDST, tzname[0], tzname[1] ); */

	p = tzname[ isDST ? 1 : 0 ];	/* choose string */
	strcpy( cp, p );		/* copy to buffer */

	return( cp + strlen( p ) );	/* return new string end */
}


GetTime( theFile, where )
char	*theFile;	/* where to look for time */
long	*where;		/* put result here */
{
	struct stat theInfo;	/* get file-info here */

	if ( theFile == NULL )
		time( where );
	else
	{
/* 		printf( "File: %s\n", theFile ); */
		if ( stat( theFile, &theInfo ) )
		{
		    perror( theFile );
		    exit( 2 );
		}
		*where = theInfo.st_mtime;
	}
}
