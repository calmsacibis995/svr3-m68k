h38239
s 00019/00000/00000
d D 1.1 86/07/31 11:42:39 fnf 1 0
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

#ident	"@(#)libc-port:stdio/fgetc.c	1.6"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include "shlib.h"
#include <stdio.h>

int
fgetc(fp)
register FILE *fp;
{
	return(getc(fp));
}
E 1
