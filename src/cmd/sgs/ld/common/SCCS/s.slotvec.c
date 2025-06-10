h23317
s 00000/00000/00139
d D 1.3 86/08/18 08:45:20 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00009/00003/00130
d D 1.2 86/07/30 14:24:09 fnf 2 1
c Add macro based C debugging package macros.
e
s 00133/00000/00000
d D 1.1 86/07/29 14:31:54 fnf 1 0
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

#ident	"@(#)ld:common/slotvec.c	1.14"
#include "system.h"

#include <stdio.h>
#include "structs.h"
#include "paths.h"
#include "slotvec.h"
#include "extrns.h"
I 2
#include "dbug.h"		/* Special macro based debugging package */
E 2


/********************************************************************
 *
 *  This module administers the slot vector used during output of the
 *  object file.  A slot is used to store the information needed to permit
 *  the relocation of all references to a single symbol.
 *
 *  Using a slot vector prevents the link editor from having to look
 *  up each symbol in the global symbol table in order to relocate it.
 *
 *  The size of the slot vector is dependant on the size of the largest
 *  input symbol table, since it is rebuilt for every input object file.
 *
 ********************************************************************/

SLOTVEC *svpageptr = NULL;		/* pointer to "page-in-core"
						array of slots */
unsigned svhighslt = 0;			/* index of next available slot */
static long svhighndx = 0L;		/* sym. tbl index of symbol assigned to
					    last assigned slot */
/*eject*/
svinit(numslots)
long numslots;		/* number of slots that are needed */
{
    char * calloc();

/*
 * Allocate a slot vector.
 */

	register int numpages;

I 2
	DBUG_ENTER ("svinit");
E 2
		if( (svpageptr = (SLOTVEC *) calloc((unsigned)numslots, (unsigned)SLOTSIZ)) == NULL ) {
			lderror(2,0,NULL,"fail to allocate %ld bytes for slotvec table",
			(long) numslots * (long) SLOTSIZ);
			}
I 2
	DBUG_VOID_RETURN;
E 2
}


svcreate(symindex, ovaddr, nvaddr, newindex, secnum, flags )
long symindex, ovaddr, nvaddr, newindex;
int secnum,flags;
{

/*
 * Get and initialize the next available slot in the slot vector
 */

	register SLOTVEC *p;

I 2
	DBUG_ENTER ("svcreate");
E 2
	p = GETSLOT(svhighslt++, WRTN);

	svhighndx = symindex;

	p->svsymndx = symindex;
	p->svovaddr = ovaddr;
	p->svnvaddr = nvaddr;
	p->svnewndx = newindex;
	p->svsecnum = secnum;
	p->svflags = flags;

I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
SLOTVEC *
svread(symndx)
long symndx;
{

/*
 * Return the pointer to the slot belonging to the symbol whose 
 * old (e.g., input) symbol table index is given by "symndx"
 *
 * Return NULL upon a failure to find a slot
 */

	register SLOTVEC *p;
	register unsigned high, low, diff, guess;

I 2
	DBUG_ENTER ("svread");
E 2
/*
 * Optimize: see if asking for the slot of the last symbol given a slot
 */

	if( symndx == svhighndx )
D 2
		return(GETSLOT(svhighslt-1, RNLY));
E 2
I 2
		DBUG_RETURN(GETSLOT(svhighslt-1, RNLY));
E 2

/*
 * Perform a binary search on the slot vector, looking for the 
 * requested slot
 */

	high = svhighslt - 1;
	low = 0;

	while( (diff = high - low) != 0 ) {
		guess = low + (diff / 2);
		p = GETSLOT(guess, RNLY);
		if( p->svsymndx == symndx )
D 2
			return(p);
E 2
I 2
			DBUG_RETURN(p);
E 2
		if( p->svsymndx > symndx ) { 
			if( high == guess )
				break;		/* not found */
			high = guess;
			}
		else {
			if( low == guess )
				break;		/* not found */
			low = guess;
			}
		}

/*
 * ERROR: the specified symbol was never assigned a slot
 */

D 2
	return( NULL );
E 2
I 2
	DBUG_RETURN( NULL );
E 2

}
E 1
