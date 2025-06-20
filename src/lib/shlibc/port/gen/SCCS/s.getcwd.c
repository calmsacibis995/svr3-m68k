h59432
s 00064/00000/00000
d D 1.1 86/07/31 10:25:10 fnf 1 0
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

#ident	"@(#)libc-port:gen/getcwd.c	1.8"
/*LINTLIBRARY*/
/*
 * Library routine to GET the Current Working Directory.
 * arg1 is a pointer to a character buffer into which the
 * path name of the current directory is placed by the
 * subroutine.  arg1 may be zero, in which case the 
 * subroutine will call malloc to get the required space.
 * arg2 is the length of the buffer space for the path-name.
 * If the actual path-name is longer than (arg2-2), or if
 * the value of arg2 is not at least 3, the subroutine will
 * return a value of zero, with errno set as appropriate.
 */

#include <stdio.h>
#include <sys/errno.h>

extern FILE *popen();
extern char *malloc(), *fgets(), *strchr();
extern int errno, pclose();

char *
getcwd(arg1, arg2)
char	*arg1;
int	arg2;
{
	FILE	*pipe;
	static char *cmd = "pwd 2>/dev/null";
	char	*trm;

	if(arg2 == 0) {
		errno = EINVAL;
		return(0);
	}
	if(arg1 == 0)
		if((arg1 = malloc((unsigned)arg2)) == 0) {
			errno = ENOMEM;
			return(0);
		}
	errno = 0;
	if((pipe = popen(cmd, "r")) == 0) {
		return(0);
	}
	errno = 0;
	if ((fgets(arg1, arg2, pipe)) == NULL) {
		errno = EACCES;
		 return(0);
		}
	(void) pclose(pipe);
	trm = strchr(arg1, '\0');
	if(*(trm-1) != '\n') {
		errno = ERANGE;
		return(0);
	}
	*(trm-1) = '\0';
	return(arg1);
}
E 1
