/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/

/*	@(#)manifest.h	7.3		*/
/*
static	char	SCCSID[] = "@(#) manifest.h:	2.1 83/07/08";
*/
# include <stdio.h>

	/* macro definitions for common cases of type collapsing */
# ifdef NOSHORT
# define SZSHORT SZINT
# define ALSHORT ALINT
# endif

# ifdef NOLONG
# define SZLONG SZINT
# define ALLONG ALINT
# endif

# ifdef NOFLOAT
# define SZFLOAT SZLONG
# define SZDOUBLE SZLONG
# define ALFLOAT ALLONG
# define ALDOUBLE ALLONG
# endif

# ifdef ONEFLOAT
# define SZFLOAT SZDOUBLE
# define ALFLOAT ALDOUBLE
# endif

/* define default assembly language comment starter */

# ifndef COMMENTSTR
# define COMMENTSTR	"#"
#endif

/*	manifest constant file for the lex/yacc interface */

# define ERROR 1
# define NAME 2
# define STRING 3
# define ICON 4
# define FCON 5
# define PLUS 6
# define MINUS 8
# define MUL 11
# define STAR (UNARY MUL)
# define AND 14
# define OR 17
# define ER 19
# define QUEST 21
# define COLON 22
# define ANDAND 23
# define OROR 24

/*	special interfaces for yacc alone */
/*	These serve as abbreviations of 2 or more ops:
	ASOP	=, = ops
	RELOP	LE,LT,GE,GT
	EQUOP	EQ,NE
	DIVOP	DIV,MOD
	SHIFTOP	LS,RS
	ICOP	INCR,DECR
	UNOP	NOT,COMPL
	STROP	DOT,STREF

	*/
# define ASOP 25
# define RELOP 26
# define EQUOP 27
# define DIVOP 28
# define SHIFTOP 29
# define INCOP 30
# define UNOP 31
# define STROP 32

/*	reserved words, etc */
# define TYPE 33
# define CLASS 34
# define STRUCT 35
# define RETURN 36
# define GOTO 37
# define IF 38
# define ELSE 39
# define SWITCH 40
# define BREAK 41
# define CONTINUE 42
# define WHILE 43
# define DO 44
# define FOR 45
# define DEFAULT 46
# define CASE 47
# define SIZEOF 48
# define ENUM 49

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

# define LP 50
# define RP 51
# define LC 52
# define RC 53
# define LB 54
# define RB 55
# define CM 56
# define SM 57
# define ASSIGN 58
	/* ASM returned only by yylex, and totally eaten by yyparse */
# define ASM 59

/*	END OF YACC */

/*	left over tree building operators */
# define COMOP 59
# define DIV 60
# define MOD 62
# define LS 64
# define RS 66
# define DOT 68
# define STREF 69
# define CALL 70
# define FORTCALL 73
# define NOT 76
# define COMPL 77
# define INCR 78
# define DECR 79
# define EQ 80
# define NE 81
# define LE 82
# define LT 83
# define GE 84
# define GT 85
# define ULE 86
# define ULT 87
# define UGE 88
# define UGT 89
# define SETBIT 90
# define TESTBIT 91
# define RESETBIT 92
# define ARS 93
# define REG 94
# define TEMP 95
# define CCODES 96
# define FREE 97
# define STASG 98
# define STARG 99
# define STCALL 100

/*	some conversion operators */
# define FLD 103
# define CONV 104
# define PMUL 105
# define PDIV 106

