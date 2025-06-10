h24842
s 00000/00000/00394
d D 1.6 86/09/16 15:28:15 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00394
d D 1.5 86/09/15 14:01:19 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00394
d D 1.4 86/09/11 13:01:52 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00394
d D 1.3 86/08/18 08:48:03 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00034/00014/00360
d D 1.2 86/08/07 08:03:08 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00374/00000/00000
d D 1.1 86/08/04 09:38:01 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)expand1.c	6.1		*/
#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "expand.h"
#include "gendefs.h"
I 2
#include "dbug.h"
E 2

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

I 2
	DBUG_ENTER ("update");
E 2
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
I 2
	DBUG_VOID_RETURN;
E 2
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

I 2
	DBUG_ENTER ("sdiclass");
E 2
	lptr = sdiptr->labptr;
	itype = sdiptr->itype;
	if ((ltype = lptr->styp & TYPE) != UNDEF) {
		if (ltype != TXT
#if MULTSECT && MC68
		     || lptr->sectnum != 0
#endif
		   )
D 2
			return(L_SDI);
E 2
I 2
			DBUG_RETURN(L_SDI);
E 2
		span = lptr->value;
	}
	else {
		if (notdone == NO)
D 2
			return(L_SDI);
E 2
I 2
			DBUG_RETURN(L_SDI);
E 2
		span = (dot->value != sdiptr->minaddr) ? dot->value :
			sdiptr->minaddr + pcincr[itype];
	}
	span += sdiptr->constant - (sdiptr->minaddr + pcincr[itype]);
	if ((span < range[itype].lbound) ||
		(span > range[itype].ubound))	/* definitely long */
D 2
		return(L_SDI);
E 2
I 2
		DBUG_RETURN(L_SDI);
E 2
	else {
		if (ltype != UNDEF) {
			span += (int)(lptr->maxval) -
				((lptr->value > sdiptr->minaddr) ?
				(sdiptr->maxaddr + idelta[itype]) : sdiptr->maxaddr);
			if ((span >= range[itype].lbound) &&
				(span <= range[itype].ubound))	/* definitely short */
D 2
				return(S_SDI);
E 2
I 2
				DBUG_RETURN(S_SDI);
E 2
		}
	}
D 2
	return(U_SDI);
E 2
I 2
	DBUG_RETURN(U_SDI);
E 2
}

expand()
{
	register short change = YES;
	register short cntr;
	register ssentry *current;
	register short sditype;

I 2
	DBUG_ENTER ("expand");
E 2
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
I 2
	DBUG_VOID_RETURN;
E 2
}

punt()
{
	register short cntr;
	register ssentry *current;
	register unsigned firstsdi;
	register ssentry *outptr;

I 2
	DBUG_ENTER ("punt");
E 2
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
I 2
	DBUG_VOID_RETURN;
E 2
}

newlab(sym)
	register symbol *sym;
{
	static short labwarn = YES;

I 2
	DBUG_ENTER ("newlab");
E 2
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
I 2
	DBUG_VOID_RETURN;
E 2
}

deflab(sym)
	register symbol *sym;
{
I 2
	DBUG_ENTER ("deflab");
E 2
	sym->maxval = PCmax;
	if (ssentctr >= 0) {
		newlab(sym);
		expand();
	}
I 2
	DBUG_VOID_RETURN;
E 2
}

#if !ONEPROC
extern FILE *fdlong;
#endif

fixsyms()
{
I 2
	DBUG_ENTER ("fixsyms");
E 2
	notdone = NO;
	expand();
#if !ONEPROC
	fwrite(islongsdi,sizeof(*islongsdi),(int)(++sdicnt),fdlong);
#endif
I 2
	DBUG_VOID_RETURN;
E 2
}

sdi(sym,const,itype)
	register symbol *sym;
	long const;
	register short itype;
{
	register long span;

I 2
	DBUG_ENTER ("sdi");
E 2
	if (sym) {
		if ((sym->styp & TYPE) == UNDEF)
D 2
			return(U_SDI);
E 2
I 2
			DBUG_RETURN(U_SDI);
E 2
		if ((sym->styp & TYPE) != TXT
#if MULTSECT && MC68
		     || sym->sectnum != 0
#endif
 		   )
D 2
			return(L_SDI);
E 2
I 2
			DBUG_RETURN(L_SDI);
E 2
	}
	else
D 2
		return(L_SDI);
E 2
I 2
		DBUG_RETURN(L_SDI);
E 2

	span = sym->value + const - (dot->value + pcincr[itype]);
	if ((span < range[itype].lbound) ||
		(span > range[itype].ubound))	/* definitely long */
D 2
		return(L_SDI);
E 2
I 2
		DBUG_RETURN(L_SDI);
E 2
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
D 2
			return(S_SDI);
E 2
I 2
			DBUG_RETURN(S_SDI);
E 2
	}
D 2
	return(U_SDI);
E 2
I 2
	DBUG_RETURN(U_SDI);
E 2
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

I 2
	DBUG_ENTER ("newsdi");
E 2
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
I 2
	DBUG_VOID_RETURN;
E 2
}

shortsdi(sym,const,itype)
	register symbol *sym;
	long const;
	register short itype;
{
	register int sditype;

I 2
	DBUG_ENTER ("shortsdi");
E 2
	if (!overflow) {
		if (sym && ((sym->styp & TYPE) == UNDEF)) {
			if (dot->styp != TXT
#if MULTSECT && MC68
			    || dot->sectnum != 0
#endif
 			   )
D 2
				return(L_SDI);
E 2
I 2
				DBUG_RETURN(L_SDI);
E 2
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
D 2
					return(L_SDI);
E 2
I 2
					DBUG_RETURN(L_SDI);
E 2
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
D 2
	return(sditype);
E 2
I 2
	DBUG_RETURN(sditype);
E 2
}

#if MC68

/* this routine forces all previously seen SDI's to be resolved.  All
   undefined labels are taken to be long references. This removes
   all uncertainty in the current PC value - JGH */
resolve()
{
I 2
	DBUG_ENTER ("resolve");
E 2
	notdone = NO;	/* set flag to NO to fake end of pass 1 */
	expand();
	notdone = YES;	/* set back to YES for the rest of pass 1 */
I 2
	DBUG_VOID_RETURN;
E 2
}
#endif
E 1
