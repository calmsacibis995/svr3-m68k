/*	@(#)proc.c	7.2		*/
#include "defs"
#include "machdefs"

#ifdef SDB
#	include <a.out.h>
char	*sdb_func_name;
#	if TARGET == PDP11
char *stabline();
#	endif
#endif

/* start a new procedure */

newproc()
{
if(parstate != OUTSIDE)
	{
	execerr("missing end statement", CNULL);
	endproc();
	}

parstate = INSIDE;
procclass = CLMAIN;	/* default */
}



/* end of procedure. generate variables, epilogs, and prologs */

endproc()
{
struct Labelblock *lp;

if(parstate < INDATA)
	enddcl();
if(ctlstack >= ctls)
	err("DO loop or BLOCK IF not closed");
for(lp = labeltab ; lp < labtabend ; ++lp)
	if(lp->stateno!=0 && lp->labdefined==NO)
		errstr("missing statement number %s", convic(lp->stateno) );

epicode();
procode();
donmlist();
dobss();
prdbginfo();

#if FAMILY == PCC
	putbracket( 0 );
#endif

procinit();	/* clean up for next procedure */
}



/* End of declaration section of procedure.  Allocate storage. */

enddcl()
{
register struct Entrypoint *ep;

parstate = INEXEC;
docommon();
doequiv();
docomleng();
for(ep = entries ; ep ; ep = ep->entnextp)
	doentry(ep);
}

/* ROUTINES CALLED WHEN ENCOUNTERING ENTRY POINTS */

/* Main program or Block data */

startproc(progname, class)
struct Extsym * progname;
int class;
{
register struct Entrypoint *p;
char *s;
#ifdef SDB
extern int fnstline;
#endif

p = ALLOC(Entrypoint);
#if TARGET==M68K
p->entrylabel = getlab();
p->backlabel = getlab();
#else
p->entrylabel = newlab();
p->backlabel = newlab();
#endif
if (progname)
	s = nounder(XL, procname = progname->extname);
else
	s = NULL;
if(class == CLMAIN)
	newentry( mkname(5, "MAIN_") );
p->entryname = progname;
entries = p;

procclass = class;
#if TARGET==M68K
retlabel = getlab();
#else
retlabel = newlabel();
#endif
fprintf(diagfile, "   %s", (class==CLMAIN ? "MAIN" : "BLOCK DATA") );
if(progname)
	fprintf(diagfile, " %s", s);
fprintf(diagfile, ":\n");
#ifdef SDB
if(sdbflag && class==CLMAIN)
#if	TARGET == PDP11
	{
	char buff[10];
  	sprintf(buff, "L%d", p->entrylabel);
	prstab("MAIN_", N_FUN, f_lineno, buff);
	p2pass( stabline("MAIN_", N_FNAME, 0, 0) );
	if(progname)
		{
		prstab(nounder(XL,progname->extname), N_ENTRY, f_lineno,buff);
/*		p2pass(stabline(nounder(XL,progname->extname),N_FNAME,0,0));	*/
		}
	}
#	else
	{
	char buff[10], *strcat();
	fnstline = f_lineno - 1;
	buff[0] = '_'; buff[1] = '\0';
	prstab("MAIN__", "MAIN__", C_EXT, T_INT | (DT_FCN << 4));
	sdb_func_name = "MAIN__";
	if (progname)
		prstab(nounder(XL, progname->extname),
		    strcat(buff, varstr(XL, progname->extname)), C_EXT,
		    T_INT | (DT_FCN << 4)); 	/* this type == int f(); */
#		if TARGET==M68K
	p2pi("\tdef\t~bf;\tval\t~;\tscl\t101;\tline\t%d;\tendef",
	    f_lineno);
#		else
  	p2pi("\t.def\t.bf;\t.val\t.;\t.scl\t101;\t.line\t%d;\t.endef",
	    f_lineno);
#		endif
	}
#endif
#endif /* SDB */

#ifdef SDB
#  if TARGET==M68K
if(sdbflag && class==CLBLOCK)
	sdb_func_name="BLOCK_DATA";
#  endif
#endif
if(class == CLMAIN)
	puthead("MAIN__", CLMAIN, p->entrylabel, p->backlabel, s);
else
	puthead(CNULL, CLBLOCK, p->entrylabel, p->backlabel, s);
}

/* subroutine or function statement */

