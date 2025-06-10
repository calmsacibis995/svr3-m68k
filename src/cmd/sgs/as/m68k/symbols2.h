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
