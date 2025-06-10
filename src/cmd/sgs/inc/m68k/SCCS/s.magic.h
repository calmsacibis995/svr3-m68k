h07567
s 00000/00000/00027
d D 1.2 86/11/12 16:51:14 fnf 2 1
c Changes from integration of latest m68k compiler.
e
s 00027/00000/00000
d D 1.1 86/07/29 14:36:51 fnf 1 0
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

#ident	"@(#)sgs-inc:3b/magic.h	1.3"
/*
 */

/* normal unix user, text and data in separate segments = 410		*/
#define	NMAGIC		0410
/* text and data squashed together = 407				*/
#define	OMAGIC		0407
/* public library created by ldp					*/
#define	PLIBMAGIC	0370
/* data library created by ldp						*/
#define	DLIBMAGIC	0371
/* ldp created kernel process						*/
#define	KMAGIC		0401
/* ldp created supervisor process					*/
#define	SMAGIC		0402
/* ldp created user process						*/
#define	USRMAGIC	0403
/* sgen created boot process						*/
#define	BOOTMAGIC	0400
E 1
