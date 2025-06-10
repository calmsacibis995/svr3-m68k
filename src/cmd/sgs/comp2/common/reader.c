/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)reader.c	7.5		*/	

static	char	SCCSID[] = "@(#)reader.c	7.5		86/10/14";

# include <string.h>
#ifdef HLOPTIM
extern int nodesin;
# include <hash.h>
# include "mfile2.h"
NODE node[TREESZ];
				/* WARNING, the #else clause takes up */	
				/* approxamitely 975 lines.	      */
#else

# include "mfile2.h"

/*	some storage declarations */

# ifdef TWOPASS
NODE node[TREESZ];
int fdef=0;
int ftnno;  /* number of current function */
int lineno;
extern int regvar;	/* register usage mask.  Must be read in */
# else
# define NOMAIN
#endif

int lflag;
int e2debug;
int udebug;
extern int zflag;
int fast;

#ifdef M68020
int align_val = 16;	/* M68020 default variable alignment; word */
#endif

int is_m68881 = 0; 	/* TRUE if system has an M68881 coprocessor;
			** double/float return values returned in
			** %fp0 only				 */
int is_m68881u= 0; 	/* TRUE if system has an M68881 coprocessor,
			** and double/float return values returned in 
			** both %fp0 and %d0/%d1 for compatibility
			** with old object code.  (u="u"niversal) */
extern char *getenv();

/* maxtemp is the maximum size (in bits) needed for temps so far */
/* maxarg is ditto for outgoing arguments */
/* maxboff is ditto for automatic variables */
/* earlier attempts to keep these on a per-block basis were silly */
int maxtemp;
int maxarg;
int maxboff;
NODE * condit();

NODE *
force(p)
register NODE *p; 
{
	register NODE *q, *r;
	if( !p ) cerror( "force" );
	q = talloc();
	*q = *p;
	r = talloc();
	*r = *p;
	q->tn.op = ASSIGN;
	q->in.right = p;
	q->in.left = r;
	r->tn.op = QNODE;
	r->tn.rval = callreg(p); /* the reg where the value will be forced */
	return( q );
}

p2init( argc, argv )
char *argv[];
{
	/* set the values of the pass 2 arguments */

	register int c;
	register char *cp;
	register files;
#ifdef M68020
	char *strval;
#endif

	allo0();  /* free all regs */
	files = 0;

#ifdef M68020
	if( ( ( strval = getenv( "DBLALIGN" ) ) != NULL ) &&
	    ( ! strcmp( strval, "YES" ) ) )
	{
		align_val = 32;	/* double word align */
	}
#endif

#ifdef M68881

	if ( ( strval = getenv( "FP" ) ) != NULL ) 
	{
		if (strval[0] == 'm')
			strval[0] = 'M'; /* reduce # of cases to cover */

		if ( !strcmp( strval, "M68881"))
		{
			++is_m68881;	/* generate code for M68881 */
					/* return float/doubles in %fp0 only */
		}

		if (( ! strcmp( strval, "M68881u" ) )||
		    ( ! strcmp( strval, "M68881U" ) ))
		{
			++is_m68881;	/* generate code for M68881 */
			++is_m68881u;	/* generate code for M68881 */
					/* return float/doubles in %fp0 */
					/* and also in %d0/%d1 */
		}
	}
#endif

	for( c=1; c<argc; ++c )
	{
		if( *(cp=argv[c]) == '-' )
		{
			while( *++cp )
			{
				switch( *cp )
				{

				case 'X':  /* pass1 flags */
					while( *++cp ) 
					{
						 /* VOID */ 
					}
					--cp;
					break;

				case 'l':  /* linenos */
					++lflag;
					break;

				case 'e':  /* expressions */
					++e2debug;
					break;

				case 'o':  /* orders */
					++odebug;
					break;

				case 'r':  /* register allocation */
					++rdebug;
					break;

				case 's':  /* shapes */
					++sdebug;
					break;

				case 'u':  /* Sethi-Ullman testing
						(machine dependent) */
					++udebug;
					break;

				case 'f':  /* try for faster compile speed */
					++fast;
					break;

#ifdef ASSYCOMMENT
				case 'z':
					++zflag; /* stin file debug info */
					break;
#endif /* ASSYCOMMENT */

				default:
					cerror( "bad option: %c", *cp );
				}
			}
		}
		else files = 1;  /* assumed to be a ftitle */
	}

	mkdope();
	return( files );
}