/*	special node operators, used for special contexts */
/* # define FORCE 107 */
# define GENLAB 108
# define CBRANCH 109
# define GENBR 110
# define CMP 111
# define GENUBR 112
# define INIT 113
# define CAST 114
# define FUNARG 115
# define VAUTO 116
# define VPARAM 117
# define RNODE 118
# define SNODE 119
# define QNODE 120
/*	a whole bunch of ops, done with unary; I don't need to tackle prec */
# define UOP0  121
# define UOP1  122
# define UOP2  123
# define UOP3  124
# define UOP4  125
# define UOP5  126
# define UOP6  127
# define UOP7  128
# define UOP8  129
# define UOP9  130
#ifdef ITREES
/*	pcc2 ops */
# define PROLOG	131
# define EPILOG	132
# define SWBEG	133
# define SWCASE	134
# define SWDEF	135
# define SWEND	136
/*	ops used in optimizer */
#ifdef HLOPTIM
# define EXPRNO 137   /* beginning of an expression */
# define SBLOCK	138   /* start of a block */
# define EBLOCK 139   /* end of a block */
# define ASMNO  140   /* asm statement */
/*	op used in second pass */
# define MANY 141
# else
# define MANY 137
#endif
#else
/*	ops used in optimizer */
#ifdef HLOPTIM
# define EXPRNO 131   /* beginning of an expression */
# define SBLOCK	132   /* start of a block */
# define EBLOCK 133   /* end of a block */
# define ASMNO  134   /* asm statement */
/*	op used in second pass */
# define MANY 135
# else 
# define MANY 131
#endif
#endif

/*	node types */
# define LTYPE 02
# define UTYPE 04
# define BITYPE 010

	/* DSIZE is the size of the dope array */
# define DSIZE MANY+1

/*	type names, used in symbol table building */
# define TNULL 0
# define FARG 1
# define CHAR 2
# define SHORT 3
# define INT 4
# define LONG 5
# define FLOAT 6
# define DOUBLE 7
# define STRTY 8
# define UNIONTY 9
# define ENUMTY 10
# define MOETY 11
# define UCHAR 12
# define USHORT 13
# define UNSIGNED 14
# define ULONG 15
# define VOID 16
# define UNDEF 17

# define ASG 1+
# define UNARY 2+
# define NOASG (-1)+
# define NOUNARY (-2)+

/*	various flags */
# define NOLAB (-1)

/* type modifiers */

# define PTR  040
# define FTN  0100
# define ARY  0140

/* type packing constants */

# define MTMASK 03
# define BTMASK 037
# define BTSHIFT 5 
# define TSHIFT 2
# define TMASK (MTMASK<<BTSHIFT)
# define TMASK1 (MTMASK<<(BTSHIFT+TSHIFT))
# define TMASK2  (TMASK||MTMASK)

/*	macros	*/

# ifndef BITMASK
	/* beware 1's complement */
# define BITMASK(n) (((n)==SZLONG)?-1L:((1L<<(n))-1))
# endif
# define ONEBIT(n) (1L<<(n))
# define MODTYPE(x,y) x = (x&(~BTMASK))|y  /* set basic type of x to y */
# define BTYPE(x)  (x&BTMASK)   /* basic type of x */
# define ISUNSIGNED(x) ((x)<=ULONG&&(x)>=UCHAR)
# define UNSIGNABLE(x) ((x)<=LONG&&(x)>=CHAR)
# define ENUNSIGN(x) ((x)+(UNSIGNED-INT))
# define DEUNSIGN(x) ((x)+(INT-UNSIGNED))
# define ISPTR(x) ((x&TMASK)==PTR)
# define ISFTN(x)  ((x&TMASK)==FTN)  /* is x a function type */
# define ISARY(x)   ((x&TMASK)==ARY)   /* is x an array type */
# define INCREF(x) (((x&~BTMASK)<<TSHIFT)|PTR|(x&BTMASK))
# define DECREF(x) (((x>>TSHIFT)&~BTMASK)|(x&BTMASK))
# define SETOFF(x,y)   if( (x)%(y) != 0 ) x = ( ((x)/(y) + 1) * (y))
		/* advance x to a multiple of y */
# define NOFIT(x,y,z)   ( ((x)%(z) + (y)) > (z) )
	/* can y bits be added to x without overflowing z */
	/* pack and unpack field descriptors (size and offset) */
# define PKFIELD(s,o) (((o)<<6)|(s))
# define UPKFSZ(v)  ((v)&077)
# define UPKFOFF(v) ((v)>>6)

