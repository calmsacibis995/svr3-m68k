/*	@(#)intr.c	7.2		*/
#include "defs"

extern ftnint intcon[14];

#if HERE == TARGET

extern long realcon[6][2];

#else

extern double realcon[6];

#endif

#ifdef SGS2
#define TYCVREAL TYREAL  /* TYCVREAL value of 20 not recognized by SGS2 code */
			 /* generator, not sure of it's purpose in SGS1 */
#else
#define TYCVREAL 20	/* special "REAL w/conversion" type for intrinsics */
#endif

union {
	int ijunk;
	struct Intrpacked bits;
	} packed;

struct Intrbits
	{
	int intrgroup /* :3 */;
	int intrstuff /* result type or number of generics */;
	int intrno /* :7 */;
#if M68881
	int intr_881;	/* true if intrinsic can be computed by M68881 */
#endif
	};

LOCAL struct Intrblock
	{
	char intrfname[VL];
	struct Intrbits intrval;
	} intrtab[ ] =
{
"int", 		{ INTRCONV, TYLONG },
"real", 	{ INTRCONV, TYREAL },
"dble", 	{ INTRCONV, TYDREAL },
"cmplx", 	{ INTRCONV, TYCOMPLEX },
"dcmplx", 	{ INTRCONV, TYDCOMPLEX },
"ifix", 	{ INTRCONV, TYLONG },
"idint", 	{ INTRCONV, TYLONG },
"float", 	{ INTRCONV, TYREAL },
"dfloat",	{ INTRCONV, TYDREAL },
"sngl", 	{ INTRCONV, TYREAL },
"ichar", 	{ INTRCONV, TYLONG },
"iachar", 	{ INTRCONV, TYLONG },
"char", 	{ INTRCONV, TYCHAR },
"achar", 	{ INTRCONV, TYCHAR },

"max", 		{ INTRMAX, TYUNKNOWN },
"max0", 	{ INTRMAX, TYLONG },
"amax0", 	{ INTRMAX, TYREAL },
"max1", 	{ INTRMAX, TYLONG },
"amax1", 	{ INTRMAX, TYREAL },
"dmax1", 	{ INTRMAX, TYDREAL },

"and",		{ INTRBOOL, TYUNKNOWN, OPBITAND },
"or",		{ INTRBOOL, TYUNKNOWN, OPBITOR },
"xor",		{ INTRBOOL, TYUNKNOWN, OPBITXOR },
"not",		{ INTRBOOL, TYUNKNOWN, OPBITNOT },
"lshift",	{ INTRBOOL, TYUNKNOWN, OPLSHIFT },
"rshift",	{ INTRBOOL, TYUNKNOWN, OPRSHIFT },

"min", 		{ INTRMIN, TYUNKNOWN },
"min0", 	{ INTRMIN, TYLONG },
"amin0", 	{ INTRMIN, TYREAL },
"min1", 	{ INTRMIN, TYLONG },
"amin1", 	{ INTRMIN, TYREAL },
"dmin1", 	{ INTRMIN, TYDREAL },

#if M68881
"aint", 	{ INTRGEN, 2, 0, 1 },
"dint", 	{ INTRSPEC, TYDREAL, 1, 1 },

"anint", 	{ INTRGEN, 2, 2, 1 },
"dnint", 	{ INTRSPEC, TYDREAL, 3, 1 },

"nint", 	{ INTRGEN, 4, 4, 1 },
"idnint", 	{ INTRGEN, 2, 6, 1 },
#else
"aint", 	{ INTRGEN, 2, 0 },
"dint", 	{ INTRSPEC, TYDREAL, 1 },

"anint", 	{ INTRGEN, 2, 2 },
"dnint", 	{ INTRSPEC, TYDREAL, 3 },

"nint", 	{ INTRGEN, 4, 4 },
"idnint", 	{ INTRGEN, 2, 6 },
#endif


#if M68881
"abs", 		{ INTRGEN, 6, 8, 1 },
#else
"abs", 		{ INTRGEN, 6, 8 },
#endif
"iabs", 	{ INTRGEN, 2, 9 },
#if M68881
"dabs", 	{ INTRSPEC, TYDREAL, 11, 1 },
#else
"dabs", 	{ INTRSPEC, TYDREAL, 11 },
#endif
"cabs", 	{ INTRSPEC, TYCVREAL, 12 },
"zabs", 	{ INTRSPEC, TYDREAL, 13 },

"mod", 		{ INTRGEN, 4, 14 },
"amod", 	{ INTRSPEC, TYCVREAL, 16 },
"dmod", 	{ INTRSPEC, TYDREAL, 17 },

"sign", 	{ INTRGEN, 4, 18 },
"isign", 	{ INTRGEN, 2, 19 },
"dsign", 	{ INTRSPEC, TYDREAL, 21 },

"dim", 		{ INTRGEN, 4, 22 },
"idim", 	{ INTRGEN, 2, 23 },
"ddim", 	{ INTRSPEC, TYDREAL, 25 },

"dprod", 	{ INTRSPEC, TYDREAL, 26 },

"len", 		{ INTRSPEC, TYLONG, 27 },
"index", 	{ INTRSPEC, TYLONG, 29 },

"imag", 	{ INTRGEN, 2, 31 },
"aimag", 	{ INTRSPEC, TYCVREAL, 31 },
"dimag", 	{ INTRSPEC, TYDREAL, 32 },

"conjg", 	{ INTRGEN, 2, 33 },
"dconjg", 	{ INTRSPEC, TYDCOMPLEX, 34 },

#if M68881
"sqrt", 	{ INTRGEN, 4, 35, 1 },
"dsqrt", 	{ INTRSPEC, TYDREAL, 36, 1 },
#else
"sqrt", 	{ INTRGEN, 4, 35 },
"dsqrt", 	{ INTRSPEC, TYDREAL, 36 },
#endif
"csqrt", 	{ INTRSPEC, TYCOMPLEX, 37 },
"zsqrt", 	{ INTRSPEC, TYDCOMPLEX, 38 },

#if M68881
"exp", 		{ INTRGEN, 4, 39, 1 },
"dexp", 	{ INTRSPEC, TYDREAL, 40, 1 },
#else
"exp", 		{ INTRGEN, 4, 39 },
"dexp", 	{ INTRSPEC, TYDREAL, 40 },
#endif
"cexp", 	{ INTRSPEC, TYCOMPLEX, 41 },
"zexp", 	{ INTRSPEC, TYDCOMPLEX, 42 },

#if M68881
"log", 		{ INTRGEN, 4, 43, 1 },
"alog", 	{ INTRSPEC, TYCVREAL, 43, 1 },
"dlog", 	{ INTRSPEC, TYDREAL, 44, 1 },
#else
"log", 		{ INTRGEN, 4, 43 },
"alog", 	{ INTRSPEC, TYCVREAL, 43 },
"dlog", 	{ INTRSPEC, TYDREAL, 44 },
#endif
"clog", 	{ INTRSPEC, TYCOMPLEX, 45 },
"zlog", 	{ INTRSPEC, TYDCOMPLEX, 46 },

#if M68881
"log10", 	{ INTRGEN, 2, 47, 1 },
"alog10", 	{ INTRSPEC, TYCVREAL, 47, 1 },
"dlog10", 	{ INTRSPEC, TYDREAL, 48, 1 },
#else
"log10", 	{ INTRGEN, 2, 47 },
"alog10", 	{ INTRSPEC, TYCVREAL, 47 },
"dlog10", 	{ INTRSPEC, TYDREAL, 48 },
#endif

#if M68881
"sin", 		{ INTRGEN, 4, 49, 1 },
"dsin", 	{ INTRSPEC, TYDREAL, 50, 1 },
#else
"sin", 		{ INTRGEN, 4, 49 },
"dsin", 	{ INTRSPEC, TYDREAL, 50 },
#endif
"csin", 	{ INTRSPEC, TYCOMPLEX, 51 },
"zsin", 	{ INTRSPEC, TYDCOMPLEX, 52 },

#if M68881
"cos", 		{ INTRGEN, 4, 53, 1 },
"dcos", 	{ INTRSPEC, TYDREAL, 54, 1 },
#else
"cos", 		{ INTRGEN, 4, 53 },
"dcos", 	{ INTRSPEC, TYDREAL, 54 },
#endif
"ccos", 	{ INTRSPEC, TYCOMPLEX, 55 },
"zcos", 	{ INTRSPEC, TYDCOMPLEX, 56 },

#if M68881
"tan", 		{ INTRGEN, 2, 57, 1 },
"dtan", 	{ INTRSPEC, TYDREAL, 58, 1 },

"asin", 	{ INTRGEN, 2, 59, 1 },
"dasin", 	{ INTRSPEC, TYDREAL, 60, 1 },

"acos", 	{ INTRGEN, 2, 61, 1 },
"dacos", 	{ INTRSPEC, TYDREAL, 62, 1 },

"atan", 	{ INTRGEN, 2, 63, 1 },
"datan", 	{ INTRSPEC, TYDREAL, 64, 1 },
#else
"tan", 		{ INTRGEN, 2, 57 },
"dtan", 	{ INTRSPEC, TYDREAL, 58 },

"asin", 	{ INTRGEN, 2, 59 },
"dasin", 	{ INTRSPEC, TYDREAL, 60 },

"acos", 	{ INTRGEN, 2, 61 },
"dacos", 	{ INTRSPEC, TYDREAL, 62 },

"atan", 	{ INTRGEN, 2, 63 },
"datan", 	{ INTRSPEC, TYDREAL, 64 },
#endif

"atan2", 	{ INTRGEN, 2, 65 },
"datan2", 	{ INTRSPEC, TYDREAL, 66 },

#if M68881
"sinh", 	{ INTRGEN, 2, 67, 1 },
"dsinh", 	{ INTRSPEC, TYDREAL, 68, 1 },

"cosh", 	{ INTRGEN, 2, 69, 1 },
"dcosh", 	{ INTRSPEC, TYDREAL, 70, 1 },

"tanh", 	{ INTRGEN, 2, 71, 1 },
"dtanh", 	{ INTRSPEC, TYDREAL, 72, 1 },
#else
"sinh", 	{ INTRGEN, 2, 67 },
"dsinh", 	{ INTRSPEC, TYDREAL, 68 },

"cosh", 	{ INTRGEN, 2, 69 },
"dcosh", 	{ INTRSPEC, TYDREAL, 70 },

"tanh", 	{ INTRGEN, 2, 71 },
"dtanh", 	{ INTRSPEC, TYDREAL, 72 },
#endif

"lge",		{ INTRSPEC, TYLOGICAL, 73},
"lgt",		{ INTRSPEC, TYLOGICAL, 75},
"lle",		{ INTRSPEC, TYLOGICAL, 77},
"llt",		{ INTRSPEC, TYLOGICAL, 79},

"epbase",	{ INTRCNST, 4, 0 },
"epprec",	{ INTRCNST, 4, 4 },
"epemin",	{ INTRCNST, 2, 8 },
"epemax",	{ INTRCNST, 2, 10 },
"eptiny",	{ INTRCNST, 2, 12 },
"ephuge",	{ INTRCNST, 4, 14 },
"epmrsp",	{ INTRCNST, 2, 18 },

"fpexpn",	{ INTRGEN, 4, 81 },
"fpabsp",	{ INTRGEN, 2, 85 },
"fprrsp",	{ INTRGEN, 2, 87 },
"fpfrac",	{ INTRGEN, 2, 89 },
"fpmake",	{ INTRGEN, 2, 91 },
"fpscal",	{ INTRGEN, 2, 93 },

"" };


