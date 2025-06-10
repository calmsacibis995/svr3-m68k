h01200
s 00000/00000/00672
d D 1.6 86/09/16 15:27:57 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00672
d D 1.5 86/09/15 14:00:59 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00672
d D 1.4 86/09/11 13:01:36 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00008/00000/00664
d D 1.3 86/08/18 08:47:50 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00038/00002/00626
d D 1.2 86/08/07 08:02:45 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00628/00000/00000
d D 1.1 86/08/04 09:37:56 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)addr1.c	6.1		*/
#include <stdio.h>
#include <filehdr.h>
#include <reloc.h>
#include <syms.h>
#include <linenum.h>
#include "systems.h"
#include "gendefs.h"
#include "symbols.h"
#include "codeout.h"
I 2
#include "dbug.h"
E 2

/*
 *
 *	"addr1.c" is a file containing routines for implementing the
 *	various addressing modes from the intermediate file.  The
 *	majority of the routines are passed a pointer to a
 *	character string and a pointer to the code buffer.
 *	The array "modes" is initialized to contain the addresses
 *	of the functions the implement the various addressing modes.
 *	Indexing this array with the addressing mode will give the
 *	correct routine for implementing that mode.
 *
 */

#define outblock(a,b,c)	fwrite((char *)(a),b,1,c)

extern char
	cfile[];

extern unsigned short
	line,
	cline;

extern short
	type,
	transvec,
	sttop;

extern int
	setsym();

extern long
	newdot,
	symbent,
	gsymbent;

extern long
	getindx();

extern symbol
	*dot;

extern FILHDR
	filhead;

extern upsymins
	*lookup();

extern stent
	*pop();
static stent
	*popptr;

extern FILE
#if !ONEPROC
	*fdstring,
#endif
	*fdline,
	*fdsymb,
	*fdgsymb;

symbol	*savsym;

unsigned short
	lineent;

short	filedef = NO;

#if FLEXNAMES
extern char	*strtab;	/* String table; referenced for symbol	*/
				/* name to pass to getindx.		*/
#endif

SYMENT	sment;

AUXENT	axent;

static short
	dimcnt;

static LINENO
	lnent;

/*ARGSUSED*/

setmagic(sym,code)
	symbol *sym;
	codebuf *code;
{
I 2
	DBUG_ENTER ("setmagic");
E 2
	filhead.f_magic = (short)(code->cvalue);
I 2
	DBUG_VOID_RETURN;
E 2
}

setfile(sym,code)
	upsymins sym;
	codebuf *code;
{
	register char c;
	register char *auxchar;
	register short index = 0;

I 2
	DBUG_ENTER ("setfile");
E 2
	filedef = YES;

	sym = *lookup(".file",INSTALL,USRNAME);
	define(sym.stp,code);

	sment.n_numaux = 1;
	auxchar = ((union auxent *)(&axent))->x_file.x_fname;
#if ONEPROC
	while (*auxchar++ = cfile[index++]) ;
#else
	while (((c = getc(fdstring)) != '\n') && (index < FILNMLEN)) {
		*auxchar++ = c;
		cfile[index++] = c;
	}
#endif
	for ( ; index < FILNMLEN; index++)
		*auxchar++ = '\0';

	cfile[index] = '\0';
	code->cvalue = (long)C_FILE;
	setscl(NULLSYM,code);
	endef(NULLSYM,code);
I 2
	DBUG_VOID_RETURN;
E 2
} /* setfile */

/*ARGSUSED*/

newstmt(sym,code)
	symbol *sym;
	codebuf *code;
{
I 2
	DBUG_ENTER ("newstmt");
E 2
	line = (unsigned short)(code->cvalue);
	dot->value = newdot; /* resynchronize */
I 2
	DBUG_VOID_RETURN;
E 2
} /* newstmt */

/*
 *
 *	"lineno" is a function that creates a line number entry with
 *	the value of the program counter. A preliminary line number
 *	entry is created by writing to the file whose descriptor
 *	appears in "fdline". The preliminary line number entry contains
 *	the value of the program counter and the line number itself.
 *
 */

