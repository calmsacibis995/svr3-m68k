h56231
s 00004/00004/00618
d D 1.6 86/11/17 11:27:30 fnf 6 5
c New preprocess does not accept random junk after #endif.
e
s 00000/00000/00622
d D 1.5 86/09/16 15:26:13 fnf 5 4
c Changes to make ident directive work.
e
s 00000/00000/00622
d D 1.4 86/09/11 13:00:35 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00006/00000/00616
d D 1.3 86/08/18 08:46:48 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00040/00010/00576
d D 1.2 86/08/07 08:07:17 fnf 2 1
c Checkpoint changes for shared library stuff.  Still does not work
c but does accept "init" and "lib" section directives without crashing...
e
s 00586/00000/00000
d D 1.1 86/08/04 09:40:25 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)addr2.c	6.3			*/
static	char	sccsid[] = "@(#) addr2.c 6.3 2/14/85";
/* static	char	sccsid[] = "@(#) addr2.c: 2.8 6/28/83"; */


#include	<stdio.h>
#include	"symbols.h"
#include	"instab.h"
#include	"gendefs.h"
#include	"codeout.h"
#include	"reloc.h"
I 2
#include	"dbug.h"
E 2


extern unsigned short
		relent;	/* Number of reloc entries written to fdrel.	*/

extern long	newdot;

extern symbol	symtab[];

extern FILE	*fdrel,	/* File of relocation table information.	*/
		*fdcode;/* File of intermediate object code.		*/

extern BYTE	*longsdi;


extern	setfile(),
	lineno(),
	linenum(),
	lineval(),
	define(),
	endef(),
	setval(),
	settyp(),
	setlno(),
	setscl(),
	settag(),
	setsiz(),
	setdim1(),
	setdim2(),
	newstmt(),
	genreloc(),
	brloc(),
	braopt(),
	bccopt(),
	bsropt(),
	absopt(),
	swbeg(),
	move(),
	iopt(),
	insti(),
	absbr(),
	ndxreloc(),
	iabsbr();
#ifdef M68881
	extern cpbccopt();
#endif


int	(*(modes[NACTION+2]))() =
	{
	0,
	setfile,
	lineno,
	linenum,
	lineval,
	define,
	endef,
	setval,
	settyp,
	setlno,
	setscl,
	settag,
	setsiz,
	setdim1,
	setdim2,
	newstmt,
	genreloc,
	brloc,
	braopt,
	bccopt,
	bsropt,
	absopt,
	swbeg,
	move,
	iopt,
	insti,
	absbr,
	ndxreloc,
	iabsbr,
#ifdef M68881
	cpbccopt,
#endif
	0
	};


genreloc(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
	char		*rsym;
	prelent		trelent;
	short		stype;
	long		testval;

I 2
	DBUG_ENTER ("genreloc");
E 2
	if (sym != NULLSYM)
	{
		code->cvalue += (sym->value &
			(code->cnbits == BITSPBY?0xff:
			(code->cnbits == BITSPOW?0xffff:0xffffffff)));
		stype = sym->styp & TYPE;
		testval = code->cvalue;
		if ((code->cnbits == BITSPBY && (testval < -128 || testval > 255)) ||
		    (code->cnbits == BITSPOW && (testval < -32768L || testval > 65535L)))
			werror("size of code generated may not hold operand");
	}
	else
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2

	switch (stype)
	{
	case ABS:
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	case TXT:
		rsym = ".text";
		break;
	case DAT:
		rsym = ".data";
		break;
I 3
	case INIT:
		rsym = ".init";
		break;
	case LIB:
		rsym = ".lib";
		break;
E 3
	case BSS:
		rsym = ".bss";
		break;
	case UNDEF:
		sym->styp |= EXTERN;	/* Make sure it gets into the	*/
		rsym = sym->_name.name;	/*	symbol table.		*/
		break;
	default:
		aerror("attempt to relocate weird type");
	}

	trelent.relval = newdot;
	trelent.relname = rsym;
	if (code->cnbits == 8)
		trelent.reltype = R_RELBYTE;
	else if (code->cnbits == 16)
		trelent.reltype = R_RELWORD;
	else
		trelent.reltype = R_RELLONG;
	fwrite((char *)(&trelent),sizeof(prelent),1,fdrel);
	++relent;
I 2
	DBUG_VOID_RETURN;
E 2
}	/* genreloc()	*/



