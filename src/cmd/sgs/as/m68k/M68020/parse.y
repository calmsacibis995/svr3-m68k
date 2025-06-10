/*	@(#)parse.y	6.6				*/
%{
static	char	sccsid[] = "@(#) parse.y 6.6 5/7/85";
/* static	char	sccsid[] = "@(#) parse.y: 2.6 6/24/83"; */


/*			Motorola 68000 assembler			*/

#include	<stdio.h>
#include	<string.h>
#include	"symbols.h"
#include	"instab.h"
#include	"gendefs.h"
#include	"fpnum.h"		/* M68881 constant defination	*/
#include	"dbug.h"

/* M68881 additions */
char	mantissa[80],exponent[80];
unsigned short	user_prec=UNSPEC;	/* user defined precision	*/
/* end M68881 additions */

extern unsigned short
		cline,		/* Line number from the .ln pseudo op.	*/
		line;		/* Current input file line number.	*/

extern FILE	*fdin;		/* Input source.			*/
extern short	opt;		/* Sdi optimization flag.		*/
extern char	file[],		/* Assembly input file name.		*/
		cfile[];	/* Original C file name.		*/

extern symbol	*dot;		/* Current pc pointer.			*/
extern long	newdot,		/* Up-to-date value of pc pointer.	*/
		dotbss;

struct arg	arglist[6];	/* 68020 instructions can have 6 arguments */
struct arg	*ap = arglist;
struct	exp	explist[100];
struct	exp	*xp = explist;

long		outword;	/* Bit field accumulator.		*/
int		bitsout;	/* Number of bits accumulated.		*/
BYTE		bytesout;	/* Output displacement.			*/
int		swbegct;	/* Number of switch locations.		*/

/*	M68020	*/
int		regseen;	/* count registers as we see them	*/
int		indexreg;	/* remember position of index register	*/
int		ssflag;		/* have we seen a size-scale		*/
int		lp_count;	/* used for matching parenthesis	*/
/*	M68020	*/


struct exp	*combine();	/* Handles expression arithmetic.	*/
extern upsymins	*lookup();	/* Symbol table lookup routine.		*/
extern		deflab();	/* Define label for sdi handler.	*/
extern		gen0op();	/* Handles no-operand instructions.	*/
extern		gen1op();	/* Handles one-operand instructions.	*/
extern		gen2op();	/* Handles two-operand instructions.	*/
extern		gen3op();	/* Handles three-operand instructions.	*/
extern		gen4op();	/* Handles four-operand instructions.	*/
extern		gen6op();	/* Handles six-operand instructions.	*/
extern		resolve();	/* resolves all SDI's to known length   */

char		err_buf[BUFSIZ];
char		yytext[BUFSIZ];
%}

/* Symbolic Debugger Support Pseudo Operations				*/
%token	IFILE	ILN	IDEF	IENDEF	IVAL
%token	ITYPE	ILINE	ISCL	ITAG	ISIZE	IDIM

/* Support for .comment sections */
%token	IIDENT

%token	ISET	IGLOBAL	IDATA	ITEXT	IBYTE	ISHORT	ILONG
%token	ISPACE	IORG	ICOMM	ILCOMM	IEVEN	ISWBEG

/* Extensions for shared library support; fnf@MCS,4-Aug-86 */
%token	ILIB	IINIT

%token	IROUND IPREC		/* 68881 options */

%token	COLON	AMP	NOCHAR	SPC	ALPH	DIGIT	SQ
%token	SH	CM	NL	LP	RP	SEMI	DQ	TWID

%token	LBRKT	RBRKT	LBRACE	RBRACE	/*	68020 support	*/

%left	PLUS	MINUS
%left	MUL	DIV
%left	UMINUS		/* Only used for precedence--no such token.	*/

%token	<yname>	NAME
%token	<yinst>	INST0	INST1	INST2	INST3	INST4	INST6	REG
%token	<yinst>	INST12  /* instruction may have one or two operand(s) */  
%token	<yinst>	INST23  /* instruction may have two or three operands */  
%token	<yinst>	INST24  /* instruction may have two to four operands  */  
%token	<yint>	SIZE	PLUS	MINUS	MUL	DIV	LP	RP
%token	<ylong>	INT
%token	<ystr>	STRING
%token	<ystr>	HEXSTR	/* 68881 floating point hex string */
%token	<ystr>	DECSTR	/* 68881 floating point decimal string */

%type	<yinst>	reg.ss					/* 68020 */
%type	<yint>	optsize	optscl	ss	lp	rp	/* 68020 */
%type	<yarg>	arg
%type	<yexp>	expr	disp
%type	<yexp>	fp_expr	/* 68881 floating point expression */
%%

file	:	/* empty */
	|	file linstruction NL
	{
		line++;
		dot->value = newdot;
		generate(0,NEWSTMT,(long) line,NULLSYM);
		ap = arglist;
		xp = explist;
	}
	|	file linstruction SEMI
	{
		dot->value = newdot;
		generate(0,NEWSTMT,(long) line,NULLSYM);
		ap = arglist;
		xp = explist;
	}
	|	file error NL
	{
		line++;
		yyerrok;
		dot->value = newdot;
		ap = arglist;
		xp = explist;
	}
	;


linstruction:	labels instruction
	;

labels	:	/* empty */
	|	labels NAME COLON
	{
		if (($2->styp & TYPE) != UNDEF)
		{
			strcpy(err_buf,"multiply defined label--");
			strcat(err_buf,yytext);
			yyerror(err_buf);
		}
		$2->value = newdot;
		$2->styp |= dot->styp;
		$2->sectnum = dot->sectnum;
		if (opt && ((dot->styp & TYPE) == TXT) && dot->sectnum == 0)
			deflab($2);
	}
	;

