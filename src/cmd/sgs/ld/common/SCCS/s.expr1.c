h43089
s 00000/00000/00410
d D 1.3 86/08/18 08:44:41 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00053/00041/00357
d D 1.2 86/07/30 14:21:08 fnf 2 1
c Add macro based C debugging package macros.
e
s 00398/00000/00000
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

#ident	"@(#)ld:common/expr1.c	1.8"
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

static int expsect;
/*eject*/
ADDRESS
eval(p)
ENODE *p;
{

/*
 * Evaluate an expression node, returning the (long) value of the
 * expression.
 *
 * The extern variable "cur_dot" is used when processing exprs
 * inside of an output section specification, and is set to
 * -1L after allocation, when all sections have been relocated.
 */

	ADDRESS val1, val2;
	SYMTAB *symp;
	OUTSECT *s;
	ADDRESS findnext();

I 2
	DBUG_ENTER ("eval");
E 2
	switch (p->gnode.exop) {
	case INT:
D 2
		return( p->vnode.exvalue );
E 2
I 2
		DBUG_RETURN( p->vnode.exvalue );
E 2
	case DOT:
		/*
		 * Assign to DOT, if not a misuse of the assignment
		 */
		if( p->nnode.exsymptr == 0L ) {
			if( cur_dot == -1L ) {
				lderror(1, lineno, curfilnm,
					"misuse of DOT symbol in assignment instruction");
D 2
				return (0L);
E 2
I 2
				DBUG_RETURN (0L);
E 2
				}
D 2
			return(cur_dot);
E 2
I 2
			DBUG_RETURN(cur_dot);
E 2
			}
		else {
			/*
			 * Use of dot in assign to symbol
			 */
			symp = getsym(p->nnode.exsymptr);
			s = (OUTSECT *) symp->smscnptr;
			val1 = symp->sment.n_value + s->oshdr.s_paddr;
			if ( expsect == 0 )
				expsect = s->ossecnum;
D 2
			return (val1);
E 2
I 2
			DBUG_RETURN (val1);
E 2
			}
	case NAME:
		symp = getsym(p->nnode.exsymptr);
		val1 = symp->smnewval;
		if (symp->sment.n_scnum > 0) {
			if(expsect == 0)
				expsect = symp->sment.n_scnum;
			else
				expsect = -1;
			}
		else if( symp->sment.n_scnum == 0 )
			lderror(1, lineno, curfilnm,
				"undefined symbol in expression");
D 2
		return (val1);
E 2
I 2
		DBUG_RETURN (val1);
E 2
	case ALIGN:
		if( cur_dot == -1L ) {
			lderror(1, lineno, curfilnm,
				"ALIGN illegal in this context");
D 2
		return(0L);
E 2
I 2
		DBUG_RETURN(0L);
E 2
		}
		val2 = eval(p->gnode.exleft);
		val1 = (cur_dot + val2 - 1) & (~(val2-1));
D 2
		return(val1);
E 2
I 2
		DBUG_RETURN(val1);
E 2

	case PHY:
		/*
		 * If (cur_dot != -1L) here, then eval() was
		 * called from pboslist(), which is processing
		 * the output section list.  Moreover, we have
		 * an assignment to DOT, because eval() is not
		 * called otherwise.  The statement is
		 *	. = ... PHY(...) ...
		 * which is invalid.
		 */
		if( cur_dot != -1L ) {
			lderror(1, lineno, curfilnm,
				"illegal assignment of physical address to DOT");
D 2
			return (0L);
E 2
I 2
			DBUG_RETURN (0L);
E 2
			}
		expsect = -1;
		switch( p->gnode.exleft->gnode.exop ) {
		case NAME:
			symp = getsym(p->gnode.exleft->nnode.exsymptr);
			val1 = symp->smnewval;
			if ( symp->smscnptr == NULL ) {
				switch ( symp->sment.n_scnum ) {
				case -1 :	/* absolute */
					lderror(1, lineno, curfilnm,
						"phy of absolute symbol %s is illegal",
						SYMNAME(symp->sment));
					break;
				case 0 :	/* undefined */
					lderror(1, lineno, curfilnm,
						"symbol %s is undefined",
						SYMNAME(symp->sment));
					break;
				}
				}
D 2
			return ( val1 );
E 2
I 2
			DBUG_RETURN ( val1 );
E 2
		case DOT:
			if( p->gnode.exleft->nnode.exsymptr == 0L ) {
				lderror(1,lineno, curfilnm,
					"internal error: no symtab entry for DOT");
D 2
				return (0L);
E 2
I 2
				DBUG_RETURN (0L);
E 2
				}
			symp = getsym(p->gnode.exleft->nnode.exsymptr);
			val1 = symp->sment.n_value;
			val1 += ((OUTSECT *) symp->smscnptr)->oshdr.s_paddr;

D 2
			return ( val1 );
E 2
I 2
			DBUG_RETURN ( val1 );
E 2
		default:
			lderror(1, lineno, curfilnm,
				"operand of PHY must be a name");
D 2
			return(0L);
E 2
I 2
			DBUG_RETURN(0L);
E 2
		}


	case UMINUS:	val1 = eval(p->gnode.exleft);
D 2
			return(-val1);
E 2
I 2
			DBUG_RETURN(-val1);
E 2
	case NOT:	val1 = eval(p->gnode.exleft);
D 2
			return(!val1);
E 2
I 2
			DBUG_RETURN(!val1);
E 2
	case BNOT:	val1 = eval(p->gnode.exleft);
D 2
			return(~val1);
E 2
I 2
			DBUG_RETURN(~val1);
E 2
	case AND:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 & val2);
E 2
I 2
			DBUG_RETURN(val1 & val2);
E 2
	case ANDAND:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 && val2);