NODE *
dlabel( p, l )
register NODE *p; 
{
	/* define a label after p is executed */
	register NODE *q;
	if( !p ) cerror( "dlabel" );
	q = talloc();
	q->tn.type = p->tn.type;
	q->in.left = p;
	q->tn.op = GENLAB;
	q->bn.label = l;
	return( q );
}

NODE *
genbr( o, l, p )
register NODE *p; 
register o,l;
{
	/* after evaluating p, generate a branch to l */
	/* if o is 0, unconditional */
	register NODE *q;
	if( !p ) cerror( "genbr" );
#ifndef TWOPASS
	if( l < 0 ) cerror( "genbr1" );
#endif
	q = talloc();
	q->tn.op = o?GENBR:GENUBR;
	q->in.left = p;
	q->bn.label = l;
	q->bn.lop = o;
	if( o && logop(p->tn.op) ) 
	{
		p->in.op = CMP;
#ifdef M68881
		p->in.type = p->in.left->in.type;
#endif
	}
	q->tn.type = p->tn.type;
	return( q );
}

int negrel[] = 
{
	 NE, EQ, GT, GE, LT, LE, UGT, UGE, ULT, ULE 
} ;  /* negatives of relationals */

tcond( p )
register NODE *p; 
{
	/* return 1 if p is always true, 0 otherwise */
	register o = p->tn.op;
	register NODE *q,
		      sav;

	switch( o ) 
	{

	case ICON:
		return( p->tn.lval || p->tn.name != (char *) 0 );

	case COMOP:
		return( tcond( p->in.right ) );

	case OROR:
		if( tcond( p->in.left ) )
		{
			/* always true, but right tree never evaluated*/
			/*delete '||' and right sub tree, promote left*/
			sav = *p->in.left;
			tfree( p->in.right );	     
			p->in.left->in.op = FREE;    /* free old left node */
			*p = sav;
			return( 1 );
		}
		return( 0 );

	case ANDAND:
		return( tcond( p->in.left ) && tcond( p->in.right ) );

	case NOT:
		return( fcond( p->in.left ) );

	case QUEST:
		q = p->in.right;
		if( tcond( p->in.left ) ) return( tcond( q->in.left ) );
		if( fcond( p->in.left ) ) return( tcond( q->in.right ) );
		return( tcond( q->in.left ) && tcond( q->in.right ) );

#ifdef m68k
	case UGE:
		if( p->in.right->in.op == ICON &&
		    p->in.right->tn.lval == 0  &&
		    p->in.right->in.name == (char *) 0 ) return( 1 );
		return( 0 );

#endif
	default:
		return( 0 );
	}
}

fcond( p )
register NODE *p; 
{
	/* return 1 if p is always false, 0 otherwise */
	register o = p->tn.op;
	register NODE *q,
		      sav;

	switch( o ) 
	{

	case ICON:
		return( !p->tn.lval && p->tn.name == (char *) 0 );

	case COMOP:
		return( fcond( p->in.right ) );

	case ANDAND:
		if( fcond( p->in.left ) )
		{
			/*always false, but right tree never evaluated*/
			/*delete '&&' and right subtree, promote left */
			sav = *p->in.left;
			tfree( p->in.right );	     
			p->in.left->in.op = FREE;    /* free old left node */
			*p = sav;		     /* .. and promte it to p */
			return( 1 );
		}
		return( 0 );

	case OROR:
		return( fcond( p->in.left ) && fcond( p->in.right ) );

	case NOT:
		return( tcond( p->in.left ) );

	case QUEST:
		q = p->in.right;
		if( tcond( p->in.left ) ) return( fcond( q->in.left ) );
		if( fcond( p->in.left ) ) return( fcond( q->in.right ) );
		return( fcond( q->in.left ) && fcond( q->in.right ) );

#ifdef m68k
	case ULT:
		if( p->in.right->in.op == ICON &&
		    p->in.right->tn.lval == 0  &&
		    p->in.right->in.name == (char *) 0 ) return( 1 );
		return( 0 );

#endif
	default:
		return( 0 );
	}
}

NODE *
rcomma( p )
register NODE *p; 
{
	/* p is a COMOP; return the shrunken version thereof */

	if( p->tn.op != COMOP ) cerror( "rcomma" );

	if( p->in.left && p->in.right ) return( p );
	p->tn.op = FREE;
	if( !p->in.left ) return( p->in.right );
	return( p->in.left );
}

