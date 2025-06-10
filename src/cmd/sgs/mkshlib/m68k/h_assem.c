/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:m68k/h_assem.c	1.2"

#include <stdio.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "shlib.h"
#include "hst.h"
#include "dbug.h"

static long bytecount = 0;

/* prlal() prints the leading lines of an assembly file */
void
prlal(assemf, curname)
FILE	*assemf;
char	*curname;
{
	DBUG_ENTER ("prlal");
	/* Print leading lines of assembly file.
	 * The beginning of the assembly file should look like:
	 *
	 *      file   "<filename>"
	 *      .ident  "<ident string>"
	 *      .section	.init,"x"
	 */
	(void)fprintf(assemf,"\tfile\t\"%s\"\n",curname);
#ifndef m68k	/* disable this for now on m68k (fnf) */
	if (idstr != 0)
		(void)fprintf(assemf,".ident  \"%s\"\n",idstr);
#endif
	(void)fputs("\tinit\n",assemf);
	DBUG_VOID_RETURN;
}


/* Print initialization code */
void
initpr(assemf, import, pimport)
FILE	*assemf;
char	*import;
char	*pimport;
{
	DBUG_ENTER ("initpr");
	/* Generate initialization code.
	* Each initialization specification line should result in
	* the generation of the following code:
	*
	*      mov.l    &<import> <pimport>
	*
	*/
	(void)fprintf(assemf,"\tmov.l\t&%s,%s\n",import,pimport);
	bytecount += 10;
	DBUG_VOID_RETURN;
}

/*
 *	Pad initialization code with NOP's til multiple of 4 bytes.
 *	The NOP is required on the 68K family, because the mov.l
 *	sequence above is 10 bytes.  Many small fragments such as
 *	this get combined by the loader from different object files.
 *	Each fragment MUST be a multiple of 4 bytes (see COFF spec).
 */

void
padpr(assemf)
FILE	*assemf;
{
	DBUG_ENTER ("padpr");
	switch (bytecount % 4) {
		case 0:
			break;
		case 1:
		case 3:
			(void)fprintf(stderr, "Warning: botched init sect!\n");
			break;
		case 2:
			(void)fprintf(assemf,"\tnop\n");
			break;
	}
	bytecount = 0;
	DBUG_VOID_RETURN;
}
