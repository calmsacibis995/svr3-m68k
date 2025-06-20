/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/ld2.c	1.31"
#include "system.h"

#include <stdio.h>
#include "reloc.h"
#include "linenum.h"
#include "structs.h"
#include "y.tab.h"

#if TRVEC
#include "tv.h"
#include "ldtv.h"
#endif

#include "extrns.h"
#include "list.h"
#include "params.h"
#include "sgsmacros.h"
#include "patch.h"
#include "instr.h"
#include "dbug.h"		/* Special macro based debugging package */


#define BASIC_TYPES (STYP_TEXT | STYP_DATA | STYP_BSS)
#define	K8	0x2000L
static ITEMID dotacid;
static OUTSECT *dotosptr;
/*eject*/
pboslist()
{

/*
 * Process the BLDOUTSC List
 */

	register OUTSECT *osptr;
	register INSECT *insptr;
	register INFILE *inflptr;
	OUTSECT *dfn_scn_grp();
	long	check_sect();


	DBUG_ENTER ("pboslist");
/*
 * Process the list of output sections.  Output sections can be 
 * defined in two places:
 *
 *	1. PASS 1: user-supplied; bldcnt will be > 0, and the bldoutsc 
 *			list head will be NULL
 *	2. PASS 2: default-generation; bldcnt will be 0, and the
 *			bldoutsc list will describe the output
 *			sections
 *
 *	These two uses are mutually exclusive
 */

	/*
	 * We now know everything there is to know about the
	 * transfer vector:
	 *	At the end of ploadfil(), we knew how big it
	 *	had to be.
	 *	Here we know whether it was defined by ld or
	 *	whether it is defined by .tv input sections
	 *	together with a .tv SECTION directive
	 * Go check everything and complete tvspec definition.
	 */

#if TRVEC
	if ( tvflag )
		tvupdat();	/* update definition of tv */
#endif

	if( bldoutsc.head ) {
		register ACTITEM *aptr, *a2ptr;

		aptr = (ACTITEM *) bldoutsc.head;
		while( aptr ) {

			osptr = dfn_scn_grp(aptr, 1);
	
			copy(osptr->oshdr.s_name, aptr->dfnscn.ainame, 8);
			if( aptr->dfnscn.aitype == AIDFNSCN )
				numoutsc++;
	
			osptr->oshdr.s_paddr = -1L;
			listadd(l_OS, &outsclst, osptr);

			a2ptr = aptr;
			aptr = aptr->dfnscn.ainext;
			free( (char *) a2ptr);
			}
		}

/*
 * Make sure all input sections are allocated
 * into some output section.
 */

	for( inflptr = (INFILE *) infilist.head; inflptr; inflptr = inflptr->flnext ) {
		for( insptr = inflptr->flishead; insptr; insptr=insptr->isnext ) {
			if (insptr->isoutsec <= 0) {
				if ((osptr = fndoutsec(insptr->ishdr.s_name)) != NULL)
					inscadd(insptr,osptr);
				else {
					osptr = (OUTSECT *) mycalloc( sizeof( OUTSECT ));
					numoutsc++;
					copy(osptr->oshdr.s_name, insptr->ishdr.s_name, 8);
					osptr->osfill = globfill;
					osptr->oshdr.s_paddr = -1l;
					osptr->osnlnno = 1;
					osptr->osnflnm = "*default*";
					listadd(l_OS,&outsclst,osptr);
					inscadd(insptr,osptr);
					}
				}
			}
		}

	/*
	 * for each output section, check the flags, and for a group,
	 * check the size.
	 */

	for( osptr = (OUTSECT *) outsclst.head; osptr; osptr = osptr->osnext ) 
		{
#if PAGING
			ACTITEM *aptr;
			ADDRESS textbegin, textsize;
			textbegin =hflag + FILHSZ + SCNHSZ * numoutsc + textorg;
#endif
		if (osptr->oshdr.s_flags & STYP_GROUP) {
			OUTSECT *op;
			for( op = ((OUTSECT *) osptr->osinclhd); op; op = op->osnext ) {
#if PAGING
				if (bond_t_d && !Nflag && equal(op->oshdr.s_name,_DATA,8)) {
					osptr->osalign = NULL;
					aptr = (ACTITEM *) mycalloc(sizeof(ACTITEM));
					aptr->bond.aitype = AIBOND;
					aptr->bond.aiinflnm = "*default.bond.file*";
					aptr->bond.aiinlnno = 3;
					aptr->bond.aioutsec = osptr;
					aptr->bond.aiadrbnd =  cnstnode(((textsize + textbegin) / BOUNDARY + 1) 
						* BOUNDARY + (textsize + textbegin) % K8);
					listadd(l_AI,&bondlist,aptr);
					}
#endif
				osptr->oshdr.s_size += check_sect( op );
				}
			if (osptr->oshdr.s_size > MAXSCNSIZE)
				lderror(1,0,NULL, "GROUP containing section %.8s is too big",
				    osptr->osinclhd->ishdr.s_name);
		} else {
		/* begin KLUDGE */
			adjsize(osptr);
		/* end KLUDGE */
#if PAGING
			if (bond_t_d && !Nflag && equal(osptr->oshdr.s_name,_TEXT,8)) {
				textsize = osptr->oshdr.s_size;
				aptr = (ACTITEM *) mycalloc(sizeof(ACTITEM));
				aptr->bond.aitype = AIBOND;
				aptr->bond.aiinflnm = "*default.bond.file*";
				aptr->bond.aiinlnno = 2;
				aptr->bond.aioutsec = osptr;
				aptr->bond.aiadrbnd = cnstnode( textbegin );
				listadd(l_AI,&bondlist,aptr);
				}
#endif
			check_sect( osptr );
			}
		}

	/*
	 * If .tv is user-defined, we now know the true size of
	 * the section ( the sum of the sizes of the input
	 * sections) which we didn't know before. Now call the
	 * the final update routine to insure .tv symtab entry
	 * is correct
	 */

#if TRVEC
	if ( tvflag )
		tvupdt2();	/* final update of tvrange and tvlength */
#endif
	DBUG_VOID_RETURN;
}
/*eject*/
OUTSECT *
dfn_scn_grp(aptr, dflt)

