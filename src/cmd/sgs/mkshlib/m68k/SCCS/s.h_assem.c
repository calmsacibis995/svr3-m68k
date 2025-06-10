h36204
s 00007/00002/00053
d D 1.3 86/08/18 08:49:37 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00007/00005/00048
d D 1.2 86/08/11 09:54:01 fnf 2 1
c Checkpoint changes to date.
e
s 00053/00000/00000
d D 1.1 86/07/31 14:38:47 fnf 1 0
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

D 2
#ident	"@(#)mkshlib:m32/h_assem.c	1.2"
E 2
I 2
#ident	"@(#)mkshlib:m68k/h_assem.c	1.2"
E 2

#include <stdio.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "shlib.h"
#include "hst.h"
I 3
#include "dbug.h"
E 3

/* prlal() prints the leading lines of an assembly file */
void
prlal(assemf, curname)
FILE	*assemf;
char	*curname;
{
I 3
	DBUG_ENTER ("prlal");
E 3
	/* Print leading lines of assembly file.
	 * The beginning of the assembly file should look like:
	 *
D 2
	 *      .file   "<filename>"
E 2
I 2
	 *      file   "<filename>"
E 2
	 *      .ident  "<ident string>"
	 *      .section	.init,"x"
	 */
D 2
	(void)fprintf(assemf,".file   \"%s\"\n",curname);
E 2
I 2
	(void)fprintf(assemf,"\tfile\t\"%s\"\n",curname);
#ifndef m68k	/* disable this for now on m68k (fnf) */
E 2
	if (idstr != 0)
		(void)fprintf(assemf,".ident  \"%s\"\n",idstr);
D 2
	(void)fputs(".section\t.init,\"x\"\n",assemf);
E 2
I 2
#endif
	(void)fputs("\tinit\n",assemf);
I 3
	DBUG_VOID_RETURN;
E 3
E 2
}


/* Print initialization code */
void
initpr(assemf, import, pimport)
FILE	*assemf;
char	*import;
char	*pimport;
{
I 3
	DBUG_ENTER ("initpr");
E 3
	/* Generate initialization code.
	* Each initialization specification line should result in
	* the generation of the following code:
	*
D 3
	*      movw    &<import> <pimport>
E 3
I 3
	*      mov.l    &<import> <pimport>
E 3
	*/
D 2
	(void)fprintf(assemf,"movw\t&%s,%s\n",import,pimport);
E 2
I 2
D 3
	(void)fprintf(assemf,"\tmov.w\t&%s,%s\n",import,pimport);
E 3
I 3
	(void)fprintf(assemf,"\tmov.l\t&%s,%s\n",import,pimport);
	DBUG_VOID_RETURN;
E 3
E 2
}
E 1
