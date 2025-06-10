h20672
s 00005/00000/00057
d D 1.2 86/08/18 08:49:27 fnf 2 1
c Checkpoint changes at first working shared library version
e
s 00057/00000/00000
d D 1.1 86/07/31 14:39:27 fnf 1 0
c Initial copy from 5.3 distribution for AT&T 3b2.
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

#ident	"@(#)mkshlib:common/t_mktarget.c	1.3"

#include <stdio.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "paths.h"
#include "shlib.h"
#include "trg.h"
I 2
#include "dbug.h"
E 2


void
mktarget()
{
I 2
	DBUG_ENTER ("mktarget");
E 2
	mkplt();	/* make the intermediate partially loaded target, tpltnam */

	/* Build the branch table and create btname which will contain the branch
	 * table as well as the .lib section for the target.
	 * Also, modify tpltnam so that each symbol which has a branch table
	 * entry has its name changed in the symbol table of tpltnam to the
	 * destination label of its corresponding jump in the branch table and 
	 * put this modified file in pltname.
	 */
	mkbt();

	/* Create the target by loading pltname and btname together at the
	 * addresses specified in ifil2name. */
	if (execute(ldname,ldname,"-c","-o",trgname,ifil2name,btname,
								pltname,(char *)0))
		fatal("Internal %s invocation failed",ldname);
I 2
	DBUG_VOID_RETURN;
E 2
}


void
mkplt()
{
	int	i;
	FILE	*ifil;

I 2
	DBUG_ENTER ("mkplt");
E 2
	/* open ifil1name */
	if ((ifil= fopen(ifil1name,"w")) == NULL)
		fatal("Cannot open temp file");

	/* now place all the input object files in ifil1name */
	for (i=0; i<numobjs; i++)
		(void)fprintf(ifil,"%s\n",trgobjects[i]);
	(void)fclose(ifil);

	if (execute(ldname,ldname,"-r","-o",tpltnam, ifil1name,(char *)0))
		fatal("Internal %s invocation failed",ldname);
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
