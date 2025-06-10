h61265
s 00007/00001/00017
d D 1.2 86/11/12 16:51:13 fnf 2 1
c Changes from integration of latest m68k compiler.
e
s 00018/00000/00000
d D 1.1 86/07/29 14:35:36 fnf 1 0
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

D 2
#ident	"@(#)sgs-inc:m32/tv.h	1.3"
E 2
I 2
#ident	"@(#)sgs-inc:m68k/tv.h	1.3"
E 2
/*
 */

struct tventry {
	long	tv_addr;
	};

#define TVENTRY struct tventry
#define TVENTSZ sizeof(TVENTRY)
I 2
#ifdef m68k
#define MAXTVSIZE	0x7ffc		/* Maximum size of Mc68 SGS 
					   transfer vector using 
					   MC_IND16 reloc */
#else
E 2
#define N3BTVSIZE	0x20000		/* size of 1 segment (128K) */
I 2
#endif
E 2
E 1
