/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)comp:common/cgram.y	1.13"
%}
%{
%}

/******/
/*	Note: In this file there are comments of the form
/*		/*CXREF <line-of-code> */
/*	When this file is used to build cxref, the beginning
/*	of the comment and the CXREF are removed along with
/*	the trailing end of the comment.  Therefore, these
/*	comments cannot be removed with out changing cxref's
/*	version of this file.
/******/

%term NAME  2
%term STRING  3
%term ICON  4
%term FCON  5
%term PLUS   6
%term MINUS   8
%term MUL   11
%term AND   14
%term OR   17
%term ER   19
%term QUEST  21
%term COLON  22
%term ANDAND  23
%term OROR  24

/*	special interfaces for yacc alone */
/*	These serve as abbreviations of 2 or more ops:
	ASOP	=, = ops
	RELOP	LE,LT,GE,GT
	EQUOP	EQ,NE
	DIVOP	DIV,MOD
	SHIFTOP	LS,RS
	ICOP	ICR,DECR
	UNOP	NOT,COMPL
	STROP	DOT,STREF

	*/
%term ASOP  25
%term RELOP  26
%term EQUOP  27
%term DIVOP  28
%term SHIFTOP  29
%term INCOP  30
%term UNOP  31
%term STROP  32

/*	reserved words, etc */
%term TYPE  33
%term CLASS  34
%term STRUCT  35
%term RETURN  36
%term GOTO  37
%term IF  38
%term ELSE  39
%term SWITCH  40
%term BREAK  41
%term CONTINUE  42
%term WHILE  43
%term DO  44
%term FOR  45
%term DEFAULT  46
%term CASE  47
%term SIZEOF  48
%term ENUM 49


/*	little symbols, etc. */
/*	namely,

	LP	(
	RP	)

	LC	{
	RC	}

	LB	[
	RB	]

	CM	,
	SM	;

	*/

%term LP  50
%term RP  51
%term LC  52
%term RC  53
%term LB  54
%term RB  55
%term CM  56
%term SM  57
%term ASSIGN  58
%term ASM 59

/* at last count, there were 7 shift/reduce, 1 reduce/reduce conflicts
/* these involved:
	if/else
	recognizing functions in various contexts, including declarations
	error recovery
	*/

%left CM
%right ASOP ASSIGN
%right QUEST COLON
%left OROR
%left ANDAND
%left OR
%left ER
%left AND
%left EQUOP
%left RELOP
%left SHIFTOP
%left PLUS MINUS
%left MUL DIVOP
%right UNOP
%right INCOP SIZEOF
%left LB LP STROP
%{
# include "mfile1"
# include "messages.h"
%}

	/* define types */
%start ext_def_list

%type <intval> con_e ifelprefix ifprefix doprefix switchpart
		enum_head str_head name_lp
%type <nodep> e .e term attributes oattributes type enum_dcl struct_dcl
		cast_type null_decl funct_idn declarator fdeclarator nfdeclarator
		elist

%token <intval> CLASS NAME STRUCT RELOP CM DIVOP PLUS MINUS SHIFTOP MUL AND OR ER ANDAND OROR
		ASSIGN STROP INCOP UNOP ICON
%token <nodep> TYPE

%%

%{
	extern int wloop_level;	/* specifies while loop code generation */
	extern int floop_level;	/* specifies for loop code generation */
	static int fake = 0;
#ifdef FLEXNAMES
	static char fakename[24];	/* room enough for pattern */
#else
	static char fakename[NCHNAM+1];
#endif
%}

ext_def_list:	   ext_def_list external_def
		|
		{
			/* do implementation dep. stuff now */
			beg_file();	
			ftnend();
		}
		;
external_def:	   data_def
			={ curclass = SNULL;  blevel = 0; }
		|  error
			={ curclass = SNULL;  blevel = 0; }
		|  ASM SM
			{ asmout();  curclass = SNULL; blevel = 0; }
		;
