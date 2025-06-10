/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)badtrk.c	7.1	*/
/*
 *	badtrk: perform bad track avoidance for disk I/O
 *
 * INTERFACE
 *
 *	This module has been designed to be independent of other UNIX
 *	Kernel modules. It only includes "types.h" for the data types,
 *	"param.h" for PRIBIO, and "dk.h" for the bad track entry data
 *	structure. Three routines comprise the entire external interface:
 *
 *	1. badinit(badinfo,badcnt): which initializes the bad track support
 *		for a given disk. "badinfo" is a pointer to the beginning of
 *		the disk's hash table and binary search tree (explained below).
 *		"badcnt" is the number of bad tracks described in the bad
 *		track tree. badinit() returns a pointer into the "badptr"
 *		array to be used in future calls to baddel() and badchk().
 *		badinit() returns NULL whenever the disk is perfect, or when
 *		an error condition occurs. The disk driver that calls the
 *		badinit() routine should take care to check the case where
 *		"badcnt" is not equal to zero and badinit() returns NULL.
 *		Most drivers should then reject the open(2) request. badinit()
 *		allocates a "badptr" array entry, allocates memory from the
 *		"badpool", copies the "badinfo" into the pool and returns
 *		the pointer to the allocated "badptr" entry.
 *
 *	2. baddel(ep): which deletes bad track support for a disk. If "ep"
 *		is NULL, then nothing needs to be done since nothing was
 *		done in badinit(). Otherwise, free up the allocated memory
 *		back to the "badpool."
 *
 *	3. badchk(ep,currtrk,count,alt): which makes the check of the bad
 *		track information and returns a track count that is guaranteed
 *		to work. That is, badchk() will always return a track count
 *		less than or equal to "count" that it knows will always be
 *		safe to do I/O to (ie, to bad tracks). The alorithm proceeds
 *		as follows:
 *
 *		a. Check the hash table. If the hash table entry is greater
 *			than or equal to "count", then return "count."
 *		b. If not, then search the bad tree looking for the entry
 *			whose "b_bad" value is smallest that is larger than
 *			"currtrk."
 *		c. Return the "b_alt" value in "*alt" and return the value
 *			"b_bad" value less "currtrk" (or "count", whichever
 *			is less). Note that if "currtrk" is bad, then
 *			"b_bad - currtrk" is zero ... the signal to the driver
 *			that the current track is bad.
 *
 * HASH TABLE
 *
 *	The hash table is currently set to contain 256 single byte entries.
 *	Each entry contains a track count that represents the greatest number
 *	of tracks that may be safely accessed from the current track. An entry
 *	of zero states that the current track is bad. An entry of 1 states that
 *	the current track is good but that the next track is bad. An entry of N
 *	states that the next N tracks (including the current track) are good
 *	but that the N+1st track is bad. Of course, the "current" track refers
 *	to any track that hashes to that entry.
 *
 *	The hash function is the track number mod 256. That means that every
 *	256'th track hashes to the same hash table entry. Hence, the entry
 *	must contain the minimum of the maximum permissible track counts.
 *
 *	Mathematically:
 *
 *	Notation:	(x)y		(for all x, y)
 *			(Ex)y		(there exists an x where y is true)
 *			iff		if, and only if
 *			P(x):(x)y	P(x) over all values x where y is true
 *
 *	Let T be the current track number.
 *	Let C be the requested track count.
 *	Let H(n) be the n'th hash table entry.
 *	Let R(t) be the number of tracks that can be read from
 *		track t without hitting a bad track.
 *
 *	Then:
 *
 *	(R(t) == 0) iff	(t is a bad track)
 *	(R(t) == n) iff ((Ex)((R(x) == 0) and (x == t+n)))
 *	H(n) = min(R(x):(x)(x%256 == n))
 *
 *	Hence (offered without proof):
 *
 *	R(t) >= H(t%256)
 *	(H(t%256) == 0) iff ((Ex)((x%256 == t%256) and (R(x) == 0)))
 *
 *	Therefore:
 *
 *	if H(T) >= C, then the I/O may proceed unhindered.
 *
 * BAD TREE
 *
 *	The bad tree is a packed binary search tree of "struct badent."
 *	For node N (N>0), the left (less than) child is in node 2*N and
 *	the right (greater than) child is in node 2*n+1. (Note that the
 *	nodes are numbered from 1, not from 0. For that reason, the saved
 *	address for the bad track tree in the "badptr" array (in b_hash)
 *	is set to the beginning of the tree less the size of one entry.)
 *
 *	When the hash table lookup fails to yield a large enough number,
 *	the tree will be searched. The alternate found during this search
 *	will always be returned whether or not the current track is bad.
 *	In this way, the driver may save the alternate of the next bad
 *	track is must encounter in the current sequence. For example, if
 *	the driver requested C tracks and badchk() only found N (N<C), then
 *	the alternate A will be returned in "*alt." The driver can then
 *	perform the N track "preamble" I/O, and the alternate before calling
 *	badchk() again for the "postscript."
 *
 * BADPOOL
 *
 *	The "badpool" is a chunk of memory that is used to store the bad
 *	track information for an opened disk. Normally, a disk driver will
 *	read the bad information from the disk and call badinit(). The
 *	driver will call badchk() on all I/O and eventually call baddel()
 *	when the disk is completely close(2)'d. (Note that the driver could
 *	implement a "hardware assisted" bad track algorithm only calling
 *	badchk() when a bad track is encountered during I/O. A small penalty
 *	is paid since the hash table lookup that is done before the tree
 *	search will always yield 0; since the track is known to be bad.)
 *
 *	The "badpool" is set up so that a minimum of "MINDISK" disks can
 *	be simultaneously supported. Since the "badpool" is efficiently
 *	(space) managed, then it's possible that more than "MINDISK" disks
 *	can be supported. Hence, "MAXDISK" (some value greater than "MINDISK")
 *	entries are allocated for the "badptr" array ... possibly supporting
 *	up to "MAXDISK" disks. Each disk will use, at most, and estimated
 *	size of 3 times "HASHSIZE." This estimate comes from the following
 *	analysis: if the bad track tree is greater than 2 times "HASHSIZE",
 *	then there are at lease "HASHSIZE"/2 bad tracks on the disk. At this
 *	point, the hash table entries are so small that it is nearly useless.
 *	Hence, if media becomes so poor that this many bad tracks becomes
 *	common, then either (i) the hash table approach needs to be abandoned,
 *	or (ii) the size of the hash table needs to be doubled.
 *
 *	Three procedures manage the badpool:
 *
 *	1. badalloc(size): The bad pool allocation procedure searches the free
 *		list (pointed to by "allptr") looking for a block of free
 *		memory greater than or equal to size. If it finds one, it
 *		reconstructs the free list (since the address of the list
 *		entry is the address allocated). If not, it returns NULL to
 *		indicate failure.
 *
 *	2. badfree(fptr,size): The bad pool free procedure searches the free
 *		list looking for the point to reinsert the block being freed.
 *		(Thus, the free list is always sorted by address of each
 *		list entry.) When it is reinserted, the block is combined
 *		with adjacent free blocks to provide improved allocation
 *		and collection performance. There are several cases on the
 *		free operation:
 *
 *		A. Insertion occurs at the end of the freelist:
 *			i. If the list is empty, then simply point "allptr"
 *				at the new free list entry ("fptr").
 *			ii. Else, if the previous last list item can be
 *				combined with the new block, then simply
 *				increase the size of the last block.
 *			iii. Otherwise, link in the new entry following
 *				the last entry.
 *
 *		B. Insertion occurs in the middle of the freelist (4 cases):
 *			i. Pure insert: new block cannot be combined with
 *				either the previous or the next blocks.
 *			ii. Downward Combination: the new block can only
 *				be combined with the previous block.
 *			iii. Upward Combination: the new block can only
 *				be combined with the next block.
 *			iv. Total Combination: the new block can be combined
 *				with both the previous and the next blocks.
 *
 *	3. collect(): Perform a garbage collection on the "badpool." The
 *		idea of the garbage collection is to move all allocated
 *		chunks of the "badpool" to the beginning of the pool. In
 *		this way, the result of the collect() function is that
 *		exactly one free block will exist in the free list and it
 *		will point to a block whose end is at the end of the
 *		"badpool." Note that allocated areas can be moved since
 *		the only pointers into the "badpool" are in the free list
 *		and the "badptr" array. Both can be modified when the
 *		free list is modified or an allocated region is moved.
 *
 * BADPOOL FREE LIST
 *
 *	The "badpool" free list is managed using "struct pent" (pool entries)
 *	in the "badpool" itself. For each free area in the "badpool", a
 *	"struct pent" resides in the first "sizeof(struct pent)" bytes of
 *	the area. The pool entry contains a pointer to the next free area
 *	and the size of the current free area. Hence, if a region at location
 *	L of size S is not allocated, then at location L a "struct pent" will
 *	point to the next free area and the size recorded there is S. The
 *	end of that free area is at location L+S (a fact used thoughout this
 *	module to combine adjacent free areas). If S' bytes are allocated out
 *	of this entry (where S' < S), then a new "struct pent" will be created
 *	at L+S' and will describe an area of size S-S'.
 *
 * SUGGESTIONS FOR READERS
 *
 *	Draw lots of pictures.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/dk.h"

#ifdef DEBUG
static char *assertctl = "ASSERTION failed: %s\n";

#define ASSERT(exp, desc)	{if (!(exp)) {\
					printf(assertctl, desc);\
					}}
#define STATIC 
#else
#define ASSERT(exp, desc)
#define STATIC static
#endif

#define	INUSE	1

#define	PENTSIZE	(sizeof(struct pent))

#define	round(x)	(((x)+PENTSIZE-1)&(-PENTSIZE))

extern	struct	bentry	badptr[];
extern	struct	pent	badpool[];
extern	int badmax;
extern	int badnpent;

#define	MAXDISK		badmax
#define POOLENTS	badnpent

STATIC	struct	pent	*allptr = NULL;
STATIC	char initial = 0;
STATIC	char badinuse = 0;

/*
 *	badalloc: badpool allocation
 */

