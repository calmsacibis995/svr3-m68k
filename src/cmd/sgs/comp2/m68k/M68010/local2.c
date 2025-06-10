/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)local2.c	7.4		*/
static	char	SCCSID[] = "@(#) local2.c:	@(#)local2.c	7.4		00:00:22";

# include "mfile2.h"
#define STACKSAFTEY 256		/* saftey factor for stack checking */

int rsmask;

/* the stack frame looks as follows:
** 
** incoming args
** saved PC (4 bytes)
** saved old FP (%a6) (4 bytes)
** fp->
** saved regs (the mask is M%n)
** autos (maxboff keeps track of this)
** temps (maxtemp keeps track of this)
** outgoing args (4 byte hole left for the first one )
** sp->
** 
** the first saved reg is at S%n(%fp)
** the last auto ends at S%n(%fp)
** the last temp ends at T%n(%fp)
** the first incoming arg is at 8(%fp)
** F%n is the size of the frame (argument to "link" instruction)
*/

lineid( l, fn )
char *fn;
{
	/* identify line l and file fn */
	printf( "%s  line %d, file %s\n", COMMENTSTR, l, fn );
}

eobl2()
{
	register off;
	register rsave;
	register i, m;
	/* count bits in rsmask */
	for( rsave=i=0, m=1; i<16; ++i, m<<=1 )
	{
		if( m&rsmask ) rsave += 4;
	}
	maxboff /= SZCHAR;
	maxtemp /= SZCHAR;
	maxarg /= SZCHAR;

	off = rsave;
	printf( "	set	S%%%d,%d\n", ftnno, -off );
	off += maxboff;
	printf( "	set	T%%%d,%d\n", ftnno, -off );
	off += maxtemp+4;  /* 4 byte hole for last arg */
	printf( "	set	F%%%d,%d\n", ftnno, -off );
	if( (off+STACKSAFTEY) >= (1L<<15) )
	{
		uerror("Too many local variables, reduce local space.");
	}
	printf( "	set	M%%%d,0x%.4x\n", ftnno, rsmask );
	rsmask = 0;
}

char *rnames[] =
{
	/* keyed to register number tokens */
	"0", "1", "2", "3", "4", "5", "6", "7", "?8", "?9", "?10"
};

special( sh, p )
NODE *p;
register sh;
{
	register int val;

	sh &= SVMASK;		/* clear all but the special code */
	switch (sh)
	   {
	   case 1:		/* power of 2 */
	   case 2:		/* sum of 2 powers of 2 */
	   case 3:		/* difference of 2 powers of 2 */

		if (p->tn.op != ICON || p->tn.name != (char *) 0)
		   return(0);
		val = p->tn.lval;
		if (sh == 1)	/* power of 2 */
		   return (val > 0 && (val & (val-1)) == 0);
		else if (sh == 2 && val > 0)	/* sum of powers of 2 */
		   {
		   /* (word) multiply faster than shift by >12 on 68010 */
		   if (!(p->tn.type & (TLONG | TULONG)) && val > 4096)
			return(0);
		   return ((val &= (val-1)) != 0 && (val & (val-1)) == 0);
		   }
		else if (sh == 3 && val > 0)	/* diff of powers of 2 */
		   {
		   /* (word) multiply faster than shift by >12 on 68010 */
		   if (!(p->tn.type & (TLONG | TULONG)) && val > 4096)
			return(0);
		   while ((val & 1) == 0)	/* get to '00..011...1' */
		      val >>= 1;
		   return ((val & (val+1)) == 0);
		   }
		else 
		   return(0);
		break;

	   default:
		e2print(p);
		cerror( "special called!" );
		return( INFINITY );
	   }
}

static int	toff;
static int	toffbase;

popargs( size )
register size;
{
	/* pop arguments from the stack */
	toff -= size/2;
	if( toff == 0 )
	{
		size -= 2*toffbase;
		toffbase = toff = 0;
	}
	if(!size)
		return;

	printf( "	add.l	&%d,%%sp\n", size );
}


