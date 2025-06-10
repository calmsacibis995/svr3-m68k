/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/machdep.c	1.18"

/*
 ****		MACHINE and OPERATING SYSTEM DEPENDENT
 ****		Routines which deal with the run-time stack
 */

#include "head.h"
#include "coff.h"


extern BKPTR	bkpthead;
extern MSG		NOPCS;

long rdsym();			/* in symt.c */
extern SYMENT syment;		/* rdsym() stores symbol table entry */
extern AUXENT auxent[];		/* rdsym() stores auxiliary entry */
extern int gflag;		/* initfp() in symt.c sets */
extern FILHDR filhdr;
static int frameflg;		/* set if now past last frame */

#define EVEN	(-2)

/* initialize frame pointers to top of call stack */
/*  MACHINE DEPENDENT */
struct proct *
initframe() {
	register struct proct *procp;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("initframe");
	}
	else if (debugflag == 2)
	{
		enter2("initframe");
		closeparen();
	}
#endif
	if ((pid == 0) && (fcor < 0 || fakecor)) {
		frameflg = 1;
		return(badproc);
	}

#if vax || u3b
	argp = SDBREG(AP);
	frame = SDBREG(FP);
#else
#if u3b5
	argp = regvals[10];
	frame = regvals[9];
#else
#if m68k
	frame = argp = SDBREG(FP);
	callpc = SDBREG(PC);
#endif
#endif
#endif
#if vax || u3b
	callpc = SDBREG(PC);
#else
#if u3b5
	callpc = regvals[15];
#endif
#endif

#if u3b || u3b5


	/* FP in 3B starts at USRSTACK and increases as stack grows */
	if (frame < USRSTACK) {	/* 3B ?? other checks ?? */
		frameflg = 1;
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("initframe");
			endofline();
		}
#endif
		return(badproc);
	}

#else
#if vax

	/* FP in VAX starts at maxstor = 1L << 31 and decreases */
	if ((frame & 0xf0000000) != 0x70000000) {
		frameflg = 1;
		return(badproc);
	}
#else
#if m68k
	/* FP in m68k is initialized by kernel to be 0L */
	if (frame == 0L) {
		frameflg = 1;
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("initframe");
			endofline();
		}
#endif
		return(badproc);
	}
#endif
#endif
#endif

	procp = adrtoprocp(callpc);
	frameflg = 0;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("initframe");
		printf("0x%x",procp);
		endofline();
	}
#endif
	return(procp);
}

struct proct *
nextframe() {
	register struct proct *procp;
	union word word;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("nextframe");
	}
	else if (debugflag == 2)
	{
		enter2("nextframe");
		closeparen();
	}
#endif
#if m68k
	callpc = get(NEXTCALLPC, DSP);
	frame = get(NEXTFRAME, DSP) & EVEN;
	argp = frame;
#else
	callpc = get(NEXTCALLPC, DSP);
	argp = get(NEXTARGP, DSP);
#endif
#if vax || u3b
	frame = get(NEXTFRAME, DSP) & EVEN;
#else
#if u3b5
	frame = get(NEXTFRAME,DSP);
#endif
#endif
#if vax || u3b 
#if u3b 
	if (callpc > USRSTACK)	/* 3B ?? other checks ?? */
#else
	if (callpc > 0x70000000)	/* error handler kludge */
#endif
	{
		callpc = get(argp+12, DSP);
		argp = get(NEXTARGP, DSP);
		frame = get(NEXTFRAME, DSP) & EVEN;
	}
#endif
#if u3b || u3b5


	if (frame < USRSTACK) {	/* 3B ?? other checks ?? */
		frameflg = 1;
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("nextframe");
			printf("0x%x",badproc);
			endofline();
		}
#endif
		return(badproc);
	}
#else
#if vax

	if ((frame & 0xf0000000) != 0x70000000) {
		frameflg = 1;
		return(badproc);
	}
#else
#if m68k
	if( frame == 0L ) /* first frame pointer initialized to 0L by kernel */
	{
		frameflg = 1;
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("nextframe");
			printf("0x%x",badproc);
			endofline();
		}
#endif
		return(badproc);
	}
#endif
#endif
#endif
	procp = adrtoprocp(callpc-1);

#if u3b || u3b5
	word.w = get( procp->paddress, ISP );
