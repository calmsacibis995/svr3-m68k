h56845
s 00000/00000/00129
d D 1.3 86/08/18 08:45:12 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00003/00000/00126
d D 1.2 86/07/30 14:23:16 fnf 2 1
c Add macro based C debugging package macros.
e
s 00126/00000/00000
d D 1.1 86/07/29 14:31:50 fnf 1 0
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

#ident	"@(#)ld:common/maps.c	1.7"

#include "system.h"
#include <stdio.h>
#include "structs.h"
#include "attributes.h"
#include "extrns.h"
#include "sgsmacros.h"
I 2
#include "dbug.h"		/* Special macro based debugging package */
E 2


/*
 * This version of maps.c, for m80ld, does NOT have:
 *	1. REGION directives
 *	2. physical addresses
 */
/*eject*/
ldmap()
{

/*
 * Generate the output map requested through the "-m" flag
 */

	register ANODE *a;

I 2
	DBUG_ENTER ("ldmap");
E 2
/*
 * Step 1: Output any non-default description of memory
 */

	if (memlist.head != memlist.tail) {
		register MEMTYPE *p;
		printf("\n\n\n\t\t\tMEMORY CONFIGURATION\n");
		printf("\nname\t\torigin\t\tlength\t\tattributes\n\n");
		for( p = (MEMTYPE *) memlist.head; p; p = p->mtnext ) {
			printf("%-8.8s\t%08.2lx\t%08.2lx\t  ",
				p->mtname, p->mtorig, p->mtlength);
			if (p->mtattr & att_R) printf("R");
			if (p->mtattr & att_W) printf("W");
			if (p->mtattr & att_I) printf("I");
			if (p->mtattr & att_X) printf("X");
			printf("\n");
			}
		printf("\n\n");
		}

/*
 * Step 2: Output the virtual memory map
 */

	printf("\t\tLINK EDITOR MEMORY MAP\n\n");
	printf("\noutput\t\tinput\t\tvirtual");
	printf("\nsection\t\tsection\t\taddress\t\tsize\n");

	for (a = (ANODE *) avlist.head; a; a = a->adnext) {
		long disp, ndx, vaddr;
		unsigned fill;
		register INSECT *isp;
		register OUTSECT *osp;

		if (a->adtype == ADPAD)
			continue;

		vaddr = a->adpaddr;

		if (a->adtype == ADAVAIL) {
			printf("\n*avail*\t\t\t\t%08.2lx\t%08.2lx\n",
				vaddr, a->adsize);
			continue;
			}

		osp = a->adscnptr;
		printf("\n%-8.8s\t\t\t%08.2lx\t%08.2lx",
			osp->oshdr.s_name, vaddr, a->adsize);
		if (osp->oshdr.s_flags & STYP_DSECT)
			printf("\tDSECT");
		else if (osp->oshdr.s_flags & STYP_NOLOAD)
			printf("\tNOLOAD");
		else if (osp->oshdr.s_flags & STYP_COPY)
			printf("\tCOPY");
		else if (osp->oshdr.s_scnptr == 0L)
			printf("\tuninitialized");
		printf("\n");

		ndx = 0;
		isp = osp->osinclhd;
		if( (osp->oshdr.s_scnptr != 0L || isp)  &&
		    (! equal(_TV,osp->oshdr.s_name,8)) )
			while (ndx < osp->oshdr.s_size) {
				if (isp)
					disp = isp->isdispl;
				else
					disp = osp->oshdr.s_size;
				if (ndx < disp) {
					if (osp->osflags & FILL)
						fill = osp->osfill;
					else
						fill = globfill;
					printf("\t\t*fill*\t\t%08.2lx\t%08.2lx\t%06.2x\n",
					  	vaddr + ndx,
					  	disp - ndx, fill);
					ndx = disp;
					}
				if (isp) {
					if (isp->ishdr.s_size > 0) {
						printf( "\t\t%-8.8s\t%08.2lx\t%08.2lx %s",
							isp->ishdr.s_name,
        						vaddr + ndx,
        						isp->ishdr.s_size,
        						sname(isp->isfilptr->flname));
						if( isp->isfilptr->flfilndx != -1 )
							printf( " (%d)", isp->isfilptr->flfilndx);
						printf( "\n" );
        					ndx += isp->ishdr.s_size;
    						}
					isp = isp->isincnxt;
					}
				}
		}
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
