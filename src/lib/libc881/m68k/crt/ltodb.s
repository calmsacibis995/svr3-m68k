#	@(#)ltodb.s	6.2	
# "@(#)ltodb.s	6.1	";
	file	"ltodb.s"

#+
#
#	M68000 IEEE Format Double Precision Routines
#
#	(C) Copyright 1985 by Motorola Inc.
#
#  no error conditions possible in these routines
#
# globals: 
#        ultodb%% --- unsigned 32 bit integer to ieee d.p.
#        ltodb%% ---   signed 32 bit integer to ieee d.p.
#
# xrefs: none
#-
 
        text
	global	ultodb%%	
	global	ltodb%%	
# double ltodb(a)
# long a;
# {
ltodb%%:
	link	%fp,&-12
# register double tmp_res = a;
	fmov.l	%d0,%fp0
# double tmp_res2 = (double) tmp_res;
	fmov.d	%fp0,-8(%fp)
# return( tmp_res2);
	mov.l	-8(%fp),%d0
	mov.l	-4(%fp),%d1
# }
	unlk	%fp	
	rts
 
 
 
 
 
# double ultodb(a)
# long a;
# {
ultodb%%:
	link	%fp,&-12
# register double tmp_res;
# double tmp_res2;
# register long big_num = 0;
	clr.l	%d1
# if (a > max_unsigned)
	tst.l	%d0			# is a > max_signed???
	bpl.b	L%small
#	{
# a -= 0x80000000;
	and.l	&0x7fffffff,%d0		# make it fit unsigned pos.  range
# big_num = 1;
	mov.l	&1,%d1
#	}
L%small:
# tmp_res = a;
	fmov.l	%d0,%fp0
# if (big_num)
	tst.l	%d1
	beq.b	L%notbig
#	{
#		tmp_res += (double) 0x80000000;
	mov.l	&0x40000000,%d1
	fadd.l	%d1,%fp0
	fadd.l	%d1,%fp0		# add 0x80000000 to our double res
#	}
L%notbig:
# tmp_res2 = tmp_res;
	fmov.d	%fp0,-8(%fp)
# return( tmp_res2);
	mov.l	-8(%fp),%d0
	mov.l	-4(%fp),%d1
# }
	unlk	%fp	
	rts