NODE *
gtb( p, l )
register NODE *p; 
register l;
{
	register NODE *q;
	/* replace p by a trivial branch to l */
	/* if l is -1, return NULL */
	q = condit( p, CEFF, -1, -1 );
#ifdef m68k
				/* -1 only value indicating no label */
	if( l==-1 ) return( q );
#else
	if( l<0 ) return( q );
#endif
	if( !q ) 
	{
		q = talloc();
		q->tn.op = ICON;
		q->tn.lval = 0;
		q->tn.name = (char *) 0;
		q->tn.type = TINT;
	}
	return( genbr( 0, l, q ) );
}

NODE *
condit( p, goal, t, f )
register NODE *p; 
register goal,t,f;
{
	/* generate code for conditionals in terms of GENLAB and GENBR nodes */
	/* goal is either CEFF, NRGS, or CCC */
	/* also, delete stuff that never needs get done */
	/* if goal==CEFF, return of null means nothing to be done */

	register o, lt, lf, l;
	register NODE *q, *q1, *q2;

	o = p->tn.op;

	if( odebug >2 ) 
	{
		printf( "condit( %d (%s), %s, %d, %d )\n", (int)(p-node),
		opst[o], goal==CCC?"CCC":(goal==NRGS?"NRGS":"CEFF"),
		t, f );
	}
	if( o == CBRANCH ) 
	{
		p->in.right->tn.op = p->tn.op = FREE;
		l = p->in.right->tn.lval;
		p = p->in.left;
		if( fcond( p ) ) return( gtb(p,l) );
		if( tcond( p ) ) return( gtb(p,-1) );
		return( condit( p, CCC, -1, l ) );
	}

#ifdef ITREES
	switch (o)
	{
	case GENLAB:		/* pass genlab from front end */
		if( p->in.left->tn.op == ICON && p->in.left->tn.lval == 1)
		    {
		    printf("L%%%d:\n", p->tn.rval);
		    tfree(p);
		    return(NIL);
		    }
		 break;
	case GENUBR:	/* pass genlab/genubr from front end */
		if( p->in.left->tn.op == ICON && p->in.left->tn.lval == 1)
		    {
		    printf("\tbra\tL%%%d\n", p->tn.rval);
		    tfree(p);
		    return(NIL);
		    }
		 break;
	case PROLOG:
		doprolog(p);
		tfree(p);
		return(NIL);
	case EPILOG:
		doepilog(p);
		tfree(p);
		return(NIL);
	case SWBEG:
		doswitch(p);
		tfree(p);
		return(NIL);
	}
#endif
	/* a convenient place to diddle a few special ops */
	if( callop(o) )
	{
		if( optype(o) == UTYPE ) p->stn.argsize = 0;
		else p->stn.argsize = argsize(p->in.right);
		if( goal==CEFF ) goal = NRGS;
		/* flow on, so that we can handle if( f(...) )... */
	}
	else if( goal==CEFF && (asgop(o) || o==STASG || o==INIT)) goal=NRGS;

	/* do a bit of optimization */

	if( goal == NRGS ) 
	{
		if( logop(o) )
		{
			/* must make p into ( p ? 1 : 0 ), then recompile */
			q1 = talloc();
			q1->tn.op = ICON;
			q1->tn.name = (char *) 0;
			q1->tn.lval = 1;
			q1->tn.type = p->tn.type;
			q2 = talloc();
			*q2 = *q1;
			q2->tn.lval = 0;
			q = talloc();
			q->tn.op = COLON;
			q->tn.type = p->tn.type;
			q->in.left = q1;
			q->in.right = q2;
			q1 = talloc();
			q1->tn.op = o = QUEST;
			q1->tn.type = p->tn.type;
			q1->in.left = p;
			q1->in.right = q;
			p = q1;  /* flow on, and compile */
		}
	}

	if( goal != CCC ) 
	{
		if( o == QUEST ) 
		{
			/* rewrite ? : when goal not CCC */
			lf = getlab();
			l = getlab();
			p->tn.op = COMOP;
			q = p->in.right;
			q1 = condit( q->in.left, goal, -1, -1 );
			q->in.right = condit( q->in.right, goal, -1, -1 );
			if( tcond( p->in.left ) ) 
			{
				q->tn.op = FREE;
				tfree( q->in.right );
				p->in.right = q1;
				p->in.left=condit( p->in.left, CEFF, -1, -1 );
				return( rcomma( p ) );
			}
			if( fcond( p->in.left ) ) 
			{
				q->tn.op = FREE;
				tfree( q1 );
				p->in.right = q->in.right;
				p->in.left=condit( p->in.left, CEFF, -1, -1 );
				return( rcomma( p ) );
			}
			if( !q1 ) 
			{
				if( !q->in.right ) 
				{
					/* nothing to do */
					tfree( p->in.left );
					p->tn.op = q->tn.op = FREE;
					return( q1 );
				}
				/* rhs done if condition is false */
				p->in.left = condit( p->in.left, CCC, l, -1 );
				p->in.right = dlabel( q->in.right, l );
				q->tn.op = FREE;
				return( p );
			}
			else if( !q->in.right ) 
			{
				/* lhs done if condition is true */
				p->in.left=condit( p->in.left, CCC, -1, lf );
				p->in.right = dlabel( q1, lf );
				q->tn.op = FREE;
				return( p );
			}

			/* both sides exist and the condition is nontrivial */
			p->in.left = condit( p->in.left, CCC, -1, lf );
			q1 = force(q1);
			q->in.right = force(q->in.right);
			q1 = genbr( 0, l, q1 );
			q->in.left = dlabel( q1, lf );
			q->tn.op = COMOP;
			return( dlabel( p, l ) );
		}

		if( goal == CEFF ) 
		{
			/* some things may disappear */
			switch( o ) 
			{

 			case GOTO:
			case CBRANCH:
			case GENBR:
			case GENUBR:
			case CALL:
			case UNARY CALL:
			case FORTCALL:
			case UNARY FORTCALL:
			case STCALL:
			case UNARY STCALL:
			case STASG:
			case INIT:
			case MOD:   /* do these for the side effects */
			case DIV:
			case UOP0:
			case UOP1:
			case UOP2:
			case UOP3:
			case UOP4:
			case UOP5:
			case UOP6:
			case UOP7:
			case UOP8:
			case UOP9:
				goal = NRGS;
			}
		}

		switch( optype(o) ) 
		{
		case LTYPE:
			if( goal == CEFF ) 
			{
				p->tn.op = FREE;
				return( NIL );
			}
			break;

		case BITYPE:
			p->in.right = condit( p->in.right, goal, -1, -1 );
		case UTYPE:
			p->in.left = condit( p->in.left, o==COMOP?CEFF:goal,
			-1, -1 );
		}

		if( goal == CEFF || o==COMOP ) 
		{
			/* lhs or rhs may have disappeared */
			/* op need not get done */

			switch( optype(o) )
			{

			case BITYPE:
				p->tn.op = COMOP;
				return( rcomma( p ) );

			case UTYPE:
				p->tn.op = FREE;
				return( p->in.left );

			case LTYPE:
				p->tn.op = FREE;
				return( NIL );
			}
		}
		return( p );
	}

	/* goal must = CCC from here on */

	switch( o ) 
	{

	case ULE:
	case ULT:
	case UGE:
	case UGT:
	case EQ:
	case NE:
	case LE:
	case LT:
	case GE:
	case GT:
		if(t == -1) 
		{
			o = p->tn.op = negrel[o-EQ];
			t = f;
			f = -1;
		}

#ifndef NOOPT
		if( p->in.right->in.op == ICON &&
		    p->in.right->tn.lval == 0 &&
		    p->in.right->in.name == (char *) 0 ) 
		{

			/* the question here is whether we can assume that */
			/* unconditional branches preserve condition codes */
			/* if this turned out to be no, we would have to */
			/* explicitly handle this case here */

			switch( o ) 
			{

			case UGT:
			case ULE:
				o = p->in.op = (o==UGT)?NE:EQ;
			case EQ:
			case NE:
			case LE:
			case LT:
			case GE:
			case GT:
				if( logop( p->in.left->tn.op ) )
				{
					/* situation like (a==0)==0 */
					/* ignore optimization */
					goto noopt;
				}
				break;

			case ULT:  /* never succeeds */
				return( gtb( p, f ) );

			case UGE:
				/* always succeeds */
				return( gtb( p, t ) );
			}
			p->tn.op = p->in.right->tn.op = FREE;
			p = condit( p->in.left, NRGS, -1, -1 );
			p = genbr( o, t, p );
			if( f == -1 ) return( p );
			else return( genbr( 0, f, p ) );
		}
noopt:
# endif

		p->in.left = condit( p->in.left, NRGS, -1, -1 );
		p->in.right = condit( p->in.right, NRGS, -1, -1 );
		p = genbr( o, t, p );
		if( f != -1 ) p = genbr( 0, f, p );
		return( p );

	case COMOP:
		p->in.left = condit( p->in.left, CEFF, -1, -1 );
		p->in.right = condit( p->in.right, CCC, t, f );
		return( rcomma( p ) );

	case NOT:
		p->tn.op = FREE;
		return( condit( p->in.left, CCC, f, t ) );

	case ANDAND:
		lf = f == -1 ? getlab() : f;
		p->tn.op = COMOP;
		if( tcond( p->in.left ) )
		{
			p->in.left = condit( p->in.left, CEFF, -1, -1 );
		}
		else p->in.left = condit( p->in.left, CCC, -1, lf );
		if( tcond( p->in.right ) )
		{
			p->in.right = condit( p->in.right, CEFF, -1, -1 );
		}
		else p->in.right = condit( p->in.right, CCC, t, f );
		p = rcomma( p );
		if( f == -1 ) p = dlabel( p, lf );
		return( p );

	case OROR:
		lt = t == -1 ? getlab() : t;
		p->tn.op = COMOP;
		if( fcond( p->in.left ) )
		{
			lt = -1;
			l = CEFF;
		}
		else
			l = CCC;
		if( fcond( p->in.right ) )
		{
		/*	p->in.left = condit(p->in.left, l, lt = -1, f); */
			p->in.left = condit(p->in.left, l, lt, f);
			p->in.right = condit( p->in.right, CEFF, -1, -1 );
		}
		else
		{
			p->in.left = condit(p->in.left, l, lt, -1);
			p->in.right = condit( p->in.right, CCC, t, f );
		}
		p = rcomma( p );
		if( t == -1 && lt != -1) p = dlabel( p, lt );
		return( p );

		/* do ICON here, someday ... */

	case QUEST:
		lf = f == -1 ? getlab() : f;
		lt = t == -1 ? getlab() : t;
		p->in.left = condit( p->in.left, CCC, -1, l=getlab() );
		q = p->in.right;
		q1 = condit( q->in.left, goal, lt, lf );
		q->in.left = dlabel( q1, l );
		q->in.right = condit( q->in.right, goal, t, f );
		p->tn.op = COMOP;
		q->tn.op = COMOP;
		if( t == -1 ) p = dlabel( p, lt );
		if( f == -1 ) p = dlabel( p, lf );
		return( p );

	default:
		/* get the condition codes, generate the branch */
		switch( optype(o) )
		{
		case BITYPE:
			p->in.right = condit( p->in.right, NRGS, -1, -1 );
		case UTYPE:
			p->in.left = condit( p->in.left, NRGS, -1, -1 );
		}
		if( t != -1 ) p = genbr( NE, t, p );
		if( f != -1 ) p = genbr( (t!=-1)?0:EQ, f, p );
		return( p );
	}
}

