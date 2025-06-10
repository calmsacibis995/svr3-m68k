h39076
s 00004/00000/00054
d D 1.2 86/07/31 11:35:03 fnf 2 1
c Avoid conflict in MAXSIG definition with that in sys/signal.h
e
s 00054/00000/00000
d D 1.1 86/07/31 10:25:41 fnf 1 0
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

#ident	"@(#)libc-port:gen/ssignal.c	1.4"
/*LINTLIBRARY*/
/*
 *	ssignal, gsignal: software signals
 */
#include <signal.h>

/* Highest allowable user signal number */
I 2
/* avoid conflict from sys/signal.h */
#ifdef MAXSIG
#	undef	MAXSIG
#endif
E 2
#define MAXSIG 16

/* Lowest allowable signal number (lowest user number is always 1) */
#define MINSIG (-4)

/* Table of signal values */
static int (*sigs[MAXSIG-MINSIG+1])();

int
(*ssignal(sig, fn))()
register int sig, (*fn)();
{
	register int (*savefn)();

	if(sig >= MINSIG && sig <= MAXSIG) {
		savefn = sigs[sig-MINSIG];
		sigs[sig-MINSIG] = fn;
	} else
		savefn = SIG_DFL;

	return(savefn);
}

int
gsignal(sig)
register int sig;
{
	register int (*sigfn)();

	if(sig < MINSIG || sig > MAXSIG ||
				(sigfn = sigs[sig-MINSIG]) == SIG_DFL)
		return(0);
	else if(sigfn == SIG_IGN)
		return(1);
	else {
		sigs[sig-MINSIG] = SIG_DFL;
		return((*sigfn)(sig));
	}
}
E 1
