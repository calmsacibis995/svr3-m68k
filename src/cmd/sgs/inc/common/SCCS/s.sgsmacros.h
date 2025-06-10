h45300
s 00023/00000/00000
d D 1.1 86/07/29 14:34:51 fnf 1 0
c Baseline code from 5.3 release for 3b2
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

#ident	"@(#)sgs-inc:common/sgsmacros.h	1.4"
/*
 */

#define MAXLDIRS	8
#define COMF77		"(common)"

#define max(a,b)		(a<b ? b: a)
#define min(a,b)		(a>b ? b: a)

#define size(str)		(strlen(str) + 1)

#define equal(a,b,l)		(strncmp(a,b,l) == 0)

#define copy(a,b,l)		strncpy(a,b,l)

E 1
