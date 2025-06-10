h61248
s 00000/00000/00104
d D 1.6 86/09/16 15:28:18 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00104
d D 1.5 86/09/15 14:01:22 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00104
d D 1.4 86/09/11 13:01:54 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00104
d D 1.3 86/08/18 08:48:05 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00007/00000/00097
d D 1.2 86/08/07 08:03:12 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00097/00000/00000
d D 1.1 86/08/04 09:38:02 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)getstab.c	6.1		*/
/*
 *
 *	"getstab.c"  is  a  file  that contains routines for post-pass1
 *	processing of the symbol table.   The  following  routines  are
 *	provided:
 *
 *	getstab(file)	This  function  reads  the  symbol  table  from
 *			a file whose name  is  given  as  a  parameter.
 *			This is needed by the second and third passes.
 *
 *	fixstab(addr,incr,type)	This  function  "fixes" elements of the
 *			symbol table by adding  "incr"  to  all  values
 *			of symbols of type "type" that are greater than
 *			"addr".   This  is necessary when the length of
 *			of an instruction is increased.
 *
 */

#include <stdio.h>
#include "systems.h"
#include "gendefs.h"
#include "symbols.h"
I 2
#include "dbug.h"
E 2

#if !ONEPROC
/*
 *
 *	"getstab" is a routine that gets the symbol table from an
 *	intermediate file at the beginning of pass 2.  It
 *	opens the file whose name is passed as an argument, and reads
 *	lines of the form:
 *
 *	'action'	'symbol table index'	'nbits'	'value'
 *
 *	It calls "lookup" for each symbol, and stores the "type" and
 *	"value" into the entry returned by "lookup".
 *
 */

extern FILE *fopen();

extern upsymins *lookup();

getstab(file)
	char *file;
{
	FILE	*fd;
	symbol	symb;
	register symbol
		*ptr;

I 2
	DBUG_ENTER ("getstab");
E 2
	if((fd = fopen(file,"r"))==NULL)
		aerror("Cannot Open Temporary (symbol table) File");
	while(fread((char *)&symb,SYMBOLL,1,fd) == 1) {
		ptr = (*lookup(symb.name,INSTALL,USRNAME)).stp;
		ptr->styp = symb.styp;
		ptr->value = symb.value;
	}
	fclose(fd);
I 2
	DBUG_VOID_RETURN;
E 2
}
#endif

/*
 *
 *	"fixstab" is a function that "fixes" elements of the symbol
 *	table by adding "incr" to the value of all symbols of type
 *	"type" with a value greater than "addr".  This function stores
 *	its parameters into the global variables "fxaddr", "fxincr",
 *	and "fxtype".  It then calls "traverse" causing it to call
 *	"fix" to fix all elements in the table of ordinary symbols,
 *	and calls "ltraverse" causing it to call "lfix" to fix all
 *	elements in the table of compiler generated symbols.
 *
 */

static long fxaddr;
static long fxincr;
static short fxtype;

fix(ptr)
	symbol *ptr;
{
I 2
	DBUG_ENTER ("fix");
E 2
	if (((ptr->styp & TYPE) == fxtype) &&
		(ptr->value >= fxaddr))
		ptr->value += fxincr;
I 2
	DBUG_VOID_RETURN;
E 2
}

fixstab(addr,incr,type)
	long addr;
	long incr;
	short type;
{
I 2
	DBUG_ENTER ("fixstab");
E 2
	fxaddr = addr;
	fxincr = incr;
	fxtype = type;
	traverse(fix);
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
