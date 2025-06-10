#	@(#)fpdata.s	6.1	

	file	"fpdata.s"
	data 1
	even
	global _MaXdOuB,_MaXfLoA,_MiNdOuB ,_MiNfLoA
_MaXdOuB:
	long 0x7fefffff
	long 0xffffffff
_MiNdOuB:
	long 0x00100000
	long 0x0
_MaXfLoA:
	long 0x7f7fffff
_MiNfLoA:
	long 0x00800000