/* print the address of p+d */
next_address( p, d )
register NODE *p;
register int d;	/* the displacement */
{
       register NODE *ptemp,*r,*l;

       if (d) {
	  ptemp = talloc();
	  *ptemp= *p;
	  switch ( ptemp->in.op )
	  {
	    case REG:
	       ptemp->tn.rval += 1;
	       break;
	    case TEMP: 	/* temps may be in registers */
	       if (ptemp->tn.rval) 
		  goto bad;
	       /* else FALLTHRU */
	    case VAUTO:
	    case VPARAM:
	       ptemp->tn.lval += d;
	       break;
	    case NAME:
	       printf("%d+", d);	/* d+L%xx */
	       break;
	    case STAR:
	       p = p->in.left;
	       switch( p->in.op )
	       {
	       case REG:
				   /* form ( STAR (PLUS REG ICON(4) ) */
		  l = ptemp->in.left = talloc();
		  *l = *p;		/* copy type and other info */
		  l->tn.op = PLUS;
		  *(l->in.left = talloc())  = *p;	/* copy REG node */
		  r = l->in.right = talloc();	/* node for icon */
		  *r = *p;				/* copy type and other info */
		  r->tn.op = ICON;
		  r->tn.lval = d;
		  break;

	       case MINUS:
		  if (p->in.right->in.op != ICON)
			goto bad;
		  d = -d;
		  /* FALL THRU */
	       case PLUS:
		  if (p->in.right->in.op == ICON)
		     {
				   /* Make copy of address tree with ICON */
				   /* modified by adding d */
		     l = ptemp->in.left = talloc();
		     *l = *p;		/* copy PLUS node */
		     r = l->in.right = talloc();
		     *r= *(p->in.right);	/* copy ICON node */
		     if ( r->in.op != ICON ) goto bad;
		     r->tn.lval += d;
		     l->in.left = talloc();
		     l = l->in.left;
		     p = p->in.left;
		     *l = *p;		/* make copy of either REG or REG+REG */
		     if( l->in.op == REG ) break;	/* Done with copy */
		     if( l->in.op != PLUS) goto bad;	/* Bad address tree */
		     *(l->in.right=talloc()) = *(p->in.right); /* Copy REG */
		     l->in.left = talloc();
		     l = l->in.left;
		     p = p->in.left;
		     *l  = *p;				/* Copy REG or CONV */
		     if( l->in.op == REG ) break;	/* Done with copy */
		     if( l->in.op != CONV) goto bad;	/* Bad address tree */
		     *(l->in.left=talloc())  = *(p->in.left);  /* Copy REG */
		     break;

		     }
		  else if (p->in.right->in.op == REG)
		     {
				   /* Make copy of address tree with ICON */
				   /* adding d */
		     l = ptemp->in.left = talloc();
		     *l = *p;		/* copy PLUS node */
		     r = l->in.right = talloc();
		     *r = *p;
		     r->tn.op = ICON;
		     r->tn.lval = d;
		     l->in.left = talloc();
		     l = l->in.left;
		     *l = *p;		/* copy PLUS node */
		     r = l->in.right = talloc();
		     *r= *(p->in.right);	/* copy REG node */
		     l->in.left = talloc();
		     *(l->in.left) = *(p->in.left); /* make copy of REG */
		     break;
		     }

	       default: goto bad;
	       };
	       break;
	    default:
	       goto bad;
	  } /* switch */
	  adrput( ptemp );
	  tfree( ptemp );
       }
       else
	  adrput(p);
       return;
bad:
       e2print( ptemp );
       cerror("Illegal byte address (ZB)\n");
       return(-1);
}