LOCAL struct Specblock
	{
	char atype;
	char rtype;
	char nargs;
	char spxname[XL];
	char othername;	/* index into callbyvalue table */
	} spectab[ ] =
{
	{ TYREAL,TYCVREAL,1,"r_int" },
	{ TYDREAL,TYDREAL,1,"d_int" },

	{ TYREAL,TYCVREAL,1,"r_nint" },
	{ TYDREAL,TYDREAL,1,"d_nint" },

	{ TYREAL,TYSHORT,1,"h_nint" },
	{ TYREAL,TYLONG,1,"i_nint" },

	{ TYDREAL,TYSHORT,1,"h_dnnt" },
	{ TYDREAL,TYLONG,1,"i_dnnt" },

	{ TYREAL,TYCVREAL,1,"r_abs" },
	{ TYSHORT,TYSHORT,1,"h_abs" },
	{ TYLONG,TYLONG,1,"i_abs" },
	{ TYDREAL,TYDREAL,1,"d_abs" },
	{ TYCOMPLEX,TYCVREAL,1,"c_abs" },
	{ TYDCOMPLEX,TYDREAL,1,"z_abs" },

	{ TYSHORT,TYSHORT,2,"h_mod" },
	{ TYLONG,TYLONG,2,"i_mod" },
	{ TYREAL,TYCVREAL,2,"r_mod" },
	{ TYDREAL,TYDREAL,2,"d_mod" },

	{ TYREAL,TYCVREAL,2,"r_sign" },
	{ TYSHORT,TYSHORT,2,"h_sign" },
	{ TYLONG,TYLONG,2,"i_sign" },
	{ TYDREAL,TYDREAL,2,"d_sign" },

	{ TYREAL,TYCVREAL,2,"r_dim" },
	{ TYSHORT,TYSHORT,2,"h_dim" },
	{ TYLONG,TYLONG,2,"i_dim" },
	{ TYDREAL,TYDREAL,2,"d_dim" },

	{ TYREAL,TYDREAL,2,"d_prod" },

	{ TYCHAR,TYSHORT,1,"h_len" },
	{ TYCHAR,TYLONG,1,"i_len" },

	{ TYCHAR,TYSHORT,2,"h_indx" },
	{ TYCHAR,TYLONG,2,"i_indx" },

	{ TYCOMPLEX,TYCVREAL,1,"r_imag" },
	{ TYDCOMPLEX,TYDREAL,1,"d_imag" },
	{ TYCOMPLEX,TYCOMPLEX,1,"r_cnjg" },
	{ TYDCOMPLEX,TYDCOMPLEX,1,"d_cnjg" },

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_sqrt"},
	{ TYDREAL,TYDREAL,1,"d_sqrt"},
#else
	{ TYREAL,TYCVREAL,1,"r_sqrt", 1 },
	{ TYDREAL,TYDREAL,1,"d_sqrt", 1 },
#endif
	{ TYCOMPLEX,TYCOMPLEX,1,"c_sqrt" },
	{ TYDCOMPLEX,TYDCOMPLEX,1,"z_sqrt" },

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_exp"},
	{ TYDREAL,TYDREAL,1,"d_exp"},
#else
	{ TYREAL,TYCVREAL,1,"r_exp", 2 },
	{ TYDREAL,TYDREAL,1,"d_exp", 2 },
#endif
	{ TYCOMPLEX,TYCOMPLEX,1,"c_exp" },
	{ TYDCOMPLEX,TYDCOMPLEX,1,"z_exp" },

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_log"},
	{ TYDREAL,TYDREAL,1,"d_log"},
#else
	{ TYREAL,TYCVREAL,1,"r_log", 3 },
	{ TYDREAL,TYDREAL,1,"d_log", 3 },
#endif
	{ TYCOMPLEX,TYCOMPLEX,1,"c_log" },
	{ TYDCOMPLEX,TYDCOMPLEX,1,"z_log" },

	{ TYREAL,TYCVREAL,1,"r_lg10" },
	{ TYDREAL,TYDREAL,1,"d_lg10" },

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_sin"},
	{ TYDREAL,TYDREAL,1,"d_sin"},
#else
	{ TYREAL,TYCVREAL,1,"r_sin", 4 },
	{ TYDREAL,TYDREAL,1,"d_sin", 4 },
#endif
	{ TYCOMPLEX,TYCOMPLEX,1,"c_sin" },
	{ TYDCOMPLEX,TYDCOMPLEX,1,"z_sin" },

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_cos"},
	{ TYDREAL,TYDREAL,1,"d_cos"},
#else
	{ TYREAL,TYCVREAL,1,"r_cos", 5 },
	{ TYDREAL,TYDREAL,1,"d_cos", 5 },
#endif
	{ TYCOMPLEX,TYCOMPLEX,1,"c_cos" },
	{ TYDCOMPLEX,TYDCOMPLEX,1,"z_cos" },

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_tan"},
	{ TYDREAL,TYDREAL,1,"d_tan"},
#else
	{ TYREAL,TYCVREAL,1,"r_tan", 6 },
	{ TYDREAL,TYDREAL,1,"d_tan", 6 },
#endif

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_asin"},
	{ TYDREAL,TYDREAL,1,"d_asin"},
#else
	{ TYREAL,TYCVREAL,1,"r_asin", 7 },
	{ TYDREAL,TYDREAL,1,"d_asin", 7 },
#endif

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_acos"},
	{ TYDREAL,TYDREAL,1,"d_acos"},
#else
	{ TYREAL,TYCVREAL,1,"r_acos", 8 },
	{ TYDREAL,TYDREAL,1,"d_acos", 8 },
#endif

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_atan"},
	{ TYDREAL,TYDREAL,1,"d_atan"},
#else
	{ TYREAL,TYCVREAL,1,"r_atan", 9 },
	{ TYDREAL,TYDREAL,1,"d_atan", 9 },
#endif

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,2,"r_atn2"},
	{ TYDREAL,TYDREAL,2,"d_atn2"},
