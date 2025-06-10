/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/dirent.h	10.4"

/*
*/

#define MAXNAMLEN	512		/* maximum filename length */
#define DIRBUF		1048		/* buffer size for fs-indep. dirs */

struct dirent				/* data from readdir() */
	{
	long		d_ino;		/* inode number of entry */
	off_t		d_off;		/* offset of disk direntory entry */
	unsigned short	d_reclen;	/* length of this record */
	char		d_name[1];	/* name of file */
	};

typedef struct
	{
	int	dd_fd;			/* file descriptor */
	int	dd_loc;			/* offset in block */
	int	dd_size;		/* amount of valid data */
	char	*dd_buf;	/* directory block */
	}	DIR;			/* stream data from opendir() */

extern DIR		*opendir();
extern struct dirent	*readdir();
extern long		telldir();
extern void		seekdir();
extern int		closedir();

#define rewinddir( dirp )	seekdir( dirp, 0L )