zzzcode( p, ppc , q)
register NODE *p;
register char **ppc;
OPTAB *q;
{
	register int c;
	register int ii;
	register NODE *newp, *tnode;

	static int labels[10];	/* labels for use in patterns */

	switch( c= *++(*ppc) )
	{

	case 'p':
		popargs( 4 );
		return;
	case 'q':
		popargs( 8 );
		return;

	case 'c':
		popargs( p->stn.argsize/SZCHAR );
		return;

	case '1':
		if( toff )
		{
			putchar( '-' );
		}
		printf( "(%%sp)" );
		if( !toff ) toffbase = 1;
		++toff;
		return;

	case '2':
		if( toff )
		{
			putchar( '-' );
		}
		printf( "(%%sp)" );
		if( !toff ) toffbase = 2;
		toff+=2;
		return;

	case '0':
		/* like 2 but doesn't print -(%%sp) */
		/* used for pea. leaves toffbase at 0, also */
		toff += 2;
		return;

	case 's':  /* like s, but needs value afterwards */
	case 'S':  /* structure assignment */
		if( p->in.op == STASG )
		{
			stas( p, p->stn.stsize, c );
		}
		else if( p->in.op == STARG )
		{
			/* store an arg onto the stack */
			star( p->in.left, p->stn.stsize ,q);
		}
		else
		{
			e2print(p);
			cerror( "ZS bad" );
		}
		break;

	case 'z':  /* adjust register on right */
		{
			register long adj = p->stn.stsize;
			if( adj == SZSHORT || adj == SZLONG ) return;
			adj = ((adj+SZSHORT)/SZLONG) - 1;
			if( adj )
			{
				printf( "\tsub.l\t&%ld,%%a%d\n",
					(SZLONG/SZCHAR)*adj,
				p->in.right->tn.rval );
			}
		}
		return;

	case 'I':
		cbgen( p->bn.lop, p->bn.label, c );
		return;

	case 'i':
		c = p->in.left->in.left->tn.rval;  /* register number */
		dbccgen( p->bn.lop, p->bn.label, c );
		return;

	case 'b':
		++*ppc;
		p = getadr(p,ppc);
		if((ii = ispow2(p->tn.lval))>=0)
		{
			printf( "&%d", ii );
			break;
		}
		cerror( "Zb but not power of 2" );

	case 'n':
		++*ppc;
		next_address( getadr(p,ppc), 4 );
		break;

	case 'f':	/* shift by number of lowest bit on */
		++*ppc;
		newp = getadr(p, ppc);
		c = newp->tn.lval;
		if (c > 0)
		   {
		   for (ii = 0; (c & 1) == 0; ii++)
			   c >>= 1;
		   doshift(ii, ppc, p);
		   }
		else
		   cerror( "Zf on zero or negative value" );
		break;

	case 'd':	/* shift by diff. of powers of 2 (for sums of powers) */
		++*ppc;
		newp = getadr(p, ppc);
		c = newp->tn.lval;
		if (c > 0)
		   {
		   while ((c & 1) == 0)
			   c >>= 1;
		   if ((c >>= 1) != 0 && (c & (c-1)) == 0) /* 1 bit left */
		      {
		      for (ii = 1; (c & 1) == 0; ii++)
			      c >>= 1;
		      doshift(ii, ppc, p);
		      break;
		      }
		   }
		cerror( "Zd on improper value" );
		break;

	case 'D':	/* shift by diff. of powers of 2 (for diff.) */
		++*ppc;
		newp = getadr(p, ppc);
		c = newp->tn.lval;
		if (c > 0)
		   {
		   while ((c & 1) == 0)	/* get to '00...011...1' */
			   c >>= 1;
		   if (c != 0 && (c & (c+1)) == 0) /* string of 1's */
		      {
		      for (ii = 0; (c & 1); ii++) /* count 1's */
			      c >>= 1;
		      doshift(ii, ppc, p);
		      break;
		      }
		   }
		cerror( "ZD on improper value" );
		break;

	case 'B':	/* byte address for a bset, bclr */
		++*ppc;
		newp = getadr(p, ppc);
		if((ii = ispow2(newp->tn.lval)) < 0)
		   {
		   cerror( "ZB but not power of 2" );
		   return;
		   }
		printf("&%d,", ii & 7);	/* output the bit number (mod 8) */
		++*ppc;
		newp = getadr(p, ppc);
		if (newp->tn.type & (TLONG | TULONG))	/* calc displacement */
		   c = 3 - (ii >> 3);
		else if (newp->tn.type & (TSHORT | TUSHORT))
		   c = 1 - (ii >> 3);
		else
		   c = 0;
		next_address(newp, c);
		break;

	case 't':	/* take address of temp */
		++*ppc;
		newp = getadr(p, ppc);		/* the temp */
		++*ppc;
		p = getadr(p, ppc);		/* the result reg */
		if (newp->tn.rval == 0)		/* memory temp? */
		   {
		   printf("\n\tlea.l\t");
		   adrput(newp);
		   putchar(',');
		   adrput( p );
		   putchar('\n');
		   break;
		   }
		tnode = talloc();		/* the memory temp */
		*tnode = *newp;			/* copy stuff */
		tnode->tn.lval = freetemp(1);	/* get a memory temp */
		tnode->tn.lval = BITOOR(tnode->tn.lval);
		tnode->tn.rval = 0;		/* not in register */
		if (tnode->tn.type & (TPOINT | TLONG | TULONG | 
		    TFLOAT | TDOUBLE))
		   c = 'l';			/* move long */
		else if (tnode->tn.type & (TSHORT | TUSHORT))
		   c = 'w';			/* move word */
		else 
		   c = 'b';			/* move byte */
		printf("\tmov.%c\t", c);
		adrput(newp);
		putchar(',');
		adrput( tnode );
		printf("\n\tlea.l\t");
		adrput(tnode);
		putchar(',');
		adrput( p );
		putchar('\n');
		tfree( tnode );
		break;

	case 'L':	/* first use of label in pattern */
	case 'l':	/* other uses of label in pattern */
		ii = *++(*ppc) - '0';	/* label number */
		if (ii < 0 || ii > 9)
			cerror( "illegal %c code: %c", c, ii+'0');
		if (c == 'L')		/* define it */
			labels[ii] = getlab();
#ifdef TWOPASS
		if (labels[ii] < 0 )
			printf("L%%_%d", -labels[ii] );
	  	else
#endif
		printf("L%%%d", labels[ii]);
		break;

	default:
		e2print(p);
		cerror( "illegal zzzcode: %c", c );
	}
}

