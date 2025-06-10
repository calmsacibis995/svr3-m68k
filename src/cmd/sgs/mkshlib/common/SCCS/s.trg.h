h53104
s 00000/00000/00045
d D 1.2 86/08/18 08:49:29 fnf 2 1
c Checkpoint changes at first working shared library version
e
s 00045/00000/00000
d D 1.1 86/07/31 14:39:30 fnf 1 0
c Initial copy from 5.3 distribution for AT&T 3b2.
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

#ident	"@(#)mkshlib:common/trg.h	1.3"

/* target dependent definitions */
#define BTSIZ	503	/* size of branch hash table, btlst */
#define MAXBT	503	/* initial size of the btorder array */
#define NONE	-3	/* illegal section index number */

/* structure used to hod information about the branch table */
typedef struct btrec {
        char    *name;  /* symbol name */
        long    pos;    /* position in branch table */
        int     found;  /* indicates if symbol was found in pltname (initially 0) */
        struct btrec *next;
} btrec;

extern char	*pltname,
		*tpltnam,
		*btname,
		*ifil1name,
		*ifil2name;
extern char	*trgpath;
extern char	*assemnam;
extern btrec	*btlst[BTSIZ];
extern long	tabsiz;
extern btrec	**btorder;


/* User supplied functions */
extern void	genbtassem();
extern void	genlibassem();
extern void	labelassem();
extern char	*makelab();
extern void	mkassem();
extern void	mkplt();
extern void	mktarget();
extern btrec	*newbtrec();
extern void	patchsym();
extern void	prlbtassem();
E 1