data_def:
		   oattributes  SM
			={  $1->in.op = FREE; }
		|  oattributes init_dcl_list  SM
			={  $1->in.op = FREE; }
		|  oattributes fdeclarator {
/* try to keep this as local as possible */
/* got a function. is it inline? */
#ifdef IN_MACRO
				if (curclass == INLINE)
				{
/* squirrelmacro will purge stdin up to closing curly. then it'll try to */
/* get the parser to see "}" by ungetc('}') to believe the "function body" */
/*  part (it's already seen the "{") */
					defid(tymerge($1, $2), INLINE);
					squirrelmacro(stab[$2->tn.rval].sname,
					    paramno);
				} else

#endif
					
				defid( tymerge($1,$2), curclass==STATIC?STATIC:EXTDEF );
				/*CXREF bbcode(); */
				}  function_body
			={  
			    if( blevel ) cerror( "function level error" );
			    if( reached ) retstat |= NRETVAL; 
			    $1->in.op = FREE;
			    ftnend();
			    }
		;

function_body:	   arg_dcl_list compoundstmt
		;
arg_dcl_list:	   arg_dcl_list declaration
		| 	={  blevel = 1; }
		;

stmt_list:	   stmt_list statement
		|  /* empty */
			{
				bccode();
				locctr(PROG);
			}
		;

dcl_stat_list	:  dcl_stat_list attributes SM
			={  $2->in.op = FREE; }
		|  dcl_stat_list attributes init_dcl_list SM
			={  $2->in.op = FREE; }
		|  /* empty */
		;
declaration:	   attributes declarator_list  SM
			={ curclass = SNULL;  $1->in.op = FREE; }
		|  attributes SM
			={ curclass = SNULL;  $1->in.op = FREE; }
		|  error  SM
			={  curclass = SNULL; }
		;
oattributes:	  attributes
		|  /* VOID */
			={  $$ = mkty(INT,0,INT);  curclass = SNULL; }
		;
attributes:	   class type
			={  $$ = $2; }
		|  type class
		|  class
			={  $$ = mkty(INT,0,INT); }
		|  type
			={ curclass = SNULL ; }
		|  type class type
			={  $1->in.type =
				types( $1->in.type, $3->in.type, UNDEF );
			    $3->in.op = FREE;
			    }
		;


class:		  CLASS
			={  curclass = $1; }
		;

type:		   TYPE
		|  TYPE TYPE
			={  $1->in.type = types( $1->in.type, $2->in.type, UNDEF );
			    $2->in.op = FREE;
			    }
		|  TYPE TYPE TYPE
			={  $1->in.type = types( $1->in.type, $2->in.type, $3->in.type );
			    $2->in.op = $3->in.op = FREE;
			    }
		|  struct_dcl
		|  enum_dcl
		;

enum_dcl:	   enum_head LC moe_list optcomma RC
			={ $$ = dclstruct($1); }
		|  ENUM NAME
			={  $$ = rstruct($2,0);  stwart = instruct;
				/*CXREF ref($2, lineno); */ }
		;

enum_head:	   ENUM
			={  $$ = bstruct(-1,0); stwart = SEENAME; }
		|  ENUM NAME
			={  $$ = bstruct($2,0); stwart = SEENAME;
				/*CXREF ref($2, lineno); */ }
		;

moe_list:	   moe
		|  moe_list CM moe
		;

moe:		   NAME
			={  moedef( $1 ); /*CXREF def($1, lineno); */ }
		|  NAME ASSIGN con_e
			={  strucoff = $3;  moedef( $1 );
				/*CXREF def($1, lineno); */ }
		;

struct_dcl:	   str_head LC type_dcl_list optsemi RC
			={ $$ = dclstruct($1);  }
		|  STRUCT NAME
			={  $$ = rstruct($2,$1); /*CXREF ref($2, lineno); */ }
		;

str_head:	   STRUCT
			={  $$ = bstruct(-1,$1);  stwart=0; }
		|  STRUCT NAME
			={  $$ = bstruct($2,$1);  stwart=0;
				/*CXREF def($2, lineno); */ }
		;