struct Extsym *newentry(v)
register Namep v;
{
register struct Extsym *p;

p = mkext( varunder(VL, v->varname) );

if(p==NULL || p->extinit || ! ONEOF(p->extstg, M(STGUNKNOWN)|M(STGEXT)) )
	{
	if(p == 0)
		dclerr("invalid entry name", v);
	else	dclerr("external name already used", v);
	return(0);
	}
v->vstg = STGAUTO;
v->vprocclass = PTHISPROC;
v->vclass = CLPROC;
p->extstg = STGEXT;
p->extinit = YES;
return(p);
}


entrypt(class, type, length, entry, args)
int class, type;
ftnint length;
struct Extsym *entry;
chainp args;
{
register Namep q;
register struct Entrypoint *p, *ep;

if(class == CLENTRY)
	fprintf(diagfile, "       entry ");
fprintf(diagfile, "   %s:\n", nounder(XL, entry->extname));
q = mkname(VL, nounder(XL,entry->extname) );

if( (type = lengtype(type, (int) length)) != TYCHAR)
	length = 0;
if(class == CLPROC)
	{
	procclass = CLPROC;
	proctype = type;
	procleng = length;

#if TARGET==M68K
	retlabel = getlab();
#else
  	retlabel = newlabel();
#endif
	if(type == TYSUBR)
#if TARGET==M68K
		ret0label = getlab();
#else
  		ret0label = newlabel();
#endif
	}

p = ALLOC(Entrypoint);

if(entries)	/* put new block at end of entries list */
	{
	for(ep = entries; ep->entnextp; ep = ep->entnextp)
		;
	ep->entnextp = p;
	}
else
	entries = p;

p->entryname = entry;
p->arglist = args;
#if TARGET==M68K
p->backlabel = getlab();
p->entrylabel = getlab();
#else
p->backlabel = newlab();
p->entrylabel = newlab();
#endif
p->enamep = q;
if(class != CLENTRY)
	puthead( varstr(XL, procname = entry->extname), class,
	   p->entrylabel, p->backlabel, NULL);
else
	{
	putgoto(p->entrylabel);
	puthead( varstr(XL, entry->extname) , class, p->entrylabel,
	   p->backlabel, NULL);
	}

#ifdef SDB
if(sdbflag)
#if	TARGET == PDP11
	{
	char buff[10];
  	sprintf(buff, "L%d", p->entrylabel);
	prstab(nounder(XL, entry->extname),
		(class==CLENTRY ? N_ENTRY : N_FUN),
		f_lineno, buff);
	if(class != CLENTRY)
	p2pass( stabline( nounder(XL,entry->extname), N_FNAME, 0, 0) );
	}
#	else
	{
	fnstline = f_lineno - 1;
#ifdef SDB
	sdb_func_name = nounder(XL,p->entryname);
#endif
	prstab(nounder(XL,p->entryname), varstr(XL, p->entryname),
	    C_EXT, T_INT | (DT_FCN << 4));
#		if TARGET==M68k
	p2pi("\tdef\t~bf;\tval\t~;\tscl\t101;\tline\t%d;\tendef",f_lineno);
#		else
  	p2pi("\t.def\t.bf;\t.val\t.;\t.scl\t101;\t.line\t%d;\t.endef",f_lineno);
#		endif
	}
#endif
#endif /* SDB */

if(class == CLENTRY)
	{
	class = CLPROC;
	if(proctype == TYSUBR)
		type = TYSUBR;
	}

q->vclass = class;
q->vprocclass = PTHISPROC;
settype(q, type, (int) length);
/* hold all initial entry points till end of declarations */
if(parstate >= INDATA)
	doentry(p);
}

/* generate epilogs */

LOCAL epicode()
{
register int i;

if(procclass==CLPROC)
	{
	if(proctype==TYSUBR)
		{
		putlabel(ret0label);
		if(substars)
			putforce(TYINT, ICON(0) );
		putlabel(retlabel);
		goret(TYSUBR);
		}
	else	{
		putlabel(retlabel);
		if(multitype)
			{
			typeaddr = autovar(1, TYADDR, PNULL);
			putbranch( cpexpr(typeaddr) );
			for(i = 0; i < NTYPES ; ++i)
				if(rtvlabel[i] != 0)
					{
					putlabel(rtvlabel[i]);
					retval(i);
					}
			}
		else
			retval(proctype);
		}
	}

else if(procclass != CLBLOCK)
	{
	putlabel(retlabel);
	goret(TYSUBR);
	}
}


