h42900
s 00022/00000/00000
d D 1.1 86/07/31 11:47:11 fnf 1 0
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

#ident	"@(#)libc-port:sys/execle.c	1.6"
/*
 *	execle(file, arg1, arg2, ..., 0, envp)
 */

execle(file, args)
	char	*file;
	char	*args;			/* first arg */
{
	register  char  **p;

	p = &args;
	while(*p++);
	return(execve(file, &args, *p));
}
E 1