#if u3b
	if( word.c[ 0 ] != 0x7A )	/* opcode for 3B 'save' instr */
#else
	if (word.c[0] != 0x10)		/* opcode for 3B5 'save' instr */
#endif
	{
		frameflg = 1;
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("nextframe");
			printf("0x%x",badproc);
			endofline();
		}
#endif
		return( badproc );	/* cannot chain back anymore */
	}
#endif

#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("nextframe");
		printf("0x%x",procp);
		endofline();
	}
#endif
	return(procp);
}

/* returns core image address for variable */
/* formaddr() should be in symt.c ?? */
ADDR
#ifdef m68k
/* needs type so stackreg can find float reg var properly*/
formaddr(class, addr, type)
short type;
#else
formaddr(class, addr)
#endif
register char class;
ADDR addr; {
	switch(class) {
	case C_REG:
	case C_REGPARM:
#ifdef m68k
		{
			char *desc;
			desc = typetodesc(type);
			if ( (*desc=='g') || (*desc=='f') )
				return(stackreg(addr, 1));
			else
				return(stackreg(addr, 0));
		}
#else
		return(stackreg(addr));
#endif
	case C_EXT:
	case C_STAT:
		return(addr);
	case C_MOS:
	case C_MOU:
	case C_MOE:
	case C_FIELD:			/* ?? */
		return(addr);
		
	case C_ARG:
		return(argp+addr);
		
	case C_AUTO:
		return(frame+addr);	/* addr was negated in outvar() */

	default:
		fprintf(FPRT1,"Bad class in formaddr: %d(%#x)\n", class, class);
		return(0);
	}
}

/*
 *  stackreg(reg):
 * This routine is intended to be used to find the address of a register
 * variable.  The stack is searched backwards from the current frame
 * until the procedure with the register variable is found.  If the
 * register assigned to the variable appears on the stack, then its
 * address is returned.  Otherwise, the register variable is still in
 * the named register, so the register number is returned.  We distinguish
 * addresses from register numbers by noting that register numbers are less
 * than NUMREGS (16) and that stack addresses are greater.
 *
 *  MACHINE DEPENDENT
 */
/* new stackreg() for 3B Unix
 *	C stores 'n' registers starting with %R8 and going backwards
 *	can only get 'n' by looking at text -- bits 8->11 from beginning of proc
 */

#if u3b || u3b5
#define REGSAV1		8
#define ISREGSAV(xr,xn)	(((REGSAV1-xn) < xr) && (xr <= REGSAV1))
#if u3b
#define NUMREGSOFF	1
#define REGADDR(xr)	(frame - (REGSAV1-xr+1)*REGSIZE)
#else
#define NUMREGSOFF	(9 - (getval(procp->st_offset+1,'bu',DSP)&0x0f))
#define REGADDR(xr,xn)	(frame - (ADDR)(15-xr-xn)*REGSIZE)
#endif
ADDR
stackreg(reg)
ADDR reg;
{
	register struct proct *procp;
	ADDR regaddr;
	unsigned int nrs;
	/* unsigned int nn;	don't adjust addresses - see below */

#if DEBUG
	if (debugflag ==1)
	{
		enter1("stackreg");
	}
	else if (debugflag == 2)
	{
		enter2("stackreg");
		arg("reg");
		printf("0x%x",reg);
		closeparen();
	}
#endif
	/*  if no procedure to search down to, return reg */
	if (proc[0] == '\0')
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("stackreg");
			printf("0x%x",reg);
			endofline();
		}
#endif
		return(reg);
	}


	regaddr = reg;
	for (procp=initframe(); procp != badproc; procp=nextframe()) {
		if (sl_procp == procp) break;
#if u3b
		nrs = getval(procp->paddress +NUMREGSOFF, "bu", DSP);
		nrs >>= 4;	/* get high four bits */
		if (ISREGSAV(reg,nrs)) {
			regaddr = REGADDR(reg);
#else
		nrs = NUMREGSOFF;
		if (ISREGSAV(reg,nrs)) {
			regaddr = REGADDR(reg,nrs);
#endif
			/*  3B stores chars and shorts in registers
			 * right justified, even though they are stored
			 * left justified in core.  Register variables
			 * are stored in register image on the stack,
			 * and that is the way they should be treated,
			 * so that the calling function doesn't have
			 * to check again whether they are actually
			 * in the registers or not.
			 * nn = WORDSIZE - typetosize(sl_type, sl_size);
			 * if (nn > 0)
			 * 	regaddr += nn;
			 */
		}
	}
	if (procp == badproc) {
		fprintf(FPRT1, "Stackreg() error: frame=%#x\n", frame);
		regaddr = ERROR;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("stackreg");
		printf("0x%x",regaddr);
		endofline();
	}
#endif
	return(regaddr);
}