brloc(sym,code)
	register symbol	*sym;
	register codebuf	*code;
{
	prelent		trelent;
	register short	stype;
	register long	offset;

I 2
	DBUG_ENTER ("brloc");
E 2
	if ( sym == NULLSYM )
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	offset = sym->value - newdot;
	if (code->cnbits == BITSPBY)
		offset -= 1;
	code->cvalue += offset;
	stype = sym->styp & TYPE;

	if (stype == UNDEF)
	{
		sym->styp |= EXTERN;	/* Make sure it gets into the	*/
		trelent.relval = newdot;/*	symbol table.		*/
		trelent.relname = sym->_name.name;
		if (code->cnbits == 8)
			trelent.reltype = R_PCRBYTE;
		else
		if (code->cnbits == 16)
			trelent.reltype = R_PCRWORD;
		else
			trelent.reltype = R_PCRLONG;
		fwrite((char *)(&trelent),sizeof(prelent),1,fdrel);
		++relent;
	}
	else				/* check for valid offset */
	{
	    if (offset < 0)
		offset = ~offset;	/* force sign bits to 0 */
	    if (   (code->cnbits == 8  && (offset & ~0x7fL)   != 0)
		|| (code->cnbits == 16 && (offset & ~0x7fffL) != 0)
		)
		aerror("branch offset is too remote");
	}
I 2
	DBUG_VOID_RETURN;
E 2
}	/* brloc()	*/



braopt(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
I 2
	DBUG_ENTER ("braopt");
E 2
	if (*++longsdi)
	{	/* Original bra.w is no good, and the second word	*/
		/* is only good for the symbol it contains.  We need	*/
		/* a jmp abs.l to the symbol in the second code chunk.	*/
#ifndef M68020
		codgen(BITSPOW,0x4ef9);			/*	jmp	*/
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		code->cnbits = 2*BITSPOW;
		code->cvalue = 0;
		genreloc(sym,code);
#else
		/* 68020 allows bra.l */
		codgen(BITSPOW,0x60ff);			/*	bra	*/
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		code->cnbits = 2*BITSPOW;
		code->cvalue = 0;
		brloc(sym,code);
D 6
#endif M68020
E 6
I 6
#endif /* M68020 */
E 6
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	}
	/* Otherwise, original bra.w is ok, so codgen gets code as-is.	*/
I 2
	DBUG_VOID_RETURN;
E 2
}	/* braopt()	*/



bccopt(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
I 2
	DBUG_ENTER ("bccopt");
E 2
	if (*++longsdi)
	{	/* Original bcc.w is no good, and the second word	*/
		/* is only good for the symbol it contains.  Negate	*/
		/* the direction of the branch, then use this to branch	*/
		/* around a jmp.  Direction is reversed by toggling	*/
		/* eighth bit.						*/
#ifndef M68020
		code->cvalue ^= 0x100;

		/* Add a displacement which will get past the jmp.	*/
		code->cvalue |= 6;
		codgen(code->cnbits,code->cvalue);
		codgen(BITSPOW,0x4ef9);			/*	jmp	*/
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		code->cnbits = 2*BITSPOW;
		code->cvalue = 0;
		genreloc(sym,code);	
#else
		/* 68020 allows bcc.l */
		codgen(BITSPOW,code->cvalue | 0xff);	/*	bcc	*/
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		code->cnbits = 2*BITSPOW;
		code->cvalue = 0;
		brloc(sym,code);	
D 6
#endif M68020
E 6
I 6
#endif /* M68020 */
E 6
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	}
	/* Otherwise, original bcc.w is ok, so codgen gets code as-is.	*/
I 2
	DBUG_VOID_RETURN;
E 2
}	/* bccopt()	*/