/* generate code to return value of type  t */

LOCAL retval(t)
register int t;
{
register Addrp p;

switch(t)
	{
	case TYCHAR:
	case TYCOMPLEX:
	case TYDCOMPLEX:
		break;

	case TYLOGICAL:
		t = tylogical;
	case TYADDR:
	case TYSHORT:
	case TYLONG:
		p = (Addrp) cpexpr(retslot);
		p->vtype = t;
		putforce(t, p);
		break;

	case TYREAL:
	case TYDREAL:
		p = (Addrp) cpexpr(retslot);
		p->vtype = t;
		putforce(t, p);
		break;

	default:
		badtype("retval", t);
	}
goret(t);
}


/* Allocate extra argument array if needed. Generate prologs. */

LOCAL procode()
{
register struct Entrypoint *p;
Addrp argvec;

#if TARGET==GCOS
	argvec = autovar(lastargslot/SZADDR, TYADDR, PNULL);
#else	/*  not GCOS */
	if(lastargslot>0 && nentry>1)
	{
#if TARGET == VAX || TARGET==M68K
		argvec = autovar(1 + lastargslot/SZADDR, TYADDR, PNULL);
#ifdef SGS2
		/* adjust auto area size to allow for moved parameter block */
		autoleng += ARGOFFSET;
#endif	/* SGS2 */
#else	/* not VAX || M68K */
		argvec = autovar(lastargslot/SZADDR, TYADDR, PNULL);
#endif	/* VAX || M68K */
	}
	else
		argvec = NULL;
#endif	/* GCOS */


#if TARGET == PDP11
	/* for the optimizer */
	if(fudgelabel)
		putlabel(fudgelabel);
#endif

for(p = entries ; p ; p = p->entnextp)
	{
	prolog(p, argvec);

	}
#if FAMILY == PCC
	if (procclass != CLBLOCK)
		putrbrack(procno, p);
#endif

prendproc();
}

/*
   manipulate argument lists (allocate argument slot positions)
 * keep track of return types and labels
 */

LOCAL doentry(ep)
struct Entrypoint *ep;
{
register int type;
register Namep np;
chainp p;
register Namep q;
Addrp mkarg();

++nentry;
if(procclass == CLMAIN)
	{
	putlabel(ep->entrylabel);
	return;
	}
else if(procclass == CLBLOCK)
	return;

impldcl( np = mkname(VL, nounder(XL, ep->entryname->extname) ) );
type = np->vtype;
if(proctype == TYUNKNOWN)
	if( (proctype = type) == TYCHAR)
		procleng = (np->vleng ? np->vleng->constblock.const.ci : (ftnint) (-1));

if(proctype == TYCHAR)
	{
	if(type != TYCHAR)
		err("noncharacter entry of character function");
	else if( (np->vleng ? np->vleng->constblock.const.ci : (ftnint) (-1)) != procleng)
		err("mismatched character entry lengths");
	}
else if(type == TYCHAR)
	err("character entry of noncharacter function");
else if(type != proctype)
	multitype = YES;
if(rtvlabel[type] == 0)
#if TARGET==M68K
	rtvlabel[type] = getlab();
#else
  	rtvlabel[type] = newlabel();
#endif
ep->typelabel = rtvlabel[type];

if(type == TYCHAR)
	{
	if(chslot < 0)
		{
		chslot = nextarg(TYADDR);
		chlgslot = nextarg(TYLENG);
		}
	np->vstg = STGARG;
	np->vardesc.varno = chslot;
	if(procleng < 0)
		np->vleng = (expptr) mkarg(TYLENG, chlgslot);
	}
else if( ISCOMPLEX(type) )
	{
	np->vstg = STGARG;
	if(cxslot < 0)
		cxslot = nextarg(TYADDR);
	np->vardesc.varno = cxslot;
	}
else if(type != TYSUBR)
	{
	if(nentry == 1)
		retslot = autovar(1, TYDREAL, PNULL);
	np->vstg = STGAUTO;
	np->voffset = retslot->memoffset->constblock.const.ci;
	}

for(p = ep->arglist ; p ; p = p->nextp)
	if(! (( q = (Namep) (p->datap) )->vdcldone) )
		q->vardesc.varno = nextarg(TYADDR);

for(p = ep->arglist ; p ; p = p->nextp)
	if(! (( q = (Namep) (p->datap) )->vdcldone) )
		{
		impldcl(q);
		q->vdcldone = YES;
#ifdef SDB
		if(sdbflag)
#	if TARGET == PDP11
			prstab(varstr(VL,q->varname), N_PSYM,
				stabtype(q),
				convic(q->vardesc.varno + ARGOFFSET) );
#	else
			{
			int t;
			t = stabtype(q);
			if (ISARY(t))
				prarstab(varstr(VL, q->varname),
				    convic(q->vardesc.varno + ARGOFFSET), C_ARG,
				    t, q);
			else
				prstab(varstr(VL, q->varname),
				    convic(q->vardesc.varno + ARGOFFSET), C_ARG,
				    t);
			}
#	endif
#endif
		if(q->vtype == TYCHAR)
			{
			if(q->vleng == NULL)	/* character*(*) */
				q->vleng = (expptr)
						mkarg(TYLENG, nextarg(TYLENG) );
			else if(nentry == 1)
				nextarg(TYLENG);
			}
		else if(q->vclass==CLPROC && nentry==1)
			nextarg(TYLENG) ;
		}

putlabel(ep->entrylabel);
}



