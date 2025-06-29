
# line 3 "../common/cgram.y"
static	char	SCCSID[] = "@(#) cgram.y:	2.1 83/07/08";
# define NAME 2
# define STRING 3
# define ICON 4
# define FCON 5
# define PLUS 6
# define MINUS 8
# define MUL 11
# define AND 14
# define OR 17
# define ER 19
# define QUEST 21
# define COLON 22
# define ANDAND 23
# define OROR 24
# define ASOP 25
# define RELOP 26
# define EQUOP 27
# define DIVOP 28
# define SHIFTOP 29
# define INCOP 30
# define UNOP 31
# define STROP 32
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
# define LP 50
# define RP 51
# define LC 52
# define RC 53
# define LB 54
# define RB 55
# define CM 56
# define SM 57
# define ASSIGN 58
# define ASM 59

# line 112 "../common/cgram.y"
# include "mfile1.h"
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;

# line 131 "../common/cgram.y"
	static sulvl=0;	     /* counts depth of nesting of structure and
			     ** union declarations.  Used to index into
			     ** "is_struc". */
	static char is_struc[16]; /* nonzero if current STRUCT is 'struct'
				 ** and not 'union'.  Allows 15 deep nesting */
	static int fake = 0;
	static char fakename[NCHNAM+1];
# define YYERRCODE 256

# line 854 "../common/cgram.y"


NODE *
mkty( t, d, s ) unsigned t; {
	return( block( TYPE, NIL, NIL, t, d, s ) );
	}

NODE *
bdty( op, p, v ) NODE *p; {
	register NODE *q;

	q = block( op, p, NIL, INT, 0, INT );

	switch( op ){

	case UNARY MUL:
	case UNARY CALL:
		break;

	case LB:
		q->in.right = bcon(v);
		break;

	case NAME:
		q->tn.rval = v;
		break;

	default:
		cerror( "bad bdty" );
		}

	return( q );
	}

dstash( n ){ /* put n into the dimension table */
	if( curdim >= DIMTABSZ-1 ){
		cerror( "dimension table overflow");
		}
	dimtab[ curdim++ ] = n;
	}

savebc() {
	if( psavbc > & asavbc[BCSZ-4 ] ){
		cerror( "whiles, fors, etc. too deeply nested");
		}
	*psavbc++ = brklab;
	*psavbc++ = contlab;
	*psavbc++ = flostat;
	*psavbc++ = swx;
	flostat = 0;
	}

resetbc(mask){

	swx = *--psavbc;
	flostat = *--psavbc | (flostat&mask);
	contlab = *--psavbc;
	brklab = *--psavbc;

	}

addcase(p) NODE *p; { /* add case to switch */

	p = optim( p );  /* change enum to ints */
	if( p->in.op != ICON ){
		uerror( "non-constant case expression");
		return;
		}
	if( swp == swtab ){
		uerror( "case not in switch");
		return;
		}
	if( swp >= &swtab[SWITSZ] ){
		cerror( "switch table overflow");
		}
	swp->sval = p->tn.lval;
	deflab( swp->slab = getlab() );
	++swp;
	tfree(p);
	}

adddef(){ /* add default case to switch */
	if( swtab[swx].slab >= 0 ){
		uerror( "duplicate default in switch");
		return;
		}
	if( swp == swtab ){
		uerror( "default not inside switch");
		return;
		}
	deflab( swtab[swx].slab = getlab() );
	}

swstart(){
	/* begin a switch block */
	if( swp >= &swtab[SWITSZ] ){
		cerror( "switch table overflow");
		}
	swx = swp - swtab;
	swp->slab = -1;
	++swp;
	}

swend(){ /* end a switch block */

	register struct sw *swbeg, *p, *q, *r, *r1;
	CONSZ temp;
	int tempi;

	swbeg = &swtab[swx+1];

	/* sort */

	r1 = swbeg;
	r = swp-1;

	while( swbeg < r ){
		/* bubble largest to end */
		for( q=swbeg; q<r; ++q ){
			if( q->sval > (q+1)->sval ){
				/* swap */
				r1 = q+1;
				temp = q->sval;
				q->sval = r1->sval;
				r1->sval = temp;
				tempi = q->slab;
				q->slab = r1->slab;
				r1->slab = tempi;
				}
			}
		r = r1;
		r1 = swbeg;
		}

	/* it is now sorted */

	for( p = swbeg+1; p<swp; ++p ){
		if( p->sval == (p-1)->sval ){
			uerror( "duplicate case in switch, %d", tempi=p->sval );
			return;
			}
		}

	reached = 1;
	genswitch( swbeg-1, (int)(swp-swbeg) );
	swp = swbeg-1;
	}
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	2, 20,
	11, 20,
	50, 20,
	57, 20,
	-2, 0,
-1, 21,
	56, 78,
	57, 78,
	-2, 8,
-1, 26,
	58, 75,
	-2, 77,
-1, 28,
	58, 76,
	-2, 81,
-1, 34,
	52, 43,
	-2, 41,
-1, 36,
	52, 35,
	-2, 33,
-1, 58,
	53, 47,
	57, 47,
	-2, 0,
	};
