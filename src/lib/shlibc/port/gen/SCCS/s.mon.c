h38322
s 00120/00000/00000
d D 1.1 86/07/31 10:25:33 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
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

#ident	"@(#)libc-port:gen/mon.c	2.10"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 *	Environment variable PROFDIR added such that:
 *		If PROFDIR doesn't exist, "mon.out" is produced as before.
 *		If PROFDIR = NULL, no profiling output is produced.
 *		If PROFDIR = string, "string/pid.progname" is produced,
 *		  where name consists of argv[0] suitably massaged.
 */
#include <mon.h>
#define PROFDIR	"PROFDIR"

extern int creat(), write(), close(), getpid();
extern void profil(), perror();
extern char *getenv(), *strcpy(), *strrchr();

char **___Argv = NULL; /* initialized to argv array by mcrt0 (if loaded) */
char *_countbase;

static int ssiz;
static struct hdr *sbuf;
static char mon_out[100];
static char progname[15];

void
monitor(lowpc, highpc, buffer, bufsize, nfunc)
char	*lowpc, *highpc;	/* boundaries of text to be monitored */
WORD	*buffer;	/* ptr to space for monitor data (WORDs) */
int	bufsize;	/* size of above space (in WORDs) */
int	nfunc;		/* max no. of functions whose calls are counted
			(default nfunc is 300 on PDP11, 600 on others) */
{
	int scale;
	long text;
	register char *s, *name = mon_out;

	if (lowpc == NULL) {		/* true only at the end */
		if (sbuf != NULL) {
			register int pid, n;
			int fd;

			if (progname[0] != '\0') { /* finish constructing
						    "PROFDIR/pid.progname" */
			    /* set name to end of PROFDIR */
			    name = strrchr(mon_out, '\0');  
			    if ((pid = getpid()) <= 0) /* extra test just in case */
				pid = 1; /* getpid returns something inappropriate */
			    for (n = 10000; n > pid; n /= 10)
				; /* suppress leading zeros */
			    for ( ; ; n /= 10) {
				*name++ = pid/n + '0';
				if (n == 1)
				    break;
				pid %= n;
			    }
			    *name++ = '.';
			    (void)strcpy(name, progname);
			}

			profil((char *)NULL, 0, 0, 0);
			if ((fd = creat(mon_out, 0666)) < 0 ||
			    write(fd, (char *)sbuf, (unsigned)ssiz) != ssiz)
				perror(mon_out);
			if (fd >= 0)
				(void) close(fd);
		}
		return;
	}
	_countbase = (char *)buffer + sizeof(struct hdr);
	sbuf = NULL;
	ssiz = (sizeof(struct hdr) + nfunc * sizeof(struct cnt))/sizeof(WORD);
	if (ssiz >= bufsize || lowpc >= highpc)
		return;
	if ((s = getenv(PROFDIR)) == NULL) /* PROFDIR not in environment */
		(void)strcpy(name, MON_OUT); /* use default "mon.out" */
	else if (*s == '\0') /* value of PROFDIR is NULL */
		return; /* no profiling on this run */
	else { /* set up mon_out and progname to construct
		  "PROFDIR/pid.progname" when done profiling */

		while (*s != '\0') /* copy PROFDIR value (path-prefix) */
			*name++ = *s++;
		*name++ = '/'; /* two slashes won't hurt */
		if (___Argv != NULL) /* mcrt0.s executed */
			if ((s = strrchr(___Argv[0], '/')) != NULL)
			    strcpy(progname, s + 1);
			else
			    strcpy(progname, ___Argv[0]);
	}
	sbuf = (struct hdr *)buffer;	/* for writing buffer at the wrapup */
	sbuf->lpc = lowpc;	/* initialize the first */
	sbuf->hpc = highpc;	/* region of the buffer */
	sbuf->nfns = nfunc;
	buffer += ssiz;			/* move ptr past 2'nd region */
	bufsize -= ssiz;		/* no. WORDs in third region */
					/* no. WORDs of text */
	text = (highpc - lowpc + sizeof(WORD) - 1)/
			sizeof(WORD);
	/* scale is a 16 bit fixed point fraction with the decimal
	   point at the left */
	if (bufsize < text)  {
		/* make sure cast is done first! */
		double temp = (double)bufsize;
		scale = (temp * (long)0200000L) / text;
	} else  {
		/* scale must be less than 1 */
		scale = 0xffff;
	}
	bufsize *= sizeof(WORD);	/* bufsize into # bytes */
	ssiz = ssiz * sizeof(WORD) + bufsize;	/* size into # bytes */
	profil((char *)buffer, bufsize, (int)lowpc, scale);
}
E 1
