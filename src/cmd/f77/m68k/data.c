/*	@(#)data.c	7.1		*/
#include "defs"

/* ROUTINES CALLED DURING DATA STATEMENT PROCESSING */

static char datafmt[] = "%s\t%05ld\t%05ld\t%05ld\t%d" ;

/* another initializer, called from parser */
dataval(repp, valp)
register expptr repp, valp;
{
register int i;
int nrep;
ftnint elen, vlen, range;
register Addrp p;
Addrp nextdata();
enum Boolean isarray;

if(repp == NULL)
	nrep = 1;
else if (ISICON(repp) && repp->constblock.const.ci >= 0)
	nrep = repp->constblock.const.ci;
else
	{
	err("invalid repetition count in DATA statement");
	frexpr(repp);
	goto ret;
	}
frexpr(repp);

if( ! ISCONST(valp) )
	{
	err("non-constant initializer");
	goto ret;
	}

if(toomanyinit) goto ret;
for(i = nrep; i > 0; i -= range)
	{
	if ((p = nextdata(&elen, &vlen, &isarray, &range, i)) == NULL)
		{
		err("too many initializers");
		toomanyinit = YES;
		goto ret;
		}
	else if (isarray)
		{
		if (i <= range)
			range = i;
		setdata(p, valp, elen, vlen, range);
		}
	else
		setdata(p, valp, elen, vlen, 1L);
	frexpr(p);
	}

ret:
	frexpr(valp);
}


Addrp nextdata(elenp, vlenp, isaryp, nelemp, ntodo)
ftnint *elenp, *vlenp, *nelemp;
int ntodo;
enum Boolean *isaryp;
{
register struct Impldoblock *ip;
struct Primblock *pp;
register Namep np;
register struct Rplblock *rp;
tagptr p;
expptr neltp;
register expptr q;
int skip;
ftnint off;

*isaryp = False;
*nelemp = 1;
while(curdtp)
	{
	p = curdtp->datap;
	if(p->tag == TIMPLDO)
		{
		ip = &(p->impldoblock);
		if(ip->implb==NULL || ip->impub==NULL || ip->varnp==NULL)
			fatali("bad impldoblock 0%o", (int) ip);
		if(ip->isactive)
			ip->varvp->const.ci += ip->impdiff;
		else
			{
			q = fixtype(cpexpr(ip->implb));
			if( ! ISICON(q) )
				goto doerr;
			ip->varvp = (Constp) q;

			if(ip->impstep)
				{
				q = fixtype(cpexpr(ip->impstep));
				if( ! ISICON(q) )
					goto doerr;
				ip->impdiff = q->constblock.const.ci;
				frexpr(q);
				}
			else
				ip->impdiff = 1;

			q = fixtype(cpexpr(ip->impub));
			if(! ISICON(q))
				goto doerr;
			ip->implim = q->constblock.const.ci;
			frexpr(q);

			ip->isactive = YES;
			rp = ALLOC(Rplblock);
			rp->rplnextp = rpllist;
			rpllist = rp;
			rp->rplnp = ip->varnp;
			rp->rplvp = (expptr) (ip->varvp);
			rp->rpltag = TCONST;
			}

		if( (ip->impdiff>0 && (ip->varvp->const.ci <= ip->implim))
		 || (ip->impdiff<0 && (ip->varvp->const.ci >= ip->implim)) )
			{ /* start new loop */
			curdtp = ip->datalist;
			goto next;
			}

		/* clean up loop */

		if(rpllist)
			{
			rp = rpllist;
			rpllist = rpllist->rplnextp;
			free( (charptr) rp);
			}
		else
			fatal("rpllist empty");

		frexpr(ip->varvp);
		ip->isactive = NO;
		curdtp = curdtp->nextp;
		goto next;
		}

	pp = (struct Primblock *) p;
	np = pp->namep;
	skip = YES;

	if(p->primblock.argsp==NULL && np->vdim!=NULL)
		{   /* array initialization */
		q = (expptr) mkaddr(np);
		*isaryp = True;
		if (off = typesize[np->vtype] * curdtelt)
			{
			if (np->vtype == TYCHAR)
				off *= np->vleng->constblock.const.ci;
			q->addrblock.memoffset =
			  mkexpr(OPPLUS, q->addrblock.memoffset, mkintcon(off));
			}
		if( (neltp = np->vdim->nelt) && ISCONST(neltp))
			*nelemp = neltp->constblock.const.ci - curdtelt;
		else
			err("attempt to initialize adjustable array");
		if (ntodo < *nelemp)
			{
			skip = NO;
			curdtelt += ntodo;
			}
		}
	else
		q = mklhs( cpexpr(pp) );
	if(skip)
		{
		curdtp = curdtp->nextp;
		curdtelt = 0;
		}
	if(q->headblock.vtype == TYCHAR)
		if(ISICON(q->headblock.vleng))
			*elenp = q->headblock.vleng->constblock.const.ci;
		else	{
			err("initialization of string of nonconstant length");
			continue;
			}
	else	*elenp = typesize[q->headblock.vtype];

	if(np->vstg == STGCOMMON)
		*vlenp = extsymtab[np->vardesc.varno].maxleng;
	else if(np->vstg == STGEQUIV)
		*vlenp = eqvclass[np->vardesc.varno].eqvleng;
	else	{
		*vlenp =  (np->vtype==TYCHAR ?
				np->vleng->constblock.const.ci :
					typesize[np->vtype]);
		if(np->vstg==STGBSS && *vlenp>0)
			np->vstg = STGINIT;
		if(np->vdim)
			*vlenp *= np->vdim->nelt->constblock.const.ci;
		}
	return( (Addrp) q );

doerr:
		err("nonconstant implied DO parameter");
		frexpr(q);
		curdtp = curdtp->nextp;

next:	curdtelt = 0;
	}

return(NULL);
}