ACTITEM *aptr;		/* DFNSCN or DFNGRP entry    */
int dflt;		/* user-supplied or ld-default entry */
{

/*
 * Process one AIDFNSCN or AIDFNGRP data structure from
 * the list of output sections
 */

	OUTSECT *osptr, *op;
	INFILE *inflptr;
	INSECT *insptr;
	ENODE *ep;
	long val;
	int fill, fillflag;
	ACTITEM *a2ptr;


	DBUG_ENTER ("dfn_scn_grp");
	osptr = (OUTSECT *) mycalloc(sizeof(OUTSECT));
	if (aptr->dfnscn.aibndadr != NULL) {		/* section bonded */
		a2ptr = (ACTITEM *) mycalloc(sizeof(ACTITEM));
		a2ptr->bond.aitype = AIBOND;
		a2ptr->bond.aiinflnm = aptr->dfnscn.aiinflnm;
		a2ptr->bond.aiinlnno = aptr->dfnscn.aiinlnno;
		a2ptr->bond.aioutsec = osptr;
		a2ptr->bond.aiadrbnd = aptr->dfnscn.aibndadr;
		listadd(l_AI,&bondlist,a2ptr);
		}
	else if (aptr->dfnscn.aiowname[0] != '\0') {  /*owner specified */
		a2ptr = (ACTITEM *) mycalloc(sizeof(ACTITEM));
		a2ptr->dfownr.aitype = AIDFOWNR;
		a2ptr->dfownr.aiinflnm = aptr->dfnscn.aiinflnm;
		a2ptr->dfownr.aiinlnno = aptr->dfnscn.aiinlnno;
		a2ptr->dfownr.aioutsec = osptr;
		copy(a2ptr->dfownr.ainamown, aptr->dfnscn.aiowname, 8);
		listadd(l_AI,&ownlist,a2ptr);
		}
	else if (aptr->dfnscn.aiattown != 0) {	/* attr of owner */
		a2ptr = (ACTITEM *) mycalloc(sizeof(ACTITEM));
		a2ptr->ownatr.aitype = AIOWNATR;
		a2ptr->ownatr.aiinflnm = aptr->dfnscn.aiinflnm;
		a2ptr->ownatr.aiinlnno = aptr->dfnscn.aiinlnno;
		a2ptr->ownatr.aioutsec = osptr;
		a2ptr->ownatr.aiownatt = aptr->dfnscn.aiattown;
		listadd(l_AI,&atownlst,a2ptr);
		}

	if (aptr->dfnscn.aitype == AIDFNGRP)
		osptr->oshdr.s_flags = STYP_GROUP;
	else
		osptr->oshdr.s_flags = aptr->dfnscn.aisctype;
	osptr->osalign = aptr->dfnscn.aialign;
	osptr->osblock = aptr->dfnscn.aiblock;
	osptr->osfill = aptr->dfnscn.aifill;
	osptr->osflags |= aptr->dfnscn.aifillfg;
	osptr->osnlnno = aptr->dfnscn.aiinlnno;
	osptr->osnflnm = aptr->dfnscn.aiinflnm;

/*
 * A DFNSCN need have no subordinate entries, since the following is
 * legal:
 *		.text : {}
 */

	if( aptr->dfnscn.sectspec.head == NULL )
		DBUG_RETURN( osptr );

/*
 * A DFNGRP is a sequence of one or more DFNSCN entries
 */

	if( aptr->dfnscn.aitype == AIDFNGRP ) {
		a2ptr = (ACTITEM *) aptr->dfnscn.sectspec.head;
		do {
			op = dfn_scn_grp(a2ptr, dflt);
			op->oshdr.s_paddr = -1L;
			listadd(l_GRP, osptr, op);
			numoutsc++;
			copy(op->oshdr.s_name, a2ptr->dfnscn.ainame, 8);
			if( (a2ptr = a2ptr->dfnscn.ainext) == NULL )
				DBUG_RETURN( osptr );

			} while( 1 );
		}

/*
 * Process a DFNSCN with one or more subordinate entries, of type
 * AIADFILE, AIADDSCN, or AIEVEXPR
 */

	a2ptr = (ACTITEM *) aptr->dfnscn.sectspec.head;
	do {
		switch( a2ptr->adfile.aitype ) {

		case AIADFILE:

			if( ! dflt )
				a2ptr->adfile.aifilnam = (char *) ((int) a2ptr->adfile.aifilnam + (int) strbase);

			if( (inflptr = fndinfil(a2ptr->adfile.aifilnam)) == NULL )
				break;
			if( a2ptr->adfile.ainadscs == 0 ) {
				insptr = inflptr->flishead;
				while( insptr ) {
					if( insptr->isoutsec == NULL ) {
						inscadd(insptr, osptr);
						insptr->isfill = a2ptr->adfile.aifill2;
						insptr->isfillfg = a2ptr->adfile.aifilflg;
						}
					insptr = insptr->isnext;
					/*
					 * In case inflptr is library,
					 *  get next one
					 */
					if( (insptr == NULL)  &&  (inflptr->flfiloff > 0L) )
						while( (inflptr = inflptr->flnext) )
							if (strcmp(a2ptr->adfile.aifilnam,inflptr->flname) == 0) {
								insptr = inflptr->flishead;
								break;
								}
					}
				}
			else {
				fill = a2ptr->adfile.aifill2;
				fillflag = a2ptr->adfile.aifilflg;
				}
			break;

		case AIADDSCN:

			if( ! dflt )
				a2ptr->addscn.aiscfile = (char *) ((int) a2ptr->addscn.aiscfile + (int) strbase);

			if( ! a2ptr->addscn.aiscfile) {
				/*
				 * search all files for unallocated sections
				 * with name 'a2ptr->addscn.ainame' and
				 * allocate to current output section
				 */
				
				for (inflptr = (INFILE*)infilist.head; inflptr; inflptr = inflptr->flnext)
					for (insptr = inflptr->flishead; insptr; insptr = insptr->isnext)
						if (! insptr->isoutsec && equal(a2ptr->addscn.ainame, insptr->ishdr.s_name,8))
							inscadd(insptr, osptr);
				break;
				}
			if( (inflptr = fndinfil(a2ptr->addscn.aiscfile)) == NULL )
				break;
			insptr = inflptr->flishead;
			while(insptr) {
				if (equal(a2ptr->addscn.ainame,insptr->ishdr.s_name,8))  {
					if (insptr->isoutsec) {
						lderror(1, a2ptr->addscn.aiinlnno, a2ptr->addscn.aiinflnm,
						   "adding %s(%.8s) to multiple output sections",
						   inflptr->flname,
						   insptr->ishdr.s_name);
						goto contin1;
						}
					inscadd(insptr,osptr);
					insptr->isfill = fill;
					insptr->isfillfg = fillflag;
					insptr = NULL;
					if (inflptr->flfiloff > 0L)
					/*
					 * In case inflptr is library,
					 *  get next one
					 */
						while( (inflptr = inflptr->flnext) )
							if (strcmp(a2ptr->addscn.aiscfile,inflptr->flname) == 0) {
								insptr = inflptr->flishead;
								break;
								}
					if ( insptr != NULL )
						continue;
					else
						goto contin1;
					}
				insptr = insptr->isnext;
				}
			lderror(1, a2ptr->addscn.aiinlnno, a2ptr->addscn.aiinflnm,
				"%s(%.8s) not found",
				a2ptr->addscn.aiscfile,a2ptr->addscn.ainame);
			break;

		case AIEVEXPR:
			aptr = a2ptr;
			a2ptr = a2ptr->adfile.ainext;
			ep = aptr->evexpr.aiexptr;
			cur_dot = osptr->oshdr.s_size;

			if (ep->gnode.exleft->gnode.exop != DOT) {
				/*
				 * LHS of assnment is regular symbol
				 */
				dotacid = 0;
				dotosptr = osptr;
				chgdot(ep->gnode.exright);
				listadd(l_AI, &explist, aptr);
				}
			else {
				/*
				 * LHS is DOT symbol
				 */
				curexp = aptr;
				val = eval(ep->gnode.exright);
				if (val < cur_dot)
					lderror(1, aptr->evexpr.aiinlnno, aptr->evexpr.aiinflnm,
						"attempt to decrement DOT");
				else
					osptr->oshdr.s_size = val;

				clrexp( aptr );
				}
			goto contin2;
		}

	contin1:
		a2ptr = a2ptr->adfile.ainext;
	contin2:
		if (a2ptr == NULL)
			DBUG_RETURN( osptr );

		}
	while( 1 );		/* What the f**k is this??! */
	DBUG_VOID_RETURN;
}
/*eject*/
inscadd(pin,pout)
INSECT *pin;
OUTSECT *pout;
{

/*
 * Add an input section to an output section.
 *
 * The text, relocation, and lineno info of the input section
 * are allocated at the end of the output section.
 */

	long btype;

	DBUG_ENTER ("inscadd");
	if (pin->ishdr.s_flags & STYP_OVER)
		lderror(2,0,NULL,
		"cannot use an OVERLAY section as an input section");
	if (pin->ishdr.s_flags & STYP_NOLOAD && pout->oshdr.s_size)
		lderror(2,0,NULL,
		"cannot combine NOLOAD input sections with other input sections;\npossibly caused by shared library address space overlap");
	pin->isoutsec = pout;
	pin->isdispl = pout->oshdr.s_size;
	pin->islndisp = ((long) pout->oshdr.s_nlnno * (long) LINESZ);
	pout->oshdr.s_size += pin->ishdr.s_size;
	pout->oshdr.s_nlnno += pin->ishdr.s_nlnno;

	if (rflag) {
		pin->isrldisp = ((long) pout->oshdr.s_nreloc * (long) RELSZ);
		pout->oshdr.s_nreloc += pin->ishdr.s_nreloc;
		}

	btype = pin->ishdr.s_flags & BASIC_TYPES;
	DBUG_3 ("flags", "basic type of input section = %x", btype);
	if (btype) {
		/*
		 * The s_flags word has all the basic type bits reset
		 * as a bug workaround.  When linking programs using
		 * shared libraries, ishdr.s_flags had STYP_TEXT set
		 * and oshdr.s_flags had STYP_DATA set already.  This
		 * resulted in the .text section having both set.
		 * This fix is a KLUDGE.  The real bug should be found
		 * and fixed!  (fnf)
		 */
		pout->oshdr.s_flags &= ~(BASIC_TYPES);
		pout->oshdr.s_flags |= btype;
	} else if ( equal( _TEXT, pin->ishdr.s_name, 8 ))
		pout->oshdr.s_flags |= STYP_TEXT;
	else if ( equal( _DATA, pin->ishdr.s_name, 8 ))
		pout->oshdr.s_flags |= STYP_DATA;
	else if ( equal( _BSS, pin->ishdr.s_name, 8 ))
		pout->oshdr.s_flags |= STYP_BSS;
	DBUG_3 ("flags",
		"basic type of output section = %x", pout->oshdr.s_flags);

	pout->osflags |= (pin->ishdr.s_flags & STYP_DSECT) ? OSDSECT : OSREG;
	if (pin->ishdr.s_flags & STYP_INFO)
		pout->osflags |= OSINFO;
	if (pin->ishdr.s_flags & STYP_LIB) {
		pout->osflags |= OSLIB;
		if (pout->osflags & (~(OSLIB | OSDSECT))) /* DSECT on for LIB */
			lderror(2,0,NULL,
				"cannot combine section type STYP_LIB with input sections of other types");
		}
	if (pin->ishdr.s_flags & STYP_COPY)
		pout->osflags |= OSCOPY;
	if (pin->ishdr.s_flags & STYP_NOLOAD) {
		ACTITEM *aptr;
		pout->oshdr.s_flags |= STYP_NOLOAD;
		aptr = (ACTITEM *) mycalloc(sizeof(ACTITEM));
		aptr->bond.aitype = AIBOND;
		aptr->bond.aiinflnm = "*default.NOLOAD.bond.file*";
		aptr->bond.aiinlnno = 1;
		aptr->bond.aioutsec = pout;
		aptr->bond.aiadrbnd = cnstnode( pin->ishdr.s_paddr );
		listadd(l_AI,&bondlist,aptr);
		}

	listadd(l_INC,pout,pin);
	DBUG_VOID_RETURN;
}
/*eject*/
INFILE *
fndinfil(fnam)
char *fnam;
{

/*
 * Locate the input file structure by a given name, and
 * return a pointer to the structure.
 *
 *  Note: fnam must be a null terminated string.
 */

	register INFILE *p;

	DBUG_ENTER ("fndinfil");
	p = (INFILE *) infilist.head;
	while (p) {
		if (strcmp(fnam,p->flname) == 0) 
			DBUG_RETURN(p);
		p = p->flnext;
		}

	DBUG_RETURN(NULL);
}
/*eject*/
chgdot(p)
ENODE *p;
{
	register SYMTAB *sym;
	ITEMID add_dot();

	DBUG_ENTER ("chgdot");
/*
 * This function is called when an assignment is encountered
 * within an output section specification, and the LHS of the
 * assnment is to a symbol other than DOT.  Any occurances of
 * DOT within the RHS expression must be replaced by a reference
 * to a dummy symbol table entry, which will have the correct
 * value after the output section is relocated.
 */

	switch(p->gnode.exop) {
	case DOT:
		if( dotacid == 0 )
			dotacid = add_dot(cur_dot, dotosptr);
		p->nnode.exsymptr = dotacid;
		break;
	case NAME:
	case INT:
	case ADDR:
	case SIZEOF:
	case NEXT:
		break;

	default:
		if (p->gnode.exleft) 
			chgdot(p->gnode.exleft);
		if (p->gnode.exright)
			chgdot(p->gnode.exright);
	}
	DBUG_VOID_RETURN;
}
/*eject*/
add_pad()
{

/*
 * If the "-B" flag was specified, add any necessary "padding sections"
 * to the outpt file
 */

	register ANODE *ap;		/* ptr into avlist */
	ANODE *newap;			/* ptr to padding section node */
	register OUTSECT *osp;
	register INSECT *isp;
	int no_need_pad,		/* = 1 iff don't need padding */
	    counter;			/* used to make unique names */
	char padname[8];		/* padding section name */

/*
 * Look for an output section which meets one of two conditions:
 *
 *	1. The section is comprised entirely of uninitialized .bss
 *		sections
 *	2. The section is of zero length
 *
 * For every output section of these types, add an additional output
 * section immediatly after it.  This "padding section" has only two
 * non-zero/non-NULL/non-trivial characteristics:
 *	1. It is composed of Bflag bytes of zero
 *	2. It is written under control of BLOCK(Bflag)
 *
 * Note that padding sections have a virtual/physical address
 * of -1 (to generate errors from any user program or utility
 * attempting to use these values), and flags indicating that
 * it is a padding section
 */

	DBUG_ENTER ("add_pad");
	counter = 0;
	for( ap = (ANODE *) avlist.head; ap; ap = ap->adnext ) {
		if( ap->adtype != ADSECT )
			continue;

		osp = ap->adscnptr;
		no_need_pad = (osp->osflags & FILL);
		for( isp = osp->osinclhd; isp; isp = isp->isincnxt )
			if( (isp->ishdr.s_scnptr > 0)  ||  (isp->isfillfg) )
				no_need_pad = 1;
		if( ap->adsize == 0L )
			no_need_pad = 0;
		
		if( no_need_pad )
			continue;
		
		osp = (OUTSECT *) mycalloc(sizeof(OUTSECT));
		numoutsc++;
		sprintf( padname, "-pad%2d-", counter++ );
		copy( osp->oshdr.s_name, padname, 8 );
		osp->oshdr.s_paddr = -1L;
		osp->oshdr.s_vaddr = -1L;
		osp->oshdr.s_size = Bflag;
		osp->oshdr.s_flags = STYP_PAD;
		osp->osblock = Bflag;
		
		newap = (ANODE *) mycalloc(sizeof(ANODE));
		newap->adtype = ADPAD;
		newap->adsize = Bflag;
		newap->adscnptr = osp;

		listins( l_ADR, &avlist, ap, newap );
		}
	DBUG_VOID_RETURN;
}
/*eject*/
updatsms()
{

/*
 * After allocation, this function sets
 *	1. The section number of all output sections
 *	2. The virtual addresses of all input sections (isnewvad)
 *	3. The file offset of output sections
 *
 * In addition, the relocated value for all global symbols is 
 * computed and stored in the symbol table, along with the new
 * section number.
 *
 * The virtual addresses of output sections are set by
 * audit_regions in allocation.
 */

	register SYMTAB *symp;
	register OUTSECT *osp;
	register INSECT *isp;
	register INFILE *fp;
	int snum, init;
	long vaddr, indx;
	long filoffst, lnnoptr, nrelocs;
	ANODE *ap;
	long i;

/*
 * Compute the starting file offset of the first output section
 *
 * ********** NOTE NOTE NOTE NOTE **********
 *
 *	Increase the size of the optional header by the amount of
 *	space needed to generate a patch list
 *
 * ********** NOTE NOTE NOTE NOTE **********
 */

	DBUG_ENTER ("updatsms");
	hflag += psize();
	filoffst = (long) FILHSZ + hflag + SCNHSZ * numoutsc;

	nrelocs = 0L;
	snum = 0;

	for( ap = (ANODE *) avlist.head; ap; ap = ap->adnext ) {
		if (ap->adtype == ADSECT) {
			osp = ap->adscnptr;
			/*
			 ***** Special processing *****
			 *
			 * The description of the _TV output section is
			 * changed, so as to make it equivalent to that
			 * section of the transfer vector actually used
			 * in this ld run
			 *
			 * Note that in the default case - no ASSIGN or
			 * RANGE commands, the entire transfer vector
			 * is used, and hence this code produces no
			 * change.
			 *
			 * When ASSIGN and RANGE commands are present,
			 * the actual section of the transfer vector
			 * used is typically a subset of the entire
			 * vector. Output only the piece used
			 */
#if TRVEC
			if( tvflag && equal(osp->oshdr.s_name,_TV,8) ) {
				osp->oshdr.s_paddr +=
					(tvspec.tvrange[0] * TVENTSZ);
				osp->oshdr.s_vaddr += 
					(tvspec.tvrange[0] * TVENTSZ);
				osp->oshdr.s_size =
					((tvspec.tvrange[1]-tvspec.tvrange[0]+1)*TVENTSZ);
				}
#endif
			init = (osp->osflags & FILL);	/* section is initialized*/
			osp->ossecnum = ++snum;
			vaddr = osp->oshdr.s_vaddr;
			for( isp = osp->osinclhd; isp; isp = isp->isincnxt ) {
				isp->isnewvad = vaddr + isp->isdispl;
				isp->isnewpad = osp->oshdr.s_paddr + isp->isdispl;
				if (isp->ishdr.s_scnptr > 0 || isp->isfillfg)
					init = 1;
				}

			if ((osp->oshdr.s_flags & STYP_NOLOAD) || (osp->oshdr.s_flags & STYP_DSECT))
				init = 0;
			if (init) {
				if(cflag && (equal(osp->oshdr.s_name,_TEXT,8) ||
					equal(osp->oshdr.s_name, _DATA, 8)))
					filoffst += alignment(K8,filoffst);
				else
					filoffst += alignment(osp->osblock,filoffst);
				osp->oshdr.s_scnptr = filoffst;
				filoffst += osp->oshdr.s_size;
				if (rflag)
					nrelocs += (long) osp->oshdr.s_nreloc;
				if (filoffst % 2)
					++filoffst;
				}
			else
				osp->oshdr.s_scnptr = 0L;
			}
		else if( ap->adtype == ADPAD ) {
			osp = ap->adscnptr;
			osp->ossecnum = ++snum;
			filoffst += alignment(osp->osblock,filoffst);
			osp->oshdr.s_scnptr = filoffst;
			filoffst += osp->oshdr.s_size;
			if( filoffst % 2 )
				++filoffst;
			}
		}

/*
 * Do the same for the "dummy" sections (DSECTS)
 */

	for( osp = (OUTSECT *) dsectlst.head; osp; osp = osp->osdsnext ) {
		osp->ossecnum = ++snum;
		for( isp = osp->osinclhd; isp; isp = isp->isincnxt ) {
			isp->isnewvad = osp->oshdr.s_vaddr + isp->isdispl;
			isp->isnewpad = osp->oshdr.s_paddr + isp->isdispl;
			}
		if ( osp->oshdr.s_flags &
			(STYP_COPY | STYP_INFO | STYP_LIB | STYP_OVER)) {
			filoffst += alignment(osp->osblock,filoffst);
			osp->oshdr.s_scnptr = filoffst;
			filoffst += osp->oshdr.s_size;
			if ( rflag )
				nrelocs += (long) osp->oshdr.s_nreloc;
			if ( filoffst % 2)
				++filoffst;
		} else {
			osp->oshdr.s_nreloc = 0;
			osp->oshdr.s_nlnno  = 0;
			}
		}

/*
 * Loop through sections a 2nd time to set the
 * file pointer to relocation and line numbers for each section
 */

	lnnoptr = filoffst + nrelocs * (long) RELSZ;
	for( ap = (ANODE *) avlist.head; ap; ap = ap->adnext ) {
		if (ap->adtype == ADSECT) {
			osp = ap->adscnptr;
			if (osp->oshdr.s_flags & STYP_NOLOAD) {
				osp->oshdr.s_nreloc = 0;
				osp->oshdr.s_nlnno  = 0;
				continue;
				}
			if( rflag  &&  (osp->oshdr.s_nreloc != 0) ) {
				osp->oshdr.s_relptr = filoffst;
				filoffst += ((long)osp->oshdr.s_nreloc *
							(long) RELSZ);
				}
			if( (!sflag && !xflag) && (osp->oshdr.s_nlnno != 0) ) {
				osp->oshdr.s_lnnoptr = lnnoptr;
				lnnoptr += ((long)osp->oshdr.s_nlnno *
							(long) LINESZ);
				}
			}
		}

/*
 * Do the same for the "copy" sections (COPY)
 */

	for( osp = (OUTSECT *) dsectlst.head; osp; osp = osp->osdsnext ) {
		if ( osp->oshdr.s_flags &
			(STYP_COPY | STYP_INFO | STYP_LIB | STYP_OVER)) {
			if ( rflag && (osp->oshdr.s_nreloc != 0) ) {
				osp->oshdr.s_relptr = filoffst;
				filoffst += ((long) osp->oshdr.s_nreloc
						* (long) RELSZ);
				}
			if ( (! sflag) && (osp->oshdr.s_nlnno != 0) ) {
				osp->oshdr.s_lnnoptr = lnnoptr;
				lnnoptr += ((long) osp->oshdr.s_nlnno *
							(long) LINESZ);
				}
			}
		}

	symtborg = filoffst = lnnoptr;
	fp = (INFILE *) infilist.head;
	indx = 0;
	while (fp) {
		fp->flsymndx = indx;
		indx += fp->flnlsyms;
		fp = fp->flnext;
		}

	i = 1;
	while ( i <= numldsyms )
	{
		symp = getsym(i++);
		if( symp->sment.n_scnum > 0 ) {
			isp = symp->smscnptr;
			symp->sment.n_scnum = isp->isoutsec->ossecnum;
			symp->smnewval = symp->sment.n_value 
				+ isp->isnewpad - isp->ishdr.s_paddr;
			symp->smnewvirt = symp->smnewval
				- isp->isnewpad + isp->isnewvad;
			if( symp->smlocflg ) {
				symp->smoutndx += isp->isfilptr->flsymndx;
				}
			else {
				symp->smoutndx = indx++;
				indx += symp->sment.n_numaux;
				}
			}
		else {
			if ((symp->sment.n_zeroes == 0) ||
					(strncmp( symp->sment.n_name, ".dot", 4 ) != 0))
				symp->smoutndx = indx++;
			symp->smnewval = symp->sment.n_value;
			symp->smnewvirt = symp->sment.n_value;
			indx += symp->sment.n_numaux;
			}
		}
#if FLEXNAMES
	tnumsyms = indx;
#endif
	DBUG_VOID_RETURN;
}
/*eject*/
OUTSECT *
findoscn(number)
int number;		/* number of desired section */
{

/*
 * Look through the output section list, outsclst,
 * for a section with the specified number.  Return
 * a pointer to that element of the list.  If no
 * section exists with that number, return NULL.
 */

	register OUTSECT *osptr;

	DBUG_ENTER ("findoscn");
	osptr = (OUTSECT *) outsclst.head;
	while ( osptr != NULL ) {
		if ( osptr->ossecnum == number )
			DBUG_RETURN (osptr);
		osptr = osptr->osnext;
		}

	DBUG_RETURN (NULL);
}




