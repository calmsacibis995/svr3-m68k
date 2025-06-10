h45942
s 00000/00000/00059
d D 1.3 86/08/18 08:46:06 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00005/00000/00054
d D 1.2 86/07/30 14:27:54 fnf 2 1
c Add macro based C debugging package macros.
e
s 00054/00000/00000
d D 1.1 86/07/29 14:33:36 fnf 1 0
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

#ident	"@(#)ld:m32/special1.c	1.4"

#include <stdio.h>

#include "system.h"
#include "structs.h"
#include "extrns.h"
I 2
#include "dbug.h"		/* Special macro based debugging package */
E 2



#if TRVEC
void
chktvorg(org, tvbndadr)

long	org, *tvbndadr;
{
	/*
	 * check user-supplied .tv origin for legality
	 * if illegal, side-effect tvspec.tvbndadr
	 *  and issue warning message
	 */

I 2
	DBUG_ENTER ("chktvorg");
E 2
			if( (org & 0xf) != 0 )
				yyerror("tv origin (%10.0lx) must be a multiple of 16", org);
			*tvbndadr = (org + 0xfL) & ~0xfL;

I 2
	DBUG_VOID_RETURN;
E 2
}
#endif

void
specflags(flgname, argptr)
char *flgname;
char **argptr;
{

I 2
	DBUG_ENTER ("specflags");
E 2
	/*
	 * process special flag specifications for m32ld
	 * these flags have fallen through switch of argname in ld00.c
	 */

	switch ( *flgname ) {

		default:
			yyerror("unknown flag: %s", flgname);

		}
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
