h35904
s 00032/00000/00000
d D 1.1 86/07/31 12:00:19 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
# "@(#)lmul.s	6.1	";
# long multiply

	file	"lmul.s"
	global	lmul%%

lmul%%:
	link	%fp,&0
	mov.l	%d2,%a0
	mov.l	%d3,%a1
	mov.w	%d0,%d2
	mov.w	%d0,%d1
	ext.l	%d1
	swap.w	%d1
	swap.w	%d0
	sub.w	%d0,%d1
	mov.w	10(%fp),%d0
	mov.w	%d0,%d3
	ext.l	%d3
	swap.w	%d3
	sub.w	8(%fp),%d3
	muls.w	%d0,%d1
	muls.w	%d2,%d3
	add.w	%d1,%d3
	muls.w	%d2,%d0
	swap.w	%d0
	sub.w	%d3,%d0
	swap.w	%d0
	mov.l	%a0,%d2
	mov.l	%a1,%d3
	unlk	%fp
	rts
E 1
