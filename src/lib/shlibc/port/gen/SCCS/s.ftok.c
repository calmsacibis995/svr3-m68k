h50867
s 00023/00000/00000
d D 1.1 86/07/31 10:25:08 fnf 1 0
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

#ident	"@(#)libc-port:gen/ftok.c	1.4"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

key_t
ftok(path, id)
char *path;
char id;
{
	struct stat st;

	return(stat(path, &st) < 0 ? (key_t)-1 :
	    (key_t)((key_t)id << 24 | ((long)(unsigned)minor(st.st_dev)) << 16 |
		(unsigned)st.st_ino));
}
E 1