instruction:
		IIDENT STRING
	{
		comment($2);
	}
	|	IFILE STRING
	{
		if (cfile[0] != '\0')
			yyerror("only one .file allowed");
		strcpy(cfile,$2);
		generate(0,SETFILE,0,NULLSYM);
	}
	|	ILN expr
	{
		cline = (short) $2->xvalue;
		generate(0,LINENBR,$2->xvalue,$2->xsymptr);
	}
	|	ILN expr CM expr
	{
		cline = (short) $2->xvalue;
		generate(0,LINENUM,$2->xvalue,$2->xsymptr);
		generate(0,LINEVAL,$4->xvalue,$4->xsymptr);
	}
	|	IDEF NAME
	{
		generate(0,DEFINE,0L,$2);
	}
	|	IENDEF
	{
		generate(0,ENDEF,0,NULLSYM);
	}
	|	IVAL expr
	{
		generate(0,SETVAL,$2->xvalue,$2->xsymptr);
	}
	|	ITYPE expr
	{
		generate(0,SETTYP,$2->xvalue,$2->xsymptr);
	}
	|	ILINE expr
	{
		generate(0,SETLNO,$2->xvalue,$2->xsymptr);
	}
	|	ISCL expr
	{
		generate(0,SETSCL,$2->xvalue,$2->xsymptr);
	}
	|	ITAG NAME
	{
		generate(0,SETTAG,0,$2);
	}
	|	ISIZE expr
	{
		generate(0,SETSIZ,$2->xvalue,$2->xsymptr);
	}
	|	dotdim
	|	ISET NAME CM expr
	{
		if (($4->xtype & TYPE) == UNDEF)
			yyerror("illegal expression in .set");
		if ($2 == NULL)
			yyerror("unable to define symbol in .set");
		else
		{
			$2->value = $4->xvalue;
			$2->styp = $4->xtype;
		}
	}
	/* M68881 addition */
	|	ISET NAME CM fp_expr
	{
		if (($4->xtype & TYPE) == UNDEF)
			yyerror("illegal expression in .set");
		if ($2 == NULL)
			yyerror("unable to define symbol in .set");
		else
		{	if ( $4->xvaltype == FP_DEC )
				get_value($2);
			else
			{
				$2->value = $4->xvalue;
				$2->value2 = $4->xvalue2;
				$2->value3 = $4->xvalue3;
			}
			$2->styp = $4->xtype;
		}
	}
	/* end M68881 addition */
	|	IGLOBAL names
	|	IDATA
	{
		cgsect(DAT,0);
	}
	|	IDATA expr
	{
		cgsect(DAT,$2->xvalue);
	}
	|	ITEXT
	{
		cgsect(TXT,0);
	}
	|	ITEXT expr
	{
		cgsect(TXT,$2->xvalue);
	}
	|	IINIT
	{
		cgsect(INIT,0);
	}
	|	ILIB
	{
		cgsect(LIB,0);
	}
	|	IBYTE {bytesout = 1;} explist
	{
		if (bitsout)
		{
			generate(BITSPBY*bytesout,0,outword,NULLSYM);
			outword = 0;
			bitsout = 0;
		}
	}
	|	ISHORT {bytesout = 2;} explist
	{
		if (bitsout)
		{
			generate(BITSPBY*bytesout,0,outword,NULLSYM);
			outword = 0;
			bitsout = 0;
		}
	}
	|	ILONG {bytesout = 4;} explist
	{
		if (bitsout)
		{
			generate(BITSPBY*bytesout,0,outword,NULLSYM);
			outword = 0;
			bitsout = 0;
		}
	}
	|	ISPACE expr
	{
		if ($2->xtype != ABS)
			yyerror("space size not absolute");
		fill($2->xvalue);
	}
	|	IORG expr
	{
		if ($2->xtype != ABS)
			yyerror("org requires absolute operand");
		if (($2->xvalue - dot->value) < 0)
			yyerror("backwards org");
		else
		{
			if (opt && ((dot->styp & TYPE) == TXT) 
			    && dot->sectnum == 0)
			   resolve();	/* force PC to known value */
			fill($2->xvalue - dot->value);
		}
	}
	|	ICOMM NAME CM expr
	{
		if ($4->xtype != ABS)
			yyerror("comm size not absolute");
		if (($2->styp & TYPE) != UNDEF)
			yyerror("illegal attempt to redefine symbol");
		else
		{
			$2->styp = (EXTERN | UNDEF);
			$2->value = $4->xvalue;
		}
	}
	|	ILCOMM NAME CM expr
	{
		if ($4->xtype != ABS)
			yyerror("lcomm size not absolute");
		if (($2->styp & TYPE) != UNDEF)
			yyerror("illegal attempt to redefine symbol");
		else
		{
		/* even align all items > 1 byte;
		** long align all items > 3 bytes.
		*/
			if ($4->xvalue > 3 && (dotbss & 03) != 0)
			    dotbss += (4 - (dotbss & 03));
			else if ($4->xvalue > 1 && (dotbss & 01) != 0)
			    dotbss++;
			$2->value = dotbss;
			dotbss += $4->xvalue;
			$2->styp |= BSS;
		}
	}
	|	IEVEN
	{
		ckalign(2);
	}
	|	ISWBEG arg
	{
		generate(BITSPOW,SWBEG,0x4afc,NULLSYM);
		generate(BITSPOW,SWBEG,$2->xp->xvalue,NULLSYM);
		swbegct = $2->xp->xvalue;
	}
	/* M68881 options	*/
	|	IPREC optsize	
	{
		get_fp_prec($2);
	}
	|	IROUND optsize	
	{
		get_fp_round($2);
	}
	/* end M68881 options */
	|	INST0
	{
		/* No operands, and no size attribute.			*/
		gen0op($1);
	}
	|	INST1 optsize arg
	{
		gen1op($1,$2,$3);
	}
	|	INST2 optsize arg CM arg
	{
		gen2op($1,$2,$3,$5);
	}
	|	INST3 optsize arg CM arg CM arg
	{
		gen3op($1,$2,$3,$5,$7);
	}
	|	INST3 optsize arg CM arg COLON arg
	{	/* 68881 instruction syntax */
		gen3op($1,$2,$3,$5,$7);
	}
	|	INST3 optsize arg LBRACE arg COLON arg RBRACE
	{
		gen3op($1,$2,$3,$5,$7);
	}
	|	INST4 optsize arg LBRACE arg COLON arg RBRACE CM arg
	{
		gen4op($1,$2,$3,$5,$7,$10);
	}
	|	INST4 optsize arg CM arg LBRACE arg COLON arg RBRACE
	{
		gen4op($1,$2,$3,$5,$7,$9);
	}
	|	INST6 optsize arg COLON arg CM arg COLON arg CM LP arg RP  COLON LP arg RP 
	{
		gen6op($1,$2,$3,$5,$7,$9,$12,$16);
	}
	|	INST12 optsize arg
	{
		gen1op($1,$2,$3);
	}
	|	INST12 optsize arg CM arg
	{
		gen2op($1,$2,$3,$5);
	}
	|	INST23 optsize arg CM arg
	{
		gen2op($1,$2,$3,$5);
	}
	|	INST23 optsize arg CM arg COLON arg
	{
		gen3op($1,$2,$3,$5,$7);
	}
	|	INST23 optsize arg CM arg LBRACE arg RBRACE
	{	/* 68881 instruction syntax */
		gen3op($1,$2,$3,$5,$7);
	}
	|	INST24 optsize arg CM arg	
	{
		gen2op($1,$2,$3,$5);
	}
	|	INST24 optsize arg CM arg CM arg	
	{
		gen3op($1,$2,$3,$5,$7);
	}
	|	INST24 optsize arg CM arg CM arg CM arg 
	{
		gen4op($1,$2,$3,$5,$7,$9);
	}
	|	/* empty */
	;

dotdim	:	IDIM expr
	{
		generate(0,SETDIM1,$2->xvalue,$2->xsymptr);
	}
	|	dotdim CM expr
	{
		generate(0,SETDIM2,$3->xvalue,$3->xsymptr);
	}
	;