split_scns()
{

/*
 * Run though the output section list.  If the size of any output
 * section is greater then MAXSCNSZ, split the output section into
 * two (or more) output sections
 *
 * The name of a new section is the first char of the old name,
 * followed by a number, followed by the rest of the old
 * name.  Only eight characters are kept.
 */

	OUTSECT	*osp, *newosp, *nxtosp;
	INSECT	*isp, *nxtisp;
	char	*oldname, newname[20];		/* 8 + 4 + extra */
	int	scnnumber;
	long	newdisp, startdisp;	/* OUTSECT displacements */

/*
 * "osp" scans down the outsclst, and "nxtosp" will
 * always point to the next section on the original
 * list.  If we have to split osp, the new node
 * will be newosp, and nxtosp will not change.
 */

	DBUG_ENTER ("split_scns");
	for( osp = (OUTSECT *) outsclst.head; osp; osp = nxtosp ) {
		nxtosp = osp->osnext;
		if (osp->oshdr.s_flags & STYP_DSECT)
			continue;

		if( (osp->osinclhd == osp->osincltl)  ||  (osp->oshdr.s_size <= MAXSCNSZ) ) {
			/* Either there is only one input section in the
			 * output section, or the output section is
			 * within the permissible size
			 */
			if( (isp = osp->osinclhd) == NULL )
				continue;
			if( isp->ishdr.s_size > MAXSCNSIZE ) {
				lderror(1,0,NULL, "section %.8s in file %s too big",
					isp->ishdr.s_name, isp->isfilptr->flname);
				osp->osinclhd = osp->osincltl = NULL;
				}
			continue;
			}

		/*
		 * An output section is too large, and must be split.
		 *
		 * Walk through the list of input sections, to build
		 * two (or more) output sections which are all within
		 * the permissible size range
		 */

		osp->oshdr.s_nreloc = 0;
		osp->oshdr.s_nlnno = 0;
		isp = osp->osinclhd;
		osp->osinclhd = osp->osincltl = NULL;
		scnnumber = 0;
		oldname = osp->oshdr.s_name;

		newdisp = 0L;
		if( isp != NULL )
			startdisp = osp->oshdr.s_size = isp->isdispl;
		else
			startdisp = osp->oshdr.s_size = 0L;

		/*
		 * "isp" points to the current input section which we
		 * are trying to add to the output section given by
		 * "osp"
		 */

		for(; isp; isp = nxtisp ) {
			nxtisp = isp->isincnxt;
			if( osp->oshdr.s_size + isp->ishdr.s_size <= MAXSCNSZ ) {
				newdisp = isp->isdispl - startdisp;
				if( osp->oshdr.s_size > newdisp )
					lderror(1,0, NULL,
						"internal error: split_scns, size of %.8s exceeds its new displacement",
						osp->oshdr.s_name);

				osp->oshdr.s_size = newdisp;
				isp->isincnxt = NULL;
				inscadd(isp, osp);
				}
			else if ( osp->oshdr.s_size != 0 ) {
				nxtisp = isp;	/* back up to reprocess */
				newosp = (OUTSECT *) mycalloc(sizeof(OUTSECT));
				numoutsc++;
				sprintf(newname, "%c%x%.6s", *oldname,
					scnnumber++, oldname + 1);
				copy(newosp->oshdr.s_name, newname, 8);
				newosp->oshdr.s_paddr = -1L;
				newosp->osnext = osp->osnext;
				osp->osnext = newosp;
				if( (OUTSECT *) outsclst.tail == osp )
					outsclst.tail = (char *) newosp;
				osp = newosp;
				newdisp = 0L;
				startdisp = isp->isdispl;
				}
			else if ( isp->ishdr.s_size <= MAXSCNSIZE ) {
				isp->isincnxt = NULL;
				inscadd(isp, osp);
				}
			else
				lderror(1,0,NULL, "section %.8s in file %s is too big",
					isp->ishdr.s_name, isp->isfilptr->flname);
			}

		}

	DBUG_VOID_RETURN;
}
/*eject*/
psize()
{
/*
 * Determine the size of the patch list. This is done by looking for
 * all output sections having the name of ".patchnn", where nn is a
 * two-digit number
 *
 * This routine returns, as an int value, the size of the patch list
 */

	register int index, space;
	char pname[10];
	OUTSECT *fndoutsec();

	DBUG_ENTER ("psize");
	if( ! pflag )
		DBUG_RETURN( 0 );
	else
		space = sizeof(long);

/*
 * Patch sections, if they exist, must be named consecutively, starting
 * at ".patch00"
 */

	for( index = 0; index < 100; index++ ) {
		if (index < 10)
			sprintf(pname, ".patch0%d", index);
		else
			sprintf( pname, ".patch%d", index );
		if( fndoutsec(pname) == NULL )
			break;
		space += PESIZE;
		}

	DBUG_RETURN( space );
}
/*eject*/
OUTSECT *
fndoutsec(nam)
char *nam;
{
	register OUTSECT *p, *q;

	DBUG_ENTER ("fndoutsec");
	for( p = (OUTSECT *) outsclst.head; p; p=p->osnext ) {
		if (p->oshdr.s_flags & STYP_GROUP)
			for( q = (OUTSECT *) p->osinclhd; q; q=q->osnext ) {
				if (equal(nam,q->oshdr.s_name,8))
					DBUG_RETURN(q);
			}
		else
			if (equal(nam,p->oshdr.s_name,8))
				DBUG_RETURN(p);
	}
	DBUG_RETURN(NULL);
}




