h59589
s 00009/00000/00094
d D 1.3 86/08/18 08:49:41 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00009/00006/00085
d D 1.2 86/08/11 09:54:06 fnf 2 1
c Checkpoint changes to date.
e
s 00091/00000/00000
d D 1.1 86/07/31 14:38:50 fnf 1 0
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

#ident	"@(#)mkshlib:m32/t_btassem.c	1.2"

#include <stdio.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "shlib.h"
#include "trg.h"
I 3
#include "dbug.h"
E 3


/* This function prints the leading lines of assemble code for the branch table */
void
prlbtassem(assembt)
FILE	*assembt;
{
I 3
	DBUG_ENTER ("prlbtassem");
E 3
	/* Print leading lines of assembly file.
	 * The beginning of the assemble file should look like:
	 *
	 *	.file	"<filename>"
	 *	.ident	"<ident string>"  #if ident string exists
	 *	.text
	 */
D 2
	(void)fprintf(assembt,".file\t\"branchtab\"\n");
E 2
I 2
	(void)fprintf(assembt,"\tfile\t\"branchtab\"\n");
#ifndef m68k
E 2
	if (idstr != 0)
		(void)fprintf(assembt,".ident\t\"%s\"\n",idstr);
D 2
	(void)fprintf(assembt,".text\n");
E 2
I 2
#endif
	(void)fprintf(assembt,"\ttext\n");
I 3
	DBUG_VOID_RETURN;
E 3
E 2
}


/* This function generates the assembly code for a label in the branch table */
void
labelassem(symname, assembt)
char	*symname;
FILE	*assembt;
{
I 3
	DBUG_ENTER ("labelassem");
E 3
D 2
	(void)fprintf(assembt,".globl\t%s\n%s:\n",symname,symname);
E 2
I 2
	(void)fprintf(assembt,"\tglobal\t%s\n%s:\n",symname,symname);
I 3
	DBUG_VOID_RETURN;
E 3
E 2
}

/* This function generates the assembly code for the branch table */
void
genbtassem(label, assembt)
char	*label;
FILE	*assembt;
{
I 3
	DBUG_ENTER ("genbtassem");
E 3
	/* Each entry in the branch table will have the following format:
	 *
	 *		jmp	<label name>
	 */
	(void)fprintf(assembt,"\tjmp\t%s\n",label);
I 3
	DBUG_VOID_RETURN;
E 3
}


/* This function generates the assembly code for the .lib section */
void
genlibassem(assembt)
FILE	*assembt;
{
	int	size;	/* length of target pathname */
	int	i;
	char	*p;
	int	slong,
		len;

I 3
	DBUG_ENTER ("genlibassem");
E 3
	/* The .lib contents will have the following format:
	 *
	 *	.section	.lib,"l"
	 *	.word	num,2	# where num is the size of the current entry
	 *			# i.e. length of the target pathname + other info.
	 *	.byte	<target pathname in hex>
	 */
D 2
	(void)fprintf(assembt,".section\t.lib,\"l\"\n");
E 2
I 2
	(void)fprintf(assembt,"\tlib\n");
E 2
	len= strlen(trgpath)+1;
	slong= sizeof(long);
	size= (len+(slong-1))/slong + 2; 	/* the last term represents the two
						   leading words- the 1st describes
						   the total length and the 2nd
						   describes the offset of the
						   non-optional information */
D 2
	(void)fprintf(assembt,".word\t%d,2\n",size);
	(void)fprintf(assembt,".byte\t");
E 2
I 2
	(void)fprintf(assembt,"\tlong\t%d,2\n",size);
	(void)fprintf(assembt,"\tbyte\t");
E 2
	for (p=trgpath;*p!='\0';p++) {
		(void)fprintf(assembt,"0x%x,",*p);
	}
	(void)putc('0',assembt);
I 2
	(void)putc('\n',assembt);
I 3
	DBUG_VOID_RETURN;
E 3
E 2
}
E 1