STATIC unsigned char *badalloc(size)
register size;		/* in bytes */

{	register struct pent *aptr = allptr;
	register struct pent *lptr = NULL;
	register struct pent *nptr = NULL;

	if( initial == 0 ) {	/* initialize allocation pointer */
		initial++;
		allptr = aptr = &badpool[0];
		aptr->nxtblk = NULL;
		aptr->blksize = sizeof(struct pent) * POOLENTS;
	}

	/*
	 *	size is forced to be a multiple of the size
	 *		of struct pent
	 */

	size = round(size);

	ASSERT(size > 0, "badalloc() size > 0");
	/*
	 *	search for an area large enough
	 */

	while( aptr != NULL ) {
		if( aptr->blksize > size ) {	/* found a larger spot */

			/*
			 *	calculate the new entry's contents
			 */

			nptr = (struct pent *)((unsigned)aptr+size);

			/*
			 *	dequeue aptr and enqueue nptr
			 */

			if( lptr == NULL ) {
				allptr = nptr;
			} else {
				lptr->nxtblk = nptr;
			}

			/*
			 *	set up nptr entry
			 */

			nptr->nxtblk = aptr->nxtblk;
			nptr->blksize = aptr->blksize - size;

			/*
			 *	return found spot
			 */

			ASSERT(nptr != nptr->nxtblk, 
				"nptr != nptr->nxtblk");

			return((unsigned char *)aptr);
		}
		else if (aptr->blksize == size) {

			/* perfect fit - just link around the block */

			if (lptr == NULL)	/* at the begining */
				allptr = aptr->nxtblk;
			else
				lptr->nxtblk = aptr->nxtblk;
			return((unsigned char *)aptr);
		}

		lptr = aptr;
		aptr = aptr->nxtblk;
		ASSERT(aptr != nptr, "aptr != nptr");
	}

	return(NULL);
}