#if ILDOPT
add_extra()
{

/* If the "-ild" flag was specified, add sections to the output file to
 * cover any remaining configured memory.
 */

	register ANODE *ap, *newap;	/* ptr into avlist */
	OUTSECT *osp1, *osp2;		/* ptrs to the new output section headers */
	int counter;			/* used to make unique names */
	char sectname[9];		/* new section name */

/* For each remaining area of available memory, create a fake output section
 * that fits in that area. The purpose of this is to cover all of configured
 * memory so that the incremental ld will be able to determine the memory
 * configuration. Increase numoutsc to leave room for the .history section
 * header.
 */

	DBUG_ENTER ("add_extra");
	numoutsc++;
	counter = 0;
	for (ap = (ANODE *) avlist.head; ap; ap = ap->adnext) {
		if (counter > 99)
			break;
		if (ap->adtype != ADAVAIL)
			continue;
		if ( ap != (ANODE *) avlist.head && ap->adprev->adtype != ADAVAIL &&
		      ap->adprev->adscnptr != NULL &&
		      (ap->adprev->adscnptr->oshdr.s_flags & STYP_NOLOAD)  &&
		     ap != (ANODE *) avlist.tail && ap->adnext->adtype != ADAVAIL &&
		      ap->adnext->adscnptr != NULL &&
		      (ap->adnext->adscnptr->oshdr.s_flags & STYP_NOLOAD)    )
			continue;

		ap->adtype = ADSECT;
		newap = (ANODE *) mycalloc(sizeof(ANODE) + 2 * sizeof(OUTSECT));
		osp1 = (OUTSECT *) (newap + 1);
		osp2 = osp1 + 1;
		numoutsc += 2;
		if (counter < 10)
			sprintf(sectname, ".i_l_d0%d", counter++);
		else
			sprintf( sectname, ".i_l_d%d", counter++);
		copy( osp1->oshdr.s_name, sectname, 8 );
		osp1->oshdr.s_flags = STYP_DSECT;
		ap->adscnptr = osp1;

		if (counter < 10)
			sprintf( sectname, ".i_l_d0%d", counter++);
		else
			sprintf( sectname, ".i_l_d%d", counter++ );
		copy( osp2->oshdr.s_name, sectname, 8 );
		osp2->oshdr.s_paddr = ap->adpaddr;
		osp2->oshdr.s_vaddr = ap->adpaddr;
		osp2->oshdr.s_size = ap->adsize;
		osp2->oshdr.s_flags = STYP_DSECT;
		osp2->osflags = 0;

		newap->adtype = ADSECT;
		newap->adpaddr = ap->adpaddr;
		newap->adsize = ap->adsize;
		newap->adscnptr = osp2;
		newap->admemp = ap->admemp;
		newap->adregp = ap->adregp;
		listins( l_ADR, &avlist, ap, newap );
	}
	DBUG_VOID_RETURN;
}
#endif