bsropt(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
I 2
	DBUG_ENTER ("bsropt");
E 2
	if (*++longsdi)
	{	/* Original bsr.w is no good, and the second word	*/
		/* is only good for the symbol it contains.  We need	*/
		/* a jsr abs.l to the symbol in the second code chunk.	*/
#ifndef M68020
		codgen(BITSPOW,0x4eb9);			/*	jsr	*/
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		code->cnbits = 2*BITSPOW;
		code->cvalue = 0;
		genreloc(sym,code);
#else
		/* 68020 allows bsr.l */
		codgen(BITSPOW,0x61ff);			/*	bsr	*/
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		code->cnbits = 2*BITSPOW;
		code->cvalue = 0;
		brloc(sym,code);
D 6
#endif M68020
E 6
I 6
#endif /* M68020 */
E 6
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	}
	/* Otherwise, original bsr.w is ok, so codgen gets code as-is.	*/
I 2
	DBUG_VOID_RETURN;
E 2
}	/* bsropt()	*/



absopt(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
I 2
	DBUG_ENTER ("absopt");
E 2
	if (*++longsdi)
	{	/* original abs.w is no good.  The mode field needs to	*/
		/* be changed to abs.l, and the next word has to be	*/
		/* expanded to two words.				*/
		code->cvalue |= 0x1;
	}
	codgen(code->cnbits,code->cvalue);
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	if (*longsdi)
		code->cnbits = 2*BITSPOW;
	sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
	genreloc(sym,code);
I 2
	DBUG_VOID_RETURN;
E 2
}	/* absopt()	*/



swbeg(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
	int		swbegct;
	int		i;
	long		swtabbase;

I 2
	DBUG_ENTER ("swbeg");
E 2
	codgen(code->cnbits,code->cvalue);
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	swbegct = code->cvalue;
	codgen(code->cnbits,code->cvalue);
	/* Swbeg line ends in newline, followed by a line with just	*/
	/* the switch table beginning, then a newline, then "arg"	*/
	/* switch table elements, each accompanied by a newline.	*/
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
	newstmt(sym,code);
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
	newstmt(sym,code);
	swtabbase = newdot;	/* The value of the location counter	*/
				/*	for the right hand side of the	*/
				/*	label subtraction expression.	*/

	for (i = 1; i < swbegct; i++)
	{
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = symtab + code->cindex - 1;
		codgen(code->cnbits,sym->value - swtabbase);
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		newstmt(sym,code);
	}

	/* We've completed all but one of the elements of the switch	*/
	/* table.  Codout (the routine which invoked this routine)	*/
	/* will call codgen with what we leave in code.  Therefore, the	*/
	/* last element of the switch table is left in code for codout.	*/

	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	sym = symtab + code->cindex - 1;
	code->cvalue = sym->value - swtabbase;
I 2
	DBUG_VOID_RETURN;
E 2
}	/* swbeg()	*/



move(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
	int		mode1;
	int		mode2;

I 2
	DBUG_ENTER ("move");
E 2
	codgen(code->cnbits,code->cvalue);
	mode1 = code->cvalue & 070;
	mode2 = code->cvalue & 0700;
	if (mode1 > 040)	/* Source has more operand data.	*/
	{
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		if (code->caction == GENRELOC)
		{
			sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
			genreloc(sym,code);
		}
		codgen(code->cnbits,code->cvalue);
	}
	if (mode2 > 0400)	/* Destination has more operand data.	*/
	{
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		if (code->caction == GENRELOC)
		{
			sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
			genreloc(sym,code);
		}
		codgen(code->cnbits,code->cvalue);
	}
	code->cnbits = 0;
I 2
	DBUG_VOID_RETURN;
E 2
}	/* move()	*/



iopt(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
I 2
	DBUG_ENTER ("iopt");
E 2
	if (*++longsdi)
		code->cvalue |= 0x1;
	codgen(code->cnbits,code->cvalue);
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	if (code->caction == GENRELOC)
	{
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		genreloc(sym,code);
	}
	codgen(code->cnbits,code->cvalue);
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	if (*longsdi)
		code->cnbits = 2*BITSPOW;
	sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
	genreloc(sym,code);
I 2
	DBUG_VOID_RETURN;
E 2
}	/* iopt()	*/



