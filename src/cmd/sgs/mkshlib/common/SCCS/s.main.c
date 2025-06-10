h23560
s 00011/00002/00110
d D 1.3 86/08/18 08:49:13 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00007/00001/00105
d D 1.2 86/08/11 09:54:45 fnf 2 1
c Checkpoint changes to date.
e
s 00106/00000/00000
d D 1.1 86/07/31 14:39:21 fnf 1 0
c Initial copy from 5.3 distribution for AT&T 3b2.
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/main.c	1.4"

#include <stdio.h>
#include <signal.h>
#include "paths.h"
#include "filehdr.h"
#include "ldfcn.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "shlib.h"
#include "hst.h"
#include "trg.h"
I 3
#include "dbug.h"		/* Special macro based debugging package */
E 3

main(argc, argv)
int	argc;
char	**argv;
{
I 3
	DBUG_ENTER ("main");
E 3
	progname= argv[0];
	prefix= getpref(progname);
	ldname= concat(prefix,"ld");
	asname= concat(prefix,"as");

	cmdline(argc, argv);	/* process the command line */
	mktmps();		/* get names of all temporary files */
	catchsigs();		/* catch signals to prevent leaving temp files */
	rdspec();		/* read shared lib. specification file */

	if (maketarg == TRUE)	/* make the target */
		mktarget();

	if (makehost == TRUE)	/* make the host */
		mkhost();
	rmtmps();		/* remove all the temporary files */
D 3
	return(0);
E 3
I 3
	DBUG_RETURN(0);
E 3
}

/* This routine processes the command line using getopt(3) */
void
cmdline(namec, namev)
int 	namec;
char	**namev;
{
	extern char	*optarg;
	extern int	optind;
	int	c;
	int	sflag=FALSE,
		tflag=FALSE,
		nflag=FALSE;

I 3
	DBUG_ENTER ("cmdline");
E 3
	/* process command line and set specfnam */
D 2
	while ((c = getopt(namec, namev, "s:t:h:nq")) != EOF)
E 2
I 2
D 3
	while ((c = getopt(namec, namev, "s:t:h:nqvd")) != EOF)
E 3
I 3
	while ((c = getopt(namec, namev, "s:t:h:nqvd#:")) != EOF)
E 3
E 2
		switch (c) {
I 3
			case '#':		/* C debug package */
				DBUG_PUSH (optarg);
				break;
E 3
I 2
			case 'd':		/* debug flag */
				dflag=TRUE;
				break;
			case 'v':		/* verbose flag */
				vflag=TRUE;
				break;
E 2
			case 's':
				sflag=TRUE;
				specname= optarg;
				break;
			case 't':
				tflag=TRUE;
				trgname= optarg;
				break;
			case 'h':
				makehost= TRUE;
				hstname= optarg;
				break;
			case 'n':
				nflag=TRUE;
				break;
			case 'q':
				qflag=TRUE;
				break;
			case '?':
			default:
				usagerr();
		}

	if (!sflag) {
		(void)fprintf(stderr,"The -s option must be supplied.\n");
		usagerr();
	}

	if (!tflag) {
		(void)fprintf(stderr,"The -t option must be supplied.\n");
		usagerr();
	}

	if (optind < namec)
		usagerr();

	if (!nflag)
		maketarg= TRUE;
I 3
	DBUG_VOID_RETURN;
E 3
}

void
usagerr()
{
I 3
	DBUG_ENTER ("usagerr");
E 3
	(void)fprintf(stderr,"usage: %smkshlib -s specfil -t target [-h host] [-n]\n", prefix);
	exit(1);
I 3
	DBUG_VOID_RETURN;
E 3
}
E 1
