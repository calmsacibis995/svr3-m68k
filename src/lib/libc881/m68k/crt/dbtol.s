#	@(#)dbtol.s	6.1	
# "@(#)dbtol.s	6.1	";
	file	"dbtol.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#
#        entry : <%d0,%d1> = ieee d.p. number
#        exit  : <%d0>    = signed (or unsigned) 32 bit integer
#        fltolng --- ieee d.p. to 32 bit signed integer
#        fltouln --- ieee d.p. to 32 bit unsigned integer
#
# xrefs: none
#-
 
	text
	global dbtol%%,dbtoul%%
 
#        on entry,  <%d0,%d1> = ieee d.p. number
#        on  exit,  <%d0>    = a truncated (not rounded) 32 bit signed integer.
#
#        an overflow results in the largest integer of propper sign
#
# some bit masks for condition codes in M68881's %status register
	set	FPCC_NAN,0x01000000
	set	FPCC_I,0x02000000
	set	FPCC_N,0x08000000

# long dbtol(a)
# double a;
# {
dbtol%%:	
	link	%fp,&-12
# double tmpdbl;
# register double tmpresult;
# tmpdbl = a;
	mov.l	%d0,-8(%fp)
	mov.l	%d1,-4(%fp)
# tmpresult = tmpdbl;
	fmov.d	-8(%fp),%fp0
	fintrz.x %fp0,%fp0
	fmov.l	%fp0,%d0		# result is in %d0
	mov.l	%d0,%a0			# now in %a0
	fmovm.l %status,%d0
	mov.l	%d0,%d1			# save a copy of %status
	and.l	&FPCC_NAN+FPCC_I,%d0 # infinity or nan treated as overflow
	beq.b	L%normal		# all is okay

	and.l	&FPCC_N,%d1		# was value negative?
	beq.b	L%posint

	mov.l	&0x80000000,%d0		# return minlong
	bra.b	L%thru

L%posint: mov.l &0x7fffffff,%d0		# return maxlong
	bra.b	L%thru

# return ( (long) tmpresult);
L%normal:
	mov.l	%a0,%d0
# }
L%thru:
	unlk	%fp
        rts




#        on entry,  <%d0,%d1> = ieee d.p. number
#        on exit,   <%d0>    = a truncated (not rounded) 32bit unsigned integer
#
#        an overflow results in the largest unsigned integer;
#	 negative inputs return a 0 (smallest unsigned).
#
# unsigned long dbtoul%%(a)
# double a;
# {
#
dbtoul%%:
	link	%fp,&-12
# double tmpdbl = a;
	mov.l	%d0,-8(%fp)
	mov.l	%d1,-4(%fp)
# register double tmpresult;
# register unsigned long more_long = 0, result;
	clr.l	%d1
# if (a<0)
	tst.l	%d0
	bge.b	L%Pos

# return(0);
	clr.l	%d0
	bra.b	L%Thru

# tmpresult = tmpdbl;
L%Pos:
	fmov.d	-8(%fp),%fp0
	ftest.x	%fp0

	fbun.w	L%Ovrflw		# a NaN
	fcmp.d	%fp0,&0x7ff0000000000000 # is it infinity?
	fbeq.w	L%Ovrflw

# tmp_dbl = (truncated) tmp_dbl;
	fintrz.x %fp0,%fp0		# truncate the double number
# tmp_dbl = (double) 0xfffffffe;	# All numbers greater than this are
					# represented by 0xffffffff
	mov.l	&0x7fffffff,%d0
	fmov.l	%d0,%fp1
	fadd.l	%d0,%fp1
	fcmp.x	%fp0,%fp1
	fbogt.w L%Ovrflw		# Can't be represented as a long,
					# except as "max unsigned".
	fcmp.l	%fp0,&0x7fffffff	# greater than "max signed" ?
	fbole.w L%storit		# No, can store it now
# more_long = max_signed + 1;
	mov.l	&0x80000000,%d1		
# tmp_dbl -= 0x80000000;
	mov.l	&0x40000000,%d0
	fsub.l  %d0,%fp0
	fsub.l	%d0,%fp0		# finally, %fp0 will not overflow 
					# a signed long
L%storit:
# result = (long) tmp_dbl;
	fmov.l	%fp0,%d0
	bra.b	L%Thru
L%Ovrflw:
#	result = 0xffffffff;
	mov.l	&0xffffffff,%d0
# }
L%Thru:
# result += more_long;
# return (result);
	or.l	%d1,%d0
	unlk	%fp
        rts
