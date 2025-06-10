#	@(#)link.s	6.1 
# C library -- link

# error = link(old-file, new-file);

	file	"link.s"
	set	link%,9
	global	link
	global	cerror%

link:
	MCOUNT
	mov.l	&link%,%d0
	trap	&0
	bcc.b 	noerror
	jmp 	cerror%
noerror:
	mov.l	&0,%d0
	rts
