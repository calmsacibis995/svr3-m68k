h31053
s 00042/00000/00000
d D 1.1 86/07/31 10:26:04 fnf 1 0
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

#ident	"@(#)libc-port:gen/tfind.c	1.4"
/*LINTLIBRARY*/
/*
 * Tree search algorithm, generalized from Knuth (6.2.2) Algorithm T.
 *
 * The NODE * arguments are declared in the lint files as char *,
 * because the definition of NODE isn't available to the user.
 */

#include <search.h>
typedef char *POINTER;
typedef struct node { POINTER key; struct node *llink, *rlink; } NODE;

#define	NULL	0


/*	tfind - find a node, or return 0	*/
NODE *
tfind(key, rootp, compar)
POINTER	key;			/* Key to be located */
register NODE	**rootp;	/* Address of the root of the tree */
int	(*compar)();		/* Comparison function */
{
	if (rootp == NULL)
		return (NULL);
	while (*rootp != NULL) {			/* T1: */
		int r = (*compar)(key, (*rootp)->key);	/* T2: */
		if (r == 0)
			return (*rootp);	/* Key found */
		rootp = (r < 0) ?
		    &(*rootp)->llink :		/* T3: Take left branch */
		    &(*rootp)->rlink;		/* T4: Take right branch */
	}
	return (NODE *)(NULL);
}
E 1