/*
 *	BEGIN: ported from VAX pcc to make two pass version
 */

# ifndef NOMAIN

mainp2( argc, argv ) char *argv[]; {
	register files;
	register temp;
	register c;
	register char *cp;
	register NODE *p;

	files = p2init( argc, argv );
	tinit();

	if( files ){
		while( files < argc ) {
			if( *(argv[files]) != '-' ) switch( ++fdef )
			{
			case 1:
			case 2:
				if( freopen(argv[files], fdef==1 ? "r" : "w", fdef==1 ? stdin : stdout) == NULL)
				{
					fprintf(stderr, "ccom:can't open %s\n", argv[files]);
					exit(1);
				}
				break;

			default:
				cerror("usage error: c1 [<options>] [<source file> [<destination file>]] : \"%s\" purpose unknown", argv[files]);
			}
			++files;
		}
	    }
	while( (c=getchar()) > 0 ) switch( c ){
	case ')':
	default:
		/* copy line unchanged */
		if ( c != ')' )
			PUTCHAR( c );  /*  initial tab  */
		while( (c=getchar()) > 0 ){
			PUTCHAR(c);
			if( c == '\n' ) break;
			}
		continue;

	case BBEG:
	{
		register myftn, aoff;

		/* beginning of a block */
		myftn = rdin(10);  /* ftnno */
		/* autooff for block gives max offset of autos in block */
		aoff = (unsigned int) rdin(10); 
		SETOFF( aoff, ALSTACK );
		regvar = rdin(10);
		if( getchar() != '\n' ) cerror( "intermediate file format error");

		if( myftn != ftnno )	/* beginning of a function */
		{
			maxboff = aoff;
			ftnno = myftn;
			maxtemp = 0;
			maxarg = 0;
		}
		else 
		{
			/* maxoff at end of ftn is max of autos and temps
			   over all blocks in the function */
			if( aoff > maxboff ) maxboff = aoff;
		}
# ifdef SETREGS
			SETREGS(regvar);
# endif
		continue;
		} /* end the BBEG block */

	case BEND:  /* end of block */
#ifdef MC68K
		{
		int level;

		level = rdin (10);
		regvar = rdin (10);

		if (level != 0)
		   {
		   if( getchar() != '\n' ) cerror( "intermediate file format error");
		   continue;
		   }
		}
#endif
		SETOFF( maxboff, ALSTACK );
		SETOFF( maxarg, ALSTACK );
		SETOFF( maxtemp, ALSTACK );
		eobl2();
		maxboff = maxarg = maxtemp = 0;
		while( (c=getchar()) != '\n' ){
			if( c <= 0 ) cerror( "intermediate file format eof" );
			}
		continue;

	case EXPR:	/* corresponds to p2compile() below */
		/* compile code for an expression */
		lineno = rdin( 10 );
		for( cp=ftitle; (*cp=getchar()) != '\n'; ++cp ) ; /* VOID, reads filename */
		*cp = '\0';

	        if( lflag ) lineid( lineno, ftitle );
	        tmpoff = 0;  /* expression at top level reuses temps */
# ifdef MC68K
		{
		    extern int regtmp;
		    regtmp = 0;		/* Also reuse temp. registers */
		}
# endif
		p = eread();		/* get a tree */
	/* generate code for the tree p */

# ifdef MYREADER
	MYREADER(p);  /* do your own laundering of the input */
# endif
# ifndef NODBG
		if( p && odebug>2 ) e2print(p);
# endif
	/* eliminate the conditionals (moved comment dn 3 lines bjp) */
		p = condit( p, CEFF, -1, -1 );
		if( p ) 
		{
			/* expression does something */
			/* generate the code */
# ifndef NODBG
			if( odebug>2 ) e2print(p);
# endif
			codgen( p );
		}
# ifndef NODBG
		else if( odebug>1 ) printf( "null effect\n" );
# endif
		allchk();
		tcheck();
		continue;

		}

	/* EOF */
	return(nerrors);

	}

