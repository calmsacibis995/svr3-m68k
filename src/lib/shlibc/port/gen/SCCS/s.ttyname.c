h10968
s 00071/00000/00000
d D 1.1 86/07/31 10:26:06 fnf 1 0
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

#ident	"@(#)libc-port:gen/ttyname.c	1.8"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * ttyname(f): return "/dev/ttyXX" which the the name of the
 * tty belonging to file f.
 *
 * This program works in two passes: the first pass tries to
 * find the device by matching device and inode numbers; if
 * that doesn't work, it tries a second time, this time doing a
 * stat on every file in /dev and trying to match device numbers
 * only. If that fails too, NULL is returned.
 */

#define	NULL	0
#include <sys/types.h>
#include <sys/fs/s5dir.h>
#include <sys/stat.h>

extern int open(), read(), close(), stat(), fstat(), isatty();
extern char *strcpy(), *strcat();
extern long lseek();

static char rbuf[32], dev[]="/dev/";

char *
ttyname(f)
int	f;
{
	struct stat fsb, tsb;
	struct direct db;
	register int df, pass1;

	if(isatty(f) == 0)
		return(NULL);
	if(fstat(f, &fsb) < 0)
		return(NULL);
	if((fsb.st_mode & S_IFMT) != S_IFCHR)
		return(NULL);
	if((df = open(dev, 0)) < 0)
		return(NULL);
	pass1 = 1;
	do {
		while(read(df, (char *)&db, sizeof(db)) == sizeof(db)) {
			if(db.d_ino == 0)
				continue;
			if(pass1 && db.d_ino != fsb.st_ino)
				continue;
			(void) strcpy(rbuf, dev);
			(void) strcat(rbuf, db.d_name);
			if(stat(rbuf, &tsb) < 0)
				continue;
			if(tsb.st_rdev == fsb.st_rdev &&
				(tsb.st_mode&S_IFMT) == S_IFCHR &&
				(!pass1 || tsb.st_ino == fsb.st_ino)) {
				(void) close(df);
				return(rbuf);
			}
		}
		(void) lseek(df, 0L, 0);
	} while(pass1--);
	(void) close(df);
	return(NULL);
}
E 1