#else
#if vax
/*  VAX version - C stores 'n' registers arbitrarily.
 *	A mask (16 bits) is stored as the high order short in the word which is
 *	offset one word from the beginning of the frame (frame + WORDSIZE).
 *	The 0'th bit of this mask tells whether register 0 has been saved,
 *	the 1'st bit tells wheter register 1 has been saved, etc.
 *	Each saved register in the frame occupies another word of space,
 *	and they are stored in order, from lowest to highest numbered.
 *	The first saved register is at frame + 5 * WORDSIZE.
 */

#define REGOFF 5 * WORDSIZE
#define MASKOFF WORDSIZE
ADDR
stackreg(reg) {
	register int regfl, mask, i;
	register struct proct *procp;
	ADDR regaddr;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("stackreg");
	}
	else if (debugflag == 2)
	{
		enter2("stackreg");
		arg("reg");
		printf("0x%x",reg);
		closeparen();
	}
#endif
	if (proc[0] == '\0')
		return(reg);
	regaddr = reg;
	regfl = 0x10000 << reg;
	for (procp=initframe(); procp!=badproc; procp=nextframe()) {
		if (sl_procp == procp)
			break;
		if ((mask = get(frame + MASKOFF)) & regfl) {
			regaddr = frame + REGOFF;
			for (i=0; i < reg; i++) {
				if (mask & 0x10000) regaddr += WORDSIZE;
				mask >>= 1;
			}
		}
	}
	if (procp == badproc) {
		fprintf(FPRT1, "Stackreg() error: frame=%#x\n", frame);
		regaddr = ERROR;
	}
	return(regaddr);
}
#else
#if m68k
/*  68000 version - C stores 'n' registers arbitrarily.
 *	A mask (16 bits) is stored as a  short following a movem 
 *	instruction in the object module. (Unlike the vax, this mask is not 
 *	saved on the stack.) Starting at the entry point of the proc, the
 *	object code is searched for the presence of a movem. The search
 *	currently is set up terminate after 15 instructions or a rts, whichever
 *	comes first. The MOVEM instruction is decoded to fined the mask and EA.
 *      Once the mask is gotten, the stack is then searched.
 *	The 0'th bit of this mask tells whether register 0 has been saved,
 *	the 1'st bit tells wheter register 1 has been saved, etc.
 *	Each saved register in the frame occupies another word of space,
 *	and they are stored in order, from D0 to A7
 *	in ascending locations on the stack.
 *
 *	The first saved register is at:
 *
 * 		EA - WORDSIZE*(number of saved registers)
 *
 *	where (number of saved registers) is computed by counting the
 *  	bits in the MOVEM mask.
 *	
 *	EA is determined by decoding the addressing mode of the MOVEM
 *	instruction.  Currently valid EA's for the MOVEM are:
 *
 *		mode 010  reg 110		(%fp)
 *		mode 101  reg 110		disp(%fp)	(16 bit disp)
 *		mode 101  reg 111		disp(%sp)	(16 bit disp)
 *			(requires link_size, the size of the activation
 *			 record which is decoded from the link.[wl] instruction)
 *
 *	Additions for 881 support:
 *		the argument fpflag=1 denotes type double or float.
 *		If a M68881 floating point co-processor is present,
 *		double or float register variables are stacked in
 *		extended form using the fmovm instruction.  The logic
 *		for finding these stacked register variables is very
 *		similar to that descrived above for "disp(%sp)" case
 *		of the movm.  The mask, however, is only eight bits long
 *		and registers saved are indicated so that bit 0 corresponds
 *		to fp register 7, bit 1 to fp reg 6, etc., therefore
 *		the logic of decoding the mask is a little different.
 *		Also, each register requires 4*WORDSIZE bytes on the
 *		stack.
 */
