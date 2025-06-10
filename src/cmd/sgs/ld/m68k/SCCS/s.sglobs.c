h39168
s 00000/00000/00037
d D 1.3 86/08/18 08:46:05 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00037
d D 1.2 86/07/30 14:27:52 fnf 2 1
c Add macro based C debugging package macros.
e
s 00037/00000/00000
d D 1.1 86/07/29 14:33:35 fnf 1 0
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

#ident	"@(#)ld:m32/sglobs.c	1.5"

#include <stdio.h>

#include "system.h"
#include "structs.h"
#include "tv.h"
#include "ldtv.h"
#include "sgs.h"

char	Xflag = 1;      /* generate optional header with "old"
/*eject*/
/*
 * Structure of information needed about the transfer vector (and the
 * .tv section).  Part of this structure is added to outsclst, so that
 * the list must not be freed before the last use of tvspec.
 */

TVINFO	tvspec = {
	NULL,		/* tvosptr */
	"",		/* tvfnfill: fill name for tv slots	*/
	-1L,		/* tvfill: fill value for tv slots	*/
	NULL,		/* tvinflnm: file containing tv specs	*/
	0,		/* tvinlnno: line nbr of tv directive	*/
	0,		/* tvlength: tv area length		*/
	-1L,		/* tvbndadr: tv area bond address	*/
	0, 0		/* tvrange				*/
	};

unsigned short magic = (unsigned short) MAGIC;
E 1