/* put a shift left of size s of register A? from string "sA?" */
doshift (n, ppc, p)
	register int n;
	register char **ppc;
	register NODE *p;
	{
	register char size;

	size= *++(*ppc);
	++*ppc;
	p = getadr(p, ppc);
	if (n == 1)
	   {
	   printf("\tadd.%c\t", size);
	   adrput(p);
	   putchar(',');
	   adrput(p);
	   putchar('\n');
	   }
	else if (n > 1)
	   {
	   printf("\tasl.%c\t&%d,", size, n);
	   adrput(p);
	   putchar('\n');
	   }
	/* null for shift of 0 */
	}

star( p, sz , q)
register NODE *p;
register sz;
OPTAB *q;
{
	/* put argument onto the stack */
	/* p is a REG */

	sz /= SZCHAR;

	if( p->tn.op != REG ) uerror( "star of non-REG" );
	while( (sz -= 4) >= 0 )
	{
		printf( "\tmov.l\t%d", sz );
		expand( p, INREG, "U.,Z2\n" , q);
	}
	if( sz == -2 )
	{
		expand( p, INREG, "\tmov.w\t0U.,Z1\n" , q);
	}
	else if( sz != -4 )
	{
		e2print(p);
		cerror( "ZS sz %d", sz );
	}
}

