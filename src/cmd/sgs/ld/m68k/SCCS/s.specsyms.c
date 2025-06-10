h03335
s 00000/00000/00083
d D 1.3 86/08/18 08:46:12 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00004/00001/00079
d D 1.2 86/07/30 14:28:10 fnf 2 1
c Add macro based C debugging package macros.
e
s 00080/00000/00000
d D 1.1 86/07/29 14:33:39 fnf 1 0
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

#ident	"@(#)ld:m32/specsyms.c	1.4"

#include <stdio.h>

#include "system.h"
#include "structs.h"
#include "sgsmacros.h"
#include "extrns.h"
I 2
#include "dbug.h"		/* Special macro based debugging package */
E 2
set_spec_syms()
{

/*
 * For non-relocatable link edits, set up the special symbols.
 *
 *	1. _TORIGIN : segment register value for the REGION containing
 *			text
 *	   _DORIGIN : segment register value for the REGION containing
 *			data
 *	   _TVORIG  : segment register value for the REGION containing
 *			the tv
 *	
 *	2. _ETEXT : next available address after the end of the output
 *			section _TEXT
 *	   _EDATA : next available address after the end of the output
 *			section _DATA
 *	   _END   : next available address after the end of the output
 *			section _BSS
 *
 *	3. _SORIGIN : first address at or following _END which is 
 *			aligned to a 16-byte boundary
 *
 *	4. _TV : special symbol with auxiliary entry giving
 *		 tvlength and tvrange.
 */

	register OUTSECT *osptr;
	register long base;
	SYMTAB	*sym;
	AUXENT aux;

I 2
	DBUG_ENTER ("set_spec_syms");
E 2
	if ( ! aflag )
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2


	osptr = (OUTSECT *) outsclst.head;
	while (osptr) {
		if( equal(osptr->oshdr.s_name,_TEXT, 8))
			creatsym(_ETEXT,
				osptr->oshdr.s_paddr + osptr->oshdr.s_size);
		else if( equal(osptr->oshdr.s_name,_DATA, 8))
			creatsym(_EDATA,
				osptr->oshdr.s_paddr + osptr->oshdr.s_size);
		else if( equal(osptr->oshdr.s_name,_BSS, 8)) {
			base = osptr->oshdr.s_paddr + osptr->oshdr.s_size;
			creatsym(_END, base);
			}
#if TRVEC
		else if( equal(osptr->oshdr.s_name,_TV,8) ) {
			creatsym(_TV, ((tvspec.tvbndadr == -1L) ?
					0L : tvspec.tvbndadr));
			zero(&aux, AUXESZ);
			aux.x_tv.x_tvlen = tvspec.tvlength;
			aux.x_tv.x_tvran[0] = tvspec.tvrange[0];
			aux.x_tv.x_tvran[1] = tvspec.tvrange[1];
			aux.x_tv.x_tvfill = -1L;
			sym = findsym(_TV);
			putaux( makeaux(sym, &aux, 0), 1 );
			putsym(sym, 1);
			}
#endif
		osptr = osptr->osnext;
		}
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
