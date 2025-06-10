/*	@(#)expand2.c	6.1			*/
static	char	sccsid[] = "@(#) expand2.c 6.1 11/5/84";
/* static	char	sccsid[] = "@(#) expand2.c: 2.3 6/30/83"; */


#include	"symbols.h"
#include	"expand.h"
#include	"expand2.h"

rangetag	range[NITYPES] =
		{
			{-32759L,32759L},	/* SDI1   BRA	*/
			{-32759L,32759L},	/* SDI2   BCC	*/
			{-32759L,32759L},	/* SDI3   BSR	*/
			{-32759L,32759L},	/* SDI4   ABS	*/
			{-32759L,32759L},	/* SDI5   IS	*/
			{-32759L,32759L}	/* SDI6   IL	*/
		};

char	pcincr[NITYPES] =
	{
		4,
		4,
		4,
		4,
		6,
		8
	};

char	idelta[NITYPES] =
	{
		2,
#ifndef M68020
		4,
#else
		2,
#endif /* M68020 */
		2,
		2,
		2,
		2
	};
