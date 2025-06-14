h48814
s 00000/00000/01094
d D 1.3 86/08/18 08:44:34 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00052/00022/01042
d D 1.2 86/07/30 14:20:42 fnf 2 1
c Add macro based C debugging package macros.
e
s 01064/00000/00000
d D 1.1 86/07/29 14:31:34 fnf 1 0
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

#ident	"@(#)ld:common/alloc.c	1.22"
#include "system.h"

#include <stdio.h>
#include "structs.h"
#include "extrns.h"
#include "list.h"
#include "params.h"
#include "sgsmacros.h"
#include "ldmacros.h"
I 2
#include "dbug.h"		/* Macro based debugging package */
E 2

#if IAPX
#include "sgs.h"
#endif

#if TRVEC
#include "tv.h"
#include "ldtv.h"
#endif

alloc()
{

I 2
	DBUG_ENTER ("alloc");
E 2
/*
 * Perform the allocation of the output sections into the configured
 * memory
 */

	bldmemlist();	/* build initial address space from memlist	*/

#if USEREGIONS
#if IAPX
	if ( IAPX16MAGIC(magic) )
#endif
	bldreglist();	/* collate regions into available space		*/
#endif /* USEREGIONS */

	alc_bonds();	/* allocate bonded output sections		*/
	alc_owners();	/* allocate out sects assigned to owners	*/
	alc_attowns();	/* allocate out sects assigned to attributes	*/

#if IANDD && USEREGIONS
#if IAPX
	if( IAPX16MAGIC(magic) )
#endif
	if( iflag  &&  ( ! tvflag ) )
		alc_iandd();	/* separate I and D */
#endif /* IANDD && USERREGIONS */

	alc_the_rest();		/* allocate everything as yet unallocated	*/

#if USEREGIONS
#if IAPX
	if( IAPX16MAGIC(magic) )
#endif
	if (tvflag && (reglist.head == NULL))
		bld_regions();
#endif /* USEREGIONS */

	audit_groups();

	audit_regions();

	if (aflag)
		set_spec_syms();   /* generate special symbols for absolute load */

I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
bldmemlist()
{

	register MEMTYPE *mp;
	register ANODE *ap;

I 2
	DBUG_ENTER ("bldmemlist");
E 2
/*
 * Build the initial address space from the memlist
 */

	for( mp = (MEMTYPE *) memlist.head; mp != NULL; mp = mp->mtnext ) {
		ap = (ANODE *) mycalloc( sizeof( ANODE ));
		ap->adtype = ADAVAIL;
		ap->adpaddr = mp->mtorig;
		ap->adsize  = mp->mtlength;
		ap->admemp  = mp;
		mp->mtaddrhd=mp->mtaddrtl = ap;
		listadd(l_ADR, &avlist, ap);
		}
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
alc_bonds()
{

/*
 * Process the bond.list, allocating all output sections which have
 * been bonded to specific addresses
 */

	ACTITEM *aip,		/* ptr to bonded section */
		*nextaip;	/* ptr to next item on bond.list */
	ANODE	*sap,		/* ptr to memory node containing bond address */
		*eap, *newap, *splitnode();
	OUTSECT *osp,		/* ptr to output section description for the bonded section */
		*bsp;
	ADDRESS lastaddr, have, need, bond_addr;

I 2
	DBUG_ENTER ("alc_bonds");
E 2
	for( aip = (ACTITEM *) bondlist.head; aip != NULL; aip = nextaip ) {
		nextaip = aip->bond.ainext;
		osp = aip->bond.aioutsec;

		/* evaluate bond address */

		lineno = aip->bond.aiinlnno;
		curfilnm = aip->bond.aiinflnm;
		cur_dot = -1L;
		bond_addr = eval( aip->bond.aiadrbnd );
		clrlimb( aip->bond.aiadrbnd );

		/*
		 * DSECT sections are bonded at the requested address,
		 * but are not allocated any memory
		 */
		if( osp->oshdr.s_flags & STYP_DSECT) {
			osp->oshdr.s_paddr = osp->oshdr.s_vaddr = bond_addr;
			listadd(l_DS, &dsectlst, osp);
			free(aip);
			continue;
			}
		/*
		 * For the current bonded section, find out in which of
		 * the space nodes the bond address is located
		 */
		for( sap = (ANODE *) avlist.head; sap != NULL; sap = sap->adnext ) {
			if( sap->adpaddr+(ADDRESS)sap->adsize > bond_addr)
				break;
			}

		if(sap == NULL) {
			lderror(1, aip->bond.aiinlnno, aip->bond.aiinflnm,
				"bond address %.2lx for %.8s is outside all configured memory",
				bond_addr, (*(osp->oshdr.s_name) == '\0') ? "GROUP" : osp->oshdr.s_name);
			free(aip);
			continue;
			}
		/*
		 * If an space node exists for the bond address, sap
		 * points to it
		 */
		if( sap->adpaddr > bond_addr ) {
			lderror(1, aip->bond.aiinlnno, aip->bond.aiinflnm,
				"bond address %.2lx for %.8s is not in configured memory",
				bond_addr, (*(osp->oshdr.s_name) == '\0') ? "GROUP" : osp->oshdr.s_name);
			free(aip);
			continue;
			}
		/*
		 * Make sure the space node is of type ADAVAIL (i.e, that
		 * it is unallocated memory
		 */
		if(sap->adtype != ADAVAIL) {
			bsp=sap->adscnptr;
			if((!strncmp(osp->oshdr.s_name,bsp->oshdr.s_name,8)) &&
				(bond_addr == sap->adpaddr)) {
				free(aip);
				continue;
				}
			lderror(1, aip->bond.aiinlnno, aip->bond.aiinflnm,
				"bond address %.2lx for %.8s overlays previously allocated section %.8s at %.2lx",
				bond_addr, (*(osp->oshdr.s_name) == '\0') ? "GROUP" : osp->oshdr.s_name,
				bsp->oshdr.s_name, sap->adpaddr);
			free(aip);
			continue;
			}
		/*
		 * If the bond address is not the start of an AVAIL
		 * node, split the AVAIL node into two nodes. The
		 * first will be AVAIL, the second SECT
		 */
		if( sap->adpaddr < bond_addr ) {
			newap = splitnode(sap, bond_addr );
			sap = newap;
			}
		/*
		 * Make sure the entire bonded section will fit into memory
		 *	1. The bonded section must fit into configured memory
		 *	2. Each space node into which the section falls
		 *		must be of type AVAIL
		 *	3. Each space node must be contiguous
		 */
		have = sap->adpaddr + sap->adsize;
		/*
		 * special processing for special people:
		 *	if one needs to allocate more memory
		 *	than is explicitly demanded, do
		 *	so here [ in special.c ]
		 */

		need = sap->adpaddr + osp->oshdr.s_size;
		adjneed(&need, osp, sap);
		eap = sap;
		lastaddr = sap->adpaddr;
		while ( have < need ) {
			lastaddr += eap->adsize;
			eap = eap->adnext;
			if(eap == NULL) {
				lderror(1, aip->bond.aiinlnno, aip->bond.aiinflnm,
					"%.8s, bonded at %.2lx, won't fit into configured memory",
					 (*(osp->oshdr.s_name) == '\0') ? "GROUP" : osp->oshdr.s_name,  bond_addr );
				free(aip);
				break;
				}
			if(eap->adtype != ADAVAIL) {
				bsp = eap->adscnptr;
				lderror(1, aip->bond.aiinlnno, aip->bond.aiinflnm,
					"%.8s at %.2lx overlays previously allocated section %.8s at %.2lx",
					 (*(osp->oshdr.s_name) == '\0') ? "GROUP" : osp->oshdr.s_name,  bond_addr,
					 bsp->oshdr.s_name, eap->adpaddr);
				eap = NULL;
				free(aip);
				break;
				}
			if(eap->adpaddr != lastaddr) {
				lderror(1, aip->bond.aiinlnno, aip->bond.aiinflnm,
					"%.8s enters unconfigured memory at %.2lx",
					 (*(osp->oshdr.s_name) == '\0') ? "GROUP" : osp->oshdr.s_name, lastaddr);
				eap = NULL;
				free(aip);
				break;
				}
			have += eap->adsize;
			}
		if( eap == NULL )
			continue;
		/*
		 * If the bonded section does not end at the end of the
		 * space node, split the space node up. The first will
		 * be SECT, the second AVAIL
		 */
		if ( have > need )
			newap = splitnode(eap, need);
		/*
		 * Reserve and set the output section ptrs
		 * in the address subspace
		 */
		do_alloc(sap,eap,osp);

		free(aip);
		}
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
alc_owners()
{

/*
 * Allocate the output sections which have been given explicit
 * owners.  The owner of a section can be either a memory area, or a
 * region
 */

	ACTITEM *aip,		/* ptr to section having an owner */
		*nextaip;	/* ptr to next item on ownlist    */
	MEMTYPE *mp;		/* ptr to a memory area item      */
#if USEREGIONS
	REGION *rp;		/* ptr to a region item		  */
#endif
	ANODE *ap1, *ap2;
	int	failure;
	ADDRESS align;

I 2
	DBUG_ENTER ("alc_owners");
E 2
	for( aip = (ACTITEM *) ownlist.head; aip != NULL; aip = nextaip ) {
		nextaip = aip->dfownr.ainext;
		/*
		 * It is meaningless to have owners for a DSECT
		 * since they are not allocated
		 */
		if( aip->dfownr.aioutsec->oshdr.s_flags & STYP_DSECT) {
			lderror(1, aip->dfownr.aiinlnno, aip->dfownr.aiinflnm,
			    "DSECT %.8s can't be given an owner",
			    aip->dfownr.aioutsec->oshdr.s_name);
			free(aip);
			continue;
			}

		/* evaluate the alignment value */
		if (aip->dfownr.aioutsec->osalign)
		{
			lineno = aip->dfownr.aioutsec->osnlnno;
			curfilnm = aip->dfownr.aioutsec->osnflnm;
			cur_dot = -1L;
			align = eval( aip->dfownr.aioutsec->osalign );
			chkpower2( align );
			clrlimb( aip->dfownr.aioutsec->osalign );
		}
		else
			align = 0L;

		/*
		 * Look in memlist for the owner's name
		 */
		failure = 1;
		for( mp = (MEMTYPE *) memlist.head; mp != NULL; mp = mp->mtnext ) {
			if( equal(aip->dfownr.ainamown,mp->mtname,8)  &&
			    (can_alloc(mp->mtaddrhd,mp->mtaddrtl,aip->dfownr.aioutsec,
				 &ap1,&ap2,align)) ) {
				/*
				 * Found where to put the output section
				 */
				do_alloc(ap1,ap2,aip->dfownr.aioutsec);
				failure = 0;
				break;
				}
			}
#if USEREGIONS
#if IAPX
    if( IAPX16MAGIC(magic) ) {
#endif
		/*
		 * If the name was not found in the memlist, (failure still
		 * 1), then try the reglist
		 */
		if( ! failure ) {
			free(aip);
			continue;
			}

		for( rp = (REGION *) reglist.head; rp != NULL; rp = rp->rgnext ) {
			if( equal(aip->dfownr.ainamown,rp->rgname,8) &&
			    (can_alloc(rp->rgaddrhd,rp->rgaddrtl,aip->dfownr.aioutsec,
				 &ap1,&ap2,align)) ) {
				/*
				 * Output section goes in this region
				 *  between ap1 and ap2
				 */
				do_alloc(ap1,ap2,aip->dfownr.aioutsec);
				failure = 0;
				break;
				}
			}
#if IAPX
    }	/* end if IAPX16MAGIC */
#endif
#endif /* USEREGIONS */


		/*
		 * If failed in to find where to put the output section,
		 * issue an error message
		 */
		if(failure)
			lderror(1, aip->dfownr.aiinlnno, aip->dfownr.aiinflnm,
				"can't allocate section %.8s into owner %.8s",
				aip->dfownr.aioutsec->oshdr.s_name,aip->dfownr.ainamown);

		free(aip);
		}

I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
alc_attowns()
{

/*
 * Allocate the output sections assigned to any memory type
 * with a given attribute
 */

	ACTITEM *aip,		/* ptr to section tied to an attribute */
		*nextaip;	/* ptr to next item on atownlst  */
	MEMTYPE *mp;		/* ptr to an item on the memlist */
	ANODE   *ap1, *ap2;
	int	failure;
	ADDRESS	align;

I 2
	DBUG_ENTER ("alc_attowns");
E 2
	for( aip = (ACTITEM *) atownlst.head; aip != NULL; aip = nextaip ) {
		nextaip = aip->ownatr.ainext;
		/*
		 * It is meaningless to associate a DSECT with an
		 * attribute, since they are not allocated any memory
		 */
		if( aip->ownatr.aioutsec->oshdr.s_flags & STYP_DSECT) {
			lderror(1, aip->ownatr.aiinlnno, aip->ownatr.aiinflnm,
			    "DSECT %.8s can't be linked to an attribute",
			    aip->ownatr.aioutsec->oshdr.s_name);
			free(aip);
			continue;
			}

		/* evalute the alignment factor */
		if (aip->ownatr.aioutsec->osalign)
		{
			lineno = aip->ownatr.aioutsec->osnlnno;
			curfilnm = aip->ownatr.aioutsec->osnflnm;
			cur_dot = -1L;
			align = eval( aip->ownatr.aioutsec->osalign );
			chkpower2( align );
			clrlimb( aip->ownatr.aioutsec->osalign );
		}
		else
			align = 0L;

		/*
		 * Look for a memory area with the specified attribute
		 */
		failure = 1;
		for( mp = (MEMTYPE *) memlist.head; mp != NULL; mp = mp->mtnext ) {
			if( (aip->ownatr.aiownatt == mp->mtattr) &&
			    (can_alloc(mp->mtaddrhd,mp->mtaddrtl,
				     aip->ownatr.aioutsec,&ap1,&ap2,align)) ) {
				do_alloc(ap1,ap2,aip->ownatr.aioutsec);
				failure = 0;
				break;
				}
			}

		if(failure)
			lderror(1,0,NULL, "can't allocate %.8s with attr %x",
			      aip->ownatr.aioutsec->oshdr.s_name,aip->ownatr.aiownatt);

		free(aip);
		}
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
alc_the_rest()
{

/*
 * Allocate every output section that is not as yet allocated
 */

	ANODE **anlp;		/* ptr to (sorted) avail node list      */
	OUTSECT **usp;		/* ptr to (sorted) unalloc outsect list */

	ANODE *ap;		/* ptr to element from anlp	*/
	int numanods,		/* number of avail nodes 	*/
	      nanl;		/* working index into anlp 	*/
	OUTSECT *osp;		/* ptr to element from usp	*/
	int numuaos,		/* number of unalloc outsects   */
	      nuaos;		/* working index into usp	*/
	int cmp_uos(),		/* qsort sorting routine	*/
	    cmp_anl();		/* qsort sorting routine	*/

/*
 * Build a list of unallocated output sections
 */

I 2
	DBUG_ENTER ("alc_the_rest");
E 2
	numuaos = 0;
	for( osp = (OUTSECT *) outsclst.head; osp != NULL; osp = osp->osnext )
		if(osp->oshdr.s_paddr == -1L) {
			/*
			 * Allocate DSECTs so as to start at zero
			 */
			if( osp->oshdr.s_flags & STYP_DSECT) {
				osp->oshdr.s_paddr = 0L;
				listadd(l_DS,&dsectlst,osp);
				}
			else
				numuaos++;
			}
	if(numuaos == 0)
D 2
		return;		/* nothing left to allocate */
E 2
I 2
		DBUG_VOID_RETURN;		/* nothing left to allocate */
E 2

	usp = (OUTSECT **) myalloc(sizeof(OUTSECT *) * (numuaos + 1));
	nuaos = 0;
	for( osp = (OUTSECT *) outsclst.head; osp != NULL; osp = osp->osnext )
		if( (osp->oshdr.s_paddr == -1L)  &&  (! (osp->oshdr.s_flags & STYP_DSECT)) )
				usp[nuaos++] = osp;

#if USEREGIONS
/*
 * Sort only if more than 3 output sections.  This exempts the
 *	  basic load, containing only .text, .data, and .bss sections.
 *	  They will usually be in the usual order.
 *
 * NOTE:  Sort only if REGIONS directives were supplied
 */

#if IAPX
    if( IAPX16MAGIC(magic) )
#endif
	if( (numuaos > 3)  &&  (reglist.head != NULL) )
		qsort(usp, numuaos, sizeof(osp), cmp_uos);
#endif /* USEREGIONS */
	usp[nuaos] = NULL;

/*
 * Build the list of avail space nodes
 *
 * The size of the list used to hold the AVAIL pointers must be big
 * enough to allow for possible fragmentation of an existing AVAIL
 * node during allocation:
 *
 *	1 AVAIL node -> 1 AVAIL node (due to ALIGNment)
 *			1 ADSECT node (assigned to the OUTSECT)
 *			1 AVAIL node (remainder of original AVAIL node)
 *	
 */

	numanods = 0;
	for( ap = (ANODE *) avlist.head; ap != NULL; ap = ap->adnext )
		if( (ap->adtype == ADAVAIL)  &&  ((reglist.head == NULL) || (ap->adregp != NULL)) )
			numanods++;

	anlp = (ANODE **) myalloc( sizeof(ANODE *) * (numanods + 1)  +
				   sizeof(OUTSECT *) * (numuaos) );
	
	nanl = 0;
	for( ap = (ANODE *) avlist.head; ap != NULL; ap = ap->adnext )
		if( (ap->adtype==ADAVAIL)  &&  ((reglist.head == NULL) || (ap->adregp != NULL)) )
			anlp[nanl++] = ap;

#if USEREGIONS
/*
 * Sort the avail space node list, on space size, from low to high
 */

#if IAPX
	if( IAPX16MAGIC(magic) ) 
#endif
	   qsort(anlp,numanods,sizeof(ap),cmp_anl);
#endif /* USEREGIONS */
	anlp[nanl] = NULL;	/* terminating entry	*/

	alc_lists(usp, anlp, nanl);

	free(usp);
	free(anlp);
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
audit_groups()
{
	OUTSECT *grpp, *osp, *prevp, *p;
	ANODE *ap;
	ADDRESS addr;

I 2
	DBUG_ENTER ("audit_groups");
E 2
	prevp = NULL;
	grpp = (OUTSECT *) outsclst.head;
	while (grpp) {
		if( grpp->oshdr.s_flags & STYP_GROUP) {
			addr = grpp->oshdr.s_paddr;
			if( (ap=findsanode(grpp)) == NULL )
				lderror(2,0,NULL, "internal error: audit_groups, findsanode failure");
			((OUTSECT *) grpp->osinclhd)->osblock = grpp->osblock;
			for( osp = (OUTSECT *) grpp->osinclhd; osp != NULL; osp = osp->osnext ) 
				if (osp->oshdr.s_flags & STYP_DSECT) {
					osp->oshdr.s_paddr = addr;
					listadd( l_DS, &dsectlst, osp );
				} else {
					if ( !(ap->adscnptr->oshdr.s_flags & STYP_GROUP))
						ap = splitnode( ap, addr );
					ap->adscnptr = osp;
					if( ap->adpaddr != addr )
						lderror(2,0,NULL, "internal error: audit_groups, address mismatch");
					osp->oshdr.s_paddr = addr;
					addr += osp->oshdr.s_size;
				}
			if( prevp )
				prevp->osnext = (OUTSECT *) grpp->osinclhd;
			else
				outsclst.head = (char *) grpp->osinclhd;
			prevp = (OUTSECT *) grpp->osincltl;
			prevp->osnext = grpp->osnext;
			p = grpp;
			grpp = grpp->osnext;
			if( grpp == NULL )
				outsclst.tail = (char *) prevp;
			free(p);
			}
		else {
			prevp = grpp;
			grpp = grpp->osnext;
			}
		}
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
audit_regions()
{

/*
 * 1. Compute the virtual addresses of each output section
 *	The logic used by ld results in physical addresses being computed
 *	prior to virtual addresses
 *
 * 2. Perform a consistency check, to make sure sufficient regions 
 *	have been defined, where regions are necessary
 */

	register ANODE *ap;
	register OUTSECT *osp;
#if USEREGIONS
	register REGION	*rp;
I 2
#endif
E 2

I 2
	DBUG_ENTER ("audit_regions");
#if USEREGIONS
E 2
#if IAPX
	if( IAPX16MAGIC(magic) )
#endif
	   if( reglist.head == NULL ) {
		lderror(1,0,NULL, "internal error: audit_regions detected no regions built");
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
		}
#endif /* USEREGIONS */

	for( ap = (ANODE *) avlist.head; ap != NULL; ap = ap->adnext ) {
		if( ap->adtype == ADSECT) {
#if USEREGIONS
#if IAPX
		    if( IAPX16MAGIC(magic) ) {
#endif
			rp = ap->adregp;
			osp = ap->adscnptr;
			if( ap->adpaddr == osp->oshdr.s_paddr )
				if( rp != NULL)
					osp->oshdr.s_vaddr = rp->rgvaddr + ap->adpaddr - rp->rgorig;
				else
					osp->oshdr.s_vaddr = osp->oshdr.s_paddr;
			/*
		 	 * Make sure every output section goes into some
		 	 * region
		 	 */
			if( ! ap->adregp )
				lderror(1,0,NULL,"output section %.8s not allocated into a region",
					osp->oshdr.s_name);
#if IAPX
		    }else{
			osp = ap->adscnptr;
			if( ap->adpaddr == osp->oshdr.s_paddr )
				osp->oshdr.s_vaddr = osp->oshdr.s_paddr;
		    }	/* end if IAPX16MAGIC */
#endif
#else /* !USEREGIONS */
		    osp = ap->adscnptr;
		    if( ap->adpaddr == osp->oshdr.s_paddr )
			    osp->oshdr.s_vaddr = osp->oshdr.s_paddr;
#endif /* USEREGIONS */
		    }
		}

D 2
	return;
E 2
I 2
	DBUG_VOID_RETURN;
E 2

}
/*eject*/
ANODE *
findsanode(osp)
OUTSECT	*osp;
{
	
/*
 * Run down the avlist and look for a particular
 * section pointer.  Return the pointer to the
 * ANODE containing it, or return NULL.
 */

	register ANODE	*ap;

I 2
	DBUG_ENTER ("findsanode");
E 2
	for( ap = (ANODE *) avlist.head; ap != NULL; ap = ap->adnext )
		if( ap->adscnptr == osp )
			break;

D 2
	return ( ap );
E 2
I 2
	DBUG_RETURN ( ap );
E 2
}
/*eject*/
alc_lists(usl,anl,nanl)
OUTSECT *usl[];		/* sorted list of output sections */
ANODE *anl[];		/* sorted list of available space */
int nanl;		/* next free entry in the anl list*/
{
	OUTSECT *usp;
	ANODE	*ap1, *ap2, *ap3;
	int	ani,		/* index into sorted ANODEs	*/
		usi,		/* index into sorted OUTSECTs	*/
		new_node;	/* flag if ANODE was split up	*/
	int cmp_anl();		/* qsort sorting routine	*/
	ADDRESS align;

/*
 * For each unallocated output section:
 *	1. Find the first free AVAIL node which can contain it
 *	2. Allocate the OUTSECT node to the AVAIL node
 *
 * The list of unallocated output sections is sorted DECREASING,
 * by section size.
 *
 * The list of available space is sorted INCREASING, on the size of
 * the space
 */

I 2
	DBUG_ENTER ("alc_lists");
E 2
	for( usi = 0; usl[usi]; usi++ ) {

		usp = usl[usi];
		if (usp->osalign)
		{
			lineno = usp->osnlnno;
			curfilnm = usp->osnflnm;
			cur_dot = -1L;
			align = eval( usp->osalign );
			chkpower2( align );
			clrlimb( usp->osalign );
		}
		else
			align = 0L;

		for( ani = 0; anl[ani]; ani++ )
			if( anl[ani]->adsize >=
			    (alignment( align, anl[ani]->adpaddr)
			    + usp->oshdr.s_size) )
				break;

		if(anl[ani] == NULL) {
			lderror(1,0,NULL, "can't allocate output section %.8s, of size %10.1lx",
				usp->oshdr.s_name, usp->oshdr.s_size);
			continue;
			}

		new_node = (anl[ani]->adsize != usp->oshdr.s_size);
		ap3 = anl[ani]->adnext;
		if( ! can_alloc(anl[ani], anl[ani], usp, &ap1, &ap2, align)) {
			lderror(2,0,NULL, "internal error: in allocate lists, list confusion (%d %d)",
				usi, ani);
			}
		do_alloc(ap1, ap2, usp);

		if(new_node) {
			/*
			 * If the node at anl[ani] was fragmented, then
			 * update the anl list.  The current
			 * AVAIL node could have generated either one 
			 * or two smaller AVAIL nodes
			 */
			if( ap1 == anl[ani] )
				anl[ani] = ap1->adnext;
			else
#if USEREGIONS
#if IAPX
			    if( IAPX16MAGIC(magic) ) {
#endif
				if( ap2->adnext != ap3 ) {
					anl[nanl++] = ap2->adnext;
					anl[nanl] = NULL;
					}
			qsort(anl, nanl, sizeof(ap1), cmp_anl);
#if IAPX
			    } else
				if( ap2->adnext != ap3 ) {
					for( new_node = nanl; new_node > ani; new_node-- )
						anl[new_node] = anl[new_node-1];
					anl[ani+1] = ap2->adnext;
					anl[++nanl] = NULL;
					}
#endif
#else 	/* !USEREGIONS */
				if( ap2->adnext != ap3 ) {
					for( new_node = nanl; new_node > ani; new_node-- )
						anl[new_node] = anl[new_node-1];
					anl[ani+1] = ap2->adnext;
					anl[++nanl] = NULL;
					}
#endif	/* USEREGIONS */
			}
		else {
			/*
			 * The node at anl[ani] was competely used,
			 * so remove it from the list and move all
			 * the other entries up one space
			 */
			for( nanl--; anl[ani]; ani++ )
				anl[ani] = anl[ani+1];
			}
		}

D 2
	return(nanl);	/* new length of available space list	*/
E 2
I 2
	DBUG_RETURN(nanl);	/* new length of available space list	*/
E 2
}
/*eject*/
can_alloc(sap,eap,scp,pap1,pap2,align)
ANODE *sap, *eap;
OUTSECT *scp;
ANODE **pap1, **pap2;
ADDRESS align;
{

/*
 * Determine if the output section pointed to by scp can fit
 * somewhere between sap and eap.
 *
 * If it can, pap1 and pap2 are given the subrange start and end,
 * and 1 is returned. Otherwise 0 is returned
 */

	ADDRESS	disp,		/* alignment */
		have,		/* last address allocated */
		need;		/* final needed allocation */
	register ANODE *ap1, *ap2;	/* working pointers	*/
	ANODE *splitnode();
	int	success;

I 2
	DBUG_ENTER ("can_alloc");
E 2
	success = 0;
	ap2=sap;
	while(!success) {
		ap1 = ap2;
		while(! (ap1->adtype == ADAVAIL && (ap1->adregp != NULL || reglist.head == NULL)) )
			if( ap1 == eap )
D 2
				return(0);
E 2
I 2
				DBUG_RETURN(0);
E 2
			else
				ap1=ap1->adnext;

		disp=alignment( align, ap1->adpaddr );
		have = ap1->adsize + ap1->adpaddr;
		need = ap1->adpaddr + disp + scp->oshdr.s_size;
		ap2 = ap1;
		while ( need > have ) {
			if(ap2 == eap)
D 2
				return(0);
E 2
I 2
				DBUG_RETURN(0);
E 2
			ap2 = ap2->adnext;
			
			if(ap2->adtype != ADAVAIL)
				break;

			if(ap2->adprev->adpaddr+ap2->adprev->adsize != ap2->adpaddr)
				break;

			if(ap2->adregp==NULL && reglist.head != NULL )
				break;
			have += ap2->adsize;
			}
		if ( have >= need )
			success = 1;
		}

/*
 * To reach this point, success must have been achieved
 */

	if ( disp != 0L ) {
		/*
		 * Split the node because of alignment
		 */
		ANODE *newap;
		newap = splitnode(ap1,ap1->adpaddr+disp);
		if(ap1 == ap2)
			ap2 = newap;
		ap1 = newap;
		}
	if ( have > need ) {
		/*
		 * If ending in the middle of a node, split the node
		 */
		ANODE *newap;
		newap = splitnode(ap2,need);
		}

	*pap1 = ap1;	/* output parameters */
	*pap2 = ap2;

D 2
	return(1);
E 2
I 2
	DBUG_RETURN(1);
E 2
}
/*eject*/
ANODE *
splitnode(ap,addr)
ANODE *ap;
ADDRESS addr;
{
	register ANODE *newap;

I 2
	DBUG_ENTER ("splitnode");
E 2
	newap = (ANODE *) myalloc( sizeof( ANODE ));
	*newap = *ap;

	if( ap->adnext != NULL )
		ap->adnext->adprev = newap;
	newap->adprev = ap;
	ap->adnext = newap;

	ap->adsize = addr - ap->adpaddr;
	newap->adsize -= ap->adsize;
	newap->adpaddr = addr;

	if( (ANODE *) avlist.tail == ap )
		avlist.tail = (char *) newap;

	if( ap->admemp->mtaddrtl == ap )
		ap->admemp->mtaddrtl = newap;
	if( (ap->adregp != NULL)  &&  (ap->adregp->rgaddrtl == ap) )
		ap->adregp->rgaddrtl = newap;

D 2
	return(newap);
E 2
I 2
	DBUG_RETURN(newap);
E 2
}
/*eject*/
do_alloc(ap1,ap2,scp)
ANODE *ap1, *ap2;
OUTSECT *scp;
{
	register ANODE *ap;

I 2
	DBUG_ENTER ("do_alloc");
E 2
	for( ap = ap1; ap != ap2->adnext; ap = ap->adnext ) {
		ap->adtype = ADSECT;
		ap->adscnptr = scp;
		}

	scp->oshdr.s_paddr = ap1->adpaddr;
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
ANODE *
findnode(adr,flg)
ADDRESS adr;
int flg;
{

/*
 * "flg" is used if the adr falls between two nodes.
 * 	flg == 1: return next node,
 * 	flg == 0: return prev node.
 */

	register ANODE *ap;

I 2
	DBUG_ENTER ("findnode");
E 2
	for( ap = (ANODE *) avlist.head; ap != NULL; ap = ap->adnext ) {

		if(ap->adpaddr > adr)
			/*
			 * adr is between nodes
			 */
			if( flg )
D 2
				return(ap);
E 2
I 2
				DBUG_RETURN(ap);
E 2
			else
D 2
				return(ap->adprev);
E 2
I 2
				DBUG_RETURN(ap->adprev);
E 2

		if( ap->adpaddr+ap->adsize > adr )
			/*
			 * adr is within a node
			 */
D 2
			return(ap);
E 2
I 2
			DBUG_RETURN(ap);
E 2

		}

	/*
	 * adr is outside all configured memory
	 */

D 2
	return((ANODE *) -1);
E 2
I 2
	DBUG_RETURN((ANODE *) -1);
E 2
}
/*eject*/
cmp_anl(p1,p2)
ANODE **p1, **p2;
{

/*
 * Compare two ANODEs, returning:
 *
 *	-1	when p1 <  p2
 *	 0	when p1 == p2
 *	+1	when p1 >  p2
 *
 * The adsize field is used for the comparison
 */

D 2
	if( (*p1)->adsize <  (*p2)->adsize ) return(-1);
E 2
I 2
	DBUG_ENTER ("cmp_anl");
	if( (*p1)->adsize <  (*p2)->adsize ) DBUG_RETURN(-1);
E 2

D 2
	if( (*p1)->adsize == (*p2)->adsize ) return(0);
E 2
I 2
	if( (*p1)->adsize == (*p2)->adsize ) DBUG_RETURN(0);
E 2

D 2
	return(1);
E 2
I 2
	DBUG_RETURN(1);
E 2
}





cmp_uos(p1,p2)
OUTSECT **p1, **p2;
{

/*
 * Compare two OUTSECT nodes, returning:
 *
 *	+1	when p1 <  p2
 *	 0	when p1 == p2
 *	-1	when p1 >  p2
 *
 * The oshdr.s_size field is used for the comparison
 */

D 2
	if( (*p1)->oshdr.s_size < (*p2)->oshdr.s_size ) return(1);
E 2
I 2
	DBUG_ENTER ("cmp_uos");
	if( (*p1)->oshdr.s_size < (*p2)->oshdr.s_size ) DBUG_RETURN(1);
E 2

D 2
	if( (*p1)->oshdr.s_size == (*p2)->oshdr.s_size ) return(0);
E 2
I 2
	if( (*p1)->oshdr.s_size == (*p2)->oshdr.s_size ) DBUG_RETURN(0);
E 2

D 2
	return(-1);
E 2
I 2
	DBUG_RETURN(-1);
E 2
}




long
alignment(align,paddr)
ADDRESS align ;	/* this will be a power or two */
ADDRESS paddr;
{
	register ADDRESS l;

I 2
	DBUG_ENTER ("alignment");
E 2
	if ( align <= 1 )
D 2
		return(0);
E 2
I 2
		DBUG_RETURN(0);
E 2

	l = (paddr + (align - 1)) & ~(align - 1);

D 2
	return(l - paddr);
E 2
I 2
	DBUG_RETURN(l - paddr);
E 2
}
/*eject*/
creatsym(name,value)
char *name;
long value;
{

/*
 * Generate a special ld-define symbol. The name of the symbol is
 * pointed to by 'name', and its value is given by 'value'
 *
 * Such a symbol is defined to have a basic symbol type of T_NULL
 */

	register SYMTAB *p;
	SYMENT q;

I 2
	DBUG_ENTER ("creatsym");
E 2
/*
 * Special symbols are generated only under the "-a" flag
 */

	if( ! aflag )
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2

	if( (p = (SYMTAB *) findsym(name)) == NULL ) {
		/*
		 * Case 1: This is the first time the special symbol
		 *		has been encountered
		 */
		zero(&q, SYMESZ);
#if FLEXNAMES
		if (strlen(name) > 8) {
			q.n_zeroes = 0L;
			q.n_nptr = name;
			}
		else
#endif
			copy(q.n_name, name, 8);
		q.n_sclass = C_EXT;
		q.n_scnum = -1;		/* for absolute symbol */
		q.n_value = value;
		q.n_type = T_NULL;
		p = makesym(&q, NULL);
		}
	else if( p->sment.n_scnum == 0 ) {
		/*
		 * Case 2: The special symbol has been previously
		 *		referenced but not defined
		 */
		p->sment.n_sclass = C_EXT;
		p->sment.n_scnum = -1;
		p->sment.n_value = value;
		p->sment.n_type = T_NULL;
		}

	if( p->sment.n_scnum == -1 )
		p->smnewval = p->sment.n_value;

I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