type_dcl_list:	   type_declaration
		|  type_dcl_list SM type_declaration
		;

type_declaration:  type declarator_list
			={ curclass = SNULL;  stwart=0; $1->in.op = FREE; }
		|  type
			={  if( curclass != MOU ){
				curclass = SNULL;
				}
			    else {
				sprintf( fakename, "$%dFAKE", fake++ );
				/* save fakename in string table in case we get
				** more than one such name in a struct/union.
				** Otherwise we get illegal redeclaration.
				*/
				defid( tymerge($1,
					bdty(NAME,NIL,
					lookup( hash(fakename), SMOS ))), curclass );
				/*"structure typed union member must be named"*/
				WERROR(MESSAGE( 106 ));
				}
			    stwart = 0;
			    $1->in.op = FREE;
			    }
		;


declarator_list:   declarator
			={ defid( tymerge($<nodep>0,$1), curclass);  stwart = instruct; }
		|  declarator_list  CM {$<nodep>$=$<nodep>0;}  declarator
			={ defid( tymerge($<nodep>0,$4), curclass);  stwart = instruct; }
		;
declarator:	   fdeclarator
		|  nfdeclarator
		|  nfdeclarator COLON con_e
			%prec CM
			/* "field outside of structure" */
			={  if( !(instruct&INSTRUCT) ) UERROR( MESSAGE( 38 ) );
			    if( $3<0 || $3 >= FIELD ){
				/* "illegal field size" */
				UERROR( MESSAGE( 56 ) );
				$3 = 1;
				}
			    defid( tymerge($<nodep>0,$1), FIELD|$3 );
			    $$ = NIL;
			    }
		|  COLON con_e
			%prec CM
			/* "field outside of structure" */
			={  if( !(instruct&INSTRUCT) ) UERROR( MESSAGE( 38 ) );
			    falloc( stab, $2, -1, $<nodep>0 );  /* alignment or hole */
			    $$ = NIL;
			    }
		|  error
			={  $$ = NIL; }
		;

		/* int (a)();   is not a function --- sorry! */
nfdeclarator:	   MUL nfdeclarator		
			={  umul:
				$$ = bdty( UNARY MUL, $2, 0 ); }
		|  nfdeclarator  LP   RP		
			={  uftn:
				$$ = bdty( UNARY CALL, $1, 0 );  }
		|  nfdeclarator LB RB		
			={  uary:
				$$ = bdty( LB, $1, 0 );  }
		|  nfdeclarator LB con_e RB	
			={  bary:
				/* "zero or negative subscript" */
				if( (int)$3 <= 0 ) WERROR( MESSAGE( 119 ) );
				$$ = bdty( LB, $1, $3 );  }
		|  NAME  		
			{
			$$ = bdty( NAME, NIL, $1 );
				/*CXREF def($1, lineno); */
			}
		|   LP  nfdeclarator  RP 		
			={ $$=$2; }
		;
fdeclarator:	   MUL fdeclarator
			={  goto umul; }
		|  fdeclarator  LP   RP
			={  goto uftn; }
		|  fdeclarator LB RB
			={  goto uary; }
		|  fdeclarator LB con_e RB
			={  goto bary; }
		|   LP  fdeclarator  RP
			={ $$ = $2; }
		|  name_lp
				{if(paramno)uerror("arg list in declaration");}
			name_list  RP
			/* we can't deal with args in declarations of funcs */
			/* this is patch of a botch in the rules: char f(p); */
			={
				/* "function declaration in bad context" */
				if( blevel!=0 ) UERROR(MESSAGE( 44 ));
				$$ = bdty( UNARY CALL, bdty(NAME,NIL,$1), 0 );
				stwart = 0;
				}
		|  name_lp {if(paramno)uerror("arg list in declaration");} RP
			={
				$$ = bdty( UNARY CALL, bdty(NAME,NIL,$1), 0 );
				stwart = 0;
				}
		;