insti(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
	int		mode;

I 2
	DBUG_ENTER ("insti");
E 2
	mode = code->cvalue & 070;
	codgen(code->cnbits,code->cvalue);
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	if (code->caction == GENRELOC)
	{
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		genreloc(sym,code);
	}
	if (mode > 040)
	{
		codgen(code->cnbits,code->cvalue);
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		if (code->caction == GENRELOC)
		{
			sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
			genreloc(sym,code);
		}
	}
I 2
	DBUG_VOID_RETURN;
E 2
}	/* insti()	*/



/* 
 *	absbr(sym,code)
 *	This module is good to use iff the instruction has one
 *	instruction word before any extension words.	Nov,12,1984 SFL
 *
 */
absbr(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
I 2
	DBUG_ENTER ("absbr");
E 2
	if (*++longsdi)
	{	/* Original pc-relative instruction is no good, so we	*/
		/* convert it to the same instruction using the abs.l	*/
		/* mode.						*/
		code->cvalue ^= 03;
		codgen(code->cnbits,code->cvalue);
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		code->cnbits = 2*BITSPOW;
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		genreloc(sym,code);
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	}
	codgen(code->cnbits,code->cvalue);
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	if (code->cindex)
	{
		sym = symtab + code->cindex - 1;
		brloc(sym,code);
	}
I 2
	DBUG_VOID_RETURN;
E 2
}	/* absbr()	*/



/* 
 *	iabsbr(sym,code)
 *	
 *	This module is simular to absbr. However, it is used by
 *  instruction which may have 2 or more instruction words
 *	before extention words. Therefore, if original pc-relative
 *	instruction does't work, convert it to the same instruction
 *	using the abs.l mode and the next word after all the instruction
 *	words has to be expanded to two words. 	nov,12,1984 SFL
 *		
 */
iabsbr(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
I 2
	DBUG_ENTER ("iabsbr");
E 2
	if (*++longsdi)
	{	/* Original pc-relative instruction is no good, so we	*/
		/* convert it to the same instruction using the abs.l	*/
		/* mode.						*/
		code->cvalue ^= 03;
		codgen(code->cnbits,code->cvalue);
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		if (code->caction == GENRELOC)
		{
			sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
			genreloc(sym,code);
		}
		codgen(code->cnbits,code->cvalue);
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		code->cnbits = 2*BITSPOW;
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		genreloc(sym,code);
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	}
	codgen(code->cnbits,code->cvalue);
	if (fread(code,sizeof(*code),1,fdcode) != 1)
		aerror("unexpected EOF on temporary (code) file");
	sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
	brloc(sym,code);
I 2
	DBUG_VOID_RETURN;
E 2
}	/* iabsbr()	*/



ndxreloc(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
	prelent		trelent;

I 2
	DBUG_ENTER ("ndxreloc");
E 2
	if (sym == NULLSYM)
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	codgen(BITSPBY,code->cvalue >> BITSPBY);
	code->cvalue &= 0xff;
	code->cnbits = BITSPBY;
	genreloc(sym,code);
I 2
	DBUG_VOID_RETURN;
E 2
}	/* ndxreloc()	*/

#ifdef	M68881
cpbccopt(sym,code)
	register symbol	*sym;
	codebuf		*code;
{
I 2
	DBUG_ENTER ("cpbccopt");
E 2
	if (*++longsdi)
	{	/* Original cpbcc.w is no good, then generate a .l */
		/* turn on bit 6 for L */
		codgen(BITSPOW,code->cvalue | 0x40);	/*	cpbcc L	*/
		if (fread(code,sizeof(*code),1,fdcode) != 1)
			aerror("unexpected EOF on temporary (code) file");
		sym = code->cindex ? (symtab + code->cindex - 1) : NULLSYM;
		code->cnbits = 2*BITSPOW;
		code->cvalue = 0;
		brloc(sym,code);	
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	}
	/* Otherwise, original bcc.w is ok, so codgen gets code as-is.	*/
I 2
	DBUG_VOID_RETURN;
E 2
}	/* cpbccopt()	*/

D 6
#endif	M68881
E 6
I 6
#endif	/* M68881 */
E 6
I 2

E 2
E 1