E 2
I 2
			DBUG_RETURN(val1 && val2);
E 2
	case DIV:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 / val2);
E 2
I 2
			DBUG_RETURN(val1 / val2);
E 2
	case EQEQ:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 == val2);
E 2
I 2
			DBUG_RETURN(val1 == val2);
E 2
	case GE:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 >= val2);
E 2
I 2
			DBUG_RETURN(val1 >= val2);
E 2
	case GT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 > val2);
E 2
I 2
			DBUG_RETURN(val1 > val2);
E 2
	case LE:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 <= val2);
E 2
I 2
			DBUG_RETURN(val1 <= val2);
E 2
	case LSHIFT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 << val2);
E 2
I 2
			DBUG_RETURN(val1 << val2);
E 2
	case LT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 < val2);
E 2
I 2
			DBUG_RETURN(val1 < val2);
E 2
	case MINUS:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 - val2);
E 2
I 2
			DBUG_RETURN(val1 - val2);
E 2
	case MULT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 * val2);
E 2
I 2
			DBUG_RETURN(val1 * val2);
E 2
	case NE:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 != val2);
E 2
I 2
			DBUG_RETURN(val1 != val2);
E 2
	case OR:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 | val2);
E 2
I 2
			DBUG_RETURN(val1 | val2);
E 2
	case OROR:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 || val2);
E 2
I 2
			DBUG_RETURN(val1 || val2);
E 2
	case PC:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 % val2);
E 2
I 2
			DBUG_RETURN(val1 % val2);
E 2
	case PLUS:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 + val2);
E 2
I 2
			DBUG_RETURN(val1 + val2);
E 2
	case RSHIFT:	val1 = eval(p->gnode.exleft);
			val2 = eval(p->gnode.exright);
D 2
			return(val1 >> val2);
E 2
I 2
			DBUG_RETURN(val1 >> val2);
E 2
	case SIZEOF:
	case ADDR:
		if (cur_dot != -1L)
		{
			lderror(1, lineno, curfilnm,
				"SIZEOF/ADDR illegal in this context" );
D 2
			return( 0L );
E 2
I 2
			DBUG_RETURN( 0L );
E 2
		}
		if ((s = fndoutsec( p->pnnode.symbol )) == NULL)
		{
			lderror(1, lineno, curfilnm,
				"output section %.8s not found",
				p->pnnode.symbol );
D 2
			return( 0L );
E 2
I 2
			DBUG_RETURN( 0L );
E 2
		}

		if (p->pnnode.exop == SIZEOF)
D 2
			return( s->oshdr.s_size );
E 2
I 2
			DBUG_RETURN( s->oshdr.s_size );
E 2
		else
		{
			if (s->oshdr.s_paddr == -1L)
			{
				lderror( 1, lineno, curfilnm,
					"section %.8s not yet allocated",
					p->pnnode.symbol );
D 2
				return( 0L );
E 2
I 2
				DBUG_RETURN( 0L );
E 2
			}
D 2
			return( s->oshdr.s_paddr );
E 2
I 2
			DBUG_RETURN( s->oshdr.s_paddr );
E 2
		}

	case NEXT:
		if (cur_dot != -1L)
		{
			lderror(1, lineno, curfilnm,
				"NEXT illegal in this context" );
D 2
			return( 0L );
E 2
I 2
			DBUG_RETURN( 0L );
E 2
		}
D 2
		return( findnext( (ADDRESS) p->vnode.exvalue ));
