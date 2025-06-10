h22054
s 00003/00000/00053
d D 1.2 86/08/18 08:49:05 fnf 2 1
c Checkpoint changes at first working shared library version
e
s 00053/00000/00000
d D 1.1 86/07/31 14:39:17 fnf 1 0
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

#ident	"@(#)mkshlib:common/h_mkhost.c	1.2"

#include <stdio.h>
#include "filehdr.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "ldfcn.h"
#include "shlib.h"
#include "hst.h"
I 2
#include "dbug.h"
E 2


/* mkhost() creates the host shared library.  The creation of the host shared library
 * requires essentially 3 steps:
 *	1. Creation of the library definition object.
 *	2. Modification of the library's constituent .o files, including the 
 *	   routine.
 *	3. Actual creation of the host archive.
 */

mkhost()
{
	long	i;

I 2
	DBUG_ENTER ("mkhost");
E 2
	init_trgsymtab();	/* initialize the target symbol hash table */

	defsuffix= getdsuf();	/* get the suffix for the file definition symbols */
	libdefsym= mkdefsym(defname, defsuffix);  /* create libdefsym */
	mklibdef();	/* make the library definition file */

	/* allocate the objects array */
	if ((objects= (obj **)malloc((unsigned)(sizeof(obj *) * numobjs))) == NULL)
		fatal("Out of space");

	/* for each object file used to build the target record all pertinent
	 * information in objlst */
	for (i=0; i<numobjs; i++)
		readobj(i);

	refchain();	/* trace the unresolved reference chains and firm up the
			 * links with file definition symbols */

	checkinits();	/* associate .init sections with the proper objects */
	creatobjs();	/* create the modified constituent object files */
	archive();	/* archive the mess */
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
