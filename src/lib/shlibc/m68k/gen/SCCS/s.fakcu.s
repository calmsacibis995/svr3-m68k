h37084
s 00015/00000/00000
d D 1.1 86/07/31 12:27:00 fnf 1 0
c Pick up from Matts version for 5.3 m68k, apparently from 5.2 m32 version.
e
u
U
t
T
I 1
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
.ident	"@(#)libc-m32:gen/fakcu.s	1.3"
# dummy cleanup routine if none supplied by user.

	.globl	_cleanup

_cleanup:
	ret
E 1
