/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)clri:clri.c	1.9"
/*
 * clri filsys inumber ...
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ino.h>
#include <sys/filsys.h>

/* file system block size */
#if (vax || u3b || u3b5 || u3b2 || m68k) && (FsTYPE == 3)
#if u3b5
#define FSBSIZE (BSIZE*4)
#else
#define FSBSIZE	(BSIZE*2)
#endif
#else
#define	FSBSIZE BSIZE
#endif

#define ISIZE	(sizeof(struct dinode))
#define	NBINODE	(FSBSIZE/ISIZE)

struct filsys sblock;
int	bsize, nbinode;

struct	ino
{
	char	junk[ISIZE];
};
struct	ino	buf[NBINODE];
int	status;

main(argc, argv)
char *argv[];
{
	register i, f;
	unsigned n;
	int j, k;
	long off;

	if(argc < 3) {
		printf("usage: clri filsys inumber ...\n");
		exit(4);
	}
	f = open(argv[1], 2);
	if(f < 0) {
		printf("cannot open %s\n", argv[1]);
		exit(4);
	}
	if(lseek(f, (long)SUPERBOFF, 0) < 0
	   || read(f, &sblock, (sizeof sblock)) != (sizeof sblock)) {
		printf("cannot read super-block of %s\n", argv[1]);
		exit(4);
	}
#if FsTYPE==1
	if(sblock.s_magic == FsMAGIC && sblock.s_type == Fs2b) {
		if(execvp("/etc/clri",argv) == -1)
			printf("Cannot exec /etc/clri");
	}
	if(sblock.s_magic == FsMAGIC && sblock.s_type == Fs16b) {
		if(execvp("/etc/clri16b",argv) == -1)
			printf("Cannot exec /etc/clri16b");
	}
#endif
#if FsTYPE==2
	if(sblock.s_magic != FsMAGIC ||
	(sblock.s_magic == FsMAGIC && sblock.s_type == Fs1b)) {
		if(execvp("/etc/clri1b",argv) == -1)
			printf("Cannot exec /etc/clri1b");
	}
	if(sblock.s_magic == FsMAGIC && sblock.s_type == Fs16b) {
		if(execvp("/etc/clri16b",argv) == -1)
			printf("Cannot exec /etc/clri16b");
	}
#endif
#if FsTYPE==3
	if(sblock.s_magic == FsMAGIC && sblock.s_type == Fs16b) {
		if(execvp("/etc/clri16b",argv) == -1)
			printf("Cannot exec /etc/clri16b");
	}
#endif
#if FsTYPE==5
	if(sblock.s_magic != FsMAGIC ||
	(sblock.s_magic == FsMAGIC && sblock.s_type == Fs1b)) {
		if(execvp("/etc/clri1b",argv) == -1)
			printf("Cannot exec /etc/clri1b");
	}
	if(sblock.s_magic == FsMAGIC && sblock.s_type == Fs2b) {
		if(execvp("/etc/clri",argv) == -1)
			printf("Cannot exec /etc/clri");
	}
#endif
	if(sblock.s_magic == FsMAGIC) {
		if(sblock.s_type == Fs1b) {
			bsize = BSIZE;
		} else if((sblock.s_type == Fs2b)||(sblock.s_type == Fs16b)) {
			bsize = FSBSIZE;
		} else {
			printf("bad block type in %s\n", argv[1]);
			exit(4);
		}
	} else {
		bsize = BSIZE;
	}
	nbinode = bsize / ISIZE;
	for(i=2; i<argc; i++) {
		if(!isnumber(argv[i])) {
			printf("%s: is not a number\n", argv[i]);
			status = 1;
			continue;
		}
		n = atoi(argv[i]);
		if(n == 0) {
			printf("%s: is zero\n", argv[i]);
			status = 1;
			continue;
		}
		off = ((n-1)/nbinode + 2) * (long)bsize;
		lseek(f, off, 0);
		if(read(f, (char *)buf, bsize) != bsize) {
			printf("%s: read error\n", argv[i]);
			status = 1;
		}
	}
	if(status)
		exit(status);
	for(i=2; i<argc; i++) {
		n = atoi(argv[i]);
		printf("clearing %u\n", n);
		off = ((n-1)/nbinode + 2) * (long)bsize;
		lseek(f, off, 0);
		read(f, (char *)buf, bsize);
		j = (n-1)%nbinode;
		for(k=0; k<ISIZE; k++)
			buf[j].junk[k] = 0;
		lseek(f, off, 0);
		write(f, (char *)buf, bsize);
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(c < '0' || c > '9')
			return(0);
	return(1);
}
