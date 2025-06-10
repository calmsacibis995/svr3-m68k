h44834
s 00000/00000/00131
d D 1.6 86/09/16 15:28:08 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00131
d D 1.5 86/09/15 14:01:13 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00131
d D 1.4 86/09/11 13:01:45 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00131
d D 1.3 86/08/18 08:47:58 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00007/00000/00124
d D 1.2 86/08/07 08:03:01 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00124/00000/00000
d D 1.1 86/08/04 09:37:58 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)codeout.c	6.1		*/
#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "codeout.h"
#include "gendefs.h"
I 2
#include "dbug.h"
E 2

long	newdot;

FILE	*fdcode;
extern FILE
	*fdsect;

extern symbol *dot;

extern symbol symtab[];

#if DCODGEN /* direct code generation of arbitrary number of bytes */

codgen(nbits, val)
	register short nbits;
	long val;
{
I 2
	DBUG_ENTER ("codgen");
E 2
	if (nbits) {
		if (nbits % BITSPBY)
			aerror("illegal code generation");
		newdot += nbits / BITSPBY;
		val <<= (sizeof(long)-sizeof(int))*BITSPBY;
		fwrite((char *)&val,(nbits/BITSPBY),1,fdsect);
		};
I 2
	DBUG_VOID_RETURN;
E 2
}

#else

static short
	poscnt = 0,
	bitpos = 0;

static OUTWTYPE	outword;
static int mask[] = { 0, 1, 3, 7,
		017, 037, 077,
		0177, 0377, 0777,
		01777, 03777, 07777,
		017777, 037777, 077777,
		0177777 };
codgen(nbits, val)
	register short nbits;
	long val;
{
	register short size;
	register long value;

I 2
	DBUG_ENTER ("codgen");
E 2
start:	if (nbits)
	{
		value = val;
		if ((size = BITSPOW - poscnt) > nbits)
			size = nbits;
		value >>= nbits - size;
		value &= mask[size];
		value <<= BITSPOW - poscnt - size;
		outword |= (OUTWTYPE)value;
		poscnt += size;
		bitpos += size;
		newdot += bitpos / BITSPBY;
		bitpos %= BITSPBY;
		if (poscnt == BITSPOW)
		{
			poscnt = 0;
			OUT(outword,fdsect);
			outword = 0;
			if (nbits > size)
			{
				nbits -= size;
				goto start;
			}
		}
	}
I 2
	DBUG_VOID_RETURN;
E 2
}
#endif

extern int (*(modes[]))();

extern upsymins *lookup();

extern short sttop;

codout(file,start,type)
	char *file;
	long start;
	short type;
{
	codebuf code;
	register symbol *sym;

I 2
	DBUG_ENTER ("codout");
E 2
	dot = (*lookup(".",INSTALL,USRNAME)).stp;
	dot->value = newdot = start;
	dot->styp = type;
	if ((fdcode = fopen(file, "r")) == NULL)
		aerror("Unable to open temporary file");

#ifndef DCODGEN
	poscnt = 0;
#endif
	while (fread((char *)(&code),sizeof(code),1,fdcode) == 1) {
#if VAX
		if (code.cindex < 0)
			sym = (symbol*)(-code.cindex);
		else
			sym = (symbol*)(code.cindex);
#else
		sym = code.cindex ? (symtab + (code.cindex-1))
				: (symbol *) NULL;
#endif
		if (code.caction > NACTION)
			aerror("Invalid action routine");
		else if (code.caction != 0)
			(*(modes[code.caction]))(sym,&code);
		codgen((short)(code.cnbits),code.cvalue);
	}

	if (sttop > 0)
		aerror("Unbalanced Symbol Table Entries-Missing Scope Endings");
	fclose(fdcode);
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
