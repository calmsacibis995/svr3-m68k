/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)labelit:labelit.c	1.14"
#include <stdio.h>
#include <sys/param.h>
#ifndef	RT
#include <signal.h>
#include <sys/types.h>
#endif
#include <sys/sysmacros.h>
#include <sys/filsys.h>
#define DEV 1
#define FSNAME 2
#define VOLUME 3
 /* write fsname, volume # on disk superblock */
struct {
	char fill1[SUPERBOFF];
	union {
		char fill2[BSIZE];
		struct filsys fs;
	} f;
} super;
#ifdef RT
#define IFTAPE(s) (equal(s,"/dev/mt",7)||equal(s,"mt",2))
#define TAPENAMES "'/dev/mt'"
#else
#define IFTAPE(s) (equal(s,"/dev/rmt",8)||equal(s,"rmt",3)||equal(s,"/dev/rtp",8)||equal(s,"rtp",3))
#define TAPENAMES "'/dev/rmt' or '/dev/rtp'"
#endif

struct {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time,
		t_length,
		t_dens;
	char	t_fill[484];
} Tape_hdr;

sigalrm()
{
	signal(SIGALRM, sigalrm);
}

main(argc, argv) 
char **argv; 
{
	int fsi, fso;
	long curtime;
	int i;
#ifdef	RT
	static char usage = "Usage: labelit /dev/??? [fsname volume [-n]]\n";
#else
	static char *usage = "Usage: labelit /dev/r??? [fsname volume [-n]]\n";
#endif

	signal(SIGALRM, sigalrm);

#ifdef RT
	setio(-1,1);	/* use physical io */
#endif

	if(argc!=4 && argc!=2 && argc!=5)  {
	}
	if(argc==5) {
		if(strcmp(argv[4], "-n")!=0) {
			fprintf(stderr, usage);
			exit(2);
		}
		if(!IFTAPE(argv[DEV])) {
			fprintf(stderr, "labelit: `-n' option for tape only\n");
			fprintf(stderr, "\t'%s' is not a valid tape name\n", argv[DEV]);
			fprintf(stderr, "\tValid tape names begin with %s\n", TAPENAMES);
			fprintf(stderr, usage);
			exit(2);
		}
		printf("Skipping label check!\n");
		goto do_it;
	}

	if((fsi = open(argv[DEV],0)) < 0) {
		fprintf(stderr, "labelit: cannot open device\n");
		exit(2);
	}

	if(IFTAPE(argv[DEV])) {
		alarm(5);
		read(fsi, &Tape_hdr, sizeof(Tape_hdr));
		alarm(0);
		if(!(equal(Tape_hdr.t_magic, "Volcopy", 7)||
		    equal(Tape_hdr.t_magic,"Finc",4))) {
			fprintf(stderr, "labelit: tape not labelled!\n");
			exit(2);
		}
		printf("%s tape volume: %s, reel %d of %d reels\n",
			Tape_hdr.t_magic, Tape_hdr.t_volume, Tape_hdr.t_reel, Tape_hdr.t_reels);
		printf("Written: %s", ctime(&Tape_hdr.t_time));
		if(argc==2 && Tape_hdr.t_reel>1)
			exit(0);
	}
	if((i=read(fsi, &super, sizeof(super))) != sizeof(super))  {
		fprintf(stderr, "labelit: cannot read superblock\n");
		exit(2);
	}

#define	S	super.f.fs
	printf("Current fsname: %.6s, Current volname: %.6s,",
		S.s_fname, S.s_fpack, S.s_fsize);
	if (S.s_magic == FsMAGIC && S.s_type == Fs2b)
#ifdef u3b5
		printf(" Blocks: %ld, Inodes: %d\nFS Units: 2Kb, ",
			S.s_fsize * 4, (S.s_isize - 2) * 32);
#else
		printf(" Blocks: %ld, Inodes: %d\nFS Units: 1Kb, ",
			S.s_fsize * 2, (S.s_isize - 2) * 16);
#endif
	else
	if (S.s_magic == FsMAGIC && S.s_type == Fs16b)
		printf(" Blocks: %ld, Inodes: %d\nFS Units: 8Kb, ",
			S.s_fsize * 16, (S.s_isize - 2) * 128);
	else
		printf(" Blocks: %ld, Inodes: %d\nFS Units: 512b, ",
			S.s_fsize, (S.s_isize - 2) * 8);
	printf("Date last modified: %s", ctime(&S.s_time));
	if(argc==2)
		exit(0);
do_it:
	printf("NEW fsname = %.6s, NEW volname = %.6s -- DEL if wrong!!\n",
		argv[FSNAME], argv[VOLUME]);
	sleep(10);
	sprintf(super.f.fs.s_fname, "%.6s", argv[FSNAME]);
	sprintf(super.f.fs.s_fpack, "%.6s", argv[VOLUME]);

	close(fsi);
	fso = open(argv[DEV],1);
	if(IFTAPE(argv[DEV])) {
		strcpy(Tape_hdr.t_magic, "Volcopy");
		sprintf(Tape_hdr.t_volume, "%.6s", argv[VOLUME]);
		if(write(fso, &Tape_hdr, sizeof(Tape_hdr)) < 0)
			goto cannot;
	}
	if(write(fso, &super, sizeof(super)) < 0) {
cannot:
		fprintf(stderr, "labelit cannot write label\n");
		exit(2);
	}
	exit(0);
}
equal(s1, s2, ct)
char *s1, *s2;
int ct;
{
	register i;

	for(i=0; i<ct; ++i) {
		if(*s1 == *s2) {;
			if(*s1 == '\0') return(1);
			s1++; s2++;
			continue;
		} else return(0);
	}
	return(1);
}
