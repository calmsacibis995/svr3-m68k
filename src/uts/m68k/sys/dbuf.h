/*
 *	dbuf.h: include file for double buffering support
 *
 *	@(#)dbuf.h	1.4
 */

#define	DBUFMAX	(256*1024)
#define	DBUFMIN	(1*1024)

#define	DBUFIOCTL	('d'<<8)
#define	DBUFSET		(DBUFIOCTL|1)
#define	DBUFGET		(DBUFIOCTL|2)
#define	DBUFRETRY	(DBUFIOCTL|3)
#define	DBUFFLUSH	(DBUFIOCTL|4)

struct dbufctl {
	char	*d_seek;	/* seek pointer into buffer */
	unsigned d_size;	/* buffer size */
	char	*d_addr;	/* buffer address */
	char	*d_eaddr;	/* buffer end address */
	unsigned d_offset;	/* offset from beginning of file */
};

struct dbuf {
	struct dbufctl *d_buf;	/* current buffer */
	struct dbufctl d_buf1;	/* buffer 1 */
	struct dbufctl d_buf2;	/* buffer 2 */
	struct buf d_sbuf;	/* system buffer header */
	int	(*d_strat)();	/* procedure to call */
	int	d_flags;	/* control flags */
	int	d_dev;		/* major+minor device */
};