stas( p, sz, c )
register NODE *p;
register sz,c;
{
	/* p is an STASG node; left and right are regs */
	/* all structures are aligned on int boundaries */

	register rn, ln, zz;

	rn = p->in.right->tn.rval;
	if( c == 's' )
	{
		/* lhs is a temp moved into a reg: kluge! */
		ln = getlr( p, '1' )->tn.rval;  /* A1 */
	}
	else ln = p->in.left->tn.rval;

	zz = (sz /= SZCHAR);
	if (zz > 32)	/* move structs > 32 bytes in loop */
		{
		register int l;
		register int regno;	/* holds regster number for count */
		
		regno = getlr( p, '1' )->tn.rval;  /* A1 */
		l = getlab();
		printf("\tmov.l\t&%d,%%d%d\n", (zz-1)/4 - 1, regno);
#ifdef TWOPASS
		if (l < 0 )
			printf("L%%_%d:\n", -l );
	  	else
#endif
		printf("L%%%d:\n", l);
		printf("\tmov.l\t(%%a%d)+,(%%a%d)+\n",  rn, ln );
		printf("\tdbra\t%%d%d,", regno);
#ifdef TWOPASS
		if (l < 0 )
			printf("L%%_%d\n", -l );
	  	else
#endif
		printf("L%%%d\n", l);
		sz -= ((zz-1)/4) * 4;
		}
	else		/* produce in-line moves */
		{
		for( ; sz > 4; sz -= 4 )
			printf( "\tmov.l\t(%%a%d)+,(%%a%d)+\n", rn, ln );
		}

	if( sz == 4 ) printf( "\tmov.l\t(%%a%d),(%%a%d)\n", rn, ln );
	else if( sz == 2 ) printf( "\tmov.w\t(%%a%d),(%%a%d)\n", rn, ln );
	else
	{
		e2print(p);
		cerror( "ZS sz %d", sz );
	}
	if( ln > 1 )
	{
		/* restore clobbered register variable */
		if( zz > sz ) printf( "\tsub.l\t&%d,%%a%d\n", zz-sz, ln);
	}
	if( rn > 1 )
	{
		/* restore clobbered register variable */
		if( zz > sz ) printf( "\tsub.l\t&%d,%%a%d\n", zz-sz, rn);
	}
}

conput( p )
register NODE *p;
{
	switch( p->in.op )
	{
	case ICON:
		acon( p );
		return;
	case REG:
		if( TPOINT&p->tn.type ) printf( "%%a" );
		else printf( "%%d" );
		printf( "%s", rnames[p->tn.rval] );
		if( p->tn.rval > 7 ) werror( "bad register output" );
		return;
	default:
		e2print(p);
		cerror( "illegal conput" );
	}
}

insput( p )
NODE *p;
{
	e2print(p);
	cerror( "insput" );
}

upput( pp )
register NODE *pp;
{
	/* output the address of the second word in the
	** pair pointed to by p (for LONGs)
	*/
	register long v;
	register NODE *r, *l, *p;

	v=0;
	p = pp;
	if( p->tn.op == PLUS )
	{
		v=0;
		r = p->in.right;
		l = p->in.left;
		if( r->tn.op == ICON &&  l->tn.op == PLUS )
		{
			v = r->tn.lval;
			p = l;
			r = p->in.right;
			l = p->in.left;
		}
		if( r->tn.op == REG )
		{
			l = p->in.left;
			if( l->tn.op == REG )
			{
				printf( "%ld(%%a%d,%%d%d.l)", v, r->tn.rval,
					l->tn.rval );
				return;
			}
			if( l->tn.op != CONV ) goto ill;
			l = l->in.left;
			if( l->tn.op != REG ) goto ill;
			printf("%ld(%%a%d,%%d%d.w)",v,r->tn.rval,l->tn.rval);
			return;
		}
		if( r->tn.op != ICON ) goto ill;
		v = r->tn.lval;
		p = p->in.left;
	}
	else if( p->tn.op == MINUS )
	{
		r = p->in.right;
		if( r->tn.op != ICON || r->in.name != (char *) 0 ) goto ill;
		v = -r->tn.lval;
		p = p->in.left;
	}
	else if( p->tn.op == ASG MINUS )
	{
		r = p->in.right;
		if( r->tn.op != ICON ) goto ill;
		r = p->in.left;
		if( r->tn.op != REG ) goto ill;
		/* always do the side effect */
		else printf( "-(%%a%s)", rnames[r->tn.rval] );
		sideff = 1;  /* cream it */
		return;
	}
	else if( p->tn.op == INCR )
	{
		r = p->in.right;
		if( r->tn.op != ICON ) goto ill;
		r = p->in.left;
		if( r->tn.op != REG ) goto ill;
		if( sideff ) printf( "(%%a%s)+", rnames[r->tn.rval] );
		else printf( "(%%a%s)", rnames[r->tn.rval] );
		return;
	}
	if( p->tn.op != REG ) goto ill;
	if( v )
		printf( "%ld(%%a%s)", v, rnames[p->tn.rval] );
	else
		printf( "(%%a%s)", rnames[p->tn.rval] );
	return;
ill:
	e2print(pp);
	cerror( "illegal address: upput" );
}

