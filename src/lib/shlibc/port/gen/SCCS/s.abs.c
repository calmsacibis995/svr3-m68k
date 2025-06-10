h36932
s 00018/00000/00000
d D 1.1 86/07/31 10:24:52 fnf 1 0
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

#ident	"@(#)libc-port:gen/abs.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include "shlib.h"

int
abs(arg)
register int arg;
{
	return (arg >= 0 ? arg : -arg);
}
E 1