names	:	NAME			/* Part of .globl handling.	*/
	{
		$1->styp |= EXTERN;
	}
	|	names CM NAME
	{
		$3->styp |= EXTERN;
	}
	;

optsize	:	SIZE
	{
		$$ = $1;
	}
	|	/* empty */
	{
		$$ = UNSPEC;
	}
	;

explist	:	outexpr
	|	explist CM outexpr
	;

outexpr	:	expr
	{
		if (bitsout)
		{
			generate(BITSPBY*bytesout,0,outword,NULLSYM);
			outword = 0;
			bitsout = 0;
		}
		if (swbegct-- > 0)
			generate(BITSPBY*bytesout,SWBEG,0,$1->xsymptr);
		else
			generate(BITSPBY*bytesout,GENRELOC,$1->xvalue,$1->xsymptr);
	}
	|	expr COLON expr
	{
		if ($1->xtype != ABS)
			yyerror("width expression not absolute");
		if ($1->xvalue > (BITSPBY * bytesout))
			yyerror("expression crosses field boundary");
		if (($1->xvalue + bitsout) > (BITSPBY * bytesout))
		{
			generate(BITSPBY*bytesout,0,outword,NULLSYM);
			bitsout = 0;
			outword = 0;
		}
		$3->xvalue &= (power(2,$1->xvalue)-1);	/* mask out n bits */
		$3->xvalue <<= (bytesout*BITSPBY - bitsout - $1->xvalue);
		outword |= $3->xvalue;
		bitsout += $1->xvalue;
	}
	;

arg	:	expr
	{
		ap->atype = ABSL;
		ap->xp = $1;
		ap->areg1 = 0;
		ap->areg2 = 0;
		$$ = ap++;
	}
	|	REG
	{
		
		ap->atype = $1->tag;
		ap->areg1 = $1;
		ap->areg2 = 0;
		$$ = ap++;
	}
	|	MINUS lp REG rp
	{
		ap->atype = ADEC;
		if ($3->tag != AAREG)
			yyerror("predecrement requires address register");
		ap->areg1 = $3;
		ap->areg2 = 0;
		$$ = ap++;
	}
	|	lp reg.ss rp PLUS
	{
		/*
		 *	simple disambiguating rule:
		 *
		 *	here we only accept REG, but this conflicted
		 *	with new "ind" rules.  it is sufficient to check
		 *	that "ssflag" is not set, if it is - we've seen
		 *	a "size-scale" modifier which is illegal!
		 */

		if ( ssflag )
			yyerror("no size/scale modifier on post increment");
		ap->atype = AINC;
		if ($2->tag != AAREG)
			yyerror("postincrement requires address register");
		ap->areg1 = $2;
		ap->areg2 = 0;
		$$ = ap++;
	} 
	|	AMP expr
	{
		ap->atype = AIMM;
		ap->xp = $2;
		ap->areg1 = 0;
		ap->areg2 = 0;
		$$ = ap++;
	}
	/* M68881 addition */
	|	AMP fp_expr
	{
		ap->atype = AIMM;
		ap->xp = $2;
		ap->areg1 = 0;
		ap->areg2 = 0;
		$$ = ap++;
	}
	/* end M68881 addition */
	|	ind
	{
		$$ = ap++;
	}
	;

/*	
 *	68020 - register indirect w/ index and displacement
 *
 *	three forms:
 *
 *	1. indexed indirect w/ displacement
 *
 *		( bd[.s], An, Rn[.t][*scale] )
 *
 *	2. memory indirect w/ pre-index and displacement
 *
 *		( od[.t], [bd[.t], An, Rn[.t][*scale]] )
 *
 *	3. memory indirect w/ post-index and displacement
 *
 *		( od[.t], [bd[.t], An], Rn[.t][*scale] )
 *
 */

ind	:	nmi
	{
		;
	}
	|	memind
	{
		;
	}
	|	disp lp reg.ss rp
	{
		mk_indirect(INDIRECT,$3,$1);
	}
	|	disp lp reg.ss CM reg.ss rp	/* 68000 old forms */
	{	
		mkindex_indirect(INDIRECT,$3,$5,$1);
	}
	;


	/* nmi - No Memory Indirect */
nmi	:	lp disp CM reg.ss CM reg.ss rp
	{	
		mkindex_indirect(INDIRECT,$4,$6,$2);
	}
	|	lp reg.ss CM disp CM reg.ss rp
	{
		mkindex_indirect(INDIRECT,$2,$6,$4);
	}
	|	lp reg.ss CM reg.ss CM disp rp
	{
		mkindex_indirect(INDIRECT,$2,$4,$6);
	}
	|	lp disp CM reg.ss rp
	{
		mk_indirect(INDIRECT,$4,$2);
	}
	|	lp reg.ss CM disp rp
	{
		mk_indirect(INDIRECT,$2,$4);
	}
	|	lp reg.ss CM reg.ss rp
	{
		mkindex_indirect(INDIRECT,$2,$4,0);
	}
	|	lp reg.ss rp
	{
		if ( !ssflag && $2->tag == AAREG)
		{	/* simple indirect (An) */
			ap->atype = AIREG;
			ap->areg1=$2;
			ap->areg2=0;
		}
		else
		if ( $2->tag == AAREG || $2->tag == NULAREG 
			|| $2->tag == ADREG || $2->tag == NULDREG 
			|| $2->tag == PCREG  || $2->tag == NULPCREG )
			mkindex_indirect(INDIRECT,$2,0,0);
		else
			yyerror("indirect required address register");
	}
		/*
		 *	This rule:
		 *
		 *		|	lp disp rp
		 *		{
		 *			mkindex_indirect(INDIRECT,0,0,$2);
		 *		}
		 *
		 *	is ambiguous with:
		 *
		 *		lp expr rp
		 *	
		 *	and is picked up in absolute long.
		 */
	;


memind	:	postind
	{
		;
	}
	|	preind
	{
		;
	}
	;


postind	:	lp mbd CM reg.ss CM disp rp
	{
		mk_post_ind($4,$6);
	}
	|	lp reg.ss CM mbd CM disp rp
	{
		mk_post_ind($2,$6);
	}
	|	lp reg.ss CM disp CM mbd rp
	{
		mk_post_ind($2,$4);
	}
	|	lp disp CM reg.ss CM mbd rp
	{
		mk_post_ind($4,$2);
	}
	|	lp disp CM mbd CM reg.ss rp
	{
		mk_post_ind($6,$2);
	}
	|	lp mbd CM disp CM reg.ss rp
	{
		mk_post_ind($6,$4);
	}
	|	lp mbd CM disp rp
	{
		/* this syntax can be pre-index or post-index */
		if (ap->aflags == PRENDXMI)
			ap->xp2 = $4;
		else
			mk_post_ind(0,$4);
	}
	|	lp disp CM mbd rp
	{
		/* this syntax can be pre-index or post-index */
		if (ap->aflags == PRENDXMI)
			ap->xp2 = $2;
		else
			mk_post_ind(0,$2);
	}
	|	lp reg.ss CM mbd rp
	{
		mk_post_ind($2,0);
	}
	|	lp mbd CM reg.ss rp
	{
		mk_post_ind($4,0);
	}
	|	lp mbd rp
	{
		/* this syntax can be pre-index or post-index */
		if (ap->aflags == PRENDXMI)
			ap->xp2 = 0;
		else
			mk_post_ind(0,0);
	}
	;


