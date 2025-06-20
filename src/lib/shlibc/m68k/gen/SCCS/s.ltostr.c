h35292
s 00042/00000/00000
d D 1.1 86/07/31 12:27:02 fnf 1 0
c Pick up from Matts version for 5.3 m68k, apparently from 5.2 m32 version.
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-m32:gen/ltostr.c	1.3"
/*
 *	ltostr -- convert long to decimal string
 *
 *
 *	char *
 *	ltostr(value, ptr)
 *	long value;
 *	char *ptr;
 *
 *	Ptr is assumed to point to the byte following a storage area
 *	into which the decimal representation of "value" is to be
 *	placed as a string.  Ltostr converts "value" to decimal and
 *	produces the string, and returns a pointer to the beginning
 *	of the string.  No leading zeroes are produced, and no
 *	terminating null is produced.  The low-order digit of the
 *	result always occupies memory position ptr-1.
 *	Ltostr's behavior is undefined if "value" is negative.  A single
 *ero digit is produced if "value" is zero.
 *
 */

char *
_ltostr(value, ptr)
register long value;
register char *ptr;
{
	register long t;

	do {
		*--ptr = '0' + value - 10 * (t = value / 10);
	} while ((value = t) != 0);

	return(ptr);
}
E 1