name_lp:	  NAME LP
			={
				/* turn off typedefs for argument names */
				/*CXREF newf($1, lineno); def($1, lineno); */
				stwart = SEENAME;
				if( stab[$1].sclass == SNULL )
				    stab[$1].stype = FTN;
				}
		;

name_list:	   NAME			
			{
			    if (stab[idname].sclass == LABEL)
				/* Label used in expression */
				UERROR( MESSAGE(129) );
			    ftnarg( $1 );  stwart = SEENAME;
				/*CXREF ref($1, lineno); */ }
		|  name_list  CM  NAME 
			={ ftnarg( $3 );  stwart = SEENAME;
				/*CXREF ref($3, lineno); */ }
		|  error
		;
		/* always preceeded by attributes: thus the $<nodep>0's */
init_dcl_list:	   init_declarator
			%prec CM
		|  init_dcl_list  CM {$<nodep>$=$<nodep>0;}  init_declarator
		;
		/* always preceeded by attributes */
xnfdeclarator:	   nfdeclarator
			={  defid( $1 = tymerge($<nodep>0,$1), curclass);
			    beginit($1->tn.rval);
			    }
		|  error
		;
		/* always preceeded by attributes */
init_declarator:   nfdeclarator
			={  nidcl( tymerge($<nodep>0,$1) ); }
		|  fdeclarator
			={  defid( tymerge($<nodep>0,$1), uclass(curclass) );
			}
/* old init is out
		|  xnfdeclarator optasgn e
			%prec CM
			={  doinit( $3 );
			    endinit(); }
		|  xnfdeclarator optasgn LC init_list optcomma RC
			={  endinit(); }
*/
		|  xnfdeclarator ASSIGN e
			%prec CM
			={  doinit( $3 );
			    endinit(); }
		|  xnfdeclarator ASSIGN LC init_list optcomma RC
			={  endinit(); }
		|  error
		;

init_list:	   initializer
			%prec CM
		|  init_list  CM  initializer
		;
initializer:	   e
			%prec CM
			={  doinit( $1 ); }
		|  ibrace init_list optcomma RC
			={  irbrace(); }
		;

optcomma	:	/* VOID */
		|  CM
		;

optsemi		:	/* VOID */
		|  SM
		;

/* old init is out
optasgn		:	/* VOID */
			/* "old-fashioned initialization: use =" */
/*
			={  WERROR( MESSAGE( 88 ) ); }
		|  ASSIGN
		;
*/

ibrace		: LC
			={  ilbrace(); }
		;

/*	STATEMENTS	*/

compoundstmt:	   begin dcl_stat_list stmt_list RC
			={  --blevel;
			    /*CXREF becode(); */
			    if( blevel == 1 ) blevel = 0;
			    clearst( blevel );
			    cleardim();
			    checkst( blevel );
			    autooff = *--psavbc;
			    regvar = *--psavbc;
			    }
		;

begin:		  LC
			={  setdim();
			    if( blevel == 1 ) dclargs();
			    /*CXREF else if (blevel > 1) bbcode(); */
			    ++blevel;
			    if( psavbc > &asavbc[BCSZ-2] ) cerror( "nesting too deep" );
			    *psavbc++ = regvar;
			    *psavbc++ = autooff;
			    }
		;