preind	:	lp mbid CM disp rp
	{
		ap->xp2 = $4;
	}
	|	lp disp CM mbid rp
	{	
		ap->xp2 = $2;
	}
	|	lp mbid rp
	{
		ap->xp2 = 0;
	}
	;


mbd	:	LBRKT reg.ss CM disp RBRKT
	{
		/* If register is a data or null data register, it may 
		 * be an index register with base suppressed, then assume preindex.
		 * If the size and scale flag is on, and the register is legal
		 * index register, then assume preindex with base suppressed.
		 */
		if ( $2->tag == ADREG || $2->tag == NULDREG )
			mk_full_ind(PRENDXMI,0,$2,$4);
		else
		if ( ssflag && ($2->tag == AAREG || $2->tag == NULAREG) )
			mk_full_ind(PRENDXMI,0,$2,$4);
		else
		{
			if (ssflag)
				werror("base register should have no size/scale");
			mk_full_ind(POSTNDXMI,$2,0,$4);
		}
	}
	|	LBRKT disp CM reg.ss RBRKT
	{
		if ( $4->tag == ADREG || $4->tag == NULDREG )
			mk_full_ind(PRENDXMI,0,$4,$2);
		else
		if ( ssflag && ($4->tag == AAREG || $4->tag == NULAREG) )
			mk_full_ind(PRENDXMI,0,$4,$2);
		else
		{
			if (ssflag)
				werror("base register should have no size/scale");
			mk_full_ind(POSTNDXMI,$4,0,$2);
		}
	}
	|	LBRKT disp RBRKT
	{
		mk_full_ind(POSTNDXMI,0,0,$2);
	}
	|	LBRKT reg.ss RBRKT
	{
		/* if register is data or null data register, it may
		 * be an index register with base suppressed. Then assume preindex.
		 */
		if ( $2->tag == ADREG || $2->tag == NULDREG )
			mk_full_ind(PRENDXMI,0,$2,0);
		else
		if ( ssflag && ($2->tag == AAREG || $2->tag == NULAREG) )
			mk_full_ind(PRENDXMI,0,$2,0);
		else
		{
			if (ssflag)
				werror("base register should have no size/scale");
			mk_full_ind(POSTNDXMI,$2,0,0);
		}
	}
	;


mbid	:	LBRKT disp CM reg.ss CM reg.ss RBRKT
	{	
		mkindex_indirect(PRENDXMI,$4,$6,$2);
	}
	|	LBRKT reg.ss CM disp CM reg.ss RBRKT
	{ 
		mkindex_indirect(PRENDXMI,$2,$6,$4);
	}
	|	LBRKT reg.ss CM reg.ss CM disp RBRKT
	{
		mkindex_indirect(PRENDXMI,$2,$4,$6);
	}
	|	LBRKT reg.ss CM reg.ss RBRKT
	{
		mkindex_indirect(PRENDXMI,$2,$4,0);
	}
	;


disp	:	expr optsize
	{
		$1->xsize = $2;
		$$ = $1;
	}
	;


reg.ss	:	REG ss
	{
		regseen++;
		if( $2 == SET )		/* mark index register if seen */
			indexreg = regseen;
		$$ = $1;
	}
	;


ss	:	optsize optscl
	{ 
		if( ssflag && ($1 != UNSPEC || $2 == SET) )
		{
			yyerror("only one index register allowed");
			if ( $2 != SET ) 
				ap->ascale = X1;	/* set back to default value */
			ap->asize = $1;	/* set size of index */
			$$ = SET;
		}
		else if( $1 != UNSPEC || $2 == SET )
		{
			ssflag++;
			ap->asize = $1;	/* set size of index */
			$$ = SET;
		}
		else
			$$ = NOTSET;
	}
	;

optscl	:	MUL INT
	{
		switch($2)
		{
			case 1:
				ap->ascale = X1;
				break;
			case 2:
				ap->ascale = X2;
				break;
			case 4:
				ap->ascale = X4;
				break;
			case 8:
				ap->ascale = X8;
				break;
			default:
				yyerror("illegal scale for index register");
				ap->ascale = X1;
		}
		$$ = SET;
	}
	|	/* empty */
	{
		$$ = NOTSET;
	}
	;



lp	:	LP
	{
		if ( lp_count == 0)
		{
			ssflag = 0;
			regseen = 0;
			indexreg = 0;
			ap->areg1 = NULLREG;
			ap->areg2 = NULLREG;
			ap->asize = W;	/* default */
			ap->ascale = X1;/* default */
		}
		lp_count++;
		$$ = $1;
	}
	;

rp	:	RP
	{
		lp_count--;
		$$ = $1;
	}
	;

expr	:	NAME
	{
		if ((xp->xtype = $1->styp & TYPE) == ABS)
		{
			xp->xvalue = $1->value;
			xp->xsymptr = NULL;
			/* M68881 addition */
			xp->xvalue2 = $1->value2;
			xp->xvalue3 = $1->value3;
			/* value already in binary format */
			xp->xvaltype = FP_BIN;
			/* end M68881 addition */
		}
		else
		{
			xp->xvalue = 0;
			xp->xsymptr = $1;
		}
		$$ = xp++;
	}
	|	INT
	{
		xp->xtype = ABS;
		xp->xvalue = $1;
		xp->xsymptr = NULL;
		xp->xvaltype = FP_INT;
		$$ = xp++;
	}
	|	lp expr rp
	{
		$$ = $2;
	}
	|	expr PLUS expr
	{
		$$ = combine(PLUS,$1,$3);
	}
	|	expr MINUS expr
	{
		$$ = combine(MINUS,$1,$3);
	}
	|	expr MUL expr
	{
		$$ = combine(MUL,$1,$3);
	}
	|	expr DIV expr
	{
		$$ = combine(DIV,$1,$3);
	}
	|	MINUS expr				%prec UMINUS
	{
		xp->xtype = ABS;
		xp->xsymptr = NULL;
		xp->xvalue = 0;
		$$ = combine(MINUS,xp++,$2);
	}
	;
	/* M68881 additions */
