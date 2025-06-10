h56333
s 00000/00000/00325
d D 1.6 86/09/16 15:28:55 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00325
d D 1.5 86/09/15 14:01:50 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00325
d D 1.4 86/09/11 13:02:36 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00325
d D 1.3 86/08/18 08:48:30 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00058/00011/00267
d D 1.2 86/08/07 08:03:38 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00278/00000/00000
d D 1.1 86/08/04 09:38:08 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)symbols.c	6.1		*/

#include <stdio.h>
#include <string.h>
#include "syms.h"
#include "systems.h"
#include "symbols.h"
#include "symbols2.h"
I 2
#include "dbug.h"
E 2

/*
 *	"symbols.c" is a file containing functions for accessing the
 *	symbol table.  The following functions are povided:
 *
 *	newent(string)
 *		Creates a new symbol table entry for the symbol with name
 *		"string".  The type of the symbol is set to undefined.
 *		All other fields of the entry are set to zero.
 *
 *	lookup(string,install_flag,user_name_flag)
 *		Looks up the symbol whose name is "string" in the symbol
 *		table and returns a pointer to the entry for that symbol.
 *		Install_flag is INSTALL if symbol is to be installed,
 *		N_INSTALL if it is only to be looked up.  User_name_flag
 *		is USRNAME if the symbol is user-defined, MNEMON if it is
 *		an instruction mnemonic.
 *
 *
 *	traverse(func)
 *		Goes through the symbol table and calls the function "func"
 *		for each entry.
 *
 *	dmpstb()
 *		Dumps the symbol table out to the intermediate file
 *		"fdstab" after pass one. The file descriptor "fdstab"
 *		should be open for writing on the intermediate file that
 *		is to contain the symbol table before calling this
 *		function.  This procedure is only used in the multiple-
 *		process version of the assembler.
 *
 *	creasyms()
 *		Enters the instruction mnemonics found in instab[] into
 *		the symbol table.
 *
 *	addstr(string)
 *		Enters the "string" into the string table.  Called by
 *		newent().  Space for the string table is initially
 *		malloc()-ed in strtabinit().  If "string" would exceed
 *		the available space, then the string table is realloc()-ed
 *		with a larger size.  This procedure is only used in the
 *		flexnames version of the assembler.
 *
 *	strtabinit()
 *		Sets up the string table, with space malloc()-ed.  This
 *		procedure is only used in the flexnames version of the
 *		assembler.
 */

#if ONEPROC
extern short passnbr;
#endif

symbol
	symtab[NSYMS];
upsymins
	hashtab[NHASH];

#if FLEXNAMES
I 2
long	size,
	basicsize = 4 * BUFSIZ;
E 2
char	*strtab;
long	currindex;

char	*realloc(),
	*malloc();
#endif

short	symcnt = 0;


symbol *
newent(strptr)
	register char *strptr;
{
	register symbol *symptr;
	register char *ptr1;

I 2
	DBUG_ENTER ("newent");
E 2
	if (symcnt >= NSYMS) {
		aerror("Symbol table overflow");
D 2
		return(NULL);
E 2
I 2
		DBUG_RETURN((symbol *)NULL);
E 2
	}
	symptr = &symtab[symcnt++];
#if FLEXNAMES
	if (strlen(strptr) > SYMNMLEN)
	{
		symptr->_name.tabentry.zeroes = 0L;
		symptr->_name.tabentry.offset = currindex;
		addstr(strptr);
	}
	else
#endif
		for (ptr1 = symptr->_name.name; *ptr1++ = *strptr++; )
			;
	symptr->styp = UNDEF;
	symptr->value = 0L;
	symptr->tag = 0;
	symptr->maxval = 0;
D 2
	return(symptr);
E 2
I 2
	DBUG_RETURN(symptr);
E 2
}

#if PASS1 || ONEPROC
#if DEBUG

unsigned numcalls,
	numids,
	numcoll;
#endif
#endif

