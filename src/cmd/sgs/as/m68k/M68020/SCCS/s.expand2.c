h27966
s 00001/00001/00041
d D 1.6 86/11/18 14:21:01 fnf 6 5
c New preprocessor grumbles about random junk after #endif's.
e
s 00000/00000/00042
d D 1.5 86/09/16 15:26:16 fnf 5 4
c Changes to make ident directive work.
e
s 00000/00000/00042
d D 1.4 86/09/11 13:00:38 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00042
d D 1.3 86/08/18 08:46:52 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00042
d D 1.2 86/08/07 08:07:25 fnf 2 1
c Checkpoint changes for shared library stuff.  Still does not work
c but does accept "init" and "lib" section directives without crashing...
e
s 00042/00000/00000
d D 1.1 86/08/04 09:40:29 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
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
D 6
#endif M68020
E 6
I 6
#endif /* M68020 */
E 6
		2,
		2,
		2,
		2
	};
E 1