fp_expr	: 	HEXSTR	/* 68881 floating point hex string */
	{
		xp->xtype = ABS;
		xp->xsymptr = NULL;
		xp->xvaltype = FP_BIN;
		binstr($1);
		$$ = xp++;
	}
	|	DECSTR		/* 68881 floating point decimal string */
	{
		xp->xtype = ABS;
		xp->xsymptr = NULL;
		xp->xvaltype = FP_DEC;
		fp_mansign = POS;
		$$ = xp++;
	}
	|	PLUS DECSTR
	{
		xp->xtype = ABS;
		xp->xsymptr = NULL;
		xp->xvaltype = FP_DEC;
		fp_mansign = POS;
		$$ = xp++;
	}
	|	MINUS DECSTR
	{
		xp->xtype = ABS;
		xp->xsymptr = NULL;
		xp->xvaltype = FP_DEC;
		fp_mansign = NEG;
		$$ = xp++;
	}
	;
	/* end M68881 additions */

%%

int	type[] = {
	EOF,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	SPC,	NL,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	SPC,	0,	DQ,	SH,	0,	REG,	AMP,	SQ,
	LP,	RP,	MUL,	PLUS,	CM,	MINUS,	SIZE,	DIV,
	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,
	DIGIT,	DIGIT,	COLON,	SEMI,	0,	0,	0,	0,
	0,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	LBRKT,	0,	RBRKT,	0,	ALPH,
	0,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	LBRACE,	0,	RBRACE,	TWID,	0,
};

yylex()
{
	upsymins	istp;
	static BOOL	mnemseen = FALSE; /* Mnemonic seen on line yet?	*/
	static BOOL SIZEseen = FALSE; /* a . is seen */
	register char	*yycharptr;
	register int	val;
	register int	ctype;
	register long	intval;

	DBUG_ENTER ("yylex");
	if ( SIZEseen == TRUE ) 
	{	
		ctype = SIZE;
		SIZEseen = FALSE; 
	}
	else
	{
		while (type[(val = getc(fdin)) + 1] == SPC)
		;
		ctype = type[val+1];
	}

	switch (ctype)
	{

	case SEMI:	/* Reinitialize for next statement.		*/
		mnemseen = FALSE;
		DBUG_RETURN(SEMI);

	case SH:	/* Comment until the end of the line.		*/
		while ((val = getc(fdin)) != '\n' && val > 0)
			;
		/* Now same as in case NL.				*/

	case NL:	/* Reinitialize for next statement.		*/
		mnemseen = FALSE;
		DBUG_RETURN(NL);


	case TWID:
		val = '.';
	case ALPH:
		yycharptr = yytext;
		do
		{
#if FLEXNAMES
			if (yycharptr <= &yytext[BUFSIZ - 1])
#else
			if (yycharptr <= &yytext[NCPS])
#endif
				*yycharptr++ = val;
		} while ((ctype = type[(val = getc(fdin)) + 1]) == ALPH || ctype == DIGIT || ctype == REG);
		*yycharptr = '\0';
		while (val=='\t' || val==' ')
			val = getc(fdin);
		ungetc(val,fdin);
		if (val == ':')
			istp = *lookup(yytext,INSTALL,USRNAME);
		else
			if (mnemseen)
				istp = *lookup(yytext,INSTALL,USRNAME);
			else
			{
				istp = *lookup(yytext,N_INSTALL,MNEMON);
				if (istp.stp == NULL)
					yyerror("invalid instruction name");
				yylval.yinst = istp.itp;
				mnemseen = TRUE;
				DBUG_RETURN(istp.itp->val + 256);
			}
		yylval.yname = istp.stp;
		DBUG_RETURN(NAME);

	case REG:
		yycharptr = yytext;
		do
		{
#if FLEXNAMES
			if (yycharptr <= &yytext[BUFSIZ - 1])
#else
			if (yycharptr <= &yytext[NCPS])
#endif
				*yycharptr++ = val;
		} while ((ctype = type[(val = getc(fdin)) + 1]) == ALPH || ctype == DIGIT || ctype == REG);
		ungetc(val,fdin);
		*yycharptr = '\0';
		istp = *lookup(yytext,N_INSTALL,MNEMON);
		if (istp.stp == NULL || istp.itp->val != REG-256)
			yyerror("invalid register name");
		yylval.yinst = istp.itp;
		DBUG_RETURN(REG);

	case DIGIT:
		intval = val - '0';
		if (val == '0')
		{
			val = getc(fdin);
			if (val == 'x' || val == 'X')
			{	/* hex base	*/
				yylval.ystr = yycharptr = yytext;
				while ( hexdigit(val=getc(fdin)) )
				{
					intval <<= 4;
					if ('a' <= val && val <= 'f')
						intval += val - 'a' + 10;
					else if ('A' <= val && val <= 'F')
						intval += val - 'A' + 10;
					else intval += val - '0';
#ifdef M68881
					/* floating-point numbers can be specified explicitly as
	 				 * a series of hexadecimal digits preceded by 0x
	 				 */
					if ( yycharptr < &yytext[BUFSIZ-1] )
						*yycharptr ++ = val;
#endif /* M68881 */
				}
#ifdef M68881
				*yycharptr = '\0';
				if ( strlen(yylval.ystr) > 8 ) /* more than 32 bits */
				{
					ungetc(val,fdin);
					DBUG_RETURN(HEXSTR);
				}
#endif /* M68881 */
			}	/* hex base	*/
			else
			if (type[val + 1] == DIGIT)
					do
					{	/* octal base	*/
						intval <<= 3;
						intval += val - '0';
					} while (type[(val = getc(fdin)) + 1] == DIGIT);
#ifdef M68881
			else
			if (val == '.')
			{
				yylval.ystr = yycharptr =  yytext;
				goto parse_dig;
			}
#endif /* M68881 */
		}	/* hex or octal base	*/
		else
		{	/* decimal base	*/
			yylval.ystr = yycharptr =  yytext;
			*yycharptr ++ = val;

			while (type[(val = getc(fdin)) + 1] == DIGIT)
			{
				intval *= 10;
				intval += val - '0';
#ifdef M68881
				if ( yycharptr   <= &yytext[BUFSIZ - 1])
					*yycharptr  ++ = val;
#endif /* M68881 */
			}
#ifdef M68881
			if ( val == '.' )
	parse_dig:
			{	/* floating point fraction */

				if(type[(val = getc(fdin)) + 1] != DIGIT &&
					(val != 'e' && val != 'E'))
				{
					SIZEseen = TRUE;	/* indicate a . is seen */
					ungetc(val,fdin);
					yylval.yint = intval;
					DBUG_RETURN(INT);
				}
				/* fall through to parse_fp */
			}
			else
			{	/* decimal base */
				ungetc(val,fdin);
				yylval.yint = intval;
				DBUG_RETURN(INT);
			}
	parse_fp :
			/* a digit or E is seen after a decimal point */
			fp_p = 0;
			fp_man = mantissa;
			fp_exp = exponent;
			fp_expsign = POS;
			if ( val != 'e' && val != 'E' )
				do
				{	/* keep on packing digits to mantissa */
					/* count no. of digits after the dec. pt */
					*yycharptr ++ = val;
					fp_p ++;
				} while (type[(val = getc(fdin)) + 1] == DIGIT &&
					(yycharptr    <= &yytext[BUFSIZ - 1]) );

			*yycharptr = '\0';
			strcpy(fp_man,yylval.ystr);
			/* intialize pointer for exponent */
			yylval.ystr = yycharptr = yytext;
			if ( val == 'e' || val == 'E' )
			{
				val = getc(fdin);
				if( val  == '-' )
					fp_expsign = NEG;
				else
				if( type [val + 1] == DIGIT)
					*yycharptr ++ = val;

				while ( type[(val = getc(fdin)) + 1] == DIGIT &&
					(yycharptr    <= &yytext[BUFSIZ - 1]) )
					*yycharptr ++ = val;
			}	/* end E-real */
			*yycharptr = '\0';
			strcpy(fp_exp,yylval.ystr);
			if ( strlen(fp_man) > MANMAX )
				werror("Up to 17 significant digits are allowed");
			if ( strlen(fp_exp) > EXPMAX )
				werror("Up to 3 exponent digits are allowed");
			ungetc(val,fdin);
			DBUG_RETURN ( DECSTR );
#endif /* M68881 */
		}	/* decimal base	*/
		ungetc(val,fdin);
		yylval.yint = intval;
		DBUG_RETURN(INT);

	case SIZE:
		val = getc(fdin);
		yylval.yint = UNSPEC;
		if (val == 'b' || val == 'B')
			yylval.yint = B;
		else if (val == 'w' || val == 'W')
			yylval.yint = W;
		else if (val == 'l' || val == 'L')
			yylval.yint = L;
		else if (val == 's' || val == 'S')
			yylval.yint = S;
		else if (val == 'x' || val == 'X')
			yylval.yint = X;
		else if (val == 'p' || val == 'P')
			yylval.yint = P;
		else if (val == 'd' || val == 'D')
			yylval.yint = D;
#ifdef M68881
		/* check for round mode */
		else if (val == 'n' || val == 'N')
			yylval.yint = RN;
		else if (val == 'm' || val == 'M')
			yylval.yint = RM;
		else if (val == 'z' || val == 'Z')
			yylval.yint = RZ;
		else
		/* check for floationg point integer */
		if( type [val+1] == DIGIT)
		{
			yylval.ystr = yycharptr =  yytext;
			goto parse_fp;
		}
#endif /* M68881 */
		else
			yyerror("unknown size specification");
		DBUG_RETURN(SIZE);

	case SQ:
		if ((val = getc(fdin)) == '\n')
			line++;
		if (val == '\\')
			switch (val = getc(fdin))
			{
			case 'n':
				val = '\n';
				break;
			case 'r':
				val = '\r';
				break;
			case '\\':
			case '\'':
				break;
			case 'b':
				val = '\b';
				break;
			case 't':
				val = '\t';
				break;
			case 'v':
				val = '\013';
				break;
			case 'f':
				val = '\f';
				break;
			default:
				yyerror("illegal escaped character");
			}
		yylval.yint = val;
		DBUG_RETURN(INT);

	case DQ:
		yylval.ystr = yycharptr = yytext;
		while ((val = getc(fdin)) != '"' && val != '\n' && val != EOF)
		{
			if (yycharptr < &yytext[BUFSIZ-1])
				*yycharptr++ = val;
		}
		*yycharptr = '\0';
		if (val != '"')
			yyerror("nonterminated string");
		DBUG_RETURN(STRING);

	case 0:
		yyerror("illegal character");
		DBUG_RETURN(NOCHAR);


	default:
		DBUG_RETURN(ctype);
	}
	DBUG_VOID_RETURN;
}	/* yylex()	*/


