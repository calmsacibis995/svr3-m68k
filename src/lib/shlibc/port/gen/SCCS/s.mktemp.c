h38344
s 00049/00000/00000
d D 1.1 86/07/31 10:25:31 fnf 1 0
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

#ident	"@(#)libc-port:gen/mktemp.c	1.11"
/*LINTLIBRARY*/
/****************************************************************
 *	Routine expects a string of length at least 6, with
 *	six trailing 'X's.  These will be overlaid with a
 *	letter and the last (5) digigts of the proccess ID.
 *	If every letter (a thru z) thus inserted leads to
 *	an existing file name, your string is shortened to
 *	length zero upon return (first character set to '\0').
 ***************************************************************/
#define XCNT  6
#include "shlib.h"

extern int strlen(), access(), getpid();

char *
mktemp(as)
char *as;
{
	register char *s=as;
	register unsigned pid;
	register unsigned xcnt=0; /* keeps track of number of X's seen */

	pid = getpid();
	s += strlen(as);	/* point at the terminal null */
	while(*--s == 'X' && ++xcnt <= XCNT) {
		*s = (pid%10) + '0';
		pid /= 10;
	}
	if(*++s) {		/* maybe there were no 'X's */
		*s = 'a';
		while(access(as, 0) == 0) {
			if(++*s > 'z') {
				*as = '\0';
				break;
			}
		}
	} else
		if(access(as, 0) == 0)
			*as = '\0';
	return(as);
}
E 1