/*	operator information */

# define TYFLG 016
# define ASGFLG 01
# define LOGFLG 020

# define SIMPFLG 040
# define COMMFLG 0100
# define DIVFLG 0200
# define FLOFLG 0400
# define LTYFLG 01000
# define CALLFLG 02000
# define MULFLG 04000
# define SHFFLG 010000
# define ASGOPFLG 020000

# define SPFLG 040000

#define optype(o) (dope[o]&TYFLG)
#define asgop(o) (dope[o]&ASGFLG)
#define asgbinop(o) (dope[o]&ASGOPFLG)
#define logop(o) (dope[o]&LOGFLG)
#define callop(o) (dope[o]&CALLFLG)

/*	table sizes	*/

# define BCSZ 100 /* size of the table to save break and continue labels */
# define MAXNEST BCSZ	/* maximum nesting depth for scopestack() */
# ifndef SYMTSZ
# define SYMTSZ 750 /* size of the symbol table */
# endif
# ifdef m68k
			/* This was 1600 previously.  9/9/86 MCD -wjc */
# define DIMTABSZ 4000 /* size of the dimension/size table */
# else
# define DIMTABSZ 800 /* size of the dimension/size table */
# endif
# define PARAMSZ 150 /* size of the parameter stack */
# define ARGSZ 50 /* size of the argument stack */
# ifndef TREESZ
# ifndef HLOPTIM
# ifndef FORT
# define TREESZ 350 /* space for building parse tree */
# else
# define TREESZ 1000
# endif
# else
# define TREESZ 5000 /* parse tree space for the optimizer */
# endif
# endif
# define SWITSZ 1000 /* size of switch table * upped from 250 3/19/86 -wjc */

	char		*hash();
	char		*savestr();
	char		*tstr();
	extern int	tstrused;
	extern char	*tstrbuf[];
	extern char	**curtstr;
#define freestr()	( curtstr = tstrbuf, tstrused = 0 )

#	define NCHNAM 8  /* number of characters in a truncated name */

/*	common defined variables */

extern int nerrors;  /* number of errors seen so far */

typedef union ndu NODE;
typedef unsigned int TWORD;
typedef long CONSZ;  /* size in which constants are converted */

	/* default is byte addressing */
	/* BITOOR(x) converts bit width x into address offset */
# ifndef BITOOR
# define BITOOR(x) ((x)/SZCHAR)
# endif

# ifdef SDB
# define STABS
# endif

# define NIL (NODE *)0

extern int dope[];  /* a vector containing operator information */
extern char *opst[];  /* a vector containing names for ops */

# define NCOSTS (NRGS+4)

#ifdef HLOPTIM

typedef int VNA;

typedef struct Tbasicblock BB;
typedef struct Tbasicblock *BBPTR;
typedef struct Tbblink BBLINK;

struct Tbblink{		/* link between basic blocks */
   BBPTR bbptr;  	/* pointer to a basic block */
   BBLINK *next;	/* pointer to next element in list */
   };

struct Tbasicblock {	/* basic block structure */
   NODE *codetree;	/* pointer to code tree associated 
			** with basic block */
   BBLINK *succs;	/* pointer to the list of basic blocks
			** which are successors to the current 
			** basic block */
   BBLINK *preds;	/* pointer to the list of basic blocks 
			** which are predecessors to the current 
			** basic block */
   BBPTR next;		/* pointer to the next physical basic block */
   BBLINK *dom;		/* pointer to the list of basic blocks 
			** which this basic block dominates */
   VNA *vnaptr;		/* pointer to value number array 
			** associated with this basic block */
   BBPTR header;	/* pointer to header of containing 
			** loop, if any */
   int dfn;		/* depth first number of basic block */
   char flags;		/* attributes of basic block - header of 
			** a loop, member of a loop, exit from 
			** a loop, ... */
   char weight;		/* loop depth */
   int id;		/* identification number - order in which created
			** used for printing out the flow graph */
   };