hexdigit(val)
register int val;
{
	DBUG_ENTER ("hexdigit");
	if ( type[ val + 1] == DIGIT ||
		('a' <= val && val <= 'f') ||
		('A' <= val && val <= 'F') )
		DBUG_RETURN(1);
	else
		DBUG_RETURN(0);
}

#ifdef M68881
/*
 * binstr - convert a floating point hex string to binary value.
 *          The final result is stored in 3 long words left justified.
 * 
 *          input : char *fp_string containing legal hex characters.
 *
 *          output: long xp->xvalue
 *                       xp->xvalue2
 *                       xp->xvalue3
 *          assumptions: xp-> is the current pointer
 */
binstr(fp_string)
char *fp_string;
{
	register int val;
	register int intval;
	register int temp_fp;
	short int index;
	short int nchar;
	short int pos;	/* position (bit) of the most significant byte */
	long fp_bin[3];	/* temporary storage for fp binary */

	DBUG_ENTER ("binstr");
	if ( strlen(fp_string) > 24 )
		yyerror("floating point hex format allows up to 24 hex digits");

	/* initialize fp_bin */
	for ( index = 0; index < 3 ; index++)
		fp_bin[index] = 0;

	val = *fp_string++;
	for ( index = 0; val != '\0' && index < 3; index++) 
	{
		temp_fp = 0;
		for (nchar = 1, intval = 0 , pos = 32; 
			val != '\0' && nchar <= 8 ; nchar ++, val = *fp_string++)
		{
			/* convert each character to hex value 
			 * and pack them into a long word left justified
			 */
			if ( 'a' <= val && val <= 'f' )
				intval = val - 'a' + 10 ;
			else
			if ( 'A' <= val && val <= 'F' )
				intval = val - 'A' + 10;
			else
				intval = val - '0';

			temp_fp |= (intval <<= (pos -= 4));
		}
		
		fp_bin[index] = temp_fp;
	}


	/* return expression value */
	xp->xvalue = fp_bin[0];
	xp->xvalue2 = fp_bin[1];
	xp->xvalue3 = fp_bin[2];
	
	DBUG_VOID_RETURN;
}	/* end binstr */
#endif /* M68881 */

struct exp *
combine(operator,left,right)
int		operator;
register struct exp	*left,
		*right;
{
	DBUG_ENTER ("combine");
	switch (operator)
	{
	case PLUS:
		if (left->xsymptr == NULL)
		{
			left->xsymptr = right->xsymptr;
			left->xtype = right->xtype;
		}
		else if (right->xsymptr != NULL)
			yyerror("illegal addition");
		left->xvalue += right->xvalue;
		DBUG_RETURN(left);
	case MINUS:
		if (swbegct > 0)
			DBUG_RETURN(left);
		if (left->xsymptr == NULL)
		{
			if (right->xsymptr != NULL)
				yyerror("illegal subtraction");
			left->xvalue -= right->xvalue;
		}
		else if (right->xsymptr == NULL)
			left->xvalue -= right->xvalue;
		else if (left->xtype == right->xtype && left->xtype != UNDEF)
		{
			left->xtype = ABS;
			left->xvalue = left->xvalue - right->xvalue +
				       left->xsymptr->value - right->xsymptr->value;
			left->xsymptr = NULL;		/* no symbol now */
		}
		else
		{
			yyerror("illegal subtraction");
			left->xsymptr = NULL;
			left->xtype = ABS;
			left->xvalue = 0;
		}
		DBUG_RETURN(left);
	case MUL:
		if (left->xsymptr != NULL || right->xsymptr != NULL)
			yyerror("illegal multiplication");
		left->xsymptr = NULL;
		left->xtype = ABS;
		left->xvalue *= right->xvalue;
		DBUG_RETURN(left);
	case DIV:
		if (left->xsymptr != NULL || right->xsymptr != NULL)
			yyerror("illegal division");
		left->xsymptr = NULL;
		left->xtype = ABS;
		left->xvalue /= right->xvalue;
		DBUG_RETURN(left);
	}
}	/* combine()	*/



