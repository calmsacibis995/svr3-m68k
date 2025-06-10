#	Stub routine for distributed unix - streams #
#	@(#)stream.s	

	text
strgetmsg:	global	strgetmsg	#
strputmsg:	global	strputmsg	#
sealloc:	global	sealloc		#
sefree:		global	sefree		#
strpoll:	global	strpoll		#
stropen:	global	stropen		#
strread:	global	strread		#
strwrite:	global	strwrite	#
strioctl:	global	strioctl	#
strclean:	global	strclean	#
strclose:	global	strclose	#
runqueues:	global	runqueues	#
	rts

	data
	global	qrunflag
	global	msglistcnt
qrunflag:
msglistcnt:
	long	0			# MUST be zero


