/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/s5/s5blklist.c	10.6"
#include	"sys/types.h"
#include	"sys/param.h"
#include	"sys/fs/s5param.h"
#include	"sys/fs/s5macros.h"
#include	"sys/sysmacros.h"
#include	"sys/immu.h"
#include	"sys/fs/s5dir.h"
#include	"sys/signal.h"
#include	"sys/fs/s5inode.h"
#include	"sys/fstyp.h"
#include	"sys/mount.h"
#include	"sys/inode.h"
#include	"sys/user.h"
#include	"sys/buf.h"
#include	"sys/errno.h"
#include	"sys/debug.h"
#include	"sys/conf.h"

/*
 *  Allocate and build the block address map
 */

s5allocmap(ip)
register struct inode *ip;
{
	register int	i;
	register int	*bnptr;
	register struct s5inode *s5ip;
	register int	blkspp;
	register int	msize;
	register int	nblks;

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	if (s5ip->s5i_map) 
		return(1);

	/*	Get number of blocks to be mapped.
	 */

	ASSERT(s5ip->s5i_map == 0);
	msize = FSBSIZE(ip);		/* Size of file system's mount point */
	nblks = (ip->i_size + msize - 1)/msize;
	blkspp = max(NBPP, msize)/msize;

	/*	Round up the file size in block to an
	 *	integral number of pages.  Allocate
	 *	space for the block number list.
	 */

	i = ((nblks + blkspp - 1) / blkspp) * blkspp;
	bnptr = s5ip->s5i_map = (int *)sptalloc(ctos(i), PG_VALID, 0);
	if (bnptr == NULL)
		return(0);

	/*	Build the actual list of block numbers
	 *	for the file.
	 */

	s5bldblklst(bnptr, ip, nblks);

	/*	If the size is not an integral number of
	 *	pages long, then the last few block
	 *	number up to the next page boundary are
	 *	made zero so that no one will try to
	 *	read them in.  See code in fault.c/vfault.
	 */

	while (i%blkspp != 0) {
		*bnptr++ = -1;
		i++;
	}
	return(1);
}

/*	Read page from a file
 *
 *	return # of bytes read if no error occurs
 *	return -1 - when read error occurs
 */
s5readmap(ip, offset, size, vaddr, segflg)
register struct inode *ip;
register off_t offset;
int size;
caddr_t vaddr;
int segflg;
{
	register struct buf	*bp;
	register int	*bmap;
	register int	bn, rabn;
	register int	on, n;
	register int	bsize, msize, factor, mask;
	register dev_t	edev;
	struct s5inode *s5ip;
	int ret;
	struct buf *bread(), *breada(), *geteblk();

	/*	Get the number of blocks to read and
	 *	a pointer to the block number list.
	 */

	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	bmap = s5ip->s5i_map;
	ASSERT(bmap != 0);
	if (offset > ip->i_size) {
		u.u_error = EINVAL;
		printf("s5readpg: offset > ip->i_size\n");
		printf("s5readpg: offset=%x ip->i_size == %x\n", offset, ip->i_size);
		return(-1);
	}
	if (offset+size > ip->i_size) {
		size = ip->i_size - offset;
	}
	ret = size;
	edev = ip->i_dev;
	msize = FSBSIZE(ip);		/* Size of file system's mount point */
	bsize = FsBSIZE(msize);		/* Size of System buffers */
	factor = msize / bsize;		/* ratio should be one or greater */
	mask = factor - 1;		/* mask for consecutive blks */

	while (size > 0) {
		bn = bmap[offset / msize] * factor;
		if (bn < 0) 
			break;

		on = offset & (bsize-1);
		n = bsize - on;
		if (n > size) n = size;

		if (bn) {
			bn += (offset >> 10) & mask;	    /* 1KFS Specific! */
			rabn = bmap[(offset+n) / msize] * factor;

			if ((size > bsize)  &&  (rabn > 0)) {
				rabn += ((offset+n) >> 10) & mask;    /* 1KFS */
				bp = breada(edev, bn, rabn, bsize);
			} else {
				bp = bread(edev, bn, bsize);
			}
		} else {
			bp = geteblk();
			clrbuf(bp);
		}

		bp->b_flags |= B_AGE;
		if (bp->b_flags&B_ERROR) {
			prdev("page read error", edev);
			brelse(bp);
			return(-1);
		}

		if (segflg != 1) {
			if (copyout(bp->b_un.b_addr + on, vaddr, n)) {
				u.u_error = EFAULT;
				brelse(bp);
				printf("s5readpg:  failure in copyout\n");
				return(-1);
			}
		} else
			bcopy(bp->b_un.b_addr+on, vaddr, n);
		brelse(bp);

		size -= n;
		offset += n;
		vaddr += n;
	}

	return(ret);
}