lineno(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("lineno");
E 2
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	lnent.l_lnno = cline = (unsigned short)(code->cvalue);
	lnent.l_addr.l_paddr = newdot;
	outblock(&lnent,LINESZ,fdline);
	lineent++;
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"linenum" and "lineval" work in a pair to implement the two
 *	operand version of the ".ln" pseudo operation.  This operation
 *	will generate two intermediate file entries. "linenum"
 *	evaluates the line number and stores it in the global
 *	variable "savline". "lineval" evaluates its argument to
 *	use it as the address for the line number. This address
 *	and the line number from "savline" are written to the file
 *	whose descriptor appears in "fdline" as a completed line
 *	number entry.
 *
 */

linenum(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("linenum");
E 2
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	lnent.l_lnno = cline = (unsigned short)(code->cvalue);
I 2
	DBUG_VOID_RETURN;
E 2
}

lineval(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("lineval");
E 2
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	lnent.l_addr.l_paddr = code->cvalue;
	outblock(&lnent,LINESZ,fdline);
	lineent++;
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	The functions "define", "setval", "settyp", "setscl", "settag",
 *	"setlno", "setsiz", "setdim1" through "setdim5", "xform", and
 *	"endef" are all involved in creating the symbol table entries
 *	that appear in the object file.  The call to "define" will
 *	initialize the symbol table entry.  The calls to subsequent
 *	functions will store values into various fields of that entry.
 *	The call to "endef" will write out the completed symbol table
 *	entry to an intermediate file.  The symbol table entry will be
 *	constructed in a structure called "sment".  The auxiliary
 *	symbol table entry (if necessary) will be constructed in the
 *	structure "axent".  The completed symbol table entry will be
 *	written either to the file whose descriptor appears in "fdsymb"
 *	or the file whose descriptor appears in "fdgsymb".
 *
 */

/*
 *
 *	"define" is a function that performs initialization of a symbol
 *	table entry to be written to the object file.
 *	The arguments to this function is a pointer to the symbol for
 *	which the entry is to be created. The symbol name is stored
 *	into the object file symbol table entry, and the remainder
 *	of the fields for the entry are set to zero.
 *
 */

/*ARGSUSED*/

define(sym,code)
	register symbol *sym;
	codebuf *code;
{
	register short index;

I 2
	DBUG_ENTER ("define");
E 2
	savsym = sym;
#if FLEXNAMES
	/* Copy the internal symbol table entry for the name to the	*/
	/* structure which will be written to the symbol table file.	*/

	if (sym->_name.tabentry.zeroes == 0)
	{
		sment.n_zeroes = 0L;
		sment.n_offset = sym->_name.tabentry.offset;
	}
	else
	{
#endif
		for(index = 0; (index < SYMNMLEN)
		&& (sment.n_name[index] = sym->_name.name[index]); index++)
			;

		for(; index < SYMNMLEN; index++)
			sment.n_name[index] = '\0';
#if FLEXNAMES
	}
#endif
	sment.n_value = 0L;
	sment.n_scnum = 0;
	sment.n_type = 0;
	sment.n_sclass = 0;
	sment.n_numaux = 0;
	axent.x_sym.x_tagndx = 0L;
	axent.x_sym.x_misc.x_lnsz.x_lnno = 0;
	axent.x_sym.x_misc.x_lnsz.x_size = 0;
	for(index = 0; index < DIMNUM; ++index)
		axent.x_sym.x_fcnary.x_ary.x_dimen[index] = 0;
	axent.x_sym.x_tvndx = 0;
#if TRANVEC
	if (transvec && (sym->styp & TVDEF)) {
		sment.n_numaux = 1;	/* we can do this since transfer */
		axent.x_sym.x_tvndx = N_TV;	/* vectors are always */
						/* functions in .def's */
	}
#endif
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"setval" is a function that sets the value field of a symbol
 *	table entry. The arguments are evaluated and the result is
 *	stored into the value field of the symbol table entry, and
 *	the section number is set according to the type of the result.
 *
 */

setval(sym,code)
	register symbol *sym;
	register codebuf *code;
{
	register short stype;

I 2
	DBUG_ENTER ("setval");
E 2
	if (sym != NULLSYM) {
		code->cvalue += sym->value;
		stype = sym->styp & TYPE;
	}
	else
		stype = ABS;
	sment.n_value = code->cvalue;
	switch (stype){
		case ABS:{
			sment.n_scnum = N_ABS;
			break;
		}
		case TXT:{
			sment.n_scnum = 1;
			break;
		}
		case DAT:{
			sment.n_scnum = 2;
			break;
		}
		case BSS:{
			sment.n_scnum = 3;
			break;
		}
I 3
		case INIT:{
			sment.n_scnum = 4;
			break;
		}
		case LIB:{
			sment.n_scnum = 5;
			break;
		}
E 3
	}
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"settyp" is a function that sets the type and derived type
 *	field in a symbol table entry. The arguments are evaluated to
 *	yield the numeric value to be stored into the type and derived
 *	type word. This value is stored into the appropriate field
 *	of "sment".
 *
 */

settyp(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("settyp");
E 2
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	sment.n_type = (short)(code->cvalue);
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"setscl" is a function that sets the storage class field in a
 *	symbol table entry. The arguments are evaluated to yield a
 *	value to be stored into the appropriate field of "sment".
 *
 */

setscl(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("setscl");
E 2
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	sment.n_sclass = (char)(code->cvalue);
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"settag" is a function that sets the tag field in an auxiliary
 *	symbol table entry. The argument to this function is the
 *	name of a structure, union, or enumeration tag. This name
 *	is passed to "getindx" to obtain the symbol table index of
 *	the tag entry. If it is found, a field in the symbol table
 *	entry is set indicating the presence of an auxiliary entry
 *	and the tag field of the auxiliary entry is set to the index.
 *	If the tag was not found, "yyerror" is called to print a 
 *	diagnostic message.
 *
 */

/*ARGSUSED*/

settag(sym,code)
	register symbol *sym;
	codebuf *code;
{
	register long index;
	char *nameptr;		/* We need a pointer to the name string	*/
				/* to pass to getindx.			*/

I 2
	DBUG_ENTER ("settag");
E 2
#if FLEXNAMES
	if (sym->_name.tabentry.zeroes == 0)
		nameptr = &strtab[sym->_name.tabentry.offset];
	else
#endif
		nameptr = sym->_name.name;
	if (	((index = getindx(nameptr,C_STRTAG)) < 0) &&
		((index = getindx(nameptr,C_UNTAG)) < 0) &&
		((index = getindx(nameptr,C_ENTAG)) < 0))
		yyerror("Illegal structure, union, or enumeration tag");
	sment.n_numaux = 1;
	axent.x_sym.x_tagndx = index;
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"setlno" sets the line number field in an auxiliary symbol
 *	table entry. A field is set in the symbol table entry to
 *	indicate the presence of an auxiliary entry. The arguments
 *	are evaluated to obtain the value of the line number and
 *	this value is stored into the appropriate field of the
 *	auxiliary symbol table entry.
 *
 *
 */

setlno(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("setlno");
E 2
	sment.n_numaux = 1;
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	axent.x_sym.x_misc.x_lnsz.x_lnno = (unsigned short)(code->cvalue);
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"setsiz" sets the size field in an auxiliary symbol table entry.
 *	The arguments are evaluated to obtain the size of the object
 *	described by this symbol table entry. This value is stored into
 *	the appropriate field of the auxiliary symbol table entry and
 *	a field is set in the basic symbol table entry to indicate the
 *	presence of the auxiliary entry.
 *
 */

setsiz(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("setsiz");
E 2
	sment.n_numaux = 1;
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	axent.x_sym.x_misc.x_lnsz.x_size = (unsigned short)(code->cvalue);
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"setdim1" and "setdim2" set the fields for array dimensions
 *	one through four in an auxiliary symbol table entry. The
 *	arguments to each function are evaluated to obtain the values
 *	for the array dimensions. The function "setdim1" set a field
 *	in the basic symbol table entry to indicate the presence of
 *	an auxiliary entry. The array dimensions are stored into the
 *	appropriate fields of the auxiliary symbol table entry.
 *
 */

setdim1(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("setdim1");
E 2
	sment.n_numaux = 1;
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	axent.x_sym.x_fcnary.x_ary.x_dimen[dimcnt=0] = (unsigned short)(code->cvalue);
I 2
	DBUG_VOID_RETURN;
E 2
}

setdim2(sym,code)
	register symbol *sym;
	register codebuf *code;
{
I 2
	DBUG_ENTER ("setdim2");
E 2
	if (sym != NULLSYM)
		code->cvalue += sym->value;
	if (++dimcnt < DIMNUM)
		axent.x_sym.x_fcnary.x_ary.x_dimen[dimcnt] = (unsigned short)(code->cvalue);
	else
		werror("Too many array dimensions for symbolic debug");
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *   "dfaxent" defines an auxillary entry into the symbol table for
 *   the ".text." , ".data" , and ".bss" symbols. It is called from
 *   "symout" in file "obj.c". These auxillary entries allow the
 *   link editor to know the size of each of these sections in the
 *   original source code.
 *
 */

dfaxent(size,nrel,nlin)
       long  size;
       unsigned short nrel,
                      nlin;
{
I 2
	DBUG_ENTER ("dfaxent");
E 2
       sment.n_numaux = 1;
       axent.x_scn.x_scnlen = size;
       axent.x_scn.x_nreloc = nrel;
       axent.x_scn.x_nlinno = nlin;
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"xform" is a function called by "endef" that performs any final
 *	transformations necessary on a symbol table entry before it is
 *	written out to a temporary file.  The present transformations
 *	are as follows:
 *
 *	     1.	The section number for structure tags, union tags,
 *		enumeration tags, typedefs, and file names is set to
 *		indicate a special symbolic debugging symbol.
 *
 *	     2.	If the symbol table entry is for a function, a zero
 *		line number entry is generated that points to the symbol
 *		table entry.  The index of the next entry to be
 *		generated appears in the global variable "symbent".
 *
 *	     3.	If the symbol table entry is for a function, a enu-
 *		meration tag, a union tag, or a structure tag
 *		"setsym" is called to remember the symbol table index
 *		for the associated auxiliary entry.
 *
 *	     4.	If the symbol table entry is for a end block, a
 *		end function, or a end of structure, pop is called
 *		to enter the symbol table index of the next symbol
 *		table entry onto the symbol table element stack.
 *
 */

xform()
{
D 2

E 2
I 2
	DBUG_ENTER ("xform");
E 2
	switch (sment.n_sclass) {
	case C_STRTAG:
	case C_UNTAG:
	case C_ENTAG:
		setsym(-1);
		/* No Break */
	case C_TPDEF:
	case C_FILE:
		sment.n_scnum = N_DEBUG;
		break;
	case C_BLOCK:	/* handles ".bb" and ".eb" */
		if (sment.n_name[1] == 'b')
			setsym(-1);
		else
			pop()->fwdindex = symbent + 1L
					+ sment.n_numaux;
		break;
	case C_FCN:	/* handles ".bf" and ".ef" */
		if (sment.n_name[1] == 'e') {
			popptr = pop();
			popptr->fwdindex = symbent + 1L
					+ sment.n_numaux;
			if (sttop > 0)
				aerror("Unbalanced Symbol Table Entries-Too Many Scope Beginnings");
		} /* ".ef" */
		break;
	case C_EOS:
		/* get around auxiliary entry */
		pop()->fwdindex = symbent + 1L + sment.n_numaux;
		break;
	default:
		/* process function entry */
		if (ISFCN(sment.n_type)) {
#if FLDUPDT
			popptr = NULL;
#endif
			lnent.l_lnno = 0;
			lnent.l_addr.l_symndx = symbent;
			outblock(&lnent,LINESZ,fdline);
			lineent++;
			sment.n_numaux = 1;
			setsym(0);
		}
		break;
	} /* switch */
I 2
	DBUG_VOID_RETURN;
E 2
} /* xform */

/*
 *
 *	"endef" is a function that completes processing of a symbol
 *	table entry and writes it out to an intermediate file
 *	The arguments to this function are absolutely meaningless.
 *	The function "xform" is called to perform any final
 *	transformations necessary on the entry. A decision is made
 *	as to whether or not the symbol is a global symbol. It is
 *	global if the ".def" for it appears in the data section and
 *	it has a storage class of "C_EXT". If it is global, the
 *	entry will be written to the file whose descriptor appears
 *	in "fdgsymb" and the count "gsymbent" will be incremented.
 *	If it is not global, the entry will be written to the file
 *	whose descriptor appears in "fdsymb" and the count "symbent"
 *	will be incremented.
 *
 */

/*ARGSUSED*/

endef(sym,code)
	symbol *sym;
	codebuf *code;
{
	FILE *fd;
	register long *count;

I 2
	DBUG_ENTER ("endef");
E 2
#if FLDUPDT
	if (sment.n_sclass == (char) C_EFCN) {
		if (popptr == NULL) {
			popptr = pop();
			popptr->fwdindex = symbent;
			if (sttop > 0)
				aerror("Unbalanced Symbol Table Entries-Too Many Scope Beginnings");
		} /* popptr == NULL */
		popptr->fcnlen = newdot - savsym->value;
		/* don't put out this symbol table entry */
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2
	} /* sment.n_sclass == C_EFCN */
#endif
	xform();
	if ((dot->styp == DAT) && (sment.n_sclass == C_EXT)) {
		fd = fdgsymb;
		count = &gsymbent;
	}
	else
	{
		fd = fdsymb;
		count = &symbent;
		putindx(savsym,sment.n_sclass,symbent);
	}
	outblock(&sment,SYMESZ,fd);
	(*count)++;
	if (sment.n_numaux != 0){
		outblock(&axent,AUXESZ,fd);
		(*count)++;
	}
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