# define YYNPROD 180
# define YYLAST 1221
yytabelem yyact[]={

   232,    20,   269,    11,    10,    15,   253,    90,    88,    89,
   230,    99,    81,    78,    29,    79,    29,    99,    80,    17,
    98,    48,    94,    23,    29,    23,    98,   101,   130,     5,
   196,    96,   131,    23,    83,    82,    67,    96,     7,   146,
   243,   244,   248,    26,   252,   241,   242,   249,   250,   251,
   255,   254,    84,    76,    85,    93,   109,   231,    60,   100,
    57,   245,    24,   233,    24,   100,   162,    42,    44,   111,
    47,   228,    24,     9,   234,    22,   278,   311,   132,    19,
   277,   310,    30,    38,   262,   305,   147,   151,    75,   155,
   203,   189,   103,   139,   140,   141,   142,   143,   144,    52,
    38,    37,    95,   107,   294,    54,   273,    58,   272,   264,
    92,   135,    18,   137,   155,   164,   165,   166,   168,   170,
   172,   174,   176,   177,   179,   181,   183,   184,   185,   186,
   187,   138,   156,   157,   132,   136,   200,   191,   104,   106,
   160,   199,    42,    71,    44,   102,   147,   261,    72,   288,
   222,    95,   259,   154,   113,   110,    74,    66,   188,   148,
    50,   159,   190,   161,    51,    40,    58,    50,   206,    41,
   207,    51,   208,   222,   209,   257,   210,   198,   215,   211,
   158,   212,   153,   213,    35,   193,    33,    40,    70,   300,
   132,    41,   292,   202,    11,    10,    15,    50,    69,   284,
   226,    51,   204,   198,   220,   194,   229,    73,   221,    11,
    17,    15,   301,   109,    99,   256,   197,   223,   217,   148,
   258,    29,   216,    98,   227,    17,     4,   152,   224,   225,
    23,    65,   289,   283,   221,   282,    11,    10,    15,   281,
   270,   280,   197,   152,   275,    95,   265,   266,   267,   268,
    53,   271,    17,   291,   290,   285,    10,    55,   279,   260,
   246,    32,   100,   120,   286,    97,   105,     8,    28,    24,
    28,    97,   276,    21,   117,    49,   118,    31,    28,   120,
   116,   295,   296,   270,   298,   297,   253,    90,    88,    89,
    43,    45,    81,   192,   114,    79,   116,   119,    80,    61,
   112,    36,   307,   302,    34,    68,   270,   247,   308,   108,
   133,   117,    27,   118,    83,    82,   120,    62,    46,   201,
   243,   244,   248,    91,   252,   241,   242,   249,   250,   251,
   255,   254,    84,   116,    85,    56,   109,   117,    59,   118,
   163,   245,   120,   233,   205,   122,    64,    63,   123,    39,
   124,     3,   127,   219,   125,   126,   128,   114,   121,   116,
   119,   117,     2,   118,   150,    86,   120,    12,   117,   122,
   118,    13,   123,   120,   124,     6,   127,    25,   125,   126,
   128,   114,   121,   116,   119,    14,   218,   115,    16,   129,
   116,   119,   240,   238,   239,   237,   235,   117,   236,   118,
     1,     0,   120,     0,     0,   122,   309,     0,   123,     0,
   124,   115,   127,   129,   125,   126,   128,   114,   121,   116,
   119,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   117,     0,   118,     0,     0,   120,     0,
     0,   122,   306,     0,   123,     0,   124,   115,   127,   129,
   125,   126,   128,   114,   121,   116,   119,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   117,
     0,   118,     0,     0,   120,     0,     0,   122,   304,     0,
   123,     0,   124,   115,   127,   129,   125,   126,   128,   114,
   121,   116,   119,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   117,     0,   118,
     0,     0,   120,     0,   303,   122,     0,     0,   123,   115,
   124,   129,   127,   299,   125,   126,   128,   114,   121,   116,
   119,   117,     0,   118,     0,     0,   120,     0,     0,   122,
   246,     0,   123,     0,   124,     0,   127,     0,   125,   126,
   128,   114,   121,   116,   119,     0,   117,   115,   118,   129,
     0,   120,     0,     0,   122,     0,     0,   123,     0,   124,
     0,   127,     0,   125,   126,   128,   114,   121,   116,   119,
     0,   115,   293,   129,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   117,     0,   118,     0,
     0,   120,     0,     0,   122,   287,   115,   123,   129,   124,
     0,   127,     0,   125,   126,   128,   114,   121,   116,   119,
   117,     0,   118,     0,     0,   120,     0,     0,   122,     0,
     0,   123,     0,   124,     0,   127,   214,   125,   126,   128,
   114,   121,   116,   119,     0,     0,   115,   263,   129,     0,
     0,     0,     0,     0,     0,     0,   117,     0,   118,     0,
   117,   120,   118,     0,   122,   120,     0,   123,   122,   124,
   115,   127,   129,   125,   126,   128,   114,   121,   116,   119,
   114,   121,   116,   119,     0,     0,     0,     0,     0,     0,
     0,     0,   117,     0,   118,     0,     0,   120,     0,     0,
   122,   195,     0,   123,     0,   124,   115,   127,   129,   125,
   126,   128,   114,   121,   116,   119,   117,     0,   118,     0,
     0,   120,     0,     0,   122,     0,     0,   123,     0,   124,
     0,   127,     0,   125,   126,   128,   114,   121,   116,   119,
     0,     0,   115,     0,   129,    87,    90,    88,    89,     0,
     0,    81,     0,     0,    79,     0,     0,    80,    87,    90,
    88,    89,     0,     0,    81,     0,     0,    79,   129,     0,
    80,     0,     0,    83,    82,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    83,    82,     0,     0,
     0,    84,     0,    85,     0,     0,    87,    90,    88,    89,
     0,   182,    81,     0,    84,    79,    85,     0,    80,    87,
    90,    88,    89,     0,   180,    81,     0,     0,    79,     0,
     0,    80,     0,     0,    83,    82,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    83,    82,     0,
     0,     0,    84,     0,    85,     0,     0,    87,    90,    88,
    89,     0,   178,    81,     0,    84,    79,    85,     0,    80,
    87,    90,    88,    89,     0,   175,    81,     0,     0,    79,
     0,     0,    80,     0,     0,    83,    82,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    83,    82,
     0,     0,     0,    84,     0,    85,     0,     0,    87,    90,
    88,    89,     0,   173,    81,     0,    84,    79,    85,     0,
    80,    87,    90,    88,    89,     0,   171,    81,     0,     0,
    79,     0,     0,    80,     0,     0,    83,    82,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
    82,     0,     0,     0,    84,     0,    85,     0,     0,    87,
    90,    88,    89,     0,   169,    81,     0,    84,    79,    85,
     0,    80,    87,    90,    88,    89,     0,   167,    81,     0,
     0,    79,     0,     0,    80,     0,     0,    83,    82,   117,
     0,   118,     0,     0,   120,     0,     0,   122,     0,     0,
    83,    82,   124,     0,     0,    84,     0,    85,     0,   114,
   121,   116,   119,     0,   274,     0,     0,     0,    84,     0,
    85,     0,   134,    87,    90,    88,    89,     0,     0,    81,
     0,     0,    79,     0,     0,    80,    87,    90,    88,    89,
     0,     0,    81,     0,     0,    79,     0,     0,    80,     0,
     0,    83,    82,     0,    11,     0,    15,     0,     0,     0,
     0,     0,     0,     0,    83,    82,     0,     0,     0,    84,
    17,    85,     0,     0,     0,     0,    87,    90,    88,    89,
     0,     0,    84,     0,    85,     0,    77,    87,    90,    88,
    89,     0,     0,    81,     0,     0,    79,     0,     0,    80,
    87,    90,    88,    89,    83,    82,    81,     0,     0,    79,
     0,     0,    80,     0,     0,    83,    82,     0,     0,     0,
     0,     0,    84,     0,    85,     0,     0,     0,    83,    82,
     0,     0,   117,    84,   118,    85,   149,   120,    87,    90,
    88,    89,     0,     0,    81,     0,    84,    79,    85,     0,
    80,     0,   114,   121,   116,   119,     0,   117,     0,   118,
     0,     0,   120,     0,     0,   122,    83,    82,   123,     0,
   124,     0,   127,     0,   125,   126,     0,   114,   121,   116,
   119,     0,     0,   117,    84,   118,   145,     0,   120,     0,
     0,   122,     0,     0,   123,     0,   124,     0,     0,     0,
   125,     0,     0,   114,   121,   116,   119,   117,     0,   118,
     0,     0,   120,     0,     0,   122,     0,     0,   123,     0,
   124,     0,     0,     0,     0,     0,     0,   114,   121,   116,
   119 };