#else
	{ TYREAL,TYCVREAL,2,"r_atn2", 10 },
	{ TYDREAL,TYDREAL,2,"d_atn2", 10 },
#endif

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_sinh"},
	{ TYDREAL,TYDREAL,1,"d_sinh"},
#else
	{ TYREAL,TYCVREAL,1,"r_sinh", 11 },
	{ TYDREAL,TYDREAL,1,"d_sinh", 11 },
#endif

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_cosh"},
	{ TYDREAL,TYDREAL,1,"d_cosh"},
#else
	{ TYREAL,TYCVREAL,1,"r_cosh", 12 },
	{ TYDREAL,TYDREAL,1,"d_cosh", 12 },
#endif

#ifdef DOUBLES
	{ TYREAL,TYCVREAL,1,"r_tanh"},
	{ TYDREAL,TYDREAL,1,"d_tanh"},
#else
	{ TYREAL,TYCVREAL,1,"r_tanh", 13 },
	{ TYDREAL,TYDREAL,1,"d_tanh", 13 },
#endif

	{ TYCHAR,TYLOGICAL,2,"hl_ge" },
	{ TYCHAR,TYLOGICAL,2,"l_ge" },

	{ TYCHAR,TYLOGICAL,2,"hl_gt" },
	{ TYCHAR,TYLOGICAL,2,"l_gt" },

	{ TYCHAR,TYLOGICAL,2,"hl_le" },
	{ TYCHAR,TYLOGICAL,2,"l_le" },

	{ TYCHAR,TYLOGICAL,2,"hl_lt" },
	{ TYCHAR,TYLOGICAL,2,"l_lt" },

	{ TYREAL,TYSHORT,1,"hr_expn" },
	{ TYREAL,TYLONG,1,"ir_expn" },
	{ TYDREAL,TYSHORT,1,"hd_expn" },
	{ TYDREAL,TYLONG,1,"id_expn" },

	{ TYREAL,TYCVREAL,1,"r_absp" },
	{ TYDREAL,TYDREAL,1,"d_absp" },

	{ TYREAL,TYDREAL,1,"r_rrsp" },
	{ TYDREAL,TYDREAL,1,"d_rrsp" },

	{ TYREAL,TYCVREAL,1,"r_frac" },
	{ TYDREAL,TYDREAL,1,"d_frac" },

	{ TYREAL,TYCVREAL,2,"r_make" },
	{ TYDREAL,TYDREAL,2,"d_make" },

	{ TYREAL,TYCVREAL,2,"r_scal" },
	{ TYDREAL,TYDREAL,2,"d_scal" }
} ;

