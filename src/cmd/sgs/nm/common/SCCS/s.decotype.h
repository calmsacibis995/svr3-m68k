h16852
s 00055/00000/00000
d D 1.1 86/08/19 09:12:02 fnf 1 0
c Initial copy from 5.3 release for 3b2
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

#ident	"@(#)nm:common/decotype.h	1.4"
    /*  decotype.h contains format strings used to print out the type field of
     *  a symbol table entry
     *
     *  the minimum number of characters printed depends on the amount of room 
     *  left by the other fields in the output line (the type field may grow
     *  beyond this minimum) as well as depending on the setting of 
     *	eflag (-e) and numbase.
     */


/* FORMAT STRINGS */

static char	*prtype[3] = {
			"|%18s",
			"|%17s",
			"|%14s"
};

static char	*pretype[3] = {
			"|%16s",
			"|%13s",
			"|%14s"
};


/* TYPE NAMES */
static	char	*typelist[16] = {
			"",
			"arg",
			"char",
			"short",
			"int",
			"long",
			"float",
			"double",
			"struct",
			"union",
			"enum",
			"enmem",
			"Uchar",
			"Ushort",
			"Uint",
			"Ulong"
};

/*
*/
E 1