setdata(varp, valp, elen, vlen, n)
register Addrp varp;
ftnint elen, vlen, n;
register Constp valp;
{
union Constant con;
register int type;
register ftnint k;
int i, valtype, nperline;
ftnint offset, inclen;
char *dataname(), *varname;

varname = dataname(varp->vstg, varp->memno);
offset = varp->memoffset->constblock.const.ci;
inclen = elen * NPERLINE;
type = varp->vtype;
valtype = valp->vtype;
if(type!=TYCHAR && valtype==TYCHAR)
	{
	if(! ftn66flag)
		warn("non-character datum initialized with character string");
	varp->vleng = ICON(typesize[type]);
	varp->vtype = type = TYCHAR;
	}
else if( (type==TYCHAR && valtype!=TYCHAR) ||
	 (cktype(OPASSIGN,type,valtype) == TYERROR) )
	{
	err("incompatible types in initialization");
	return;
	}
if(type == TYADDR)
	con.ci = valp->const.ci;
else if(type != TYCHAR)
	{
	if(valtype == TYUNKNOWN)
		con.ci = valp->const.ci;
	else	consconv(type, &con, valtype, &valp->const);
	}

switch(type)
	{
	case TYLOGICAL:
		type = tylogical;
	case TYSHORT:
	case TYLONG:
		for (k = n / NPERLINE; k; --k)
			{
			dataline(varname, offset, vlen, inclen, type);
			prnicon(initfile, type, con.ci, NPERLINE);
			offset += inclen;
			}
		if (n %= NPERLINE)
			{
			dataline(varname, offset, vlen, n * elen, type);
			prnicon(initfile, type, con.ci, (int) n);
			}
		break;

	case TYADDR:
		for (k = n / NPERLINE; k; --k)
			{
			dataline(varname, offset, vlen, inclen, type);
			prnacon(initfile, con.ci, NPERLINE);
			offset += inclen;
			}
		if (n %= NPERLINE)
			{
			dataline(varname, offset, vlen, n * elen, type);
			prnacon(initfile, con.ci, (int) n);
			}
		break;

	case TYDREAL:
	case TYREAL:
		nperline = (type == TYREAL) ? NPERLINE : NO2PERLINE;
		inclen = elen * nperline;
		for (k = n / nperline; k; --k)
			{
			dataline(varname, offset, vlen, inclen, type);
			prnrcon(initfile, type, con.cd[0], nperline);
			offset += inclen;
			}
		if (n %= nperline)
			{
			dataline(varname, offset, vlen, n * elen, type);
			prnrcon(initfile, type, con.cd[0], (int) n);
			}
		break;
	case TYCOMPLEX:
		type = TYREAL;
		goto cplx;
	case TYDCOMPLEX:
		type = TYDREAL;
	cplx:
		nperline = (type == TYREAL) ? NO2PERLINE : NO4PERLINE;
		inclen = elen * nperline;
		for (k = n / nperline; k; --k)
			{
			dataline(varname, offset, vlen, inclen, type);
			prncxcon(initfile,type, con.cd[0], con.cd[1], nperline);
			offset += inclen;
			}
		if (n %= nperline)
			{
			dataline(varname, offset, vlen, n * elen, type);
			prncxcon(initfile, type, con.cd[0], con.cd[1], (int) n);
			}
		break;
	case TYCHAR:
		while (n--)
			{
			k = valp->vleng->constblock.const.ci;
			if(elen < k)
				k = elen;

			for(i = 0 ; i < k ; ++i)
				{
				dataline(varname, offset++, vlen,
				  typesize[TYCHAR], TYCHAR);
				fprintf(initfile, "\t%d\n",
					valp->const.ccp[i]);
				}
			k = elen - valp->vleng->constblock.const.ci;
			if(k > 0)
				{
				dataline(varname, offset, vlen,
				  typesize[TYBLANK], TYBLANK);
				fprintf(initfile, "\t%d\n", k);
				offset += k;
				}
			}
		break;

	default:
		badtype("setdata", type);
	}

}



/*
   output form of name is padded with blanks and preceded
   with a storage class digit
*/
char *dataname(stg,memno)
int stg, memno;
{
static char varname[XL+2];
register char *s, *t;
char *memname();

varname[0] = (stg==STGCOMMON ? '2' : (stg==STGEQUIV ? '1' : '0') );
s = memname(stg, memno);
for(t = varname+1 ; *s ; )
	*t++ = *s++;
while(t < varname+XL+1)
	*t++ = ' ';
varname[XL+1] = '\0';
return(varname);
}





frdata(p0)
chainp p0;
{
register struct Chain *p;
register tagptr q;

for(p = p0 ; p ; p = p->nextp)
	{
	q = p->datap;
	if(q->tag == TIMPLDO)
		{
		if(q->impldoblock.isbusy)
			return;	/* circular chain completed */
		q->impldoblock.isbusy = YES;
		frdata(q->impldoblock.datalist);
		free( (charptr) q);
		}
	else
		frexpr(q);
	}

frchain( &p0);
}



dataline(varname, offset, vlen, inclen, type)
char *varname;
ftnint offset, vlen, inclen;
int type;
{
fprintf(initfile, datafmt, varname, offset, vlen, inclen, type);
}
