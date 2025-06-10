h38502
s 00016/00000/00000
d D 1.1 86/07/31 10:25:04 fnf 1 0
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

#ident	"@(#)libc-port:gen/err_def.c	1.1"
/*
 * Pointer to symbols
 * sys_errlist and sys_nerr
 */


 char * (* _libc_sys_errlist) = 0;
 int    (* _libc_sys_nerr) = 0;
E 1
