/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)macdefs.h	7.3		*/
/*
static	char	SCCSID[] = "@(#) macdefs.h:	@(#)macdefs.h	7.3	";
*/

/*	MC68000 	*/
#define MC68K


#if defined( BLIT ) && (! defined( INT16BIT ))
#    define	INT16BIT
#endif


#define	ARGINIT		64	/* initial offset, in bits, of first argument on
				the stack */
#define	AUTOINIT	0	/* initial offset of first automatic variable */

/* alignments, in bits, of fundamental types */
#define	ALCHAR		8
#ifdef M68020
#define	ALINT		align_val   /* 16 or 32 bit ints aligned thus */
#define	ALDOUBLE	align_val
#define	ALPOINT		align_val
#define	ALSTRUCT	align_val
#define	ALSTACK		align_val
#define	ALINIT		align_val   /* alignment of an initialized quantity */

/* The following are for Alignments within Structures.
** The environment variable setting STALIGN=YES produces long (32 bit)
** alignment, while anything else produces word (16-bit) alignment. 
** Long alignment within structures and arrays of structures
** causes incompatibility problems with pre-existing file headers
** and pre-existing object routines (for example, the _iob file data
** structure) and will only be selected by the knowledgable user. */
#define	SAINT		align_struc
#define	SALONG		SAINT
#define	SADOUBLE	align_struc
#define	SAPOINT		align_struc
#define	SASTRUCT	align_struc
#define	SASTACK		align_struc
#define	SAINIT		align_struc
#define SACHAR		ALCHAR	/* char alignment never changes */
#else
#define	ALINT		16	/* 16 or 32 bit ints aligned thus */
#define	ALDOUBLE	16
#define	ALPOINT		16
#define	ALSTRUCT	16
#define	ALSTACK		16
#define	ALINIT		16	/* alignment of an initialized quantity */
#endif

/* size in bits of fundamental types */
#define	SZCHAR		8

#ifdef	INT16BIT		/* defined to force 16-bit integers */
#    define	SZINT		16
#    define	SZLONG		32
#    define	ALLONG		ALINT
#else
#    define	SZSHORT		16
#    define	SZINT		32
#ifdef M68020
#    define	ALSHORT		16
#    define	SASHORT		ALSHORT /* for alignment within structures */
#else
#    define	ALSHORT		ALINT
#endif
#endif	/* def INT16BIT */

	/* if defined, float and double are treated as long */
#undef	NOFLOAT
#define DOUBLES

#ifndef NOFLOAT
	/* Assume compiler switch DOUBLES that selects single/double precision*/
#  ifdef DOUBLES
		/* two floating point tupes, i.e. float != double */
#    undef  ONEFLOAT
#  else
		/* only one floating point type, i.e. float == double */
#    define ONEFLOAT
#  endif		/* DOUBLES */
#endif		/* NOFLOAT */

#ifdef ONEFLOAT
#  define SZDOUBLE	32
# else
#  define SZDOUBLE	64
#  define SZFLOAT	32
#if M68020
#  define ALFLOAT	align_val
#  define SAFLOAT	align_struc	/* for alignment within structures */
#else
#  define ALFLOAT	16
#endif
#endif

#define	SZPOINT		32

	/* use common declarations for all unitialized declarations */
#define	ALLCOMM

	/* is  &a[0]  a constant when "a" is auto */
/*#define	ANDABLE() */

	/* set aside argument space for return value */
#undef	ARGALLRET

	/* set aside argument space for returning structure value */
#undef	ARGSRET

	/* size in chars of assembly language buffer for in-line as code */
#undef	ASMBUF

	/* permit -Xs option that puts C line of code in assembler output */
#define	ASSYCOMMENT

	/* arguments run backward (downwards) on the stack */
#undef	BACKARGS

	/* automatics run backward (downwards) on the stack */
#define	BACKAUTO

	/* temporaries run backward (downwards) on the stack */
#define	BACKTEMP

	/* default cost of testing a register for condition codes */
#undef	CCTEST
	/* charged cost for load, store */
#define	CLOAD(x)	4
#define	CSTORE(x)	4

	/* character-set translation; default: do nothing */
/*#define	CCTRANS(x) */

	/* name of function so user can get control after arguments are declared
	in a function*/
/*#define	CENDARG() */

	/* characters are signed */
#define	CHSIGN

	/* name of function to reprocess tree before it is optimized */
#undef	CLOCAL

	/* string that introduces a comment to the assembler, double-quoted */
#define	COMMENTSTR	"#"

/* string containing ident assembler directive for #ident feature */

#define IDENTSTR	"ident"

/*	size in which constants are converted */
/*	should be long if feasable - from VAX pcc for two-pass conversion */
/*	CONSZ defined in manifest.h */
#define CONFMT "%ld"

	/* a function or macro called with indication of errors
	at the end of mainp1() */
/*#define	ENDJOB(errors) */

	/* make available the address of the end of a structure, useful when
	copying one onto the stack */
#undef	ENDSTRUCT

	/* allow for enum type to depend on range of values; default INT */
/*#define	ENUMSIZE(highval,lowval) */

	/* define class of external variables */
#undef	EXCLASS

	/* alternate routine to be called by cerror(); default exit */
#undef	EXIT

/* user-level fix-up at symbol definition time */

#define FIXDEF(p)	fixdef(p)

/* support for structure debug info */

#define FIXSTRUCT(p,q)	strend(p)

	/* routine to convert string to float; default atof() */
/*#define	FLOATCVT(s) */

	/* for fields within structures of types that are not long/short/char;
	i.e. Honeywell addresses are 18 bits */
#undef	FUNNYFLDS

	/* format for compiler-generated labels */
