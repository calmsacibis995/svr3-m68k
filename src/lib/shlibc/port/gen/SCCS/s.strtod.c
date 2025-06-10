h09560
s 00023/00000/00000
d D 1.1 86/07/31 10:25:50 fnf 1 0
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

#ident	"@(#)libc-port:gen/strtod.c	1.4"
/*LINTLIBRARY*/
/*
 *	C library - ascii to floating (atof) and string to double (strtod)
 *
 *	This version compiles both atof and strtod depending on the value
 *	of STRTOD, which is set in the file and may be overridden on the
 *	"cc" command line.  The only difference is the storage of a pointer
 *	to the character which terminated the conversion.
 *	Long-integer accumulation is used, except on the PDP11, where
 *	"long" arithmetic is simulated, so floating-point is much faster.
 */
#ifndef STRTOD
#define STRTOD	1
#endif
#include "atof.c"
E 1
