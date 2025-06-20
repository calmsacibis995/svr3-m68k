h38305
s 00077/00000/00000
d D 1.1 86/07/31 10:25:09 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
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

#ident	"@(#)libc-port:gen/gcvt.c	1.10"
/*LINTLIBRARY*/
/*
 * gcvt  - Floating output conversion to
 *
 * pleasant-looking string.
 */

#include "shlib.h"
extern char *ecvt();

char *
gcvt(number, ndigit, buf)
double	number;
int	ndigit;
char	*buf;
{
	int sign, decpt;
	register char *p1, *p2 = buf;
	register int i;

	p1 = ecvt(number, ndigit, &decpt, &sign);
	if (sign)
		*p2++ = '-';
	if (decpt > ndigit || decpt <= -4) {	/* E-style */
		decpt--;
		*p2++ = *p1++;
		*p2++ = '.';
		for (i = 1; i < ndigit; i++)
			*p2++ = *p1++;
		while (p2[-1] == '0')
			p2--;
		if (p2[-1] == '.')
			p2--;
		*p2++ = 'e';
		if (decpt < 0) {
			decpt = -decpt;
			*p2++ = '-';
		} else
			*p2++ = '+';
		for (i = 1000; i != 0; i /= 10) /* 3B or CRAY, for example */
			if (i <= decpt || i <= 10) /* force 2 digits */
				*p2++ = (decpt / i) % 10 + '0';
	} else {
		if (decpt <= 0) {
			*p2++ = '0';
			*p2++ = '.';
			while (decpt < 0) {
				decpt++;
				*p2++ = '0';
			}
		}
		for (i = 1; i <= ndigit; i++) {
			*p2++ = *p1++;
			if (i == decpt)
				*p2++ = '.';
		}
		if (ndigit < decpt) {
			while (ndigit++ < decpt)
				*p2++ = '0';
			*p2++ = '.';
		}
		while (*--p2 == '0' && p2 > buf)
			;
		if (*p2 != '.')
			p2++;
	}
	*p2 = '\0';
	return (buf);
}
E 1