LOCAL struct Incstblock
	{
	char atype;
	char rtype;
	char constno;
	} consttab[ ] =
{
	{ TYSHORT, TYLONG, 0 },
	{ TYLONG, TYLONG, 1 },
	{ TYREAL, TYLONG, 2 },
	{ TYDREAL, TYLONG, 3 },

	{ TYSHORT, TYLONG, 4 },
	{ TYLONG, TYLONG, 5 },
	{ TYREAL, TYLONG, 6 },
	{ TYDREAL, TYLONG, 7 },

	{ TYREAL, TYLONG, 8 },
	{ TYDREAL, TYLONG, 9 },

	{ TYREAL, TYLONG, 10 },
	{ TYDREAL, TYLONG, 11 },

	{ TYREAL, TYREAL, 0 },
	{ TYDREAL, TYDREAL, 1 },

	{ TYSHORT, TYLONG, 12 },
	{ TYLONG, TYLONG, 13 },
	{ TYREAL, TYREAL, 2 },
	{ TYDREAL, TYDREAL, 3 },

	{ TYREAL, TYREAL, 4 },
	{ TYDREAL, TYDREAL, 5 }
};

/* For each machine, two arrays must be initialized.
intcon contains
	radix for short int
	radix for long int
	radix for single precision
	radix for double precision
	precision for short int
	precision for long int
	precision for single precision
	precision for double precision
	emin for single precision
	emin for double precision
	emax for single precision
	emax for double prcision
	largest short int
	largest long int

realcon contains
	tiny for single precision
	tiny for double precision
	huge for single precision
	huge for double precision
	mrsp (epsilon) for single precision
	mrsp (epsilon) for double precision

the realcons should probably be filled in in binary if TARGET==HERE
*/