#define MOVEM	0x48800000	/* movem opcode stored in high word	*/
#define LINKW	0x4e500000	/* link.w opcode stored in high word	*/
#define LINKL	0x48080000	/* link.l opcode stored in high word	*/
#define RTS	0x4e750000	/* rts opcode stored in high word	*/
#define INSLIMIT  15		/* number of assemble language instructions */
#define FMOVM	0xf000e000	/* fline nibble, opclass of fmovm */

ADDR
stackreg(reg, fpflag) {
	register int  regfl, mask, i, ctr;
	register struct proct *procp;
	ADDR regaddr, insaddr, ins;
	long	ea,rsa_disp=0,link_size;
	long	save_frame, fpregfl=0;
	long fprsa_disp;	/* fp reg save area displacement */
	register int fpmask;	/* fp reg save mask */

	if (proc[0] == '\0')
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("stackreg");
			printf("0x%x",reg);
			endofline();
		}
#endif
		return(reg);
	}
	regaddr = reg;
	regfl = 0x10000 << reg;
	if(fpflag)
		fpregfl = 0x1 << (7-reg);
	save_frame = frame;		/* search down stack to this frame */
	for (procp=initframe(); procp!=badproc; procp=nextframe()) {
		/*if (sl_procp == procp)*/
		if (save_frame == frame)
			break;
/* search the object to see if mask is present, giveup after ctr instructions */

		insaddr = procp->paddress;	/* offset in a.out */
		ctr = INSLIMIT;			/* stop after INSLIMIT instrs */
		mask = 0;			/* initialize */
		fpmask = 0;			/* initialize */
		link_size = 0;
		while(ctr--) {
			ins = get(insaddr,ISP);
			if((ins&0xff800000) == MOVEM) {
				mask = (ins << 16);
				ea   = ( (ins>>16) & 077 );
				if (ea == 026 ) 	/* (%fp) */
					rsa_disp = 4;
				if ( ea == 056 ) 	/* d(%fp) */
				     rsa_disp=
					(short)(get(insaddr+WORDSIZE,ISP)>>16)+4;
				if ( ea == 057 ) {	/* d(%sp) */
				     rsa_disp=
					(short)(get(insaddr+WORDSIZE,ISP)>>16);
				     rsa_disp = link_size + rsa_disp;
				    };
#if defined(M68020) && defined(M68881)
				/* continue to look for fmovm */
#else
				break;
#endif
			}
#if defined(M68020) && defined(M68881)
			if((ins&0xf000e000) == FMOVM) {
				fpmask = (ins & 0xff) ;
				     fprsa_disp=
					(short)(get(insaddr+2*WORDSIZE,ISP)>>16);
				     fprsa_disp = link_size + fprsa_disp;
				break;
			}
#endif

			if( (ins&0xfff80000) == LINKW )
				link_size = (short)(ins&0x0000ffff);

			if( (ins&0xfff80000) == LINKL )  /* 32 bit link size */
				link_size = (short)(get(insaddr+WORDSIZE,ISP)>>16);

			if( (ins&0xffff0000) == RTS) 
				break;
			insaddr = dis_dot(insaddr,ISP, '\0');
		} /* end while loop */
		if ((mask & regfl) && !fpflag) {
			regaddr = frame + rsa_disp;
#ifndef M68020
			/* find offset of first saved reg from mask */
			for (i=0; i < 16;  i++)
				if(mask & (0x10000 << i))
					regaddr -= WORDSIZE;
#endif
			/* find offset of desired register */
			for (i=0; i < reg; i++) {
				if (mask & 0x10000) regaddr += WORDSIZE;
				mask >>= 1;
			}
		}
#if defined(M68020) && defined(M68881)
		if ((fpmask & fpregfl) && fpflag) {
			regaddr = frame + fprsa_disp;
			/* find offset of desired register */
			for (i=0; i < reg; i++) {
				if (fpmask & 0x80) regaddr += 3*WORDSIZE;
				fpmask <<= 1;
			}
		}
#endif
	}
	if (procp == badproc) {
		fprintf(FPRT1, "Stackreg() error: frame=%#x\n", frame);
		regaddr = ERROR;
	}
	return(regaddr);
}
#endif
#endif
#endif

/* Print the stack trace: 't' and 'T' commands
 * set top (i.e. top != 0) to print just the top procedure
 * modified to print something even if a.out stripped
 */

#if u3b

