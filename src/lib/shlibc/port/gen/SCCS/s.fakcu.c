h50729
s 00019/00000/00000
d D 1.1 86/07/31 10:25:06 fnf 1 0
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

#ident	"@(#)libc-port:gen/fakcu.c	1.4"
/*LINTLIBRARY*/
/*
 *       This is a dummy _cleanup routine to place at the end
 *       of the C library in case no other definition is found.
 *       If the standard I/O routines are used, they supply a
 *       real "_cleanup"  routine in file flsbuf.c
 */
void
_cleanup()
{
}
E 1