yytabelem yypact[]={

 -1000,   -30, -1000, -1000, -1000,    55,    22, -1000,   176,   222,
 -1000,   228, -1000, -1000,   134,   302,   132,   299, -1000, -1000,
    44,   115, -1000,   219,   219,    19,   117,    41, -1000,   200,
 -1000,   176,   224,   176, -1000,   297, -1000, -1000, -1000, -1000,
   180,   102,   117,   115,   147,   137,    92, -1000, -1000, -1000,
   156,   101,  1024, -1000, -1000, -1000,    53, -1000,    15,    89,
 -1000,    34,    12, -1000,   161, -1000, -1000,   100,  1088, -1000,
 -1000, -1000,   298, -1000, -1000,    99,   710,   960,    81,  1088,
  1088,  1088,  1088,  1088,  1126,  1011,  1075,   193, -1000, -1000,
 -1000,   129,   176,    58, -1000,   110, -1000, -1000,   212, -1000,
   212,   127,   297, -1000, -1000,   115, -1000,     9, -1000, -1000,
 -1000,   710, -1000, -1000,  1088,  1088,   909,   896,   858,   845,
   807,  1088,   794,   756,   743,  1088,  1088,  1088,  1088,  1088,
    35, -1000,   710,   960, -1000, -1000,  1088,   291, -1000,    81,
    81,    81,    81,    81,    81,  1011,   154,   650,   192, -1000,
    85,   710, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
 -1000,    33, -1000,   203,   362,   710, -1000,  1088,   252,  1088,
   252,  1088,   305,  1088, -1000,  1088,   268,  1116,  1088,   973,
  1088,   654,  1088,  1191,  1167,   614,   710,   710,   125,   960,
    35,   331, -1000,   153,  1088, -1000,   119,   166,   192,  1088,
 -1000,    15, -1000, -1000,    14,     4,   710,   710,   710,   710,
   710,   710,   710,   710,  1088, -1000, -1000,   122, -1000,  1088,
  1064,    81,    97, -1000,    96,   119,   710, -1000, -1000,    27,
 -1000, -1000,   590,    52, -1000,   284,   284,   284,   284,  1088,
   284,    51,    49,   947,   270, -1000,    23,   284,   191,   189,
 -1000,   185,   183,   177,  1088,   242,  1141, -1000,   550, -1000,
    94,   182, -1000, -1000, -1000,   215, -1000, -1000,   210,   141,
   686, -1000, -1000, -1000, -1000,   525,    47, -1000, -1000, -1000,
  1088,  1088,  1088,  1088, -1000,   501, -1000, -1000, -1000,   138,
 -1000,   162,   284, -1000, -1000,   463,   427,    28,   391, -1000,
 -1000,  1088, -1000, -1000, -1000,  1088, -1000,   355,    24,    20,
 -1000, -1000 };
yytabelem yypgo[]={

     0,   400,    36,   398,   396,   395,   394,   393,   392,   388,
   385,   377,     0,     2,    13,    38,   375,    73,   371,   367,
    39,    30,   365,    22,   266,    43,   364,   362,   351,     1,
   349,   347,   346,    74,    55,   344,    10,   340,   267,   338,
    27,    58,   335,   323,    60,   319,   318,    75,   317,   312,
    28,    32,   310,   309,   307,   305 };
yytabelem yyr1[]={

     0,     1,     1,    27,    27,    27,    28,    28,    30,    28,
    31,    32,    32,    32,    35,    35,    37,    37,    37,    16,
    16,    15,    15,    15,    15,    15,    38,    17,    17,    17,
    17,    17,    18,    18,     9,     9,    39,    39,    41,    41,
    19,    19,    10,    10,    42,    42,    44,    44,    34,    45,
    34,    23,    23,    23,    23,    25,    25,    25,    25,    25,
    25,    24,    24,    24,    24,    24,    24,    24,    11,    46,
    46,    46,    29,    48,    29,    49,    49,    47,    47,    47,
    47,    47,    50,    50,    51,    51,    40,    40,    43,    43,
    52,    33,    53,    36,    36,    36,    36,    36,    36,    36,
    36,    36,    36,    36,    36,    36,    36,    36,    36,    36,
    36,    54,    54,    54,     7,     4,     3,     5,     6,     8,
    55,     2,    13,    13,    26,    26,    12,    12,    12,    12,
    12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
    12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
    12,    14,    14,    14,    14,    14,    14,    14,    14,    14,
    14,    14,    14,    14,    14,    14,    14,    14,    14,    14,
    20,    21,    21,    21,    21,    21,    21,    21,    22,    22 };
yytabelem yyr2[]={

     0,     4,     1,     3,     3,     5,     5,     7,     1,     9,
     5,     9,     7,     1,     4,     1,     7,     9,     0,     2,
     1,     5,     4,     3,     3,     7,     3,     2,     5,     7,
     2,     2,    11,     5,     3,     5,     2,     6,     3,     7,
    11,     5,     3,     5,     2,     6,     5,     3,     3,     1,
     9,     2,     7,     5,     3,     5,     7,     7,     9,     3,
     7,     5,     7,     7,     9,     7,     7,     5,     5,     3,
     7,     2,     2,     1,     8,     3,     2,     3,     3,     7,
    13,     2,     2,     6,     3,     9,     0,     2,     0,     2,
     3,     9,     3,     5,     5,     2,     5,     5,     5,    15,
     9,     5,     5,     5,     5,     7,     7,     2,     4,     4,
     4,     5,     7,     5,     3,     9,     7,     9,    13,     9,
     1,     5,     2,     1,     2,     7,     7,     7,     7,     7,
     7,     7,     7,     7,     7,     7,     7,     7,     7,     9,
     9,     9,     9,     9,     9,     9,     9,    11,     7,     7,
     2,     5,     5,     5,     5,     5,     5,     5,     9,     9,
     9,    13,     5,     7,     7,     3,     3,     3,     3,     7,
     5,     1,     5,    11,     5,     7,     9,     7,     5,     4 };
yytabelem yychk[]={

 -1000,    -1,   -27,   -28,   256,    59,   -16,   -15,   -38,   -17,
    34,    33,   -19,   -18,   -10,    35,    -9,    49,    57,    57,
   -29,   -24,   -47,    11,    50,   -11,   -25,   -49,   256,     2,
   -17,   -38,    33,    52,     2,    52,     2,    57,    56,   -30,
    50,    54,   -25,   -24,   -25,   -24,   -46,    51,     2,   256,
    50,    54,    58,    50,   -17,    33,   -42,   -44,   -17,   -39,
   -41,     2,   -48,   -31,   -32,    51,    55,    -2,   -55,    51,
    51,    51,    56,    51,    55,    -2,   -12,    52,   -14,    11,
    14,     8,    31,    30,    48,    50,   -22,     2,     4,     5,
     3,   -43,    57,   -34,   -23,   -25,    22,   256,    11,     2,
    50,   -40,    56,    58,   -47,   -24,   -33,   -15,   -53,    52,
    55,   -12,     2,    55,    26,    56,    28,     6,     8,    29,
    11,    27,    14,    17,    19,    23,    24,    21,    25,    58,
   -50,   -51,   -12,   -52,    52,    30,    54,    32,    50,   -14,
   -14,   -14,   -14,   -14,   -14,    50,   -20,   -12,   -17,    51,
   -26,   -12,    50,    53,   -44,    56,    22,    -2,    53,   -41,
    -2,   -34,    57,   -37,   -12,   -12,   -12,    58,   -12,    58,
   -12,    58,   -12,    58,   -12,    58,   -12,   -12,    58,   -12,
    58,   -12,    58,   -12,   -12,   -12,   -12,   -12,   -40,    56,
   -50,   -12,     2,   -20,    51,    51,   -21,    50,    11,    56,
    51,   -45,    -2,    57,   -15,   -35,   -12,   -12,   -12,   -12,
   -12,   -12,   -12,   -12,    22,    53,   -51,   -40,    55,    22,
    51,   -14,    54,    51,   -21,   -21,   -12,   -23,    57,   -29,
   -36,    53,   -12,    59,   -33,    -4,    -3,    -5,    -7,    -6,
    -8,    41,    42,    36,    37,    57,   256,   -54,    38,    43,
    44,    45,    40,     2,    47,    46,   -12,    53,   -12,    55,
    -2,    51,    57,    57,    57,   -36,   -36,   -36,   -36,   -13,
   -12,   -36,    57,    57,    57,   -12,     2,    57,    53,   -36,
    50,    50,    50,    50,    22,   -12,    22,    55,    55,    50,
    39,    43,    51,    57,    57,   -12,   -12,   -13,   -12,    22,
    51,    50,   -36,    51,    51,    57,    51,   -12,   -13,    51,
    57,    57 };