# endif /* NOMAIN */

/*
 *	END: ported from VAX pcc to make two pass version
 */


# ifndef TWOPASS

p2compile( p )
register NODE *p; 
{
	if( lflag ) lineid( lineno, ftitle );
	tmpoff = 0;  /* expression at top level reuses temps */
# ifdef MC68K
        {
	   extern int regtmp;
	   regtmp = 0;		/* Also reuse temp. registers */
        }
# endif
	/* generate code for the tree p */

# ifdef MYREADER
	MYREADER(p);  /* do your own laundering of the input */
# endif
# ifndef NODBG
	if( p && odebug>2 ) e2print(p);
# endif
	/* eliminate the conditionals (moved comment dn 3 lines bjp) */
	p = condit( p, CEFF, -1, -1 );
	if( p ) 
	{
		/* expression does something */
		/* generate the code */
# ifndef NODBG
		if( odebug>2 ) e2print(p);
# endif
		codgen( p );
	}
# ifndef NODBG
	else if( odebug>1 ) printf( "null effect\n" );
# endif
	allchk();
	/* tcheck will be done by the first pass at the end of a ftn. */
	/* first pass will do it... */
}

p2bbeg( aoff, myreg ) 
register aoff,myreg;
{
	static int myftn = -1;
	SETOFF( aoff, ALSTACK );
	if( myftn != ftnno )
	{
		 /* beginning of function */
		maxboff = aoff;
		myftn = ftnno;
		maxtemp = 0;
		maxarg = 0;
	}
	else 
	{
		if( aoff > maxboff ) maxboff = aoff;
	}
# ifdef SETREGS
	SETREGS(myreg);
# endif
}

