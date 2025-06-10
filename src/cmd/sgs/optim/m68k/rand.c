/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)rand.c	7.2		*/
/* rand.c */
static char SCCSID[] = "@(#) rand.c:	2.1 83/07/08";

/*	MC68000 Optimizer */

#include "optim.h"



/* Operands are managed in a linked hash table. */

#define	RANDSIZE	511	/* number of table slots */

/* Define table entries */

struct randentry
{
    RAND rand;			/* this is the actual operand */
    struct randentry * next;	/* pointer to next linked entry */
};

typedef struct randentry RANDENTRY;

#define	RENULL	((RANDENTRY *) 0) /* null table entry */

static RANDENTRY * hashtab[RANDSIZE];	/* declare the hash table */
/* Some simple service functions first. */

/* rhash -- hash an operand string
**
** This routine returns a hash number for an operand string.
** The algorithm was arrived at empirically to give good scatter
** on a large set of test data.
*/

unsigned long
rhash(s,n)
register char * s;		/* operand string */
register int n;			/* string length */
{
    register unsigned long hashval = *s++;

    while (--n > 0)
	hashval = (hashval << 3) + *s++;
    
    return (hashval);
}
/* clrhash -- initialize the hash table */

void
clrhash()
{
    register int i;
    register RANDENTRY ** rptr;

    for (i = RANDSIZE, rptr = hashtab; i != 0; *rptr++ = RENULL, i--)
	;
    
    return;
}
/* saverand -- save an operand, return its operand pointer
**
** This routine looks for an existing RAND structure, given a character
** string.  If the structure exists already, a pointer to it is returned.
** Otherwise one of two things happens:
**	If the creation flag is set, a new RAND is built, and its
**	pointer is returned; otherwise, an RNULL pointer is returned.
*/

RAND *
saverand(ptr, len, type, ruses, create)
char * ptr;			/* operand string */
int len;			/* operand string length */
RTYPE type;			/* operand type */
REG ruses;			/* register data */
boolean create;			/* true if okay to create new one */
{
    char * memcpy();
    char * temp;
    int memcmp();
    int slot = rhash(ptr,len) % RANDSIZE;
				/* choose initial hash table slot */
    register RANDENTRY ** rptr = &hashtab[slot];
				/* prepare to search that slot's chain */
    
    /* run down the chain of entries dangling from this slot, looking for
    ** a character string match.
    */

    while (*rptr != RENULL && 
           ((temp = (*rptr)->rand.rname)[len] != '\0' ||  /* check length */
	   memcmp(temp, ptr, len) != 0))
	rptr = &((*rptr)->next);
    
    /* Either we have run out of slots or found an existing equivalent
    ** operand.  Check which.
    */

    if (*rptr != RENULL)
	return(&((*rptr)->rand)); /* an existing operand.  Return its pointer */
    
    if (! create)		/* check for permission to create, since not
				** found
				*/
	return (RNULL);
/* The desired operand is not in the table.  We must add it.  "rptr" points
** at the place to store the pointer to the new table entry.
*/

    {
	char * newname;		/* string in which operand is saved */
	RANDENTRY * newentry;	/* new table entry */

	/* prepare to copy operand string into permanent memory */

	newname = getspace((unsigned)(len + 1));
	(void) memcpy(newname, ptr, len);
	newname[len] = '\0';	/* make null-terminated string */

	newentry = GETSTR(RANDENTRY); /* get new table entry */

	/* fill in the pieces */

	*rptr = newentry;	/* put new entry at end */
	newentry->next = RENULL; /* end of the chain */

	newentry->rand.rname = newname;
	newentry->rand.rtype = type;
	newentry->rand.ruses = ruses;

	return( &newentry->rand ); /* return the completed entry */
    }
}
/* findrand -- find an existing operand string
**
** Findrand finds an existing operand and returns a pointer to its
** operand structure.
*/

RAND *
findrand(ptr, flag)
char * ptr;			/* null terminated string */
boolean flag;			/* true if operand must exist */
{
    int strlen();
    RAND * r =  saverand(ptr, strlen(ptr), (RTYPE) 0, REG_NONE, false);
				/* find operand, don't create new one */

    if (! flag || r != RNULL)	/* if operand needn't exist, return what we
				** got; always return real pointer
				*/
	return(r);

    oerr("Can't find operand");	/* otherwise error */

/*NOTREACHED*/
}