LOCAL nextarg(type)
int type;
{
int k;
k = lastargslot;
lastargslot += typesize[type];
return(k);
}

/* generate variable references */

LOCAL dobss()
{
register struct Hashentry *p;
register Namep q;
register int i;
int align;
ftnint leng, iarrl;
char *memname();
int qstg, qclass, qtype;

#if TARGET == PDP11

pruse(asmfile, USEBSS);

#endif

for(p = hashtab ; p<lasthash ; ++p)
    if(q = p->varp)
	{
	qstg = q->vstg;
	qtype = q->vtype;
	qclass = q->vclass;

#ifdef SDB
	{
	int scl, type;
	char *valbuf;

                if(sdbflag && qclass==CLVAR) switch(qstg)
#if	TARGET == PDP11
                        {
                        case STGAUTO:
                                prstab(varstr(VL,q->varname), N_LSYM,
                                        stabtype(q),
                                        convic( - q->voffset)) ;
                                prstleng(q, iarrlen(q));
                                break;

                        case STGBSS:
                                prstab(varstr(VL,q->varname), N_LCSYM,
                                        stabtype(q),
                                        memname(qstg,q->vardesc.varno) );
                                prstleng(q, iarrlen(q));
                                break;

                        case STGINIT:
                                prstab(varstr(VL,q->varname), N_STSYM,
                                        stabtype(q),
                                        memname(qstg,q->vardesc.varno) );
                                prstleng(q, iarrlen(q));
                                break;
                        }
#	else
			{
			case STGAUTO:
				scl = C_AUTO;
				valbuf = convic(-q->voffset);
				goto prtvar;
			case STGBSS:
			case STGINIT:
				scl = C_STAT;
				valbuf = memname(qstg, q->vardesc.varno);
	prtvar:
				type = stabtype(q);
#		if TARGET==M68K
				p2pass("\tdata");
#		else
  				p2pass("\t.data");
#		endif
				if (ISARY(type))
					prarstab(varstr(VL, q->varname), valbuf,
					    scl, type, q);
				else
					prstab(varstr(VL, q->varname), valbuf,
					    scl, type);
				break;
			}
#	endif
	}
#endif

	if( (qclass==CLUNKNOWN && qstg!=STGARG) ||
	    (qclass==CLVAR && qstg==STGUNKNOWN) )
		warn1("local variable %s never used", varstr(VL,q->varname) );
	else if(qclass==CLVAR && qstg==STGBSS)
		{
		align = (qtype==TYCHAR ? ALILONG : typealign[qtype]);
		if(bssleng % align != 0)
			bssleng = roundup(bssleng, align);
		prlocvar(memname(STGBSS,q->vardesc.varno), iarrl = iarrlen(q),
		    align);
		bssleng += iarrl;
		}
	else if(qclass==CLPROC && q->vprocclass==PEXTERNAL && qstg!=STGARG)
		mkext(varunder(VL, q->varname)) ->extstg = STGEXT;

	if(qclass==CLVAR && qstg!=STGARG)
		{
		if(q->vdim && !ISICON(q->vdim->nelt) )
			dclerr("adjustable dimension on non-argument", q);
		if(qtype==TYCHAR && (q->vleng==NULL || !ISICON(q->vleng)))
			dclerr("adjustable leng on nonargument", q);
		}
	}

for(i = 0 ; i < nequiv ; ++i)
	if(eqvclass[i].eqvinit==NO && (leng = eqvclass[i].eqvleng)!=0 )
		{
		bssleng = roundup(bssleng, ALIDOUBLE);
		prlocvar( memname(STGEQUIV, i), leng, ALIDOUBLE);
		bssleng += leng;
		}
#ifdef SDB
#	if TARGET != PDP11
if (sdbflag)
#		if TARGET==M68K
   if( strcmp(sdb_func_name,"BLOCK_DATA")!=0 )
	p2pi("\ttext\n\tdef\t~ef;\tval\t~;\tscl\t101;\tline\t%d;\tendef", 
	    f_lineno - fnstline);
#		else
  	p2pi("\t.text\n\t.def\t.ef;\t.val\t.;\t.scl\t101;\t.line\t%d;\t.endef", 
	    f_lineno - fnstline);
#		endif
#	endif
#endif
}



