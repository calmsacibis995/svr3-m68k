h01826
s 00034/00000/00000
d D 1.1 86/07/31 11:43:13 fnf 1 0
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

#ident	"@(#)libc-port:stdio/tmpfile.c	1.8"
/*LINTLIBRARY*/
/*
 *	tmpfile - return a pointer to an update file that can be
 *		used for scratch. The file will automatically
 *		go away if the program using it terminates.
 */
#include <stdio.h>

extern FILE *fopen();
extern int unlink();
extern char *tmpnam();
extern void perror();

FILE *
tmpfile()
{
	char	tfname[L_tmpnam];
	register FILE	*p;

	(void) tmpnam(tfname);
	if((p = fopen(tfname, "w+")) == NULL)
		return NULL;
	else
		(void) unlink(tfname);
	return(p);
}
E 1