/*
 *	badfree: free up space
 */

STATIC badfree(fptr,size)
register struct pent *fptr;
register size;

{	register struct pent *optr;
	register struct pent *lptr = NULL;

	if( initial == 0 )
		return;

	size = round(size);

	ASSERT(size > 0, "badfree() size > 0");
	ASSERT(fptr != NULL, "fptr != NULL");

	/*
	 *	find insertion point
	 */

	for( optr=allptr; (optr != NULL)&&(optr < fptr); optr=optr->nxtblk )
		lptr = optr;

	/*
	 *	if optr == NULL, then insert at end
	 */

	ASSERT(optr == NULL || fptr < optr, "optr == NULL || fptr < optr");
	ASSERT(lptr == NULL || lptr < fptr, "lptr == NULL || lptr < fptr");

	if( optr == NULL ) {

		/*
		 *	if lptr == NULL, then pool is full
		 *
		 *	that is, there are no free blocks
		 */

		if( lptr == NULL ) {
			allptr = fptr;

		/*
		 *	if last block butts up against new block, then combine
		 */

		} else if((struct pent *)((unsigned)lptr+lptr->blksize)==fptr){
			lptr->blksize += size;
			return;

		/*
		 *	otherwise, link into last spot
		 */

		} else
			lptr->nxtblk = fptr;

		/*
		 *	new block at end
		 */

		fptr->nxtblk = NULL;
		fptr->blksize = size;
		return;
	}

	/*
	 *	ASSERT: optr != NULL
	 *	IMPLIES: need to insert new block just before optr
	 */

	/*
	 *	if lptr == NULL, then insert at beginning of queue
	 */


	if( lptr == NULL ) {
		allptr = fptr;

	/*
	 *	if last block butts up against new block, then combine
	 */

	} else if((struct pent *)((unsigned)lptr+lptr->blksize) == fptr ) {
			lptr->blksize += size;
			fptr = lptr;
			size = fptr->blksize;

	/*
	 *	otherwise, insert new block after last block
	 */

	} else
		lptr->nxtblk = fptr;

	/*
	 *	if new block butts up against the next block (optr),
	 *		then combine
	 */

	if((struct pent *)((unsigned)fptr + size) == optr ) {
		fptr->nxtblk = optr->nxtblk;
		fptr->blksize = size + optr->blksize;

	/*
	 *	otherwise, insert new block before next block
	 */

	} else {
		fptr->nxtblk = optr;
		fptr->blksize = size;
	}

	ASSERT(fptr->nxtblk != fptr, "fptr->nxtblk != fptr");
}