donmlist()
{
register struct Hashentry *p;
register Namep q;

pruse(asmfile, USEINIT);

for(p=hashtab; p<lasthash; ++p)
	if( (q = p->varp) && q->vclass==CLNAMELIST)
		namelist(q);
}


doext()
{
struct Extsym *p;

for(p = extsymtab ; p<nextext ; ++p)
	prext( varstr(XL, p->extname), p->maxleng, p->extinit);
}




ftnint iarrlen(q)
register Namep q;
{
ftnint leng;

leng = typesize[q->vtype];
if(leng <= 0)
	return(-1);
if(q->vdim)
	if( ISICON(q->vdim->nelt) )
		leng *= q->vdim->nelt->constblock.const.ci;
	else	return(-1);
if(q->vleng)
	if( ISICON(q->vleng) )
		leng *= q->vleng->constblock.const.ci;
	else 	return(-1);
return(leng);
}

/* This routine creates a static block representing the namelist.
   An equivalent declaration of the structure produced is:
	struct namelist
		{
		char namelistname[16];
		struct namelistentry
			{
			char varname[16];
			char *varaddr;
			int type; # negative means -type= number of chars
			struct dimensions *dimp; # null means scalar
			} names[];
		};

	struct dimensions
		{
		int numberofdimensions;
		int numberofelements
		int baseoffset;
		int span[numberofdimensions];
		};
   where the namelistentry list terminates with a null varname
   If dimp is not null, then the corner element of the array is at
   varaddr.  However,  the element with subscripts (i1,...,in) is at
   varaddr - dimp->baseoffset + sizeoftype * (i1+span[0]*(i2+span[1]*...)
*/

namelist(np)
Namep np;
{
register chainp q;
register Namep v;
register struct Dimblock *dp;
char *memname();
int type, dimno, dimoffset;
flag bad;


preven(ALILONG);
fprintf(asmfile, LABELFMT, memname(STGINIT, np->vardesc.varno));
putstr(asmfile, varstr(VL, np->varname), 16);
dimno = ++lastvarno;
dimoffset = 0;
bad = NO;

for(q = np->varxptr.namelist ; q ; q = q->nextp)
	{
	vardcl( v = (Namep) (q->datap) );
	type = v->vtype;
	if( ONEOF(v->vstg, MSKSTATIC) )
		{
		preven(ALILONG);
		putstr(asmfile, varstr(VL,v->varname), 16);
		preven(ALILONG);
		praddr(asmfile, v->vstg, v->vardesc.varno, v->voffset);
		prconi(asmfile, TYINT,
			type==TYCHAR ?
			    -(v->vleng->constblock.const.ci) : (ftnint) type);
		if(v->vdim)
			{
			praddr(asmfile, STGINIT, dimno, (ftnint)dimoffset);
			dimoffset += (3 + v->vdim->ndim) * typesize[TYINT];
			}
		else
			praddr(asmfile, STGNULL,0,(ftnint) 0);
		}
	else
		{
		dclerr("may not appear in namelist", v);
		bad = YES;
		}
	}

if(bad)
	return;

preven(ALILONG);
putstr(asmfile, "", 16);

if(dimoffset > 0)
	{
	fprintf(asmfile, LABELFMT, memname(STGINIT,dimno));
	for(q = np->varxptr.namelist ; q ; q = q->nextp)
		if(dp = q->datap->nameblock.vdim)
			{
			int i;
			prconi(asmfile, TYINT, (ftnint) (dp->ndim) );
			prconi(asmfile, TYINT,
				(ftnint) (dp->nelt->constblock.const.ci) );
			prconi(asmfile, TYINT,
				(ftnint) (dp->baseoffset->constblock.const.ci));
			for(i=0; i<dp->ndim ; ++i)
				prconi(asmfile, TYINT,
					dp->dims[i].dimsize->constblock.const.ci);
			}
	}

}