char callbyvalue[ ][XL] =
	{
	"sqrt",
	"exp",
	"log",
	"sin",
	"cos",
	"tan",
	"asin",
	"acos",
	"atan",
	"atan2",
	"sinh",
	"cosh",
	"tanh"
	};

expptr intrcall(np, argsp, nargs)
Namep np;
struct Listblock *argsp;
int nargs;
{
int i, rettype;
Addrp ap;
register struct Specblock *sp;
register struct Chain *cp;
expptr inline(), mkcxcon(), mkrealcon();
register struct Incstblock *cstp;
expptr q, ep;
int mtype, retype;
int op;
int f1field, f2field, f3field;
#if M68881
int intr_881;
#endif

packed.ijunk = np->vardesc.varno;
f1field = packed.bits.f1;
f2field = packed.bits.f2;
f3field = packed.bits.f3;
#if M68881
intr_881 = packed.bits.intr_881;
#endif
if(nargs == 0)
	goto badnargs;

mtype = 0;
for(cp = argsp->listp ; cp ; cp = cp->nextp)
	{
/* TEMPORARY */ ep = (expptr) (cp->datap);
/* TEMPORARY */	if( ISCONST(ep) && ep->headblock.vtype==TYSHORT )
/* TEMPORARY */		cp->datap = (tagptr) mkconv(tyint, ep);
	mtype = maxtype(mtype, ep->headblock.vtype);
	}

switch(f1field)
	{
	case INTRBOOL:
		op = f3field;
		if( ! ONEOF(mtype, MSKINT|MSKLOGICAL) )
			goto badtype;
		if(op == OPBITNOT)
			{
			if(nargs != 1)
				goto badnargs;
			q = mkexpr(OPBITNOT, argsp->listp->datap, ENULL);
			}
		else
			{
			if(nargs != 2)
				goto badnargs;
			q = mkexpr(op, argsp->listp->datap,
				argsp->listp->nextp->datap);
			}
		frchain( &(argsp->listp) );
		free( (charptr) argsp);
		return(q);

	case INTRCONV:
		rettype = f2field;
		if(rettype == TYLONG)
			rettype = tyint;
		if( ISCOMPLEX(rettype) && nargs==2)
			{
			expptr qr, qi;
			qr = (expptr) (argsp->listp->datap);
			qi = (expptr) (argsp->listp->nextp->datap);
			if(ISCONST(qr) && ISCONST(qi))
				q = mkcxcon(qr,qi);
			else	q = mkexpr(OPCONV,mkconv(rettype-2,qr),
					mkconv(rettype-2,qi));
			}
		else if(nargs == 1)
			q = mkconv(rettype, argsp->listp->datap);
		else goto badnargs;

		q->headblock.vtype = rettype;
		frchain(&(argsp->listp));
		free( (charptr) argsp);
		return(q);


	case INTRCNST:
		cstp = consttab + f3field;
		for(i=0 ; i<f2field ; ++i)
			if(cstp->atype == mtype)
				goto foundconst;
			else
				++cstp;
		goto badtype;

	foundconst:
		switch(cstp->rtype)
			{
			case TYLONG:
				return(mkintcon(intcon[cstp->constno]));

			case TYREAL:
			case TYDREAL:
				return(mkrealcon(cstp->rtype,
					realcon[cstp->constno]) );

			default:
				fatal("impossible intrinsic constant");
			}

	case INTRGEN:
		sp = spectab + f3field;
		if(no66flag)
			if(sp->atype == mtype)
				goto specfunct;
			else err66("generic function");

		for(i=0; i<f2field ; ++i)
			if(sp->atype == mtype)
				goto specfunct;
			else
				++sp;
		goto badtype;

	case INTRSPEC:
		sp = spectab + f3field;
	specfunct:
		if(tyint==TYLONG && sp->rtype==TYSHORT
			&& (sp+1)->atype==sp->atype)
				++sp;

		if(nargs != sp->nargs)
			goto badnargs;
		if(mtype != sp->atype)
			goto badtype;
		fixargs(YES, argsp);
		retype = (sp->rtype == TYCVREAL) ? TYDREAL : sp->rtype;
		if(q = inline(sp-spectab, mtype, argsp->listp))
			{
			frchain( &(argsp->listp) );
			free( (charptr) argsp);
			}
		else if(sp->othername)
			{
			ap = builtin(retype,
				varstr(XL, callbyvalue[sp->othername-1]) );
			q = fixexpr( mkexpr(OPCCALL, ap, argsp) );
			goto putconv;
			}
		else
			{
			ap = builtin(retype, varstr(XL, sp->spxname) );
			q = fixexpr( mkexpr(OPCALL, ap, argsp) );
#if M68881
			if( intr_881 )
				q->exprblock.vstg = STG881INTR;
#endif
		putconv:
			if (sp->rtype == TYCVREAL)
				q = mkconv(TYREAL, q);
			}
		return(q);

	case INTRMIN:
	case INTRMAX:
		if(nargs < 2)
			goto badnargs;
		if( ! ONEOF(mtype, MSKINT|MSKREAL) )
			goto badtype;
		argsp->vtype = mtype;
		q = mkexpr( (f1field==INTRMIN ? OPMIN : OPMAX), argsp, ENULL);

		q->headblock.vtype = mtype;
		rettype = f2field;
		if(rettype == TYLONG)
			rettype = tyint;
		else if(rettype == TYUNKNOWN)
			rettype = mtype;
		return( mkconv(rettype, q) );

	default:
		fatali("intrcall: bad intrgroup %d", f1field);
	}
badnargs:
	errstr("bad number of arguments to intrinsic %s",
		varstr(VL,np->varname) );
	goto bad;

badtype:
	errstr("bad argument type to intrinsic %s", varstr(VL, np->varname) );

bad:
	return( errnode() );
}




