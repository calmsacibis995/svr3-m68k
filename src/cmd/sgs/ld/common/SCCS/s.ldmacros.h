h13222
s 00000/00000/00024
d D 1.3 86/08/18 08:45:06 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00024
d D 1.2 86/07/30 14:22:49 fnf 2 1
c Add macro based C debugging package macros.
e
s 00024/00000/00000
d D 1.1 86/07/29 14:31:48 fnf 1 0
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

#ident	"@(#)ld:common/ldmacros.h	1.5"
#define MAXKEEPSIZE	1024

extern char	stat_name[];
#if FLEXNAMES
#define	PTRNAME(x)	((x)->n_zeroes == 0L) \
				? (x)->n_nptr \
				: strncpy( stat_name, (x)->n_name, 8 )
#define SYMNAME(x)	(x.n_zeroes == 0L) \
				? x.n_nptr \
				: strncpy( stat_name, x.n_name, 8 )
#else
#define PTRNAME(x)	strncpy( stat_name, (x)->n_name, 8 )
#define SYMNAME(x)	strncpy( stat_name, x.n_name, 8 )
#endif

#define OKSCNNAME(x)	((tvflag && (!rflag || aflag)) || strncmp( x, _TV, 8 ))
E 1
