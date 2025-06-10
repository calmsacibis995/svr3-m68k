h56076
s 00024/00000/00000
d D 1.1 86/07/31 11:43:08 fnf 1 0
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

#ident	"@(#)libc-port:stdio/stdio_def.c	1.1"
/*
 * Pointers to imported
 * symbols _sibuf, _sobuf, _smbuf
 * _iob and _bufendtab and _lastbuf
 */
#include <stdio.h>

 unsigned char (* _libc__sibuf)[] = 0, (* _libc__sobuf)[] = 0;

 unsigned char (* _libc__smbuf)[][_SBFSIZ] = 0;

 FILE (* _libc__iob)[] = 0;

 FILE * (* _libc__lastbuf) = 0;

 unsigned char * (* _libc__bufendtab)[] = 0; 
E 1