yytabelem yydef[]={

     2,    -2,     1,     3,     4,     0,     0,    19,    23,    24,
    26,    27,    30,    31,     0,    42,     0,    34,     5,     6,
     0,    -2,    72,     0,     0,     0,    -2,     0,    -2,    59,
    21,    22,    28,     0,    -2,     0,    -2,     7,    73,    13,
     0,   120,    55,    61,     0,     0,     0,    67,    69,    71,
     0,   120,     0,    68,    25,    29,    88,    44,    -2,    86,
    36,    38,     0,     9,     0,    62,    63,     0,     0,    60,
    65,    66,     0,    56,    57,     0,    79,     0,   150,     0,
     0,     0,     0,     0,     0,     0,     0,   165,   166,   167,
   168,     0,    89,    46,    48,    51,   120,    54,     0,    59,
     0,     0,    87,   120,    74,    78,    10,     0,    18,    92,
    64,   121,    70,    58,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    86,    82,    84,     0,    90,   151,     0,     0,   179,   152,
   153,   154,   155,   156,   157,     0,     0,     0,   171,   162,
     0,   124,   178,    40,    45,    49,   120,    53,    32,    37,
    39,     0,    12,    15,   126,   127,   128,     0,   129,     0,
   130,     0,   131,     0,   132,     0,   133,   134,     0,   135,
     0,   136,     0,   137,   138,     0,   148,   149,     0,    87,
    86,     0,   164,     0,     0,   169,   170,   171,   171,     0,
   163,     0,    52,    11,     0,     0,   140,   141,   142,   143,
   139,   144,   145,   146,     0,    80,    83,     0,   160,     0,
   159,   158,   120,   172,     0,   174,   125,    50,    16,     0,
    14,    91,     0,     0,    95,     0,     0,     0,     0,   123,
     0,     0,     0,     0,     0,   107,     0,     0,     0,     0,
   114,     0,     0,   165,     0,     0,   147,    85,     0,   175,
     0,   177,    17,    93,    94,    96,    97,    98,     0,     0,
   122,   101,   102,   103,   104,     0,     0,   108,   109,   110,
     0,     0,   123,     0,   111,     0,   113,   161,   176,     0,
   116,     0,     0,   105,   106,     0,     0,     0,     0,   112,
   173,     0,   100,   115,   117,   123,   119,     0,     0,     0,
   118,    99 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"NAME",	2,
	"STRING",	3,
	"ICON",	4,
	"FCON",	5,
	"PLUS",	6,
	"MINUS",	8,
	"MUL",	11,
	"AND",	14,
	"OR",	17,
	"ER",	19,
	"QUEST",	21,
	"COLON",	22,
	"ANDAND",	23,
	"OROR",	24,
	"ASOP",	25,
	"RELOP",	26,
	"EQUOP",	27,
	"DIVOP",	28,
	"SHIFTOP",	29,
	"INCOP",	30,
	"UNOP",	31,
	"STROP",	32,
	"TYPE",	33,
	"CLASS",	34,
	"STRUCT",	35,
	"RETURN",	36,
	"GOTO",	37,
	"IF",	38,
	"ELSE",	39,
	"SWITCH",	40,
	"BREAK",	41,
	"CONTINUE",	42,
	"WHILE",	43,
	"DO",	44,
	"FOR",	45,
	"DEFAULT",	46,
	"CASE",	47,
	"SIZEOF",	48,
	"ENUM",	49,
	"LP",	50,
	"RP",	51,
	"LC",	52,
	"RC",	53,
	"LB",	54,
	"RB",	55,
	"CM",	56,
	"SM",	57,
	"ASSIGN",	58,
	"ASM",	59,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"ext_def_list : ext_def_list external_def",
	"ext_def_list : /* empty */",
	"external_def : data_def",
	"external_def : error",
	"external_def : ASM SM",
	"data_def : oattributes SM",
	"data_def : oattributes init_dcl_list SM",
	"data_def : oattributes fdeclarator",
	"data_def : oattributes fdeclarator function_body",
	"function_body : arg_dcl_list compoundstmt",
	"arg_dcl_list : arg_dcl_list attributes declarator_list SM",
	"arg_dcl_list : arg_dcl_list attributes SM",
	"arg_dcl_list : /* empty */",
	"stmt_list : stmt_list statement",
	"stmt_list : /* empty */",
	"dcl_stat_list : dcl_stat_list attributes SM",
	"dcl_stat_list : dcl_stat_list attributes init_dcl_list SM",
	"dcl_stat_list : /* empty */",
	"oattributes : attributes",
	"oattributes : /* empty */",
	"attributes : class type",
	"attributes : type class",
	"attributes : class",
	"attributes : type",
	"attributes : type class type",
	"class : CLASS",
	"type : TYPE",
	"type : TYPE TYPE",
	"type : TYPE TYPE TYPE",
	"type : struct_dcl",
	"type : enum_dcl",
	"enum_dcl : enum_head LC moe_list optcomma RC",
	"enum_dcl : ENUM NAME",
	"enum_head : ENUM",
	"enum_head : ENUM NAME",
	"moe_list : moe",
	"moe_list : moe_list CM moe",
	"moe : NAME",
	"moe : NAME ASSIGN con_e",
	"struct_dcl : str_head LC type_dcl_list optsemi RC",
	"struct_dcl : STRUCT NAME",
	"str_head : STRUCT",
	"str_head : STRUCT NAME",
	"type_dcl_list : type_declaration",
	"type_dcl_list : type_dcl_list SM type_declaration",
	"type_declaration : type declarator_list",
	"type_declaration : type",
	"declarator_list : declarator",
	"declarator_list : declarator_list CM",
	"declarator_list : declarator_list CM declarator",
	"declarator : nfdeclarator",
	"declarator : nfdeclarator COLON con_e",
	"declarator : COLON con_e",
	"declarator : error",
	"nfdeclarator : MUL nfdeclarator",
	"nfdeclarator : nfdeclarator LP RP",
	"nfdeclarator : nfdeclarator LB RB",
	"nfdeclarator : nfdeclarator LB con_e RB",
	"nfdeclarator : NAME",
	"nfdeclarator : LP nfdeclarator RP",
	"fdeclarator : MUL fdeclarator",
	"fdeclarator : fdeclarator LP RP",
	"fdeclarator : fdeclarator LB RB",
	"fdeclarator : fdeclarator LB con_e RB",
	"fdeclarator : LP fdeclarator RP",
	"fdeclarator : name_lp name_list RP",
	"fdeclarator : name_lp RP",
	"name_lp : NAME LP",
	"name_list : NAME",
	"name_list : name_list CM NAME",
	"name_list : error",
	"init_dcl_list : init_declarator",
	"init_dcl_list : init_dcl_list CM",
	"init_dcl_list : init_dcl_list CM init_declarator",
	"xnfdeclarator : nfdeclarator",
	"xnfdeclarator : error",
	"init_declarator : nfdeclarator",
	"init_declarator : fdeclarator",
	"init_declarator : xnfdeclarator ASSIGN e",
	"init_declarator : xnfdeclarator ASSIGN LC init_list optcomma RC",
	"init_declarator : error",
	"init_list : initializer",
	"init_list : init_list CM initializer",
	"initializer : e",
	"initializer : ibrace init_list optcomma RC",
	"optcomma : /* empty */",
	"optcomma : CM",
	"optsemi : /* empty */",
	"optsemi : SM",
	"ibrace : LC",
	"compoundstmt : begin dcl_stat_list stmt_list RC",
	"begin : LC",
	"statement : e SM",
	"statement : ASM SM",
	"statement : compoundstmt",
	"statement : ifprefix statement",
	"statement : ifelprefix statement",
	"statement : whprefix statement",
	"statement : doprefix statement WHILE LP e RP SM",
	"statement : forprefix .e RP statement",
	"statement : switchpart statement",
	"statement : BREAK SM",
	"statement : CONTINUE SM",
	"statement : RETURN SM",
	"statement : RETURN e SM",
	"statement : GOTO NAME SM",
	"statement : SM",
	"statement : error SM",
	"statement : error RC",
	"statement : label statement",
	"label : NAME COLON",
	"label : CASE e COLON",
	"label : DEFAULT COLON",
	"doprefix : DO",
	"ifprefix : IF LP e RP",
	"ifelprefix : ifprefix statement ELSE",
	"whprefix : WHILE LP e RP",
	"forprefix : FOR LP .e SM .e SM",
	"switchpart : SWITCH LP e RP",
	"con_e : /* empty */",
	"con_e : e",
	".e : e",
	".e : /* empty */",
	"elist : e",
	"elist : elist CM e",
	"e : e RELOP e",
	"e : e CM e",
	"e : e DIVOP e",
	"e : e PLUS e",
	"e : e MINUS e",
	"e : e SHIFTOP e",
	"e : e MUL e",
	"e : e EQUOP e",
	"e : e AND e",
	"e : e OR e",
	"e : e ER e",
	"e : e ANDAND e",
	"e : e OROR e",
	"e : e MUL ASSIGN e",
	"e : e DIVOP ASSIGN e",
	"e : e PLUS ASSIGN e",
	"e : e MINUS ASSIGN e",
	"e : e SHIFTOP ASSIGN e",
	"e : e AND ASSIGN e",
	"e : e OR ASSIGN e",
	"e : e ER ASSIGN e",
	"e : e QUEST e COLON e",
	"e : e ASOP e",
	"e : e ASSIGN e",
	"e : term",
	"term : term INCOP",
	"term : MUL term",
	"term : AND term",
	"term : MINUS term",
	"term : UNOP term",
	"term : INCOP term",
	"term : SIZEOF term",
	"term : LP cast_type RP term",
	"term : SIZEOF LP cast_type RP",
	"term : term LB e RB",
	"term : term LB e COLON e RB",
	"term : funct_idn RP",
	"term : funct_idn elist RP",
	"term : term STROP NAME",
	"term : NAME",
	"term : ICON",
	"term : FCON",
	"term : STRING",
	"term : LP e RP",
	"cast_type : type null_decl",
	"null_decl : /* empty */",
	"null_decl : LP RP",
	"null_decl : LP null_decl RP LP RP",
	"null_decl : MUL null_decl",
	"null_decl : null_decl LB RB",
	"null_decl : null_decl LB con_e RB",
	"null_decl : LP null_decl RP",
	"funct_idn : NAME LP",
	"funct_idn : term LP",
};
#endif /* YYDEBUG */
#ident	"@(#)yacc:yaccpar	1.10"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 2:
# line 142 "../common/cgram.y"
{
#ifndef LINT
			bg_file();
#endif
			 ftnend();
			 } break;