p2bend()
{
	SETOFF( maxboff, ALSTACK );
	SETOFF( maxarg, ALSTACK );
	SETOFF( maxtemp, ALSTACK );
	eobl2();
	maxboff = maxarg = maxtemp = 0;
}

# endif
# endif /* HLOPTIM */

# ifndef NOMAIN
NODE *
eread(){
/*
	/* call eread recursively to get subtrees, if any */

	register NODE *p;
	register i, c;
	register char *pc;
	register j;
	char	buff[257]; /* one more than largest FLEXNAME! */

# ifdef HLOPTIM
	nodesin += 1;
# endif /* HLOPTIM */

	i = rdin( 10 );

	p = talloc();

	p->in.op = i;

	i = optype(i);

 	if( p->in.op == GENBR )
 	{
 		p->bn.label = rdin( 10 );
 		p->bn.lop = rdin( 10 );
 	}
 	else
 	{
 		if( i == LTYPE ) p->tn.lval = rdin( 10 );
		if( i != BITYPE ) p->tn.rval = rdin( 10 );
 	}

#ifdef ITREES
	if (p->in.op == SWCASE)
		p->in.type = rdin(8); /* KLUGE type is not really a type */
	else
#endif
#ifdef HLOPTIM
	p->in.type = rdin(8);
#else
	p->in.type = ttype( rdin(8 ) );
#endif

	switch( p->in.op)
	{
	case STASG:
	case STARG:
	case STCALL:
	case UNARY STCALL:
		p->stn.stsize = rdin( 10 );
		p->stn.stalign = rdin(10);
		if( getchar() != '\n' ) cerror( "illegal \n" );
		break;
#ifdef M68881
	case FCON:
		((FLTDBL *)&p->fp.dval)->lg.l = rdin(10);
#ifdef HLOPTIM
		if( p->fp.type == DOUBLE )
#else
		if( p->fp.type == TDOUBLE )
#endif
			((FLTDBL *)&p->fp.dval)->lg.l2 = rdin(10);
		if( getchar() != '\n' ) cerror("illegal intermediate file \n");
		break;
#endif
	case GENBR:
		if( getchar() != '\n' ) cerror("illegal intermediate file \n");
		break;
#ifndef HLOPTIM
	case REG:
		rbusy( p->tn.rval, p->in.type );  /* not usually, but sometimes justified */
		if( getchar() != '\n' ) cerror("illegal intermediate file \n");
		break;
#endif /* HLOPTIM */
	default:	/* look for name string */
		for( pc=buff,j=0; ( c = getchar() ) != '\n'; ++j ){
				if( j < sizeof(buff) - 1 ) 
					*pc++ = c;
				}
		*pc = '\0';
#ifndef FLEXNAMES
		strncpy(p->in.name, buff, NCHNAM);
#else
				/* allocate a copy of the name */
		if ( j==0 ) p->in.name = (char *)0;	/* no name */
		else {
		      p->in.name = (char *)malloc( j+1 );
		      strcpy( p->in.name, buff );
		     }
#endif
	} 	/* casend */
	/* now, recursively read descendents, if any */

	if( i != LTYPE ) p->in.left = eread();
	if( i == BITYPE ) p->in.right = eread();

	return( p );

	}