intrfunct(s)
char s[VL];
{
register struct Intrblock *p;
char nm[VL];
register int i;
#if m68881
int intr_881;
#endif

for(i = 0 ; i<VL ; ++s)
	nm[i++] = (*s==' ' ? '\0' : *s);

for(p = intrtab; p->intrval.intrgroup!=INTREND ; ++p)
	{
	if( eqn(VL, nm, p->intrfname) )
		{
		packed.bits.f1 = p->intrval.intrgroup;
		packed.bits.f2 = p->intrval.intrstuff;
		packed.bits.f3 = p->intrval.intrno;
#if M68881
		packed.bits.intr_881 = p->intrval.intr_881;
#endif
		return(packed.ijunk);
		}
	}

return(0);
}





Addrp intraddr(np)
Namep np;
{
Addrp q;
register struct Specblock *sp;
int f3field;

if(np->vclass!=CLPROC || np->vprocclass!=PINTRINSIC)
	fatalstr("intraddr: %s is not intrinsic", varstr(VL,np->varname));
packed.ijunk = np->vardesc.varno;
f3field = packed.bits.f3;

switch(packed.bits.f1)
	{
	case INTRGEN:
		/* imag, log, and log10 arent specific functions */
		if(f3field==31 || f3field==43 || f3field==47)
			goto bad;

	case INTRSPEC:
		sp = spectab + f3field;
		if(tyint==TYLONG && sp->rtype==TYSHORT)
			++sp;
		q = builtin(sp->rtype, varstr(XL,sp->spxname) );
		return(q);

	case INTRCONV:
	case INTRMIN:
	case INTRMAX:
	case INTRBOOL:
	case INTRCNST:
	bad:
		errstr("cannot pass %s as actual",
			varstr(VL,np->varname));
		return( (Addrp) errnode() );
	}
fatali("intraddr: impossible f1=%d\n", (int) packed.bits.f1);
/* NOTREACHED */
}