fill(nbytes)
register long	nbytes;
{
	register long	fillval;

	DBUG_ENTER ("fill");
	if (nbytes <= 0)
	    DBUG_VOID_RETURN;
	
	if (newdot & 1)			/* always use FILL to get to even
					** boundary
					*/
	{
	    nbytes--;
	    generate(BITSPBY,0,FILL,NULLSYM);
	}

	fillval = ((dot->styp & TYPE) == TXT) ? TXTFILL : FILL;

	while (nbytes >= 2)
	{
		generate(BITSPOW,0,fillval,NULLSYM);
		nbytes -= 2;
	}
	if (nbytes)			/* fill out odd byte with FILL */
		generate(BITSPBY,0,FILL,NULLSYM);
	DBUG_VOID_RETURN;
}	/* fill()	*/



ckalign(size)
register int	size;
{
	register int	mod;

	DBUG_ENTER ("ckalign");
	if ((mod = newdot % size) != 0)
		fill(size - mod);
	DBUG_VOID_RETURN;
}	/* ckalign()	*/

/*
 *		68020 SUPPORT FUNCTIONS
 *
 */

#define NULLREG	(instr *)0

/* mk_indirect:
 *
 *	This module is used for addressing syntax
 *	d(reg), (d,reg), and (reg,d)
 *	It validates the input register and generates
 *	efficient addressing mode as much as possible.
 */
mk_indirect(type,reg,disp)
short type;
instr	*reg;
struct	exp	*disp;
{
	/* d(An), d(pc)
	 * 1. if ssflag is off, register may be an base register,
	 *    then check for valid base register for indirect with
	 *    displacement.
	 *    if displacement is within 16 bits then generate register
	 *    indirect with displacement or program counter indirect
	 *    with displacement.
	 * 2. if ssflag is on, register may be an index register,
	 *    then check for valid index register.
	 */
	int		distance;	/* distance between label and currect pc */
	DBUG_ENTER ("mk_indirect");
		if ( !ssflag && ( reg->tag == PCREG || reg->tag == AAREG ) )
		{
			/* d(pc), d(An) */
			if ( disp->xsize == B || disp->xsize == W )
				mkdisp_indirect(INDIRECT,reg,disp);
			else
			if ( disp->xsymptr==NULL &&
				disp->xvalue <= 32767L && disp->xvalue >= -32768L )

				mkdisp_indirect(INDIRECT,reg,disp);
			else
			if ( disp->xsymptr!=NULL &&
				(disp->xsymptr->styp & TYPE) != UNDEF &&
			 	(disp->xsymptr->styp & TYPE) == dot->styp )
			{
				distance= disp->xsymptr->value - dot->value;
				if ( distance <= 32767L && distance >= -32768L )
					mkdisp_indirect(INDIRECT,reg,disp);
				else
					mkindex_indirect(INDIRECT,reg,0,disp);
			}
			else
				mkindex_indirect(INDIRECT,reg,0,disp);
		}
		else
		if ( reg->tag == AAREG || reg->tag == NULAREG 
			|| reg->tag == ADREG || reg->tag == NULDREG 
			|| reg->tag == PCREG  || reg->tag == NULPCREG )
			mkindex_indirect(INDIRECT,reg,0,disp);
		else
			yyerror ("illegal use of register in register indirect with displacement");
	DBUG_VOID_RETURN;
}

mkdisp_indirect(type,reg,disp)
short type;
instr	*reg;
struct	exp	*disp;
{	/* make addressing register indirect with displacement or
	 * make program counter indirect with displacement
	 */
			/*  d(An) / d(PC) */
	DBUG_ENTER ("mkdisp_indirect");
			if (reg->tag == PCREG)
				ap->atype = APCOFF;
			else
			if (reg->tag == AAREG)
				ap->atype = AOFF;
			ap->xp = disp;
			ap->areg1 = reg;
			ap->areg2 = 0;
	DBUG_VOID_RETURN;
}

/*
 *	mkindex_indirect()	- enters with four arguments:
 *
 *		type (short)
 *		disp (struct *exp)
 *		reg1 (instr *)
 *		reg2 (instr *)
 *
 *	we're not absolutely sure which register is the
 *	base and which is the index when no size or scale
 *	has been specified up to this point, so we need
 *	to determine which is which.
 *	possible base registers are:
 *
 *		%an, %pc, %zan
 *
 *	and the base register may be missing altogether.
 *
 *	possible index registers are:
 *
 *		%dn, %an, %zan, %zdn
 *
 */
mkindex_indirect(type,reg1,reg2,disp)
short type;
instr	*reg1;
instr	*reg2;
struct	exp	*disp;
{
	instr	*tmpreg;

	/*
	 *	determine index register (if any and not specified)
	 *	and flip registers if necessary so base is reg1 and
	 *	index is reg2.
	 */

	/*
	 *	first: if no index register was found explicitly
	 *	(had a ".s" or "*scale" attribute), check for a 
	 *	"%d" register in the first position, and flip.
	 *	NOTE: at this point it does not matter if there is
	 *	"%d" register in the second position, this error is checked
	 *	for in the next routine called.
	 *
	 *	else, do we have a null first register and a "%a" register
	 *	with no size or scale attributes?  then move it to the
	 *	first position where it assumes the position of index register.
	 *	so we force lone "%a" registers otherwise unspecified to be
	 *	an index.
	 */

	DBUG_ENTER ("mkindex_indirect");
	if( (indexreg == 0 && reg1 && (reg1->tag == NULDREG || reg1->tag == ADREG)) 
		|| indexreg == 1 )
	{
		tmpreg = reg1;
		reg1 = reg2;
		reg2 = tmpreg;
	}
	else if( indexreg == 0 && reg1 == NULLREG && reg2 && 
		(reg2->tag == ADREG || reg2->tag == NULAREG) )
	{
		tmpreg = reg1;
		reg1 = reg2;
		reg2 = tmpreg;
	}
	else if( reg1 && (reg1->tag == AAREG || reg1->tag == NULAREG) 
			&& reg2 && (reg2->tag == PCREG || reg2->tag == NULPCREG) )
	{
		tmpreg = reg1;
		reg1 = reg2;
		reg2 = tmpreg;
	}

