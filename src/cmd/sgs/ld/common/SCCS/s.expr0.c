h25945
s 00000/00000/00098
d D 1.3 86/08/18 08:44:39 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00007/00003/00091
d D 1.2 86/07/30 14:21:03 fnf 2 1
c Add macro based C debugging package macros.
e
s 00094/00000/00000
d D 1.1 86/07/29 14:31:37 fnf 1 0
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

#ident	"@(#)ld:common/expr0.c	1.8"
#include "system.h"

#include <stdio.h>
#include "structs.h"
#include "extrns.h"
#include "sgsmacros.h"
#include "y.tab.h"
#include "ldmacros.h"
I 2
#include "dbug.h"		/* Special macro based debugging package */
E 2
/*eject*/
ENODE *
buildtree(op,l,r)
int op;
ENODE *l,*r; 
{
 
/*
 * Build a node in the expression tree and
 * return a pointer to it
 */

	register ENODE *p;

I 2
	DBUG_ENTER ("buildtree");
E 2
	p = (ENODE *) myalloc(sizeof(ENODE));

	p->gnode.exop = op;
	p->gnode.exleft = l;
	p->gnode.exright = r;
D 2
	return(p);
E 2
I 2
	DBUG_RETURN(p);
E 2
}
/*eject*/
ENODE *
symnode(symp)
char *symp;
{

/*
 * Build an expression node for NAME or DOT
 */

	register ENODE *p;
	SYMENT sym;

I 2
	DBUG_ENTER ("symnode");
E 2
	p = (ENODE *) myalloc(sizeof(ENODE));

	if( symp == NULL ) {		/* special DOT symbol */
		p->nnode.exop = DOT;
		p->nnode.exsymptr = 0;
		}
	else {
		p->nnode.exop = NAME;
		zero( (char *) &sym, SYMESZ );
#if FLEXNAMES
		if (strlen(symp) > 8) {
			sym.n_zeroes = 0L;
			sym.n_nptr = symp;
			}
		else
#endif
			copy(sym.n_name,symp,8);
		sym.n_type = T_INT;
		sym.n_sclass = C_EXT;
		p->nnode.exsymptr = (makesym(&sym, NULL))->smmyacid;
		}

D 2
	return(p);
E 2
I 2
	DBUG_RETURN(p);
E 2
}


/*
 *	Build an expression node for SIZEOF(name) or ADDR(name)
 */

ENODE *
sectnode( op, name )
	int op;
	char *name;
{
	register ENODE *p;

I 2
	DBUG_ENTER ("sectnode");
E 2
	p = (ENODE *) myalloc(sizeof(ENODE));

	p->pnnode.exop = op;
	copy( p->pnnode.symbol, name, 8 );

D 2
	return(p);
E 2
I 2
	DBUG_RETURN(p);
E 2
}
E 1