CONSZ
rdin( base ){
	register sign, c;
	CONSZ val;

	sign = 1;
	val = 0;

	while( (c=getchar()) > 0 ) {
		if( c == '-' ){
			if( val != 0 ) cerror( "illegal -");
			sign = -sign;
			continue;
			}
		if( c == '\t' ) break;
		if( c>='0' && c<='9' ) {
			val *= base;
			if( sign > 0 )
				val += c-'0';
			else
				val -= c-'0';
			continue;
			}
		cerror( "illegal character `%c' on intermediate file", c );
		break;
		}

	if( c <= 0 ) {
		cerror( "unexpected EOF");
		}
	return( val );
	}
# endif

char *cnames[] = 
{
	"CEFF",
	"NRGS",
	"CCC",
	0,
};

prgoal( goal ) 
register goal;
{
	/* print a nice-looking description of goal */

	register i, flag;

	flag = 0;
	for( i=0; cnames[i]; ++i )
	{
		if( goal & (1<<i) )
		{
			if( flag ) printf( "|" );
			++flag;
			printf( cnames[i] );
		}
	}
	if( !flag ) printf( "?%o", goal );

}

#ifndef NODBG
e2print( p )
register NODE *p; 
{
	printf( "\n********* costs=(0,...,NRGS;EFF;TEMP;CC)\n" );
	e22print( p ,"T");
	printf("=========\n");
}