LOCAL docommon()
{
register struct Extsym *p;
register chainp q;
struct Dimblock *t;
expptr neltp;
register Namep v;
ftnint size;
int type;

for(p = extsymtab ; p<nextext ; ++p)
	if(p->extstg==STGCOMMON)
		{
#ifdef SDB
		char comname[20];
		static int comnum = 0;

		if(sdbflag)
#	if TARGET == PDP11
			prstab(CNULL, N_BCOMM, 0, 0);
#	else
			{
			sprintf(comname, "COM%d", comnum++);
#		if TARGET==M68K
			p2ps("\tdef\t%s;\tscl\t10;\ttype\t010;\tsize\t4;\tendef",
			    comname);
#		else
  			p2ps("\t.def\t%s;\t.scl\t10;\t.type\t010;\t.size\t4;\t.endef",
			    comname);
#		endif
			}
#	endif
#endif
		for(q = p->extp ; q ; q = q->nextp)
			{
			v = (Namep) (q->datap);
			if(v->vdcldone == NO)
				vardcl(v);
			type = v->vtype;
			if(p->extleng % typealign[type] != 0)
				{
				dclerr("common alignment", v);
				p->extleng = roundup(p->extleng, typealign[type]);
				}
			v->voffset = p->extleng;
			v->vardesc.varno = p - extsymtab;
			if(type == TYCHAR)
				size = v->vleng->constblock.const.ci;
			else	size = typesize[type];
			if(t = v->vdim)
				if( (neltp = t->nelt) && ISCONST(neltp) )
					size *= neltp->constblock.const.ci;
				else
					dclerr("adjustable array in common", v);
			p->extleng += size;
#ifdef SDB
			if(sdbflag)
#	if TARGET == PDP11
				{
				prstssym(v);
				prstleng(v, size);
				}
#	else
				{
				prstab(nounder(XL, v->varname),
				    convic(v->voffset), 8, stabtype(v));
				}
#	endif
#endif
			}

		frchain( &(p->extp) );
#ifdef SDB
		if(sdbflag)
#	if TARGET == PDP11
			prstab(varstr(XL,p->extname), N_ECOMM, 0, 0);
#	else
			{
			char buff[BUFSIZ];
#		if TARGET==M68k
			sprintf(buff,
			    "\tdef\teos;\tval\t%d;\tscl\t102;\ttag\t%s;\tendef",
			    p->extleng, comname);
#		else
			sprintf(buff,
  			    "\t.def\t.eos;\t.val\t%d;\t.scl\t102;\t.tag\t%s;\t.endef",
			    p->extleng, comname);
#		endif
			p2pass(buff);

/* this goofy stuff is for sdb(obviously). The plot is thus:
 * give sdb a .def of a clown which has no underscores, but has a type
 * of -1 and it will know(!) that it's a common block. There is no .val
 * field since once sdb knows it's a common, it (sdb) will search for
 * the real symbol (magically) (i.e. name --> _name_)
 */

			prststab(!strncmp(p->extname,"_BLNK__", 7)?"_BLNK_":nounder(XL,p->extname),
			    "0", C_STAT, -1, comname, p->extleng); 
			}
#	endif
#endif
		}
}





LOCAL docomleng()
{
register struct Extsym *p;

for(p = extsymtab ; p < nextext ; ++p)
	if(p->extstg == STGCOMMON)
		{
		if(p->maxleng!=0 && p->extleng!=0 && p->maxleng!=p->extleng
		    && !eqn(XL,"_BLNK__ ",p->extname) )
			warn1("incompatible lengths for common block %s",
				nounder(XL, p->extname) );
		if(p->maxleng < p->extleng)
			p->maxleng = p->extleng;
		p->extleng = 0;
	}
}




