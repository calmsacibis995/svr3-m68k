h28530
s 00000/00000/00025
d D 1.6 86/09/16 15:25:26 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00025
d D 1.5 86/09/15 14:00:18 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00025
d D 1.4 86/09/11 13:00:01 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00025
d D 1.3 86/08/18 08:47:44 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00025
d D 1.2 86/08/07 08:05:37 fnf 2 1
c Checkpoint shared library changes so far.  Still does not work
c but accepts "init" and "lib" section directives without crashing...
e
s 00025/00000/00000
d D 1.1 86/08/04 09:39:32 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)symbols2.h	6.4		*/
/*	static	char	sccsid[] = "@(#) symbols2.h: 2.1 6/4/83";			*/


/*
 *	NINSTRS	= number of legal assembler instructions (from ops.out)
 *	NHASH	= the size of the hash symbol table
 *	NSYMS	= the number of symbols allowed in a user program
 */

#ifdef M68881
#define N881 215	/* total number of M68881 instructions */
#else
#define N881 0
#endif

#ifdef M68020
#define NINSTRS 249+N881	/* M68020 supports M68881 */
#else
#define	NINSTRS	166
#endif

#define NHASH	4001

#define NSYMS	(NHASH - NINSTRS - 1)
E 1