struct Tlabel {		/* entry in list of basicblocks */
   int number;		/* number of the label */
   BBPTR bb;		/* pointer to basic block defined by 
			** this label.   NOTE - if NIL then 
			** block has not been seen */
   int references;	/* number of times it was referenced. 
			** If 0 then basic block is dead unless 
			** it has an immediate predecessor. */
   BBLINK list;		/* until basic block is defined this 
			** list will contain the basic blocks 
			** which reference it. */
   };

#endif /* HLOPTIM */

union flt_dbl
{
	float  f;
	double d;
	struct { long l,l2;	 } lg;
	struct { short slo, shi; } sh;
};

typedef union flt_dbl FLTDBL;

	/* in one-pass operation, define the tree nodes */

union ndu {

	struct {
		int op;
		int goal;
		TWORD type;
		int cst[NCOSTS];
		char * name;
		char pad[NCHNAM-sizeof(char *)];	/* padding hack! */
		NODE *left;
		NODE *right;
	}in;	/* interior node */
	
	struct {
		int op;
		int goal;
		TWORD type;
		int cst[NCOSTS];
		char * name;
		char pad[NCHNAM-sizeof(char *)];	/* padding hack! */
		CONSZ lval;
		int rval;
	}tn;	/* terminal node */
	
	struct {
		int op;
		int goal;
		TWORD type;
		int cst[NCOSTS];
		char * name;
		char pad[NCHNAM-sizeof(char *)];	/* padding hack! */
		double dval;
	}fp;	/* back end FCON node */

	struct {
		int op;
		int goal;
		TWORD type;
		int cst[NCOSTS];
		int label;  /* for use with branching */
		int lop;  /* the opcode being branched on */
	}bn;	/* branch node */

	struct {
		int op;
		int goal;
		TWORD type;
		int cst[NCOSTS];
		int stsize;  /* sizes of structure objects */
		short stalign;  /* alignment of structure objects */
		short argsize;  /* size of argument list for call */
	}stn;	/* structure node */

	struct {
		int op;
		int goal;
		TWORD type;
		int cdim;
		int csiz;
	}fn;	/* front node */
	
	struct {
		/* this structure is used when a floating point constant
		   is being computed */
		int op;
		int goal;
		TWORD type;
		int cdim;
		int csiz;
		double dval;
	}fpn;	/* floating point node */

#ifdef HLOPTIM

	struct {
		/* this structure is used by the optimizer */
		int op;		/* operator */	
		NODE *parent;	/* pointer to parent of current node */
		TWORD type;	/* type of node */
		int cdim;
		int csiz;
		double dval;	/* floating point constant */
		int sourceno;	/* source number for node */
		ENTRY *valueno;	/* value number for node */
		char *name;	/* name of external or label */
		int *udduptr;	/* pointer to UD/DU information - not used */
		NODE *left;	/* pointer to left child */
		NODE *right;	/* pointer to right child */
		char flags;	/* set of attributes */
	} opn;	/* optimizer node */ 

	struct {
		/* used by an EXPRNO or SBLOCK node in the optimizer */
		int op;		/* operator */
		BBPTR bb; 	/* pointer to containing basic block */
		char *astring;	/* save the asm string */
		int filler1;
		int filler2;
		int filler3; /* fillers keep the node fields aligned */
		int filler4;
		int filler5;
		int filler6;
		NODE *pred;	/* pointer to predecessor */
		NODE *succ;	/* pointer to successor */
		NODE *left;	/* pointer to expression for EXPRNO */
		int rval;	/* line number or register mask */
	} stmt; /* this node heads a statement in the optimizer */ 

#endif /* HLOPTIM */

};

/*
 *	BEGIN: ported from VAX pcc to make two pass version
 */

# ifdef TWOPASS

# ifndef EXPR
# define EXPR '.'
# endif
# ifndef BBEG
# define BBEG '['
# endif
# ifndef BEND
# define BEND ']'
# endif

# endif

/*
 *	END: ported from VAX pcc to make two pass version
 */