/* ROUTINES DEALING WITH AUTOMATIC AND TEMPORARY STORAGE */

frtemp(p)
Addrp p;
{
/* restore clobbered character string lengths */
if(p->vtype==TYCHAR && p->varleng!=0)
	{
	frexpr(p->vleng);
	p->vleng = ICON(p->varleng);
	}

/* put block on chain of temps to be reclaimed */
holdtemps = mkchain(p, holdtemps);
}




/* allocate an automatic variable slot */

Addrp autovar(nelt, t, lengp)
register int nelt, t;
expptr lengp;
{
ftnint leng;
register Addrp q;

if(t == TYCHAR)
	if( ISICON(lengp) )
		leng = lengp->constblock.const.ci;
	else	{
		fatal("automatic variable of nonconstant length");
		}
else
	leng = typesize[t];
autoleng = roundup( autoleng, typealign[t]);

q = ALLOC(Addrblock);
q->tag = TADDR;
q->vtype = t;
if(t == TYCHAR)
	{
	q->vleng = ICON(leng);
	q->varleng = leng;
	}
q->vstg = STGAUTO;
q->ntempelt = nelt;
#if TARGET==PDP11 || TARGET==VAX || TARGET==M68K
	/* stack grows downward */
	autoleng += nelt*leng;
	q->memoffset = ICON( - autoleng );
#else
	q->memoffset = ICON( autoleng );
	autoleng += nelt*leng;
#endif

return(q);
}


Addrp mktmpn(nelt, type, lengp)
int nelt;
register int type;
expptr lengp;
{
ftnint leng;
chainp p, oldp;
register Addrp q;

if(type==TYUNKNOWN || type==TYERROR)
	badtype("mktmpn", type);

if(type==TYCHAR)
	if( ISICON(lengp) )
		leng = lengp->constblock.const.ci;
	else	{
		err("adjustable length");
		return( (Addrp) errnode() );
		}
/*
 * if an temporary of appropriate shape is on the templist,
 * remove it from the list and return it
 */

for(oldp=CHNULL, p=templist  ;  p  ;  oldp=p, p=p->nextp)
	{
	q = (Addrp) (p->datap);
	if(q->vtype==type && q->ntempelt==nelt &&
	    (type!=TYCHAR || q->vleng->constblock.const.ci==leng) )
		{
		if(oldp)
			oldp->nextp = p->nextp;
		else
			templist = p->nextp;
		free( (charptr) p);
		return(q);
		}
	}
q = autovar(nelt, type, lengp);
q->istemp = YES;
return(q);
}




Addrp mktemp(type, lengp)
int type;
expptr lengp;
{
return( mktmpn(1,type,lengp) );
}

/* VARIOUS ROUTINES FOR PROCESSING DECLARATIONS */

struct Extsym *comblock(len, s)
register int len;
register char *s;
{
struct Extsym *p;

if(len == 0)
	{
	s = BLANKCOMMON;
	len = strlen(s);
	}
p = mkext( varunder(len, s) );
if(p->extstg == STGUNKNOWN)
	p->extstg = STGCOMMON;
else if(p->extstg != STGCOMMON)
	{
	errstr("%s cannot be a common block name", s);
	return(0);
	}

return( p );
}


incomm(c, v)
struct Extsym *c;
Namep v;
{
if(v->vstg != STGUNKNOWN)
	dclerr("incompatible common declaration", v);
else
	{
	v->vstg = STGCOMMON;
	c->extp = hookup(c->extp, mkchain(v,CHNULL) );
	}
}




settype(v, type, length)
register Namep  v;
register int type;
register int length;
{
if(type == TYUNKNOWN)
	return;

if(type==TYSUBR && v->vtype!=TYUNKNOWN && v->vstg==STGARG)
	{
	v->vtype = TYSUBR;
	frexpr(v->vleng);
	}
else if(type < 0)	/* storage class set */
	{
	if(v->vstg == STGUNKNOWN)
		v->vstg = - type;
	else if(v->vstg != -type)
		dclerr("incompatible storage declarations", v);
	}
else if(v->vtype == TYUNKNOWN)
	{
	if( (v->vtype = lengtype(type, length))==TYCHAR && length>=0)
		v->vleng = ICON(length);
	}
else if(v->vtype!=type || (type==TYCHAR && v->vleng->constblock.const.ci!=length) )
	dclerr("incompatible type declarations", v);
}





