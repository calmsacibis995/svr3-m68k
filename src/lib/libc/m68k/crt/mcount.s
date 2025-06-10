#	@(#)mcount.s	6.1 
# count subroutine called during profiling

	file	"mcount.s"
	global	mcount%
	comm	_countbase,4

	text
mcount%:
	mov.l	(%a0),%d0
	beq.b	init
	mov.l	%d0,%a1
incr:
	add.l	&1,(%a1)
return:
	rts
init:
	mov.l	_countbase,%d0
	beq.b	return
	mov.l	%d0,%a1
	add.l	&8,%d0
	mov.l	%d0,_countbase
	mov.l	(%sp),(%a1)+
	mov.l	%a1,(%a0)
	bra.b 	incr