adrput( p )
register NODE *p;
{
	/* output an address, with offsets, from p */

	while( p->in.op == FLD || p->in.op == CONV )
	{
		p = p->in.left;
	}
	switch( p->in.op )
	{

	case NAME:
		acon( p );
		sideff = 0;
		return;

	case ICON:
		/* addressable value of the constant */
		printf( "&" );
		acon( p );
		sideff = 0;
		return;

	case REG:
		conput( p );
		sideff = 0;
		return;

	case STAR:
		upput( p->in.left );
		return;

	case VAUTO:
		printf( "%ld+S%%%d(%%fp)", p->tn.lval, ftnno );
		sideff = 0;
		return;

	case VPARAM:
		printf( "%ld(%%fp)", p->tn.lval );
		sideff = 0;
		return;

	case TEMP:
		if (p->tn.rval)
		   {
		   register int i = (int)p->tn.lval;

		   printf( "%%%c%d", ((i>7) ? 'a' : 'd'), i&7);
		   }
		else
		   printf( "%ld+T%%%d(%%fp)", p->tn.lval, ftnno );
		sideff = 0;
		return;

	default:
		e2print(p);
		cerror( "illegal address" );
		return;
	}
}

acon( p )
register NODE *p;
{
	/* print out a constant */

	if( p->in.name == (char *) 0)
	{
		/* constant only */
		printf( "%ld", p->tn.lval);
	}
	else if( p->tn.lval == 0 )
	{
		/* name only */
		printf( "%s", p->in.name );
	}
	else
	{
		/* name + offset */
		printf( "%s+%ld", p->in.name , p->tn.lval );
	}
}

static char	*ccbranches[] =
{
	"	beq	",
	"	bne	",
	"	ble	",
	"	blt	",
	"	bge	",
	"	bgt	",
	"	bls	",
	"	blo	",
	"	bhs	",
	"	bhi	",
};

cbgen( o, lab, mode )
register o,lab;
{
	/* printf conditional and unconditional branches */
	if( o == 0 ) printf( "	br	" );
	else if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
	else printf( ccbranches[o-EQ] );
#ifdef TWOPASS
	if (lab < 0 )
		printf("L%%_%d\n", -lab );
	  else
#endif
		printf("L%%%d\n", lab );
}

static char	*dbccops[] =
{
	"	dbeq	%%d%d,",
	"	dbne	%%d%d,",
	"	dble	%%d%d,",
	"	dblt	%%d%d,",
	"	dbge	%%d%d,",
	"	dbgt	%%d%d,",
	"	dbls	%%d%d,",
	"	dblo	%%d%d,",
	"	dbhs	%%d%d,",
	"	dbhi	%%d%d,",
};

dbccgen( o, lab, reg )
register o,reg,lab;
{
	/* printf conditional and unconditional branches */
	if( o == 0 ) printf( "	dbt	%%d%d,", reg );
	else if( o > UGT ) cerror( "bad conditional branch: %s", opst[o] );
	else printf( dbccops[o-EQ], reg );
#ifdef TWOPASS
	if ( lab < 0 )
		printf("L%%_%d\n", -lab );
	   else
#endif
		printf("L%%%d\n", lab );
}

/* see if an unused register can be used for a temp */

#define DREGS (0xff & (0xff<<NRGS))	/* the available D registers */
#define AREGS (0x3f00 & (0xff00<<NRGS))	/* the available A registers */

int regtmp = 0;

tempreg(rnum, nlongs, type)
	register int *rnum;	/* returns regiser number if one allocated */
	register int nlongs;	/* number of long words needed for the temp */
	register TWORD type;	/* type of the value to be put in the temp */
	{
	register int r, rbit, mask;
	extern int regvar;

	if (nlongs != 1 || ((type & TSTRUCT) && !(type & TPOINT)))
		return (0);	/* can't have > 1 reg or structures */
	mask = ~(regvar | regtmp);	/* consider all unallocated registers */

	if ((type & TPOINT) && (mask & AREGS))
		mask &= AREGS;	/* prefer pointers in A registers */
				/* but can be put in D registers if no A's */
	else
		mask &= DREGS;	/* non-pointers restricted to D registers */

	if (mask == 0)
		return(0);	/* none available! */

	for (r = 0, rbit = 1; (mask & rbit) == 0; r++)
		rbit <<= 1;
	/* mark it used */
	regtmp |= rbit;
	SETREGS(rbit);
	*rnum = r;
	return (1);	/* success! */
	}