case 3:
# line 150 "../common/cgram.y"
{ curclass = SNULL;  blevel = 0; } break;
case 4:
# line 152 "../common/cgram.y"
{ curclass = SNULL;  blevel = 0; } break;
case 5:
# line 154 "../common/cgram.y"
{ asmout();  curclass = SNULL; blevel = 0; } break;
case 6:
# line 157 "../common/cgram.y"
{  yypvt[-1].nodep->in.op = FREE; } break;
case 7:
# line 159 "../common/cgram.y"
{  yypvt[-2].nodep->in.op = FREE; } break;
case 8:
# line 161 "../common/cgram.y"
{   defid( tymerge(yypvt[-1].nodep,yypvt[-0].nodep),
					curclass==STATIC?STATIC:EXTDEF ); } break;
case 9:
# line 164 "../common/cgram.y"
{  
			    if( blevel ) cerror( "function level error" );
			    if( reached ) retstat |= NRETVAL; 
			    yypvt[-3].nodep->in.op = FREE;
			    ftnend();
			    } break;
case 10:
# line 173 "../common/cgram.y"
{	regvar = 0; } break;
case 11:
# line 177 "../common/cgram.y"
{ curclass = SNULL;  yypvt[-2].nodep->in.op = FREE; } break;
case 12:
# line 180 "../common/cgram.y"
{ curclass = SNULL;  yypvt[-1].nodep->in.op = FREE; } break;
case 13:
# line 181 "../common/cgram.y"
{  blevel = 1; } break;
case 15:
# line 186 "../common/cgram.y"
{  bccode();
			    locctr(PROG);
			    } break;
case 16:
# line 192 "../common/cgram.y"
{  yypvt[-1].nodep->in.op = FREE; } break;
case 17:
# line 194 "../common/cgram.y"
{  yypvt[-2].nodep->in.op = FREE; } break;
case 20:
# line 200 "../common/cgram.y"
{  yyval.nodep = mkty(INT,0,INT);  curclass = SNULL; } break;
case 21:
# line 203 "../common/cgram.y"
{  yyval.nodep = yypvt[-0].nodep; } break;
case 23:
# line 206 "../common/cgram.y"
{  yyval.nodep = mkty(INT,0,INT); } break;
case 24:
# line 208 "../common/cgram.y"
{ curclass = SNULL ; } break;
case 25:
# line 210 "../common/cgram.y"
{  yypvt[-2].nodep->in.type = types( yypvt[-2].nodep->in.type, yypvt[-0].nodep->in.type,
					UNDEF );
 			    yypvt[-0].nodep->in.op = FREE;
 			    } break;
case 26:
# line 217 "../common/cgram.y"
{  curclass = yypvt[-0].intval; } break;
case 28:
# line 222 "../common/cgram.y"
{  yypvt[-1].nodep->in.type = types( yypvt[-1].nodep->in.type, yypvt[-0].nodep->in.type, UNDEF );
			    yypvt[-0].nodep->in.op = FREE;
			    } break;
case 29:
# line 226 "../common/cgram.y"
{  yypvt[-2].nodep->in.type = types( yypvt[-2].nodep->in.type, yypvt[-1].nodep->in.type, yypvt[-0].nodep->in.type );
			    yypvt[-1].nodep->in.op = yypvt[-0].nodep->in.op = FREE;
			    } break;
case 32:
# line 234 "../common/cgram.y"
{ yyval.nodep = dclstruct(yypvt[-4].intval); } break;
case 33:
# line 236 "../common/cgram.y"
{  yyval.nodep = rstruct(yypvt[-0].intval,0);  stwart = instruct; } break;
case 34:
# line 240 "../common/cgram.y"
{  yyval.intval = bstruct(-1,0); stwart = SEENAME; } break;
case 35:
# line 242 "../common/cgram.y"
{  yyval.intval = bstruct(yypvt[-0].intval,0); stwart = SEENAME; } break;
case 38:
# line 250 "../common/cgram.y"
{  moedef( yypvt[-0].intval ); } break;
case 39:
# line 252 "../common/cgram.y"
{  strucoff = yypvt[-0].intval;  moedef( yypvt[-2].intval ); } break;
case 40:
# line 256 "../common/cgram.y"
{ yyval.nodep = dclstruct(yypvt[-4].intval);  
			  if (is_struc[sulvl]) strucdef--; /*finished struc def
						    ** ... may be nested. */
			  sulvl--;
		        } break;
case 41:
# line 262 "../common/cgram.y"
{  yyval.nodep = rstruct(yypvt[-0].intval,yypvt[-1].intval); } break;
case 42:
# line 266 "../common/cgram.y"
{  
			   sulvl++;
			   is_struc[sulvl] = (yypvt[-0].intval == INSTRUCT);
			   if (is_struc[sulvl])
			      /* struct def, not union def */
			      strucdef++;
			   yyval.intval = bstruct(-1,yypvt[-0].intval);  stwart=0; } break;