expptr inline(fno, type, args)
int fno;
int type;
struct Chain *args;
{
register expptr q, t, t1;

switch(fno)
	{
	case 8:	/* real abs */
	case 9:	/* short int abs */
	case 10:	/* long int abs */
	case 11:	/* double precision abs */
		if( addressable(q = (expptr) (args->datap)) )
			{
			t = q;
			q = NULL;
			}
		else
			t = (expptr) mktemp(type,PNULL);
		t1 = mkexpr(OPQUEST,
			mkexpr(OPLE, mkconv(type,ICON(0)), cpexpr(t)),
			mkexpr(OPCOLON, cpexpr(t),
				mkexpr(OPNEG, cpexpr(t), ENULL) ));
		if(q)
			t1 = mkexpr(OPCOMMA, mkexpr(OPASSIGN, cpexpr(t),q), t1);
		frexpr(t);
		return(t1);

	case 26:	/* dprod */
		q = mkexpr(OPSTAR, mkconv(TYDREAL,args->datap), args->nextp->datap);
		return(q);

	case 27:	/* len of character string */
		q = (expptr) cpexpr(args->datap->headblock.vleng);
		frexpr(args->datap);
		return(q);

	case 14:	/* half-integer mod */
	case 15:	/* mod */
		return( mkexpr(OPMOD, (expptr) (args->datap),
			(expptr) (args->nextp->datap) ));
	}
return(NULL);
}