/*
 *	BEGIN: ported from VAX pcc to make two pass version
 */

# ifdef TWOPASS

main( argc, argv ) char *argv[]; {
	return( mainp2( argc, argv ) );
	}

/*
 * code form trees.c to make twopass version 
 */


# ifndef MYLABELS
getlab()
{
	static crslab = -10;
	return( --crslab );
}
# endif

tprint( t )
register TWORD t; 
{
	 /* output a nice description of the type of t */
	static char * tnames[] = 
	{
		"undef",
		"farg",
		"char",
		"short",
		"int",
		"long",
		"float",
		"double",
		"strty",
		"unionty",
		"enumty",
		"moety",
		"uchar",
		"ushort",
		"unsigned",
		"ulong",
		"?", "?"
	};

	for(;; t = DECREF(t) )
	{
		if( ISPTR(t) ) printf( "PTR " );
		else if( ISFTN(t) ) printf( "FTN " );
		else if( ISARY(t) ) printf( "ARY " );
		else 
		{
			printf( "%s", tnames[t] );
			return;
		}
	}
}
/*
 * 	Copied form common/optim to make twopass version.
 */

ispow2( c ) 
register CONSZ c; 
{
	register i;
	if( c <= 0 || (c&(c-1)) ) return(-1);
	for( i=0; c>1; ++i) c >>= 1;
	return(i);
}

deflab(lab)
	int lab;
	{
	if (lab < 0)
		printf("L%%_%d:\n", -lab);
	else
		printf("L%%%d:\n", lab);
	}

# endif TWOPASS


/*
 *	END: ported from VAX pcc to make two pass version
 */

#ifdef ITREES
/*
/*	copied from local.c for ITREES mode
*/

/* output the prolog code for a procedure */
doprolog(p)
	register NODE *p;
	{

#ifdef	PROFILE
	/*
	** code to provide profiling via prof(1)
	**
	*/
	if( p->tn.lval & 1)	/* do profiling */
	{
		register int i;

		i = getlab();
#ifdef TWOPASS
		if (i<0)
			i = -i;
#endif
		printf("	mov.l	&L%%%c%d,%%a0\n", 'P', i);
		printf("	jsr	mcount%%\n");
		printf("	data\n");
		printf("	even\n");
		printf("L%%%c%d:\n", 'P', i);
		printf("	long	0\n");
		printf("	text\n");
	}
#endif	PROFILE

#ifdef	STKCK

	/*
	** code needed for M68000 to check stack growth
	** against current splimit%.  if we are about to
	** exceed current limit, ask system for more stack.
	**
	*/

	if( p->tn.lval & 2)	/* if stack checking requested */
	{
		register int spgrow_lbl;

		spgrow_lbl = getlab();
		printf( "	lea.l	F%%%d-256(%%sp),%%a0\n", p->tn.rval );
		printf( "	cmp.l	%%a0,splimit%%\n" );
#ifdef TWOPASS
		if (spgrow_lbl < 0)
		    printf( "	bhi.b	L%%_%d\n", -spgrow_lbl );
		else
#endif
		printf( "	bhi.b	L%%%d\n", spgrow_lbl );
		printf( "	jsr	spgrow%%\n" );
		deflab( spgrow_lbl );
	}
#endif STKCK

	/* routine prolog */

	printf( "	link	%%fp,&F%%%d\n", p->tn.rval );
	printf( "	movm.l	&M%%%d,S%%%d(%%fp)\n", p->tn.rval, p->tn.rval );
	}

/* output the epilog code for a procedure */

