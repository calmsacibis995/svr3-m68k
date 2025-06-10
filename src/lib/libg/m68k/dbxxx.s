#	@(#)dbxxx.s	6.1 
	file	"dbxxx.s"
        comm   _dbargs,512

        text
        global  _dbsubc
        global  _dbsubn

_dbsubc:
	link	%fp,&0
        mov.l   &_dbargs,%a0
	mov.l	(%a0)+,%a1	# address of procedure
	mov.l	(%a0)+,%d0	# number of arguments
	mov.l	%d0,%d1
	asl.l	&2,%d1
	add.l	%d1,%a0		# arguments in table are backwards
	bra.b	L%2
L%1:	mov.l	-(%a0),-(%sp)	# move arguments onto stack
L%2:	dbra	%d0,L%1
	jsr	(%a1)		# enter procedure
	unlk	%fp		# clean up stack

_dbsubn:
        trap	&1		# breakpoint - return to sdb