/*
 *	collect: garbage collect the bad pool
 *
 *		Don't do this one too often.
 */

STATIC collect()

{	register i;
	register struct pent *pp;
	register struct pent *np;
	register struct bentry *ep;
	register size;
	register len;


	if((pp = allptr) == NULL)
		return;

	/*
	 *	Walk through badpool freelist packing entries
	 *	until the current pent pointer (pp) points to
	 *	a free block whose end is at the end of the badpool.
	 */

	while(( np = (struct pent *)( (unsigned)pp + pp->blksize ))
				< (&badpool[POOLENTS])) {

		/*
		 *	search through the badptr array looking for
		 *		allocated entries. Find the entry
		 *		whose hash table pointer is equal to
		 *		the end of the free block pointed to by pp.
		 */

		for( i=0, ep = badptr; i < MAXDISK; i++, ep++ ) {

			if( ! (ep->b_stat&INUSE) )
				continue;

			if( ep->b_hash == (unsigned char *)np ) {
				break;
			}
		}

		ASSERT(i < MAXDISK, "collect() i < MAXDISK");

		/*
		 *	save the contents of *pp before moving
		 */

		np = pp->nxtblk;
		size = pp->blksize;

		/*
		 *	compute length of allocated area
		 */

		len = ep->b_tsize+HASHSIZE;
		len = round(len);

		/*
		 *	move badptr entry
		 */

		i = spl7();
		bcopy( ep->b_hash, pp, len );
		ep->b_hash = (unsigned char *)pp;
		ep->b_tree = (struct badent *)((unsigned)pp+HASHSIZE
						-sizeof(struct badent));
		splx(i);

		/*
		 *	recreate pp entry at new location
		 */

		pp = (struct pent *)((unsigned)pp+len);
		pp->blksize = size;
		pp->nxtblk = np;

		/*
		 *	if pp butts up against the next entry, then combine
		 */

		if((unsigned)pp+pp->blksize == (unsigned)np) {
			pp->blksize += np->blksize;
			pp->nxtblk = np->nxtblk;
		}
	}

	/*
	 *	only one free block in pool at pp
	 */

	ASSERT(pp->nxtblk == NULL, "collect() pp->nxtblk == NULL");
	ASSERT(np == &badpool[POOLENTS], "collect() np == &badpool[POOLENTS]");

	allptr = pp;
}