doepilog(p)
	register NODE *p;
	{
	if( p->tn.lval )	/* returns structure */
	{
		  /* copy output (in r0) to caller */
		printf( "	mov.l	&__StRet,%%a0\n" );
	}

	if ( ISPTR(p->tn.rval) || p->tn.lval )   /* returns ptr or structure */
	{
		/* copy A0 to D0 for more reliability in returns */
		/* from functions that return pointers.		 */
		printf("	mov.l	%%a0,%%d0\n");
	}
	printf( "	movm.l	S%%%d(%%fp),&M%%%d\n", ftnno, ftnno );
	printf( "	unlk	%%fp\n" );
	printf( "	rts\n" );
	}

/* output a switch structure passed from the front end */

doswitch(t)
	register NODE *t;
	{
	register int range, i, j, n, dlab, swlab;
	struct sw {
	   CONSZ sval;
	   int slab;
	   } swtab[SWITSZ];
	register struct sw *p = swtab;

	n = 0;
	p[0].slab = -1;
	for (t = t->in.left; t->tn.op != SWEND; t = t->in.left)
	   if (t->in.op == SWDEF)
	      p[0].slab = t->tn.rval;
	   else if (t->in.op == SWCASE)
	      {
	      n++;
	      p[n].slab = t->tn.rval;
	      p[n].sval = t->tn.type;
	      }

	if( n == 0 )
		range = 0;
	else
		range = p[n].sval - p[1].sval;

	/* table is faster & shorter for density > 20% - JGH */
	/*    direct: overhead=2 (for br); increment: 5*n */
	/*    table:  overhead=15; increment: 1*(range+1) */

	if( (range>=0) && (range+14 <= 5*n) )
	{
		 /* implement a direct switch */
		dlab = p->slab >= 0 ? p->slab : getlab();

		if( p[1].sval )
		{
#ifdef	INT16BIT
			printf( "	sub.w	&%ld,%%d0\n", p[1].sval );
#else
			printf( "	sub.l	&%ld,%%d0\n", p[1].sval );
#endif
		}

		/* note that this is a logical compare; it thus checks
		** for numbers below range as well as out of range.
		*/
#ifdef	INT16BIT
		printf( "	cmp.w	%%d0,&%ld\n", range );
#else
		printf( "	cmp.l	%%d0,&%ld\n", range );
#endif

#ifdef TWOPASS
		if (dlab < 0)
			printf( "	bhi	L%%_%d\n", -dlab );
		else
#endif
		printf( "	bhi	L%%%d\n", dlab );

		printf( "	add.w	%%d0,%%d0\n" );
/* Assume "swbeg &n" takes 4 bytes:  2 for swbeg, 2 for # of cases */
		printf( "	mov.w	10(%%pc,%%d0.w),%%d0\n" );
		printf( "	jmp	6(%%pc,%%d0.w)\n" );
		printf(	"	swbeg	&%d\n", range+1 );
				/* range runs from zero, so add 1 for cases */
		deflab( swlab = getlab() );


		/* output table */

		for( i=1, j=p[1].sval;  i<=n;  ++j )
		{
#ifdef TWOPASS
			register result= (j == p[i].sval) ? p[i++].slab : dlab;
			if (result < 0)
			   printf( "\tshort\tL%%_%d-L%%_%d\n", -result, -swlab);
			else
			   printf( "\tshort\tL%%%d-L%%_%d\n", result, -swlab);
#else
			printf( "	short	L%%%d-L%%%d\n",
				(j == p[i].sval) ? p[i++].slab : dlab, swlab );
#endif
		}

		if( p->slab < 0 )
			deflab( dlab );
		return;
	}
	/* out for the moment
	** if( n >= 4 )
	**	werror( "inefficient switch: %d, %d", n, (int) (range/n) );
	*/

	/* simple switch code */

	for( i=1; i<=n; ++i )
	{
		/* already in 0 */
		if( p[i].sval )
#ifdef	INT16BIT
			printf( "	cmp.w	%%d0,&%ld\n", p[i].sval );
		else
			printf( "	tst.w	%%d0\n" );
#else
			printf( "	cmp.l	%%d0,&%ld\n", p[i].sval );
		else
			printf( "	tst.l	%%d0\n" );
#endif

		printf( "	beq	L%%%d\n", p[i].slab );
	}

	if( p->slab >= 0 )
		printf( "	br	L%%%d\n", p->slab );

	}
#endif ITREES