statement:	   e   SM
			={ ecomp( $1 ); }
		|  ASM SM
			{ asmout(); }
		|  compoundstmt
		|  ifprefix statement
			={ deflab($1);
			   reached = 1;
			   }
		|  ifelprefix statement
			={  if( $1 != NOLAB ){
				deflab( $1 );
				reached = 1;
				}
			    }
		|  WHILE LP e RP
			{
				savebc();
				if (!reached)
					/* "loop not entered at top" */
					WERROR(MESSAGE(75));
				reached = 1;
				brklab = getlab();
				contlab = getlab();
				switch (wloop_level)
				{
				default:
					cerror("bad while loop code gen value");
					/*NOTREACHED*/
				case LL_TOP:	/* test at loop top */
					deflab(contlab);
					if ($3->in.op == ICON &&
						$3->tn.lval != 0)
					{
						flostat = FLOOP;
						tfree($3);
					}
					else
						ecomp(buildtree(CBRANCH, $3,
							bcon(brklab)));
					break;
				case LL_BOT:	/* test at loop bottom */
					if ($3->in.op == ICON &&
						$3->tn.lval != 0)
					{
						flostat = FLOOP;
						tfree($3);
						deflab(contlab);
					}
					else
					{
						branch(contlab);
						deflab($<intval>$ = getlab());
					}
					break;
				case LL_DUP:	/* dup. test at top & bottom */
					if ($3->in.op == ICON &&
						$3->tn.lval != 0)
					{
						flostat = FLOOP;
						tfree($3);
						deflab($<intval>$ = contlab);
					}
					else
					{
#ifndef LINT
/* don't rotate loops for lint */
						register NODE *sav;
						extern NODE *treecpy();

						sav = treecpy($3);
						ecomp(buildtree(CBRANCH,$3,
							bcon(brklab)));
						$3 = sav;
#endif
						deflab($<intval>$ = getlab());
					}
					break;
				}
			}
			statement
		{
			switch (wloop_level)
			{
			default:
				cerror("bad while loop code gen. value");
				/*NOTREACHED*/
			case LL_TOP:	/* test at loop top */
				branch(contlab);
				break;
			case LL_BOT:	/* test at loop bottom */
				if (flostat & FLOOP)
					branch(contlab);
				else
				{
					reached = 1;
					deflab(contlab);
					ecomp(buildtree(CBRANCH,
						buildtree(NOT, $3, NIL),
						bcon($<intval>5)));
				}
				break;
			case LL_DUP:	/* dup. test at top & bottom */
				if (flostat & FLOOP)
					branch(contlab);
				else
				{
					if (flostat & FCONT)
					{
						reached = 1;
						deflab(contlab);
					}
					ecomp(buildtree(CBRANCH,
						buildtree(NOT, $3, NIL),
						bcon($<intval>5)));
				}
				break;
			}
			if ((flostat & FBRK) || !(flostat & FLOOP))
				reached = 1;
			else
				reached = 0;
			deflab(brklab);
			resetbc(0);
		}
		|  doprefix statement WHILE  LP  e  RP   SM
			={  deflab( contlab );
			    if( flostat & FCONT ) reached = 1;
			    ecomp( buildtree( CBRANCH, buildtree( NOT, $5, NIL ), bcon( $1 ) ) );
			    deflab( brklab );
			    reached = 1;
			    resetbc(0);
			    }
		|  FOR LP .e SM .e SM
			{
				if ($3)
					ecomp($3);
				else if (!reached)
					/* "loop not entered at top" */
					WERROR(MESSAGE(75));
				savebc();
				contlab = getlab();
				brklab = getlab();
				reached = 1;
				switch (floop_level)
				{
				default:
					cerror("bad for loop code gen. value");
					/*NOTREACHED*/
				case LL_TOP:	/* test at loop top */
					deflab($<intval>$ = getlab());
					if (!$5)
						flostat |= FLOOP;
					else if ($5->in.op == ICON &&
						$5->tn.lval != 0)
					{
						flostat |= FLOOP;
						tfree($5);
						$5 = (NODE *)0;
					}
					else
						ecomp(buildtree(CBRANCH, $5,
							bcon(brklab)));
					break;
				case LL_BOT:	/* test at loop bottom */
					if (!$5)
						flostat |= FLOOP;
					else if ($5->in.op == ICON &&
						$5->tn.lval != 0)
					{
						flostat |= FLOOP;
						tfree($5);
						$5 = (NODE *)0;
					}
					else
						branch($<intval>1 = getlab());
					deflab($<intval>$ = getlab());
					break;
				case LL_DUP:	/* dup. test at top & bottom */
					if (!$5)
						flostat |= FLOOP;
					else if ($5->in.op == ICON &&
						$5->tn.lval != 0)
					{
						fl