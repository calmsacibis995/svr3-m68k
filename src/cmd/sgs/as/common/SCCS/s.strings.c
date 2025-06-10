h36754
s 00000/00000/00050
d D 1.6 86/09/16 15:28:53 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00050
d D 1.5 86/09/15 14:01:48 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00050
d D 1.4 86/09/11 13:02:34 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00050
d D 1.3 86/08/18 08:48:29 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00006/00003/00044
d D 1.2 86/08/07 08:03:36 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00047/00000/00000
d D 1.1 86/08/04 09:38:07 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)strings.c	6.1		*/
/*
 *
 *	"strings.c" is a file containing various functions for doing
 *	string handling and storage in the assembler.  The following
 *	functions are provided:					
 *								
 *	compare(str1,str2)	Compares one string to the other and
 *				returns one of the manifest constants
 *				LESS, EQUAL, or GREATER from "gendefs.h"
 *				to indicate how they are related in a
 *				lexicographic ordering.
 *
 */

#include "gendefs.h"
I 2
#include "dbug.h"
E 2

copystr(str1,str2)
	register char *str1, *str2;
{
	register int n = 0;

I 2
	DBUG_ENTER ("copystr");
E 2
	while (*str2++ = *str1++) ++n;
D 2
	return(n);
E 2
I 2
	DBUG_RETURN(n);
E 2
}

/*
 *
 *	"compare" is a function that compares one string to another and
 *	returns one of the constants LESS, EQUAL, or GREATER depending
 *	on how they are related in a lexicographic ordering.  Two
 *	pointers are advanced down the two strings as long as they are
 *	equal.  If the end of the string is reached in this manner,
 *	EQUAL is returned.  Otherwise, the decision to return LESS or
 *	GREATER is made depending on how the two characters that differ
 *	compare with each other.
 *
 */

compare(str1,str2)
	register char *str1,*str2;
{
I 2
	DBUG_ENTER ("compare");
E 2
	while (*str1 == *str2++) 
		if (*str1++ == '\0')
D 2
			return(EQUAL);
	return((*str1 > *--str2) ? GREATER : LESS);
E 2
I 2
			DBUG_RETURN(EQUAL);
	DBUG_RETURN((*str1 > *--str2) ? GREATER : LESS);
E 2
}
E 1