E 2
I 2
		DBUG_RETURN( findnext( (ADDRESS) p->vnode.exvalue ));
E 2

	default:
		lderror(1, lineno, curfilnm, "illegal operator in expression");
D 2
		return(0L);
E 2
I 2
		DBUG_RETURN(0L);
E 2
	}
}
/*eject*/
pexlist()
{

/*
 * After allocation, process the "expression list"
 * to define any user-specified symbols.
 */

I 2
	DBUG_ENTER ("pexlist");
E 2
	cur_dot = -1L;
	curexp = (ACTITEM *) explist.head;

	while( curexp ) {
		lineno = curexp->evexpr.aiinlnno;
		curfilnm = curexp->evexpr.aiinflnm;
		pexitem();
		clrexp( curexp );
		curexp = curexp->evexpr.ainext;
		}
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
pexitem()
{

/*
 * Process one assignment instruction
 */

	long val;
	register SYMTAB *sp;
	register OUTSECT *oscn;

I 2
	DBUG_ENTER ("pexitem");
E 2
	expsect = 0;
	val = eval(curexp->evexpr.aiexptr->gnode.exright);
	sp = getsym(curexp->evexpr.aiexptr->gnode.exleft->nnode.exsymptr);

/*
 * If the section number of the symbol we
 * are assigning a value is not zero ( = undefined),
 * the user is redefining the value of the symbol.
 */

	if( sp->sment.n_scnum )
		if( sp->smscnptr )
			lderror(0, curexp->evexpr.aiinlnno, curexp->evexpr.aiinflnm,
				"symbol %s from file %s being redefined",
				SYMNAME(sp->sment), sp->smscnptr->isfilptr->flname);
		else
			lderror(0, curexp->evexpr.aiinlnno, curexp->evexpr.aiinflnm,
				"absolute symbol %s being redefined",
				SYMNAME(sp->sment));

	if( expsect == 0 )
		sp->sment.n_scnum = -1;		/* absolute symbol */
	else
		sp->sment.n_scnum = expsect;

	if( (oscn = findoscn(sp->sment.n_scnum)) != NULL )
		sp->smnewvirt = val - oscn->oshdr.s_paddr + oscn->oshdr.s_vaddr;
	else
		sp->smnewvirt = val;
	sp->smnewval = val;
	sp->sment.n_sclass = C_EXT;

I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
clrexp(a)
ACTITEM *a;
{

/*
 * Free up all memory assigned to the assignment instruction pointed
 * to by "a"
 */
	ENODE *eptr;

I 2
	DBUG_ENTER ("clrexp");
E 2
	eptr = a->evexpr.aiexptr;

	switch( (unsigned) eptr->gnode.exop ) {
	case INT:
	case DOT:
	case NAME:
	case ADDR:
	case SIZEOF:
	case NEXT:
		break;
	default:
		if (eptr->gnode.exleft)
			clrlimb(eptr->gnode.exleft);
		if (eptr->gnode.exright)
			clrlimb(eptr->gnode.exright);
		break;
	}

	free( eptr );
	free( a );
I 2
	DBUG_VOID_RETURN;
E 2
}




clrlimb(eptr)
ENODE *eptr;
{

/*
 * Free up a non-root node of an expression tree
 */

I 2
	DBUG_ENTER ("clrlimb");
E 2
	switch( (unsigned) eptr->gnode.exop ) {
	case INT:
	case DOT:
	case NAME:
	case SIZEOF:
	case ADDR:
	case NEXT:
		break;

	default:
		if (eptr->gnode.exleft)
			clrlimb(eptr->gnode.exleft);
		if (eptr->gnode.exright)
			clrlimb(eptr->gnode.exright);
		break;
	}

	free( eptr );
I 2
	DBUG_VOID_RETURN;
E 2
}



ADDRESS
findnext( boundary )
	ADDRESS boundary;
{
	ANODE *sap;
	ADDRESS next_addr;

I 2
	DBUG_ENTER ("findnext");
E 2
	next_addr = boundary;
	for (sap = (ANODE *) avlist.head; sap; sap = sap->adnext)
	{
		if (sap->adtype != ADAVAIL)
			continue;
		while (sap->adpaddr > next_addr)
			next_addr += boundary;

		if (sap->adpaddr + (ADDRESS)sap->adsize > next_addr)
			break;
	}

	if (sap == NULL)
		lderror( 2, lineno, curfilnm,
			"boundary %d not available in configured memory",
			boundary );
	else
D 2
		return (next_addr);
E 2
I 2
		DBUG_RETURN (next_addr);
	DBUG_VOID_RETURN;
E 2
}
E 1