lengtype(type, length)
register int type;
register int length;
{
switch(type)
	{
	case TYREAL:
		if(length == 8)
			return(TYDREAL);
		if(length == 4)
			goto ret;
		break;

	case TYCOMPLEX:
		if(length == 16)
			return(TYDCOMPLEX);
		if(length == 8)
			goto ret;
		break;

	case TYSHORT:
	case TYDREAL:
	case TYDCOMPLEX:
	case TYCHAR:
	case TYUNKNOWN:
	case TYSUBR:
	case TYERROR:
		goto ret;

	case TYLOGICAL:
		if(length == typesize[TYLOGICAL])
			goto ret;
		break;

	case TYLONG:
		if(length == 0)
			return(tyint);
		if(length == 2)
			return(TYSHORT);
		if(length == 4)
			goto ret;
		break;
	default:
		badtype("lengtype", type);
	}

if(length != 0)
	err("incompatible type-length combination");

ret:
	return(type);
}





setintr(v)
register Namep  v;
{
register int k;

if(v->vstg == STGUNKNOWN)
	v->vstg = STGINTR;
else if(v->vstg!=STGINTR)
	dclerr("incompatible use of intrinsic function", v);
if(v->vclass==CLUNKNOWN)
	v->vclass = CLPROC;
if(v->vprocclass == PUNKNOWN)
	v->vprocclass = PINTRINSIC;
else if(v->vprocclass != PINTRINSIC)
	dclerr("invalid intrinsic declaration", v);
if(k = intrfunct(v->varname))
	v->vardesc.varno = k;
else
	dclerr("unknown intrinsic function", v);
}



setext(v)
register Namep  v;
{
if(v->vclass == CLUNKNOWN)
	v->vclass = CLPROC;
else if(v->vclass != CLPROC)
	dclerr("invalid external declaration", v);

if(v->vprocclass == PUNKNOWN)
	v->vprocclass = PEXTERNAL;
else if(v->vprocclass != PEXTERNAL)
	dclerr("invalid external declaration", v);
}




/* create dimensions block for array variable */

setbound(v, nd, dims)
register Namep  v;
int nd;
struct { expptr lb, ub; } dims[ ];
{
register expptr q, t;
register struct Dimblock *p;
int i;

if(v->vclass == CLUNKNOWN)
	v->vclass = CLVAR;
else if(v->vclass != CLVAR)
	{
	dclerr("only variables may be arrays", v);
	return;
	}

v->vdim = p = (struct Dimblock *)
		ckalloc( sizeof(int) + (3+2*nd)*sizeof(expptr) );
p->ndim = nd;
p->nelt = ICON(1);

for(i=0 ; i<nd ; ++i)
	{
	if( (q = dims[i].ub) == NULL)
		{
		if(i == nd-1)
			{
			frexpr(p->nelt);
			p->nelt = NULL;
			}
		else
			err("only last bound may be asterisk");
		p->dims[i].dimsize = ICON(1);;
		p->dims[i].dimexpr = NULL;
		}
	else
		{
		if(dims[i].lb)
			{
			q = mkexpr(OPMINUS, q, cpexpr(dims[i].lb));
			q = mkexpr(OPPLUS, q, ICON(1) );
			}
		if( ISCONST(q) )
			{
			p->dims[i].dimsize = q;
			p->dims[i].dimexpr = (expptr) PNULL;
			}
		else	{
			p->dims[i].dimsize = (expptr) autovar(1, tyint, PNULL);
			p->dims[i].dimexpr = q;
			}
		if(p->nelt)
			p->nelt = mkexpr(OPSTAR, p->nelt,
					cpexpr(p->dims[i].dimsize) );
		}
	}

q = dims[nd-1].lb;
if(q == NULL)
	q = ICON(1);

for(i = nd-2 ; i>=0 ; --i)
	{
	t = dims[i].lb;
	if(t == NULL)
		t = ICON(1);
	if(p->dims[i].dimsize)
		q = mkexpr(OPPLUS, t, mkexpr(OPSTAR, cpexpr(p->dims[i].dimsize), q) );
	}

if( ISCONST(q) )
	{
	p->baseoffset = q;
	p->basexpr = NULL;
	}
else
	{
	p->baseoffset = (expptr) autovar(1, tyint, PNULL);
	p->basexpr = q;
	}
}
