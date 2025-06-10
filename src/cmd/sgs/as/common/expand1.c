/*	@(#)expand1.c	6.1		*/
#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "expand.h"
#include "gendefs.h"
#include "dbug.h"

extern rangetag range[];
extern char idelta[];
extern char pcincr[];


#ifndef	MAXSS
#define MAXSS	200	/* maximum number of Selection set entries */
#endif
#ifndef	MAXSDI
#define MAXSDI	4000	/* maximum number of SDI's we can handle */
#endif
#ifndef	MAXLAB
#define MAXLAB	1000	/* max. number of labels whose address depends on SDI's */
#endif

char islongsdi[MAXSDI];

static ssentry selset[MAXSS];
static symbol *labset[MAXLAB];

unsigned short sdicnt = 0;
static short ssentctr = -1,
		labctr = -1;

static unsigned short PCmax;

extern symbol *dot;
extern long newdot;

update(ssptr,sditype)
	ssentry *ssptr;
	short sditype;
{
	register ssentry *ptr;
	register short cntr;
	register symbol *lptr;
	register symbol **ptr2;
	register unsigned short delta,
		sdipos;
	long instaddr;

	DBUG_ENTER ("update");
	delta = idelta[ssptr->itype];
	sdipos = ssptr->sdicnt;
	instaddr = ssptr->minaddr;
	PCmax -= delta;
	dot->maxval -= delta;

	if (sditype) {	/* nonzero if short */
		instaddr += ssptr->maxaddr;
		for (cntr = ssentctr, ptr = &selset[0]; cntr-- >= 0; ++ptr) {
			if (ptr->sdicnt > sdipos)
				ptr->maxaddr -= delta;
		}
		for (cntr = labctr, ptr2 = &labset[0]; cntr-- >= 0; ) {
			lptr = *ptr2;
			if (lptr->value + lptr->maxval > instaddr) {
				lptr->maxval -= delta;
				if (lptr->maxval == 0) {
					*ptr2 = labset[labctr--];
					continue;
				}
			}
			ptr2++;
		}
	}
	else {	/* long */
		dot->value += delta;
		newdot += delta;
		islongsdi[sdipos] = (char)delta;
		for (cntr = ssentctr, ptr = &selset[0]; cntr-- >= 0; ++ptr) {
			if (ptr->sdicnt > sdipos) {
				ptr->minaddr += delta;
				ptr->maxaddr -= delta;
			}
		}
		for (cntr = labctr, ptr2 = &labset[0]; cntr-- >= 0; ) {
			lptr = *ptr2;
			if (lptr->value > instaddr) {
				lptr->value += delta;
				lptr->maxval -= delta;
				if (lptr->maxval == 0) {
					*ptr2 = labset[labctr--];
					continue;
				}
			}
			ptr2++;
		}
	}
	DBUG_VOID_RETURN;
}

/*
 *	"notdone" is used to indicate when pass 1 has been completed.
 *	This helps to detect undefined externals.
 */

static short notdone = YES;

/*
 *	"overflow" is used to indicate when the maximum number of
 *	SDI's have been received (MAXSDI). When "overflow" becomes
 *	non-zero, then only "expand" is called to optimize the
 *	SDI's that have already been received.
 */

static short overflow = NO;

sdiclass(sdiptr)
	register ssentry *sdiptr;
{
	register symbol *lptr;
	register short itype;
	register short ltype;
	register long span;

	DBUG_ENTER ("sdiclass");
	lptr = sdiptr->labptr;
	itype = sdiptr->itype;
	if ((ltype = lptr->styp & TYPE) != UNDEF) {
		if (ltype != TXT
#if MULTSECT && MC68
		     || lptr->sectnum != 0
#endif
		   )
			DBUG_RETURN(L_SDI);
		span = lptr->value;
	}
	else {
		if (notdone == NO)
			DBUG_RETURN(L_SDI);
		span = (dot->value != sdiptr->minaddr) ? dot->value :
			sdiptr->minaddr + pcincr[itype];
	}
	span += sdiptr->constant - (sdiptr->minaddr + pcincr[itype]);
	if ((span < range[itype].lbound) ||
		(span > range[itype].ubound))	/* definitely long */
		DBUG_RETURN(L_SDI);
	else {
		if (ltype != UNDEF) {
			span += (int)(lptr->maxval) -
				((lptr->value > sdiptr->minaddr) ?
				(sdiptr->maxaddr + idelta[itype]) : sdiptr->maxaddr);
			if ((span >= range[itype].lbound) &&
				(span <= range[itype].ubound))	/* definitely short */
				DBUG_RETURN(S_SDI);
		}
	}
	DBUG_RETURN(U_SDI);
}

expand()
{
	register short change = YES;
	register short cntr;
	register ssentry *current;
	register short sditype;

	DBUG_ENTER ("expand");
	while (change) {
		change = NO;
		for (cntr = ssentctr, current = &selset[0]; cntr-- >= 0; ) {
			if ((sditype = sdiclass(current)) != U_SDI) {

				change = YES;
				update(current,sditype);
				*current = selset[ssentctr--];
			}
			else
				current++;
		}
	}
	DBUG_VOID_RETURN;
}

punt()
{
	register short cntr;
	register ssentry *current;
	register unsigned firstsdi;
	register ssentry *outptr;

	DBUG_ENTER ("punt");
	firstsdi = sdicnt;
	for (cntr = ssentctr, current = &selset[0]; cntr-- >= 0; ++current) {
		if (current->sdicnt < firstsdi) {
			firstsdi = current->sdicnt;
			outptr = current;
		}
	}
	update(outptr,L_SDI);
	*outptr = selset[ssentctr--];
	expand();
	DBUG_VOID_RETURN;
}

