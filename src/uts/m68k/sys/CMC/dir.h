/*	CMC/dir.h	7.0	10/10/84	*/

/*
 * This file has been created to support opendir, readdir,
 * and closedir operations on System 5 directory structures.
 *
 * October 10, 1984 - Created by Deb Brown, CMC
 */

#ifdef TARGET_SYSTEM
#if TARGET_SYSTEM == VALID
#ifndef void
#include <CMC/sysdep.h>
#endif
#endif
#endif
#ifndef NULL
#define NULL 0
#endif

#define	MAXNAMLEN	14

struct	direct {
	u_long	d_ino;			/* inode number of entry */
	char	d_name[MAXNAMLEN + 1];	/* name must be no longer than this */
};

 long	lseek();

#define closedir(fid)		(close (fid))
#define seekdir(fid, loc)	(lseek ((fid), (long) (loc), 0))
#define telldir(fid)		(lseek ((fid), (long) 0, 1))
#define rewinddir(fid)		(lseek ((fid), (long) 0, 0))


/* definitions for actual directory formats */

struct dir_s5 {				/* System 5-type direct struct */
	ino_t	d5_ino;
	char	d5_name[MAXNAMLEN];
};
