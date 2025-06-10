h51962
s 00022/00000/00000
d D 1.1 86/07/31 10:25:37 fnf 1 0
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

#ident	"@(#)libc-port:gen/pwent_def.c	1.1"
/*
 * Contains defintion
 * of pointers to the
 * functions getpwent(),
 * setpwent(), endpwent() and
 * fgetpwent.
 */

#include <pwd.h>

 struct passwd * (* _libc_getpwent)() = 0;
 struct passwd * (* _libc_fgetpwent)() = 0;
 void  (*_libc_setpwent)() = 0;
 void  (*_libc_endpwent)() = 0;
E 1