upsymins *
lookup(sptr,install,usrname)
	char *sptr;
	BYTE install;
	BYTE usrname;
{
	register upsymins
		*hp;
	register char
		*ptr1,
		*ptr2;
	unsigned short register
		ihash = 0,
		hash;
	unsigned short probe;
	upsymins
		*ohp;

I 2
	DBUG_ENTER ("lookup");
	DBUG_5 ("lk1", "sptr='%s', install=%d, usrname=%d", sptr, install, usrname);
E 2
	ptr1 = sptr;
#if PASS1 || ONEPROC
#if DEBUG
	numcalls++;
#endif
#endif
	while (*ptr1) {
		ihash = ihash*4 + *ptr1++;
	}
	probe = 1;
	ihash += *--ptr1 * 32;
	hash = ihash % NHASH;
I 2
	DBUG_3 ("lkx", "hash value = %d", hash);
E 2
	hp = ohp = &hashtab[hash];
	do {
		if ((*hp).stp == NULL) {	/* free */
			if (install) {
#if PASS1 || ONEPROC
#if DEBUG
				numids++;
#endif
#endif
				(*hp).stp = newent(sptr);
			}
D 2
			return(hp);
E 2
I 2
			DBUG_RETURN(hp);
E 2
		}
		else
		{
#if FLEXNAMES
		/* Compare the string given with the symbol string.	*/
		/* The symbol string can be in either the symbol entry	*/
		/* or the string table.					*/
D 2
 			if (strcmp(sptr,(hp->stp->_name.tabentry.zeroes != 0) ? (hp->stp->_name.name) : (&strtab[hp->stp->_name.tabentry.offset])) == 0)
E 2
I 2
			register char *namep;
			register int index;
			if (hp->stp->_name.tabentry.zeroes != 0) {
				namep = hp->stp->_name.name;
			} else {
				index = hp->stp->_name.tabentry.offset;
				DBUG_3 ("lk3", "index=%d", index);
				if (index >= size) {
					DBUG_3 ("err", "at %#x", &hp->stp->_name.tabentry.offset);
					aerror("hash table corrupted");
					namep = "**bogus**";
				} else {
					namep = &strtab[index];
				}
			}
 			DBUG_3 ("lk2", "*namep='%s'", namep);
 			if (strcmp(sptr,namep) == 0)
E 2
#else
			for (ptr1=sptr,ptr2=((*hp).stp)->_name.name; *ptr2==*ptr1++; )
				if (*ptr2++ == '\0')
#endif
			{
I 2
			    DBUG_3 ("lk4", "found match for '%s'", sptr);
E 2
#if PASS1 || ONEPROC
#if ONEPROC
			    if (passnbr == 1) {
#endif
					if (install && (*hp).itp->tag &&
						(*hp).itp->snext == NULL)
					{
						(*hp).itp->snext = newent(sptr);
					} /* if (install ...) */
					if (!usrname && (*hp).itp->tag) {
D 2
						return((upsymins *) &((*hp).itp->snext));
E 2
I 2
						DBUG_RETURN((upsymins *) &((*hp).itp->snext));
E 2
					} /* if (!usrname ...) */
#if ONEPROC
			        }
					if (!install && !usrname &&
					    (*hp).itp->tag && (passnbr==2))
D 2
				            return((upsymins *) &((*hp).itp->snext));
E 2
I 2
				            DBUG_RETURN((upsymins *) &((*hp).itp->snext));
E 2
					else
#endif
#endif
D 2
				            return(hp); /* found it */
E 2
I 2
				            DBUG_RETURN(hp); /* found it */
E 2
				} /* if (*ptr2++ == '\0') */
D 2

E 2
			hash = (hash + probe) /*% NHASH*/;
			hash -= (hash >= NHASH) ? NHASH : 0;
I 2
			DBUG_3 ("lk5", "hash collision, try probe at %d", hash);
E 2
			probe += 2;
#if PASS1 || ONEPROC
#if DEBUG
			numcoll++;
#endif
#endif
		} /* else */
	} while ((hp = &hashtab[hash]) != ohp);
	aerror("Hash table overflow");
D 2
	return(NULL); /* can't reach here since `aerror' exits */
E 2
I 2
	DBUG_RETURN((upsymins *)NULL); /* can't reach here since `aerror' exits */
E 2
}

#if !PASS1
traverse(func)
	int (*func)();
{
	register short index;

I 2
	DBUG_ENTER ("traverse");
E 2
	for (index=0; index < symcnt; ++index) {
		(*func)(&symtab[index]);
	}
I 2
	DBUG_VOID_RETURN;
E 2
}
#endif
#if PASS1 && !ONEPROC

FILE *fdstab;

dmpstb(){
I 2
	DBUG_ENTER ("dmpstb");
E 2
	fwrite((char *)symtab,symcnt,SYMBOLL,fdstab);
I 2
	DBUG_VOID_RETURN;
E 2
}

#endif
#if PASS1 || ONEPROC

extern instr instab[];

creasyms()
{
	register instr *ip;
        register upsymins *hp;

I 2
	DBUG_ENTER ("creasyms");
E 2
	for (ip = instab; ip->name[0] != '\0'; ++ip) {
		hp = lookup(ip->name,N_INSTALL,MNEMON);
#if DEBUG
		/* Sanity check is "cpp-ed out" with the flexnames change. */
		if ((*hp).itp == NULL)
#endif
		(*hp).itp = ip;
#if DEBUG
		else
			aerror("Duplicate instruction table name");
#endif
	} /* for */
I 2
	DBUG_VOID_RETURN;
E 2
}
#endif


#if FLEXNAMES
D 2
long	size,
	basicsize = 4 * BUFSIZ;
E 2

addstr(strptr)
	register char	*strptr;
{
	register int	length;

I 2
	DBUG_ENTER ("addstr");
E 2
	length = strlen(strptr);
	if (length + currindex >= size)
		if ((strtab = realloc(strtab,size += basicsize)) == NULL)
			aerror("cannot realloc string table");
	strcpy(&strtab[currindex],strptr);
	currindex += length + 1;
I 2
	DBUG_VOID_RETURN;
E 2
}	/* addstr(strptr) */


strtabinit()
{
I 2
	DBUG_ENTER ("strtabinit");
E 2
	if ((strtab = malloc(size = basicsize)) == NULL)
		aerror("cannot malloc string table");
	currindex = 4;
I 2
	DBUG_VOID_RETURN;
E 2
}	/* strtabinit() */
#endif
I 2

hashtabsanity(from)
char *from;
{
    register upsymins *hp;
    for (hp=hashtab; hp < &hashtab[NHASH]; hp++) {
	if ((*hp).stp != NULL) {
	    if (hp->stp->_name.tabentry.offset > size) {
		fprintf (stderr, "hash table corrupted from routine '%s'\n", from);
		fflush (stderr);
	    }
	}
    }
}

E 2
E 1