/*
 *	badinit: initialize bad track support for a disk
 */

struct bentry *badinit(badinfo,badcnt)
char	*badinfo;		/* hash table and tree */
int	badcnt;			/* number of bad tracks */

{	register x;
	register tsize;
	register struct bentry *ep;
	register unsigned char *hp;

	/*
	 *	if no bad tracks, then return NULL
	 */

	if( badcnt == 0 )
		return(NULL);

	/*
	 *	Only one at a time through here
	 */

	x = spl7();
	while(badinuse)
		sleep(&badinuse,PRIBIO);
	badinuse++;
	splx(x);

	/*
	 *	find a vacant entry for this disk
	 */

	for( ep=badptr, x=0; (x<MAXDISK) && (ep->b_stat&INUSE); x++, ep++ );

	if( x == MAXDISK ) {
		printf("Bad table overflow\n");
		badinuse=0;
		wakeup(&badinuse);
		return(NULL);
	}

	/*
	 *	Compute the tree size
	 */

	tsize = badcnt*sizeof(struct badent);

	/*
	 *	allocate space for hash table and tree
	 */

	if(( hp = badalloc(tsize+HASHSIZE)) == NULL ) {
		collect();
		if((hp=badalloc(tsize+HASHSIZE))==NULL) {
			printf("Bad track pool overflow\n");
			badinuse=0;
			wakeup(&badinuse);
			return(NULL);
		}
	}

	/*
	 *	set up entry
	 */

	ep->b_stat |= INUSE;
	ep->b_tsize = tsize;
	ep->b_hash = hp;
	ep->b_tree = (struct badent *)((unsigned)hp+HASHSIZE
				-sizeof(struct badent));
	bcopy( badinfo, ep->b_hash, ep->b_tsize+HASHSIZE );
	badinuse=0;
	wakeup(&badinuse);
	return(ep);
}

/*
 *	baddel: delete bad track support for this disk
 */

baddel(ep)
register struct bentry *ep;

{	register x;

	if( ep == NULL )
		return;

	/*
	 *	Only one through here at a time
	 */

	x = spl7();
	while(badinuse)
		sleep(&badinuse,PRIBIO);
	badinuse++;
	splx(x);

	ASSERT(ep->b_stat & INUSE, "baddel() ep->b_stat & INUSE");

	if (ep->b_stat & INUSE)		/* check just to prevent disaster */
		badfree(ep->b_hash, HASHSIZE + ep->b_tsize);
	ep->b_stat &= ~INUSE;

	badinuse = 0;
	wakeup(&badinuse);
}

/*
 *	badchk: bad track check routine.
 */

badchk( ep, currtrk, count, alt )
register struct bentry *ep;
union {
	long trkno;
	unsigned char tnob[4];
} currtrk;
unsigned short count;
unsigned *alt;

{	register short offset;
	register struct badent *bp;
	register char  *tp;
	register short last;
	register short tsize;
	register unsigned short ctrk;
	register unsigned short cnt;
	register unsigned char hval;

	if( ep == NULL )
		return( count );

	ASSERT(ep->b_stat & INUSE, "badchk() ep->b_stat & INUSE");

	cnt = count;

	/*
	 *	currtrk.tnob[3] == currtrk.trkno%256
	 */

	hval = ep->b_hash[ currtrk.tnob[3] ];
	if( hval >= cnt )
		return(cnt);


	ctrk = currtrk.trkno;
	offset = sizeof(struct badent);
	tsize = ep->b_tsize;
	tp = (char *)ep->b_tree;

	while( offset <= tsize ) {
		if( ctrk > ((struct badent *)(tp+offset))->b_bad ) {
			offset += offset;
			offset += sizeof(struct badent);
		} else {
			last = offset;
			offset += offset;
		}
	}
	*alt = ((struct badent *)(tp+last))->b_alt;
	ctrk = ((struct badent *)(tp+last))->b_bad - ctrk;
	return( ctrk < cnt? ctrk: cnt );
}