/*  The 3B saves 13 words worth of registers before saving arguments */
#define NARGSTACK	( ((frame - argp) / WORDSIZE) -13)
#else
#if u3b5
#define NARGSTACK	(((frame - argp) / WORDSIZE) - 9)
#else
#if vax
/*  The number of words stored as arguments is in the first byte
 *	of the zero'th argument.  The remaining bytes of the word should
 *	be zero.  Argp is set to point to the first argument.
 */
#define NARGSTACK    (argp += WORDSIZE, \
			(narg = get(argp-WORDSIZE, DSP)) & ~0xff ? 0 : narg\
		     )
#else
#if m68k
/* if stripped or -g not used, default to print NARGDEFAULT words from stack. No other way to tell how many args (no arg count on 68k) */
#define NARGSTACK    (argp += WORDSIZE*2, \
			(narg = (procp->lineno) ? 0 : NARGDEFAULT)\
                     )

#endif
#endif
#endif
#endif

prfrx(top) {
	int narg;		/* number of words that arguments occupy */
	long offs;		/* offset into symbol table */
	register char class;	/*storage class of symbol */
	register int endflg;	/* 1 iff cannot find symbolic names of
					more arguments */
	int subsproc = 0;	/* 1 iff not top function on stack */
	register char *p;	/* pointer to procedure (function) name */
	int line_num;		/* line number in calling function */
	register struct proct *procp;
	SYMENT *syp = &syment;
	AUXENT *axp = auxent;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("prfrx");
	}
	else if (debugflag == 2)
	{
		enter2("prfrx");
		arg("top");
		printf("0x%x",top);
		closeparen();
	}
#endif
	procp = initframe();
	if (frameflg) {		/*  no initial frame */
		if (pid == 0 && (fcor < 0 || fakecor))	{  /* usual error */
			errflg = "No process and no core file.";
			chkerr();
		}
		else {				/* unusual initial frame */
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("prfrx");
				endofline();
			}
#endif
			return;
		}
	}
	do {
#if u3b || u3b5
		/*  3B crt0 (start) has an old fp of zero */
		if (get(NEXTFRAME, DSP) == 0)
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("prfrx");
				endofline();
			}
#endif
			return;
		}
#else
#if vax || m68k
		/*  VAX crt0 (start) gets a current fp of zero */
		if (frame == 0)
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("prfrx");
				endofline();
			}
#endif
			return;
		}
#endif
#endif
		p = procp->pname;
#if m68k
		if (eqstr("_dbsubc", p))
			return;
#else
		if (eqstr("__dbsubc", p))	/*  3B ?? */
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("prfrx");
				endofline();
			}
#endif
			return;
		}
#endif
		if (procp == badproc) {		/*  if stripped a.out */
			printf("pc: 0x%lx;	args: (", callpc);
			endflg = 1;
		}
#if vax
		else if (p[0] == '_') {
			printf("%s(", p+1);
			endflg = 0;
		}
#endif
		else {
			printf("%s(", p);
			endflg = 0;
		}
		if (endflg == 0) {
			offs = procp->st_offset;
			do {		/*  in COFF, always just 1 ? */
				if( (offs = rdsym(offs)) == ERROR) {
					endflg++;
					break;
				}
			} while (ISFCN(syp->n_type));
			class = syp->n_sclass;
			while (! ISARGV(class)) {
				if ((offs = rdsym(offs)) == ERROR) {
					endflg++;
					break;
				}
				class = syp->n_sclass;
				if (ISFCN(syp->n_type)) {
					endflg++;
					break;
				}
			}
		}

/*
if (get(NEXTCALLPC,DSP) > 0x70000000)
	narg = 0;
else
*/
		narg = NARGSTACK;
