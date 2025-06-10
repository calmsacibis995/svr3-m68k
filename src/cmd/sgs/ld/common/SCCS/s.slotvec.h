h22834
s 00000/00000/00032
d D 1.3 86/08/18 08:45:22 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00032
d D 1.2 86/07/30 14:24:16 fnf 2 1
c Add macro based C debugging package macros.
e
s 00032/00000/00000
d D 1.1 86/07/29 14:31:55 fnf 1 0
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

#ident	"@(#)ld:common/slotvec.h	1.9"


struct slotvec{
	long svsymndx;		/* original (input) symbol table index	*/
	long svovaddr;		/* original (input) symbol virtual addr	*/
	long svnvaddr;		/* new (output) symbol virtual addr	*/
	long svnewndx;		/* new (output) symbol table index	*/
	char svsecnum;		/* new (output) section number		*/
	char svflags;
	};

#define SLOTVEC struct slotvec
#define SLOTSIZ sizeof(SLOTVEC)

#if TRVEC
#define	SV_TV		01
#define	SV_ERR		02
#else
#define SV_TV		0
#define SV_ERR		0
#endif

extern SLOTVEC	*svread( );
#define GETSLOT(slt,action)	&svpageptr[(slt)]
E 1
