h01375
s 00002/00001/00043
d D 1.3 86/08/18 08:46:13 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00044
d D 1.2 86/07/30 14:28:13 fnf 2 1
c Add macro based C debugging package macros.
e
s 00044/00000/00000
d D 1.1 86/07/29 14:33:40 fnf 1 0
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

#ident	"@(#)ld:m32/system.h	1.9"
/*
 */

#define	USEREGIONS	0	/* allows allocation by regions */
#define	TRVEC	0		/* allows transfer vectors	*/
#define	UNIX	1		/* ld used as standard UNIX tool*/
#define	DMERT	0		/* ld used to generate DMERT obj*/
#define	B16		0		/* ld generates basic-16 object	*/
#define X86		0		/* ld generates extended 8086	*/
#define	N3B		0		/* ld generates 3B-20 object	*/
D 3
#define	M32		1		/* ld generates BELLMAC-32 obj	*/
E 3
I 3
#define	M32		0		/* ld generates BELLMAC-32 obj	*/
#define	MC68		1		/* ld generates Motorola m68k obj */
E 3
#define U3B 		0		/* ld generates 3B-20Simplex obj*/
#define	VAX		0		/* ld generates VAX 11/780 obj	*/
#define	PDP		0		/* ld generates PDP 11/70  obj	*/
#define	DEC		0		/* ld generates DEC object	*/
#define	IANDD	0		/* allows separate i and d	*/
#define SWABFMT	0		/* text in reverse order 	*/
#define PAGING		1	/* ld uses UNIX 6.0 paging	*/
#define	COMMON	1		/* handle .comm's		*/
#define NATIVE		1	/* ld creates a.out in native mode	*/

#ifndef TS
#define	TS		1		/* ld executed under UNIX-TS	*/
#endif

#ifndef RT
#define	RT		0		/* ld executed under UNIX-RT	*/
#endif

#ifndef PORT5AR
#define PORT5AR		0		/* 5.0 archive format 	  */
#endif

#ifndef PORTAR
#define PORTAR		0		/* defines archive format */
#endif
E 1