newlab(sym)
	register symbol *sym;
{
	static short labwarn = YES;

	DBUG_ENTER ("newlab");
	if (++labctr == MAXLAB) {
		if (labwarn == YES) {
			werror("Table overflow: some optimizations lost (Labels)");
			labwarn = NO; /* don't warn again */
		}
		labctr--;	/* gone too far, back up */
		while (labctr == MAXLAB - 1) {
			punt();
		}	/* continue to punt until we free a label */
		labctr++;	/* now point to a free area */
	}

	labset[labctr] = sym;
	DBUG_VOID_RETURN;
}

deflab(sym)
	register symbol *sym;
{
	DBUG_ENTER ("deflab");
	sym->maxval = PCmax;
	if (ssentctr >= 0) {
		newlab(sym);
		expand();
	}
	DBUG_VOID_RETURN;
}

#if !ONEPROC
extern FILE *fdlong;
#endif

fixsyms()
{
	DBUG_ENTER ("fixsyms");
	notdone = NO;
	expand();
#if !ONEPROC
	fwrite(islongsdi,sizeof(*islongsdi),(int)(++sdicnt),fdlong);
#endif
	DBUG_VOID_RETURN;
}

sdi(sym,const,itype)
	register symbol *sym;
	long const;
	register short itype;
{
	register long span;

	DBUG_ENTER ("sdi");
	if (sym) {
		if ((sym->styp & TYPE) == UNDEF)
			DBUG_RETURN(U_SDI);
		if ((sym->styp & TYPE) != TXT
#if MULTSECT && MC68
		     || sym->sectnum != 0
#endif
 		   )
			DBUG_RETURN(L_SDI);
	}
	else
		DBUG_RETURN(L_SDI);

	span = sym->value + const - (dot->value + pcincr[itype]);
	if ((span < range[itype].lbound) ||
		(span > range[itype].ubound))	/* definitely long */
		DBUG_RETURN(L_SDI);
	else {
		/*
		 *	Now determine if the instruction is definitely
		 *	short. This calculation is different from the
		 *	similar calculation in "sdiclass" since, as an
		 *	optimization, the long form of the instruction
		 *	is not included in the calculation. This prevents
		 *	determining that the size of an instruction
		 *	is uncertain based on its maximum length. Note
		 *	any previous uncertain instruction lengths
		 *	are reflected by "dot->maxval".
		 */
		span += (long)((int)(sym->maxval) - dot->maxval);
		if ((span >= range[itype].lbound) &&
			(span <= range[itype].ubound))	/* definitely short */
			DBUG_RETURN(S_SDI);
	}
	DBUG_RETURN(U_SDI);
}

newsdi(sym,const,itype)
	register symbol *sym;
	long const;
	short itype;
{
	register ssentry *nsdi;
	static short
		sdiwarn = YES,
		selwarn = YES;

	DBUG_ENTER ("newsdi");
	if (++sdicnt == MAXSDI) {
		if (sdiwarn == YES) {
			werror("Table overflow: some optimizations lost (SDIs)");
			sdiwarn = NO; /* don't warn again */
		}
		overflow = YES;
		sdicnt--;
	}
	if (++ssentctr == MAXSS) {
		if (selwarn == YES) {
			werror("Table overflow: some optimizations lost (SelSet)");
			selwarn = NO; /* don't warn again */
		}
		ssentctr--;	/* gone too far, back up */
		punt();
		ssentctr++;	/* one sdi was removed, now point to free area */
	}
	nsdi = &selset[ssentctr];
	nsdi->sdicnt = sdicnt;
	nsdi->itype = itype;
	nsdi->minaddr = dot->value;
	nsdi->maxaddr = PCmax;
	nsdi->constant = const;
	nsdi->labptr = sym;
	DBUG_VOID_RETURN;
}

shortsdi(sym,const,itype)
	register symbol *sym;
	long const;
	register short itype;
{
	register int sditype;

	DBUG_ENTER ("shortsdi");
	if (!overflow) {
		if (sym && ((sym->styp & TYPE) == UNDEF)) {
			if (dot->styp != TXT
#if MULTSECT && MC68
			    || dot->sectnum != 0
#endif
 			   )
				DBUG_RETURN(L_SDI);
			sditype = U_SDI;
			newsdi(sym,const,itype);
			PCmax += idelta[itype];
			dot->maxval = PCmax;
		}
		else {
			if ((sditype = sdi(sym,const,itype)) == U_SDI) {
				if (dot->styp != TXT
#if MULTSECT && MC68
				    || dot->sectnum != 0
#endif
				   )
					DBUG_RETURN(L_SDI);
				if (dot == sym) {
					sditype = S_SDI;
					goto ret;
				}
				newsdi(sym,const,itype);
				PCmax += idelta[itype];
				dot->maxval = PCmax;
			}
		}
	}
ret:	if (ssentctr > 0)
		expand();
	DBUG_RETURN(sditype);
}

#if MC68

/* this routine forces all previously seen SDI's to be resolved.  All
   undefined labels are taken to be long references. This removes
   all uncertainty in the current PC value - JGH */
resolve()
{
	DBUG_ENTER ("resolve");
	notdone = NO;	/* set flag to NO to fake end of pass 1 */
	expand();
	notdone = YES;	/* set back to YES for the rest of pass 1 */
	DBUG_VOID_RETURN;
}
#endif