case 43:
# line 274 "../common/cgram.y"
{  
			   sulvl++;
			   is_struc[sulvl] = (yypvt[-1].intval == INSTRUCT);
			   if (is_struc[sulvl])
			      /* struct def, not union def */
			      strucdef++;
			   yyval.intval = bstruct(yypvt[-0].intval,yypvt[-1].intval);  stwart=0;  } break;
case 46:
# line 288 "../common/cgram.y"
{ curclass = SNULL;  stwart=0; yypvt[-1].nodep->in.op = FREE; } break;
case 47:
# line 290 "../common/cgram.y"
{  if( curclass != MOU ){
				curclass = SNULL;
				}
			    else {
				sprintf( fakename, "$%dFAKE", fake++ );
				defid( tymerge(yypvt[-0].nodep, bdty(NAME,NIL,
					lookup( fakename, SMOS ))), curclass );
				werror("union member must be named");
				}
			    stwart = 0;
			    yypvt[-0].nodep->in.op = FREE;
			    } break;
case 48:
# line 306 "../common/cgram.y"
{ defid( tymerge(yypvt[-1].nodep,yypvt[-0].nodep), curclass);
				stwart = instruct; } break;
case 49:
# line 308 "../common/cgram.y"
{yyval.nodep=yypvt[-2].nodep;} break;
case 50:
# line 309 "../common/cgram.y"
{ defid( tymerge(yypvt[-4].nodep,yypvt[-0].nodep), curclass);
				stwart = instruct; } break;
case 52:
# line 315 "../common/cgram.y"
{  if( !(instruct&INSTRUCT) )
				uerror( "field outside of structure" );
			    if( yypvt[-0].intval<0 || yypvt[-0].intval >= FIELD ){
				uerror( "illegal field size" );
				yypvt[-0].intval = 1;
				}
			    defid( tymerge(yypvt[-3].nodep,yypvt[-2].nodep), FIELD|yypvt[-0].intval );
			    yyval.nodep = NIL;
			    } break;
case 53:
# line 326 "../common/cgram.y"
{   if( !(instruct&INSTRUCT) )
				uerror( "field outside of structure" );
			    /* alignment or hole */
			    falloc( stab, yypvt[-0].intval, -1, yypvt[-2].nodep );
			    yyval.nodep = NIL;
			    } break;
case 54:
# line 333 "../common/cgram.y"
{  yyval.nodep = NIL; } break;
case 55:
# line 338 "../common/cgram.y"
{  umul:
				yyval.nodep = bdty( UNARY MUL, yypvt[-0].nodep, 0 ); } break;
case 56:
# line 341 "../common/cgram.y"
{  uftn:
				yyval.nodep = bdty( UNARY CALL, yypvt[-2].nodep, 0 );  } break;
case 57:
# line 344 "../common/cgram.y"
{  uary:
				yyval.nodep = bdty( LB, yypvt[-2].nodep, 0 );  } break;
case 58:
# line 347 "../common/cgram.y"
{  bary:
				if( (int)yypvt[-1].intval <= 0 )
					werror( "zero or negative subscript" );
				yyval.nodep = bdty( LB, yypvt[-3].nodep, yypvt[-1].intval );  } break;
case 59:
# line 352 "../common/cgram.y"
{  yyval.nodep = bdty( NAME, NIL, yypvt[-0].intval );  } break;
case 60:
# line 354 "../common/cgram.y"
{ yyval.nodep=yypvt[-1].nodep; } break;
case 61:
# line 357 "../common/cgram.y"
{  goto umul; } break;
case 62:
# line 359 "../common/cgram.y"
{  goto uftn; } break;
case 63:
# line 361 "../common/cgram.y"
{  goto uary; } break;
case 64:
# line 363 "../common/cgram.y"
{  goto bary; } break;
case 65:
# line 365 "../common/cgram.y"
{ yyval.nodep = yypvt[-1].nodep; } break;
case 66:
# line 367 "../common/cgram.y"
{
				if( blevel!=0 )
					uerror(
					"function declaration in bad context");
				yyval.nodep = bdty( UNARY CALL, bdty(NAME,NIL,yypvt[-2].intval), 0 );
				stwart = 0;
				} break;
case 67:
# line 375 "../common/cgram.y"
{
				yyval.nodep = bdty( UNARY CALL, bdty(NAME,NIL,yypvt[-1].intval), 0 );
				stwart = 0;
				} break;
case 68:
# line 382 "../common/cgram.y"
{
				/* turn off typedefs for argument names */
				stwart = SEENAME;
				if( stab[yypvt[-1].intval].sclass == SNULL )
				    stab[yypvt[-1].intval].stype = FTN;
				} break;
case 69:
# line 391 "../common/cgram.y"
{ ftnarg( yypvt[-0].intval );  stwart = SEENAME; } break;
case 70:
# line 393 "../common/cgram.y"
{ ftnarg( yypvt[-0].intval );  stwart = SEENAME; } break;
case 73:
# line 399 "../common/cgram.y"
{yyval.nodep=yypvt[-2].nodep;} break;
case 75:
# line 403 "../common/cgram.y"
{  defid( yypvt[-0].nodep = tymerge(yypvt[-1].nodep,yypvt[-0].nodep), curclass);
			    beginit(yypvt[-0].nodep->tn.rval);
			    } break;
case 77:
# line 410 "../common/cgram.y"
{  nidcl( tymerge(yypvt[-1].nodep,yypvt[-0].nodep) ); } break;
case 78:
# line 412 "../common/cgram.y"
{  defid( tymerge(yypvt[-1].nodep,yypvt[-0].nodep), uclass(curclass) );
			} break;
case 79:
# line 416 "../common/cgram.y"
{  doinit( yypvt[-0].nodep );
			    endinit(); } break;
case 80:
# line 419 "../common/cgram.y"
{  endinit(); } break;
case 84:
# line 429 "../common/cgram.y"
{  doinit( yypvt[-0].nodep ); } break;
case 85:
# line 431 "../common/cgram.y"
{  irbrace(); } break;
case 90:
# line 443 "../common/cgram.y"
{  ilbrace(); } break;
case 91:
# line 449 "../common/cgram.y"
{
			    clearst(blevel);
			    if (--blevel == 1)
			    {
				clearst(blevel);
				blevel = 0;
			    }
			    checkst(blevel);
			    autooff = *--psavbc;
			    regvar = *--psavbc;
#ifdef MC68K
			    eccode(blevel);	/* called at end of block */
#endif
			} break;
case 92:
# line 466 "../common/cgram.y"
{  if( blevel == 1 ) dclargs();
			    ++blevel;
			    if( psavbc > &asavbc[BCSZ-2] )
					cerror( "nesting too deep" );
			    *psavbc++ = regvar;
			    *psavbc++ = autooff;
			    } break;
case 93:
# line 476 "../common/cgram.y"
{ ecomp( yypvt[-1].nodep ); } break;
case 94:
# line 478 "../common/cgram.y"
{ asmout(); } break;
case 96:
# line 481 "../common/cgram.y"
{ deflab(yypvt[-1].intval);
			   reached = 1;
			   } break;
case 97:
# line 485 "../common/cgram.y"
{  if( yypvt[-1].intval != NOLAB ){
				deflab( yypvt[-1].intval );
				reached = 1;
				}
			    } break;
case 98:
# line 491 "../common/cgram.y"
{  branch(  contlab );
			    deflab( brklab );
			    if( (flostat&FBRK) || !(flostat&FLOOP)) reached = 1;
			    else reached = 0;
			    resetbc(0);
			    } break;