	mk_full_ind(type,reg1,reg2,disp);
	DBUG_VOID_RETURN;
}


/*
 *	common routine to construct indexed indirect
 *	addressing argument.
 *
 *	we enter with 4 arguments:
 *
 *		type		- type of indirect addressing, 3 kinds
 *		base		- the base register
 *		index		- the index register
 *		base_disp	- the displacement (before indirection)
 *
 *	the regs are preordered before entering this routine so we
 *	need only to check that the regs are the proper types for
 *	the given argument.
 *
 *	NOTE: this routine does not handle the memory indirect case.
 */


mk_full_ind(type,base,index,base_disp)
short	type;
instr	*base;
instr	*index;
struct exp	*base_disp;
{
	int		distance;	/* distance between label and current pc */


	DBUG_ENTER ("mk_full_ind");
	if( base && !(base->tag == AAREG || base->tag == PCREG 
		|| base->tag == NULAREG || base->tag == NULPCREG) )
	{
		yyerror("base register must be %pc, %an, %zpc or %zan");
		base = NULLREG;
	}


	if( index && !(index->tag == ADREG || index->tag == AAREG 
		|| index->tag == NULDREG || index->tag == NULAREG) )
	{
		yyerror("index register must be %dn, %an, %zdn or %zan");
		index = NULLREG;
	}

	/*
	 *	null out regs tagged NULxxx 
	 */

	if( base && base->tag == NULAREG || base->tag == NULPCREG )
		base = NULLREG;

	if( index && index->tag == NULDREG || index->tag == NULAREG )
		index = NULLREG;

	/*
	 *	set addressing mode, assign regs to argument and return
	 */

	if( base && base->tag == PCREG )
		ap->atype = APCNDX;
	else
		ap->atype = ANDX;

	ap->aflags = type;
	ap->xp1 = base_disp;

	/*
	 *	only set base and index members of ap
	 *	if set.  they are initialized to null.
	 */

	if( base )
		ap->areg1 = base;
	if( index )
		ap->areg2 = index;

	/* generate brief format:
	 * if no memory indirect, both base and index register exist
	 * (no suppressed register and displacement fixex into a byte
	 */
	if (ap->aflags == INDIRECT && base && index && ap->xp1)
	{
		if ( ap->xp1->xsize == B )
			ap->aflags = BFINDIRECT;
		else
		if (ap->xp1->xsymptr == NULL &&
			( ap->xp1->xvalue >= -128 && ap->xp1->xvalue <= 127))
			ap->aflags = BFINDIRECT;
		else
		if (ap->xp1->xsymptr != NULL &&
			(ap->xp1->xsymptr->styp!= UNDEF) &&
			(ap->xp1->xsymptr->styp & TYPE ) == dot->styp )
		{
				distance = ap->xp1->xsymptr->value - dot->value;
				if (distance >= -128 && distance <= 127)
					ap->aflags = BFINDIRECT;
		}
	} 
	DBUG_VOID_RETURN;
}


/*
 *	mk_post_ind():	add in index and displacement for 
 *			post index memory indirect.
 *
 *	This routine is called after the grunt work performed
 *	on the common sub-expression by mkindex_indirect().
 *	It will check for valid index register, and set up
 *	pointer for index and displacement.
 *
 */

mk_post_ind(index,disp)
instr	*index;
struct exp	*disp;
{
	DBUG_ENTER ("mk_post_ind");
	if(ap->aflags != POSTNDXMI )
		yyerror ("syntax error");

	if( index && !(index->tag == NULDREG || index->tag == ADREG
		|| index->tag == NULAREG || index->tag == AAREG) )
	{
		yyerror("only %an,%zan,%dn,%zdn index in memory indirect");
		index = NULLREG;
	}

	if( index && index->tag == NULDREG || index->tag == NULAREG )
		index = NULLREG;

	ap->areg2 = index;
	ap->xp2 = disp;		/* displacement after memory indirection */
	DBUG_VOID_RETURN;
}

power(x,n)	/* raise x to n-th power ; n => 0 */
int x, n;
{
	int value;
	DBUG_ENTER ("power");
	for ( value = 1; n > 0; --n)
		value = value * x;
	DBUG_RETURN (value);
}

/* Modules required for M68881 coprocessor */
get_fp_prec(prec)
short int prec;
{
	DBUG_ENTER ("get_fp_prec");
	switch((int)(unsigned char)	prec )
	{
		case S:
			user_prec = FP_S;
			break;
		case D:
			user_prec = FP_D;
			break;
		case X:
			user_prec = FP_X;
			break;
		case P:
			user_prec = FP_P;
			break;
		default :
			aerror("Illegal floating point precision in parser");
	}	/* end switch */

	DBUG_VOID_RETURN;
}	/* end get_fp_prec */

get_fp_round(rd_mode)
short int rd_mode;
{
	DBUG_ENTER ("get_fp_round");
	switch((int)(unsigned char)	rd_mode )
	{
		case RN:
			fp_round = FP_RN;
			break;
		case P:
			fp_round = FP_RP;
			break;
		case RM:
			fp_round = FP_RM;
			break;
		case RZ:
			fp_round = FP_RZ;
			break;
		case UNSPEC:
			fp_round = FP_RN;
			break;
		default :
			aerror("Illegal floating point rounding mode in parser");
	}	/* end switch */
	DBUG_VOID_RETURN;
}	/* end get_fp_round */

/*
 *	GET_VALUE
 *		Returns floating point value in binary format for symbol.
 *	Requires Global variables:
 *	FP_STR:
 *		fp_man		floating point mantissa string
 *		fp_exp		floating point exponent string
 *		fp_mansign	sign of mantissa
 *		fp_expsign	sign of exponent
 *		fp_p		no. of decimal digits to the right of dec. pt.
 *
 *	user_prec:		if not defined, precision .X will be used.
 */
get_value(sym)
symbol *sym;
{
	int	fp_num[6];

	DBUG_ENTER ("get_value");
	switch((int)(unsigned char)	user_prec )
	{
		case FP_S:
			fp_precision = FP_S;
			break;
		case FP_D:
			fp_precision = FP_D;
			break;
		case FP_X:
			fp_precision = FP_X;
			break;
		case FP_P:
			fp_precision = FP_P;
			break;
		default :
			fp_precision = FP_X;
	}	/* end switch */

	fp_extension = fp_precision;
	do_fp_num(fp_num);

	sym->value  = (fp_num[5] & 0xffff) ;
	sym->value  = (sym->value  << 16 | fp_num[4] & 0xffff);

	sym->value2  = (fp_num[3] & 0xffff) ;
	sym->value2  = (sym->value2  << 16 | fp_num[2] & 0xffff);

	sym->value3  = (fp_num[1] & 0xffff) ;
	sym->value3  = (sym->value3  << 16 | fp_num[0] & 0xffff);
	DBUG_VOID_RETURN;
}	/* end get_value */