#define	LABFMT "L%%%d"

	/* defined if magic lint comments are to be recognized */
#undef	LINT

	/* defined if fields are permitted to be long data type if int < long */
#define	LONGFIELDS

	/* number of bits from Least Significant Bit of a long in a register to
	the LSB of an int */
#undef	LOWINT

	/* if defined, arguments are evaluated left to right;
	default is right to left */
#undef	LTORARGS

	/* optional translation on a character constant */
/*#define	MAKECC(val,i) */

	/* converstion nodes from long to ptr or ptr to long should be kept */
#define	MEMONLY

	/* if defined, asmout() in scan.c ignored and user provides one */
#undef	MYASMOUT

	/* optional C statements to be run near end of bccode() in optim.c */
#undef	MYBCCODE

	/* if defined, ccast() in trees.c ignored and user provides one */
#undef	MYCCAST

	/* if defined, sconvert() in optim.c ignored and user provides one */
#undef	MYCONVERT

	/* if defined, ctype() in pftn.c ignored and user provides one */
#undef	MYCTYPE

	/* if defined, ecode() in trees.c ignored and user provides one */
#undef	MYECODE

	/* my own initialization routine, for speed */
#define	MYINIT(a,b) myinit(a,b)

	/* if defined, getlab() in trees.c ignored and user provides one */
#undef	MYLABELS

	/* if defined, locctr() in trees.c ignored and user provides one */
#undef	MYLOCCTR

	/* used to define alternate printf() for printing mask in expand() */
/*#define	MYMASK(val) */

	/* if defined, myoffcon() in optim.c ignored and user provides one */
#undef	MYOFFCON

	/* if defined, used in buildtree() to modifiy actions */
/*#define	MYOPACT(p,actions) */

	/* if defined, replaces action of p2tree; should end with return */
/*#define	MYP2TREE(p) */

	/* to produce power-of-two divide or multiply */
#undef	MYPDIV
#undef	MYPMUL

	/* if defined, replaces action of prtree; should end with return */
#undef	MYPRTREE

	/* escape within p2compile() in reader.c to "launder input" */
/*#define	MYREADER(p) */

	/* if defined, where() in common ignored and user provides one */
#undef	MYWHERE


	/* if defined, not debug code is included */
#undef	NODBG

	/* if defined as "return(class)" this will disable FORTRAN types */
	/* may need to be resolved with FORT */
#undef	NOFORTRAN

#ifdef	INT16BIT
	/* short => int */
#    define	NOSHORT
#    undef	NOLONG

#else
	/* long => int */
#    define	NOLONG
#    undef	NOSHORT
#endif	/* def INT16BIT */

	/* if defined, function calls of the form  f(g(x))  can not have their
	argument lists nested */
#undef	NONEST
	/* if defined, function calls of the form  f(g())  where g() returns
	a structure can not have their arguments nested */
#undef	STCALLBAD
	/* if defined, like STCALLBAD, except for cases where g() is unary,
	i.e. takes no arguments */
#undef	UCALLBAD
	/* if defined, like UCALLBAD, except for cases where g() is a unary
	Fortran function */
#undef	UFCALLBAD

	/* turn on optimization (may be out of date) */
#undef	NOOPT

	/* number of op table entries */
#undef	NOPTB

	/* if defined, no register arguments */
#undef	NOREGARGS

	/* unsigned quantities are forbidden */
#undef	NOUNSIGNED

	/* no longer used */
#undef	NPSH

	/* number of scratch registers */
#define	NRGS 2

	/* size of a string in sty.y */
#undef	NSTRING

	/* number of sty shapes	*/
#undef	NSTYSHPS

	/* output register number for SDB */
#define	OUTREGNO(p) (ISPTR(p->stype)?p->offset+8:p->offset)

	/* with PTR+INT, put the PTR on the left, or right */
#undef	PTRLEFT
#define	PTRRIGHT
	/* except that, with PTR+CONST, put the constant on the right */
#define	CONSRIGHT

	/* type of a pointer, may be LONG or UNSIGNED; default INT */
#ifdef	INT16BIT
#    define	PTRTYPE	LONG
#else
#    undef	PTRTYPE		/* defaults to INT */
#endif

	/* alternate putchar(), e.g. for uppercase-only assemblers */
#undef	PUTCHAR

	/* the name of the (shared) return area for structures; must collide
	with user names! */
#define	RNODNAME "__StRet"

	/* right to left bytes */
#undef	RTOLBYTES

	/* sdb is enabled */
#define	SDB	1

	/* used to construct register masks */
#define	SETREGS(x) { extern int rsmask; rsmask |= x; }

	/* define the shared structure return area */
#define	SRETNAME(sz) printf( "\tcomm\t%s,%d\n", RNODNAME, sz/SZCHAR );

	/* if defined, this is a stack machine and register allocation is
	different */
#undef	STACK

	/* structures returned in static areas */
#define	STATSRET

	/* we want the standard prtree() or p2tree() included */
#define STDPRTREE

	/* symbol table size */
#define	SYMTSZ 2200	/* upped from 2000, 2/6/86 - wjc */

	/* tree size */
#undef	TREESZ

	/* create a two-pass compiler */
#ifdef TWOPASS
#	undef ONEPASS	/* for ported VAX code from pcc */
#else
#	undef	TWOPASS
#	define	ONEPASS
#endif

	/* pointer are not all alike (currently can only handle 2 types */
#undef	TWOPTRS

	/* turn on debugging in yacc functions */
#undef	YYDEBUG

	/* ??? */
#undef	YYMAXDEPTH

	/* if defined, user defines their own routine to pass structures shorter
	than or equal to longs as longs */
#undef	simpstr

	/* number of registers needed to hold an object of type; default 1 */
/*#define	szty(type) */