/*	Build the list of block numbers for a file.  This is used
 *	for mapped files.
 */

s5bldblklst(lp, ip, nblks)
register int		*lp;
inode_t			*ip;
register int		nblks;
{
	register int	lim;
	register int	*eptr;
	register int	i;
	register struct s5inode *s5ip;
	int		*s5bldindr();

	/*	Get the block numbers from the direct blocks first.
	 */

	eptr = &lp[nblks];
	if (nblks < 10)
		lim = nblks;
	else
		lim = 10;
	
	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);
	for (i = 0  ;  i < lim  ;  i++)
		*lp++ = s5ip->s5i_addr[i];
	
	if (lp >= eptr)
		return(1);
	
	while (lp < eptr) {
		lp = s5bldindr(ip, lp, eptr, effdev(ip), s5ip->s5i_addr[i], i - 10);
		if (lp == 0)
			return(0);
		i++;
	}
	return(1);
}

int  *
s5bldindr(ip, lp, eptr, edev, blknbr, indlvl)
struct inode *ip;
register int		*lp;
register int		*eptr;
register int		edev;
int			blknbr;
int			indlvl;
{
	register struct buf *bp;
	register int	*bnptr;
	register int	msize, bsize, frac, i, cnt;
	struct buf 	*bread();

	msize = FSBSIZE(ip);		/* Size of file system's mount point */
	bsize = FsBSIZE(msize);		/* Size of System buffers */
	frac = msize / bsize;		/* ratio should be one or greater */
	blknbr *= frac;			/* convert to System's buffer size */

	for (i = 0; ((i < frac) && (lp < eptr)); i++) {
		bp = bread(edev, blknbr++, bsize);
		if (u.u_error) {
			brelse(bp);
			return((int *) 0);
		}
		bnptr = bp->b_un.b_words;
		cnt = FsNINDIR(min(msize, bsize));

		ASSERT(indlvl >= 0);
		while (cnt--  &&  lp < eptr) {
			if (indlvl == 0) {
				*lp++ = *bnptr++;
			} else {
				lp = s5bldindr(ip, lp, eptr, edev, *bnptr++, indlvl-1);
				if (lp == 0) {
					brelse(bp);
					return((int *) 0);
				}
			}
		}
		brelse(bp);
	}
	return(lp);
}

/*	Free the block list attached to an inode.
 */

s5freemap(ip)
register inode_t	*ip;
{
	register int	nblks;
	register int	blkspp;
	register struct s5inode *s5ip;
	register	msize;

	ASSERT((ip->i_flag & ITEXT) == 0);
	ASSERT(ip->i_flag & ILOCK);
	ASSERT(ip->i_count <= 1);
	
	s5ip = (struct s5inode *)ip->i_fsptr;
	ASSERT(s5ip != NULL);

	if (ip->i_ftype != IFREG || s5ip->s5i_map == NULL)
		return;

	/* Call paging routine to flush page cache */
	flushpgch(ip);

	msize = FSBSIZE(ip);		/* Size of file system's mount point */
	nblks = (ip->i_size + msize - 1)/msize;
	blkspp = max(NBPP, msize)/msize;

	/* Round up the file size (in blocks) to an integral */
	/* number of pages */
	nblks = ((nblks + blkspp - 1) / blkspp) * blkspp;
	sptfree(s5ip->s5i_map, ctos(nblks), 1);
	s5ip->s5i_map = NULL;
}
