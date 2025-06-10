h48905
s 00019/00000/00000
d D 1.1 86/07/31 12:27:01 fnf 1 0
c Pick up from Matts version for 5.3 m68k, apparently from 5.2 m32 version.
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

#ident	"@(#)libc-m32:gen/gen_def.c	1.2"
/*
 * Contains the definition of
 * the pointer to the imported symbols 
 * end and environ and the functions exit() and _cleanup()
 */

 int (* _libc_end) = 0;

 void (* _libc__cleanup)() = 0;

 char ** (* _libc_environ) = 0;
E 1