case 99:
# line 498 "../common/cgram.y"
{  deflab( contlab );
			    if( flostat & FCONT ) reached = 1;
			    ecomp( buildtree( CBRANCH,
				buildtree( NOT, yypvt[-2].nodep, NIL ), bcon( yypvt[-6].intval ) ) );
			    deflab( brklab );
			    reached = 1;
			    resetbc(0);
			    } break;
case 100:
# line 507 "../common/cgram.y"
{   deflab( contlab );
			    if( flostat&FCONT ) reached = 1;
			    if( yypvt[-2].nodep ) ecomp( yypvt[-2].nodep );
			    branch( yypvt[-3].intval );
			    deflab( brklab );
			    if( (flostat&FBRK) || !(flostat&FLOOP) ) reached=1;
			    else reached=0;
			    resetbc(0);
			    } break;
case 101:
# line 517 "../common/cgram.y"
{   if( reached ) branch( brklab );
			    deflab( yypvt[-1].intval );
			    swend();
			    deflab(brklab);
			    if( (flostat&FBRK) || !(flostat&FDEF) ) reached=1;
			    resetbc(FCONT);
			    } break;
case 102:
# line 525 "../common/cgram.y"
{  if( brklab == NOLAB ) uerror( "illegal break");
			    else if(reached) branch( brklab );
			    flostat |= FBRK;
			    if( brkflag ) goto rch;
			    reached = 0;
			    } break;
case 103:
# line 532 "../common/cgram.y"
{   if( contlab == NOLAB ) uerror( "illegal continue");
			    else branch( contlab );
			    flostat |= FCONT;
			    goto rch;
			    } break;
case 104:
# line 538 "../common/cgram.y"
{   retstat |= NRETVAL;
			    branch( retlab );
			rch:
			    if( !reached ) werror( "statement not reached");
			    reached = 0;
			    } break;
case 105:
# line 545 "../common/cgram.y"
{   register NODE *temp;
			    TWORD indtype();
			    idname = curftn;
			    temp = buildtree( NAME, NIL, NIL );
			    if(temp->in.type == (FTN|VOID))
				uerror(
				"void function %s cannot return value",
					stab[idname].sname);
			    temp->in.type = DECREF( temp->in.type );
			    temp->in.type = indtype( temp->in.type );
			    temp->tn.op = RNODE;  /* node for return */
			    temp = buildtree( ASSIGN, temp, yypvt[-1].nodep );
			    ecomp( temp );
			    retstat |= RETVAL;
			    branch( retlab );
			    reached = 0;
			    } break;
case 106:
# line 563 "../common/cgram.y"
{  register NODE *q;
			    q = block( FREE, NIL, NIL, INT|ARY, 0, INT );
			    q->tn.rval = idname = yypvt[-1].intval;
			    defid( q, ULABEL );
			    stab[idname].suse = -lineno;
			    branch( stab[idname].offset );
			    goto rch;
			    } break;
case 111:
# line 577 "../common/cgram.y"
{  register NODE *q;
			    q = block( FREE, NIL, NIL, INT|ARY, 0, LABEL );
			    q->tn.rval = yypvt[-1].intval;
			    defid( q, LABEL );
			    reached = 1;
			    } break;
case 112:
# line 584 "../common/cgram.y"
{  addcase(yypvt[-1].nodep);
			    reached = 1;
			    } break;
case 113:
# line 588 "../common/cgram.y"
{  reached = 1;
			    adddef();
			    flostat |= FDEF;
			    } break;
case 114:
# line 594 "../common/cgram.y"
{  savebc();
			    if( !reached ) werror( "loop not entered at top");
			    brklab = getlab();
			    contlab = getlab();
			    deflab( yyval.intval = getlab() );
			    reached = 1;
			    } break;
case 115:
# line 603 "../common/cgram.y"
{  ecomp( buildtree( CBRANCH, yypvt[-1].nodep, bcon( yyval.intval=getlab()) ) ) ;
			    reached = 1;
			    } break;
case 116:
# line 608 "../common/cgram.y"
{  if( reached ) branch( yyval.intval = getlab() );
			    else yyval.intval = NOLAB;
			    deflab( yypvt[-2].intval );
			    reached = 1;
			    } break;
case 117:
# line 616 "../common/cgram.y"
{  savebc();
			    if( !reached ) werror( "loop not entered at top");
			    if( yypvt[-1].nodep->in.op == ICON && yypvt[-1].nodep->tn.lval != 0 ) flostat = FLOOP;
			    deflab( contlab = getlab() );
			    reached = 1;
			    brklab = getlab();
			    if( flostat == FLOOP ) tfree( yypvt[-1].nodep );
			    else ecomp( buildtree( CBRANCH, yypvt[-1].nodep, bcon( brklab) ) );
			    } break;
case 118:
# line 627 "../common/cgram.y"
{  if( yypvt[-3].nodep ) ecomp( yypvt[-3].nodep );
			    else if( !reached ) werror( "loop not entered at top");
			    savebc();
			    contlab = getlab();
			    brklab = getlab();
			    deflab( yyval.intval = getlab() );
			    reached = 1;
			    if( yypvt[-1].nodep )
				ecomp( buildtree( CBRANCH, yypvt[-1].nodep, bcon( brklab) ));
			    else flostat |= FLOOP;
			    } break;
case 119:
# line 640 "../common/cgram.y"
{   register NODE *temp;
			    savebc();
			    temp = block( SNODE, NIL, NIL, INT, 0, INT );
			    temp = buildtree( ASSIGN, temp, yypvt[-1].nodep );
			    brklab = getlab();
			    ecomp( temp );
			    branch( yyval.intval = getlab() );
			    swstart();
			    reached = 0;
			    } break;
case 120:
# line 652 "../common/cgram.y"
{ yyval.intval=instruct; stwart=instruct=0; } break;
case 121:
# line 654 "../common/cgram.y"
{  yyval.intval = icons( yypvt[-0].nodep );  instruct=yypvt[-1].intval; } break;
case 123:
# line 658 "../common/cgram.y"
{ yyval.nodep=0; } break;
case 125:
# line 663 "../common/cgram.y"
{  goto bop; } break;
case 126:
# line 667 "../common/cgram.y"
{
			preconf:
			    if( yychar==RELOP || yychar==EQUOP || yychar==AND
					|| yychar==OR || yychar==ER ){
			precplaint:
				if( hflag ) werror(
				"precedence confusion possible: parenthesize!"
					);
				}
			bop:
			    yyval.nodep = buildtree( yypvt[-1].intval, yypvt[-2].nodep, yypvt[-0].nodep );
			    } break;
case 127:
# line 680 "../common/cgram.y"
{  yypvt[-1].intval = COMOP;
			    goto bop;
			    } break;
case 128:
# line 684 "../common/cgram.y"
{  goto bop; } break;
case 129:
# line 686 "../common/cgram.y"
{  if(yychar==SHIFTOP) goto precplaint; else goto bop; } break;
case 130:
# line 688 "../common/cgram.y"
{  if(yychar==SHIFTOP ) goto precplaint; else goto bop; } break;
case 131:
# line 690 "../common/cgram.y"
{  if(yychar==PLUS||yychar==MINUS) goto precplaint; else goto bop; } break;
case 132:
# line 692 "../common/cgram.y"
{  goto bop; } break;
case 133:
# line 694 "../common/cgram.y"
{  goto preconf; } break;
case 134:
# line 696 "../common/cgram.y"
{  if( yychar==RELOP||yychar==EQUOP ) goto preconf;  else goto bop; } break;
case 135:
# line 698 "../common/cgram.y"
{  if(yychar==RELOP||yychar==EQUOP) goto preconf; else goto bop; } break;
case 136:
# line 700 "../common/cgram.y"
{  if(yychar==RELOP||yychar==EQUOP) goto preconf; else goto bop; } break;
case 137:
# line 702 "../common/cgram.y"
{  goto bop; } break;
case 138:
# line 704 "../common/cgram.y"
{  goto bop; } break;
case 139:
# line 706 "../common/cgram.y"
{  abop:
				yyval.nodep = buildtree( ASG yypvt[-2].intval, yypvt[-3].nodep, yypvt[-0].nodep );
				} break;