e22print( p ,s)
register NODE *p; 
char *s;
{
	static down=0;
	register ty;
#ifdef m68k
				/* make debug print more robust	*/
				/* don't die on NIL pointer	*/
	if( p == NIL ) {
		int d;
		for( d=down; d>1; d -= 2 ) printf( "\t" );
		if( d ) printf( "    " );
		printf("NIL TREE\n");
		return;
	    };
#endif

	ty = optype( p->tn.op );
	if( ty == BITYPE )
	{
		++down;
		e22print( p->in.right ,"R");
		--down;
	}
	e222print( down, p, s );

	if( ty != LTYPE )
	{
		++down;
		e22print( p->in.left, "L" );
		--down;
	}
}

e222print( down, p, s )
NODE *p; 
char *s;
{
	/* print one node */
	int i, d;

	for( d=down; d>1; d -= 2 ) printf( "\t" );
	if( d ) printf( "    " );

	printf( "%s.%d) op= '%s'",s, (int)(p-node), opst[p->in.op] );
	switch( p->in.op ) 
	{
		 /* special cases */
	case REG:
		printf( " %s", rnames[p->tn.rval] );
		break;

	case ICON:
	case NAME:
	case VAUTO:
	case VPARAM:
	case TEMP:
		printf( " " );
		adrput( p );
		break;

	case STCALL:
	case UNARY STCALL:
		printf( " args=%d", p->stn.argsize );
	case STARG:
	case STASG:
		printf( " size=%d", p->stn.stsize );
		printf( " align=%d", p->stn.stalign );
		break;

	case GENBR:
		printf( " %d (%s)", p->bn.label, opst[p->bn.lop] );
		break;

	case CALL:
	case UNARY CALL:
		printf( " args=%d", p->stn.argsize );
		break;

	case GENUBR:
	case GENLAB:
		printf( " %d", p->bn.label );
		break;

	case FUNARG:
		printf( " offset=%d", p->tn.rval );

	}

	printf( ", " );
	t2print( p->in.type );
	printf( ", c=[" );
	for( i=0; i<NRGS; ++i ) cprt( p->in.cst[i], "," );
	cprt( p->in.cst[NRGS], "; " );
	cprt( p->in.cst[CEFF], "; " );
	cprt( p->in.cst[CTEMP], "; " );
	cprt( p->in.cst[CCC], "]" );
	if( p->tn.goal == CEFF ) printf( " (EFF)\n" );
	else if( p->tn.goal == CCC ) printf( " (CC)\n" );
	else if( p->tn.goal != NRGS ) printf( "(BAD GOAL: %d)\n", p->tn.goal );
	else printf( "\n" );
}

t2print( t )
TWORD t;
{
	int i;
	static struct {
		TWORD mask;
		char * string;
		} t2tab[] = {
			TANY, "ANY",
			TINT, "INT",
			TUNSIGNED, "UNSIGNED",
			TCHAR, "CHAR",
			TUCHAR, "UCHAR",
			TSHORT, "SHORT",
			TUSHORT, "USHORT",
			TLONG, "LONG",
			TULONG, "ULONG",
			TFLOAT, "FLOAT",
			TDOUBLE, "DOUBLE",
			TPOINT, "POINTER",
			TPOINT2, "POINTER2",
			TSTRUCT, "STRUCT",
			TVOID, "VOID",
			0, 0
			};

	for( i=0; t && t2tab[i].mask; ++i ) {
		if( (t&t2tab[i].mask) == t2tab[i].mask ) {
			printf( " %s", t2tab[i].string );
			t ^= t2tab[i].mask;
			}
		}
	}

# else
e2print( p )
NODE *p; 
{
	werror( "e2print not compiled" );
}
e222print( p, s )
NODE *p; 
char *s;
{
	werror( "e222print not compiled" );
}
# endif
cprt( c, s )
register char *s; 
register c;
{
	if( c >= INFINITY ) printf( "*%s", s );
	else printf( "%d%s", c, s );
}
