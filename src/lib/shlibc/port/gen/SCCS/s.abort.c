h58777
s 00030/00000/00000
d D 1.1 86/07/31 10:24:51 fnf 1 0
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

#ident	"@(#)libc-port:gen/abort.c	1.11"
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
/*
 *	abort() - terminate current process with dump via SIGABRT
 */

#include <signal.h>

extern int kill(), getpid();
static pass = 0;		/* counts how many times abort has been called*/

int
abort()
{
	int (*sig)();

	if ((sig = signal(SIGABRT,SIG_DFL)) != SIG_DFL) 
		(void) signal(SIGABRT,sig); 
	else if (++pass == 1)
		_cleanup();
	return(kill(getpid(), SIGABRT));
}
E 1
