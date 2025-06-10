h50243
s 00025/00000/00000
d D 1.1 86/07/31 11:47:12 fnf 1 0
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

#ident	"@(#)libc-port:sys/execv.c	1.6"
/*
 *	execv(file, argv)
 *
 *	where argv is a vector argv[0] ... argv[x], NULL
 *	last vector element must be NULL
 *	environment passed automatically
 */


execv(file, argv)
	char	*file;
	char	**argv;
{
	extern	char	**environ;

	return(execve(file, argv, environ));
}
E 1