long
check_sect( osptr )
OUTSECT *osptr;
{

/* 
 * check the flags in the output section. if all the input sections are dummy
 * sections, the output section should be a dummy section.
 * If no input section was a regular section and one was a copy section
 * the output will be a copy section.  If any input section was an info
 * section,  the output section will have the info flag on. (Most flags
 * are not mutually exclusive.)
 * The lib flag will work the same as the info flag.
 * If a bss section is combined with data or text, the bss section is
 * initialized, and becomes data.
 * return the size of the section if the output section is not a dummy
 * (the size is used in allocating space for a group).
 */

	DBUG_ENTER ("check_sect");
	if ((osptr->oshdr.s_flags & STYP_DSECT) == 0) { /* figure type of output
					section if DSECT not specified in ifile */
		if ((osptr->osflags & OSDSECT) && !(osptr->osflags & OSREG)) {
			osptr->oshdr.s_flags |= STYP_DSECT;
			/* remember COPY section also has OSDSECT on */
			if (osptr->osflags & OSCOPY)
				osptr->oshdr.s_flags |= STYP_COPY;
			}
		if (osptr->osflags & OSINFO)
			osptr->oshdr.s_flags |= STYP_INFO;
		if (osptr->osflags & OSLIB)
			osptr->oshdr.s_flags |= STYP_LIB;
		}

	if ((osptr->oshdr.s_flags & STYP_BSS) &&
			((osptr->oshdr.s_flags & (STYP_TEXT | STYP_DATA)) ||
			(osptr->osflags & FILL))) {
		osptr->oshdr.s_flags ^= STYP_BSS;
		osptr->oshdr.s_flags |= STYP_DATA;
		}

	DBUG_3 ("flags", 
		"initial output section flags = %x", osptr -> oshdr.s_flags);
	if ( !(osptr->oshdr.s_flags & BASIC_TYPES))
		if (equal( _TEXT, osptr->oshdr.s_name, 8 ))
			osptr->oshdr.s_flags |= STYP_TEXT;
		else if (equal( _DATA, osptr->oshdr.s_name, 8 ))
			osptr->oshdr.s_flags |= STYP_DATA;
		else if (equal( _BSS, osptr->oshdr.s_name, 8 ))
			osptr->oshdr.s_flags |= STYP_BSS;
	DBUG_3 ("flags", 
		"final output section flags = %x", osptr -> oshdr.s_flags);

	DBUG_RETURN ((osptr->oshdr.s_flags & STYP_DSECT) ? 0L : osptr->oshdr.s_size);
}
