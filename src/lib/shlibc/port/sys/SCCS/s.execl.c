h45341
s 00022/00000/00000
d D 1.1 86/07/31 11:47:10 fnf 1 0
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

#ident	"@(#)libc-port:sys/execl.c	1.5"
/*
 *	execl(name, arg0, arg1, ..., argn, 0)
 *	environment automatically passed.
 */

#include "shlib.h"

execl(name, args)
char *name, *args;
{
	extern char **environ;

	return (execve(name, &args, environ));
}
E 1
