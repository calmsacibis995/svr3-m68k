/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-inc:m68k/tv.h	1.3"
/*
 */

struct tventry {
	long	tv_addr;
	};

#define TVENTRY struct tventry
#define TVENTSZ sizeof(TVENTRY)
#ifdef m68k
#define MAXTVSIZE	0x7ffc		/* Maximum size of Mc68 SGS 
					   transfer vector using 
					   MC_IND16 reloc */
#else
#define N3BTVSIZE	0x20000		/* size of 1 segment (128K) */
#endif
