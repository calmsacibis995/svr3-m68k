/*	@(#)codeout.c	6.1		*/
#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "codeout.h"
#include "gendefs.h"
#include "dbug.h"

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
	DBUG_ENTER ("codgen");
	if (nbits) {
		if (nbits % BITSPBY)
			aerror("illegal code generation");
		newdot += nbits / BITSPBY;
		val <<= (sizeof(long)-sizeof(int))*BITSPBY;
		fwrite((char *)&val,(nbits/BITSPBY),1,fdsect);
		};
	DBUG_VOID_RETURN;
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

	DBUG_ENTER ("codgen");
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
	DBUG_VOID_RETURN;
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

	DBUG_ENTER ("codout");
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
	DBUG_VOID_RETURN;
}