#if m68k
		if (eqstr("main", p))	/* always print 3 arguments for main*/
			narg = 3;
		while(ISARGV(class) || (narg > 0)) {
#else
		while (narg) {
#endif
			if (endflg) {
				printf("%d", get(argp, DSP));
				argp += WORDSIZE;
			} else {
				int length;
				if ((syp->n_type == T_STRUCT) ||
				    (syp->n_type == T_UNION))   {
				    /* print address of structure
				     * (so that structures of, e.g.
				     * 100 element arrays, are not dumped)
				     */
#if u3b || u3b5 ||m68k
				    printf( "&%s=", syp->n_nptr );
#else
#if vax
				    /* VAX: skip leading underscore */
				    if (syp->n_nptr[0] == '_')
					printf("&%s=", syp->n_nptr+1);
				    else
					printf("&%s=", syp->n_nptr);
#endif
#endif
				    dispx(argp, "x", C_EXT, 
						(short) (-1), 0, DSP);
				    length = axp->x_sym.x_misc.x_lnsz.x_size;
				}
				else {
				    if (syp->n_type == T_ENUM)
					length =
					    axp->x_sym.x_misc.x_lnsz.x_size;
				    else
					length = typetosize(syp->n_type, 0);
#if u3b || u3b5
				    printf("%s=", syp->n_nptr);
				    /* The address of a short or char is
				     * expected to be the left byte of the
				     * variable.  However, argp points to
				     * the left end of the WORD containing
				     * the parameter, which is right justified.
				     * The address is thus adjusted accordingly.
				     * length<WORDSIZE test needed, since length
				     * could be sizeof(double) > WORDSIZE.
				     *
				     * Alternate code:
				     *  dispx(argp, "", C_REGPARAM,
				     *	    (short)syp->n_type, 0, DSP);
				     * This has the same effect, since dispx
				     * sees a "register" variable (hence
				     * right justified), on the stack, and
				     * extracts the right part.  This is
				     * more efficient, but less clean, since
				     * parameters are not necessarily
				     * register variables, they only look that
				     * way.
				     */
				    if (length < WORDSIZE) {
					dispx(argp+WORDSIZE-length,"",
					    C_EXT,(short)syp->n_type,0,DSP);
				    } else {
					dispx(argp, "", C_EXT,
					    (short)syp->n_type, 0, DSP);
				    }
#else
#if vax
				    /* VAX: skip leading underscore */
				    if (syp->n_nptr[0] == '_')
				    {
					printf( "%s=", syp->n_nptr + 1 );
				    }
				    else
				    {
					printf( "%s=", syp->n_nptr );
				    }
				    dispx(argp, "", C_EXT, 
					    (short) syp->n_type, 0, DSP);
#else
#if m68k
				    printf("%s=", syp->n_nptr);
				    dispx(argp, "", C_EXT, 
					    (short) syp->n_type, 0, DSP);
#endif
#endif
#endif
				}
				if (length > WORDSIZE) {
					argp += length;
#if !m68k
					/*  adjust for more than 1 word */
					narg -= length/WORDSIZE -1;
#endif
				}
				/* bytes, shorts, longs pushed as ints */
				else {
					argp += WORDSIZE;
				}
			}
			do {
				if (endflg) break;
				if ((offs = rdsym(offs)) == ERROR) {
					endflg++;
					break;
				}
				class = syp->n_sclass;
				if (ISFCN(syp->n_type)) {
					endflg++;
					break;
				}
			} while (! ISARGV(class));
#if m68k
			if ((--narg > 0) || ISARGV(class)) printf(",");
#else
			if (--narg != 0) printf(",");
#endif
		}
		printf(")");
		if (procp->sfptr != badfile)
			printf("   [%s",
				(procp->sfptr)->sfilename);
		if(gflag) {
			if ((line_num = adrtolineno(callpc-subsproc,procp)) > 0)
				printf(":%d", line_num);
		}
		if(procp->sfptr != badfile)
			printf("]");
		printf("\n");
		subsproc = 1;
		procp = nextframe();
	} while (!top && !frameflg);	/*  only one frame desired, or
						no frames left in backtrace */
/* Vax:	} while (((procp = nextframe()) != badproc) && !top);*/
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("prfrx");
		endofline();
	}
#endif
}


/* machine dependent initialization */
sdbenter(xflag) {
#if vax
	mkioptab();
#endif
}

/* machine dependent exit code */
sdbexit() {
	exit(0);
}

#if u3b || u3b5
/*  isubcall() to replace SUBCALL #define (more complicated on 3B) */
isubcall(loc, space)
long loc;
int space;
{
	register int opcode;
	union word word;

	word.w = get(loc,space);
	opcode = word.c[0];
#if u3b || m68k
	return(opcode == 0x79 || opcode == 0x78 || opcode == 0x77);
#else
	return(opcode == 0x2c);		/* opcode for call instruction */
#endif
}
#endif
