h43525
s 00017/00000/00000
d D 1.1 86/07/31 10:25:23 fnf 1 0
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

#ident	"@(#)libc-port:gen/mall_def.c	1.2"
/*
 * Contains the definitions
 * of the pointers to the
 * malloc, free, realloc and _assert
 */

 char * (* _libc_malloc)() = 0;
 char * (* _libc_realloc)() = 0;
 void   (* _libc_free)() = 0;
E 1