case 140:
# line 710 "../common/cgram.y"
{  goto abop; } break;
case 141:
# line 712 "../common/cgram.y"
{  goto abop; } break;
case 142:
# line 714 "../common/cgram.y"
{  goto abop; } break;
case 143:
# line 716 "../common/cgram.y"
{  goto abop; } break;
case 144:
# line 718 "../common/cgram.y"
{  goto abop; } break;
case 145:
# line 720 "../common/cgram.y"
{  goto abop; } break;
case 146:
# line 722 "../common/cgram.y"
{  goto abop; } break;
case 147:
# line 724 "../common/cgram.y"
{  yyval.nodep=buildtree(QUEST, yypvt[-4].nodep, buildtree( COLON, yypvt[-2].nodep, yypvt[-0].nodep ) );
			    } break;
case 148:
# line 727 "../common/cgram.y"
{  werror( "old-fashioned assignment operator" );
				goto bop; } break;
case 149:
# line 730 "../common/cgram.y"
{  goto bop; } break;
case 151:
# line 735 "../common/cgram.y"
{  yyval.nodep = buildtree( yypvt[-0].intval, yypvt[-1].nodep, bcon(1) ); } break;
case 152:
# line 737 "../common/cgram.y"
{ ubop:
			    yyval.nodep = buildtree( UNARY yypvt[-1].intval, yypvt[-0].nodep, NIL );
			    } break;
case 153:
# line 741 "../common/cgram.y"
{  if( ISFTN(yypvt[-0].nodep->in.type) || ISARY(yypvt[-0].nodep->in.type) ){
				werror( "& before array or function: ignored" );
				yyval.nodep = yypvt[-0].nodep;
				}
			    else goto ubop;
			    } break;
case 154:
# line 748 "../common/cgram.y"
{  goto ubop; } break;
case 155:
# line 750 "../common/cgram.y"
{
			    yyval.nodep = buildtree( yypvt[-1].intval, yypvt[-0].nodep, NIL );
			    } break;
case 156:
# line 754 "../common/cgram.y"
{  yyval.nodep = buildtree( yypvt[-1].intval==INCR ? ASG PLUS : ASG MINUS,
						yypvt[-0].nodep,
						bcon(1)  );
			    } break;
case 157:
# line 759 "../common/cgram.y"
{  yyval.nodep = doszof( yypvt[-0].nodep ); } break;
case 158:
# line 761 "../common/cgram.y"
{  yyval.nodep = buildtree( CAST, yypvt[-2].nodep, yypvt[-0].nodep );
			    yyval.nodep->in.left->in.op = FREE;
			    yyval.nodep->in.op = FREE;
			    yyval.nodep = yyval.nodep->in.right;
			    } break;
case 159:
# line 767 "../common/cgram.y"
{  yyval.nodep = doszof( yypvt[-1].nodep ); } break;
case 160:
# line 769 "../common/cgram.y"
{  yyval.nodep = buildtree( LB, yypvt[-3].nodep, yypvt[-1].nodep ); } break;
case 161:
# line 771 "../common/cgram.y"
{  yyval.nodep = xicolon( yypvt[-5].nodep, yypvt[-3].nodep, yypvt[-1].nodep ); } break;
case 162:
# line 773 "../common/cgram.y"
{  yyval.nodep=buildtree(UNARY CALL,yypvt[-1].nodep,NIL); } break;
case 163:
# line 775 "../common/cgram.y"
{  yyval.nodep=buildtree(CALL,yypvt[-2].nodep,yypvt[-1].nodep); } break;
case 164:
# line 777 "../common/cgram.y"
{  if( yypvt[-1].intval == DOT ){
				if( notlval( yypvt[-2].nodep ) )uerror(
				       "structure reference must be addressable"
					);
				yypvt[-2].nodep = buildtree( UNARY AND, yypvt[-2].nodep, NIL );
				}
			    idname = yypvt[-0].intval;
			    yyval.nodep = buildtree( STREF, yypvt[-2].nodep,
					buildtree( NAME, NIL, NIL ) );
			    } break;
case 165:
# line 788 "../common/cgram.y"
{  idname = yypvt[-0].intval;
			    /* recognize identifiers in initializations */
			    if( blevel==0 && stab[idname].stype == UNDEF ) {
				register NODE *q;
				werror( "undeclared initializer name %s",
					stab[idname].sname );
				q = block( FREE, NIL, NIL, INT, 0, INT );
				q->tn.rval = idname;
				defid( q, EXTERN );
				}
			    yyval.nodep=buildtree(NAME,NIL,NIL);
			    stab[yypvt[-0].intval].suse = -lineno;
			} break;
case 166:
# line 802 "../common/cgram.y"
{  yyval.nodep=bcon(0);
			    yyval.nodep->tn.lval = lastcon;
			    yyval.nodep->tn.rval = NONAME;
			    if( yypvt[-0].intval ) yyval.nodep->fn.csiz = yyval.nodep->in.type = ctype(LONG);
			    } break;
case 167:
# line 808 "../common/cgram.y"
{  yyval.nodep=buildtree(FCON,NIL,NIL);
			    yyval.nodep->fpn.dval = dcon;
			    } break;
case 168:
# line 812 "../common/cgram.y"
{  yyval.nodep = getstr(); /* get string contents */ } break;
case 169:
# line 814 "../common/cgram.y"
{ yyval.nodep=yypvt[-1].nodep; } break;
case 170:
# line 818 "../common/cgram.y"
{
			yyval.nodep = tymerge( yypvt[-1].nodep, yypvt[-0].nodep );
			yyval.nodep->in.op = NAME;
			yypvt[-1].nodep->in.op = FREE;
			} break;
case 171:
# line 826 "../common/cgram.y"
{ yyval.nodep = bdty( NAME, NIL, -1 ); } break;
case 172:
# line 828 "../common/cgram.y"
{ yyval.nodep = bdty( UNARY CALL, bdty(NAME,NIL,-1),0); } break;
case 173:
# line 830 "../common/cgram.y"
{  yyval.nodep = bdty( UNARY CALL, yypvt[-3].nodep, 0 ); } break;
case 174:
# line 832 "../common/cgram.y"
{  goto umul; } break;
case 175:
# line 834 "../common/cgram.y"
{  goto uary; } break;
case 176:
# line 836 "../common/cgram.y"
{  goto bary;  } break;
case 177:
# line 838 "../common/cgram.y"
{ yyval.nodep = yypvt[-1].nodep; } break;
case 178:
# line 842 "../common/cgram.y"
{  if( stab[yypvt[-1].intval].stype == UNDEF ){
				register NODE *q;
				q = block( FREE, NIL, NIL, FTN|INT, 0, INT );
				q->tn.rval = yypvt[-1].intval;
				defid( q, EXTERN );
				}
			    idname = yypvt[-1].intval;
			    yyval.nodep=buildtree(NAME,NIL,NIL);
			    stab[idname].suse = -lineno;
			} break;
	}
	goto yystack;		/* reset registers in driver code */
}
