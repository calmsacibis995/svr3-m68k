/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)tar:tar.c	1.18"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

extern void	exit();
extern FILE	*fopen();
extern char	*getcwd();
extern long	lseek();
extern char	*malloc();
extern char	*mktemp();
extern char	*strcat();
extern char	*strcpy();
extern time_t	time();
extern int	errno;
extern char	*sys_errlist[];

daddr_t bsrch();
#define TBLOCK	512
#define NBLOCK	20
#define NAMSIZ	100
union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
	} dbuf;
} dblock, tbuf[NBLOCK];

struct linkbuf {
	ino_t	inum;
	dev_t	devnum;
	int	count;
	char	pathname[NAMSIZ];
	struct	linkbuf *nextp;
} *ihead;

struct stat stbuf;

int	rflag, xflag, vflag, tflag, mt, cflag, mflag, oflag, chrflg;
int	term, chksum, wflag, recno, first, linkerrok;
int	freemem = 1;
int	nblock = 1;

daddr_t	low;
daddr_t	high;

FILE	*tfile;
char	tname[] = "/tmp/tarXXXXXX";

char	*usefile;
char	*usefil1;
char	magtape[]	= "/dev/mt/0m";
char	magtap1[]	= "/dev/mt0";

main(argc, argv)
int	argc;
char	*argv[];
{
	char *cp;
	char er_strng[64];			/* old copy of sys_errlist[errno] */
	int speed;
	void onintr(), onquit(), onhup();
						/* fflg & uflg used to reject     */
	int fflg = 0, uflg = 0;			/* multiple keys		  */
	int bflg = 0;				/* bflg used to insure b function */
						/* modifier is specified for read */
						/* of char special device	  */
	if (argc < 2)
		usage();

	tfile = NULL;
	usefile =  magtape;
	usefil1 =  magtap1;
	argv[argc] = 0;
	argv++;
	for (cp = *argv++, speed = 0; *cp; cp++, speed--)
		switch(*cp) {
		case 'f':
			if (bflg) {
				fprintf(stderr, "tar: function modifier f must precede function modifier b\n");
				usage();
			}
			fflg++;
			usefile = *argv++;
			if (nblock == 1)
				nblock = 0;
			break;
		case 'c':
			cflag++;
			rflag++;
			break;
		case 'u':
			if (++uflg > 1) {
				fprintf(stderr, "tar: Only one instance of the u key is allowed\n");
				usage();
			}
			mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL) {
				fprintf(stderr, "tar: cannot create temporary file (%s)\n", tname);
				done(1);
			}
			fprintf(tfile, "!!!!!/!/!/!/!/!/!/! 000\n");
			/* FALL THROUGH */
		case 'r':
			rflag++;
			if (nblock != 1 && cflag == 0) {
noupdate:
				fprintf(stderr, "tar: Blocked tapes cannot be updated (yet)\n");
				done(1);
			}
			break;
		case 'v':
			vflag++;
			break;
		case 'w':
			wflag++;
			break;
		case 'x':
			xflag++;
			break;
		case 't':
			tflag++;
			break;
		case 'm':
			if (speed != 1)		/* if speed=1 'm' means */
				mflag++;	/* medium speed		*/
			break;
		case '-':
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			magtape[8] = *cp;
			magtap1[7] = *cp;
			speed = 2;		/* next time will be 1 */

			usefile = magtape;
			usefil1 = magtap1;
			break;
		case 'b':
			bflg++;
			nblock = atoi(*argv++);
			if (nblock > NBLOCK || nblock <= 0) {
				fprintf(stderr, "tar: invalid blocksize, %s. (Max %d)\n",
				*(argv - 1), NBLOCK);
				done(1);
			}
			if (rflag && !cflag)
				goto noupdate;
			break;
		case 'l':
			if (speed == 1) {
				magtape[9] = *cp;
				usefile = magtape;
			}
			else
				linkerrok++;
			break;
		case 'o':
			oflag++;
			break;
		case 'h':
			if (speed == 1) {
				magtape[9] = *cp;
				usefile = magtape;
				break;
			}
			/*	else fall thru -- error */
		default:
			fprintf(stderr, "tar: %c: unknown option\n", *cp);
			usage();
		}

	if ((xflag + tflag + rflag) > 1 || (uflg + rflag) > 2 || (cflag + rflag) > 2) {
		fprintf(stderr, "tar: Only one of the c, x, t, u or r keys is allowed\n");
		usage();
	}
	if (bflg  > 1 || fflg  > 1 || linkerrok > 1 ||
	    mflag > 1 || oflag > 1 || vflag     > 1 || wflag > 1) {
		fprintf(stderr, "tar: Only one instance each of the b, f, l, m, o, v and w function modifiers is allowed\n");
		usage();
	}
	if ((linkerrok && xflag) || (linkerrok && tflag)) {
		fprintf(stderr, "tar: Function modifier l valid only with c, r and u keys\n");
		usage();
	}
	if (mflag && !xflag) {
		fprintf(stderr, "tar: Function modifier m valid only with x key\n");
		usage();
	}
	if (wflag && tflag) {
		fprintf(stderr, "tar: Function modifier w invalid with t key\n");
		usage();
	}
	if (oflag && !xflag) {
		fprintf(stderr, "tar: Function modifier o valid only with x key\n");
		usage();
	}
	if ((xflag || tflag || (rflag && !cflag)) && fflg && !bflg &&
						(strcmp(usefile, "-") != 0)) {
		if (stat(usefile, &stbuf) == -1) {
			fprintf(stderr, "tar: stat() of %s failed, %s\n", usefile, sys_errlist[errno]);
			done(1);
		}
		if ((stbuf.st_mode&S_IFMT) == S_IFCHR)
			chrflg++;
	}

	if (rflag) {
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			(void) signal(SIGINT, onintr);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			(void) signal(SIGHUP, onhup);
		if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
			(void) signal(SIGQUIT, onquit);

		if (strcmp(usefile, "-") == 0) {
			if (cflag == 0) {
				fprintf(stderr, "tar: Standard output archives available only for the c key\n");
				done(1);
			}
			mt = dup(1);
			nblock = 1;
		}
		else
#if m68k
		if ((mt = open(usefile,O_RDWR)) == -1) {
#else
		if ((mt = open(usefile,O_RDWR | O_NDELAY)) == -1) {
#endif
			strcpy(er_strng, sys_errlist[errno]);
			if (fflg) {
				if (cflag) {	/* rflag && fflg && cflag */
					if ((mt = creat(usefile, 0666)) == -1) {
						fprintf(stderr,
							"tar: cannot create %s\ntar: %s\n",
							usefile, sys_errlist[errno]);
						done(1);
					}
				}
				else {		/* rflag && fflg && !cflag */
					fprintf(stderr, "tar: cannot open %s\ntar: %s\n",
						usefile, sys_errlist[errno]);
					done(1);
				}
			}
			else {			/* rflag && !fflg */
#if m68k
				if ((mt = open(usefil1, O_RDWR)) == -1) {
#else
				if ((mt = open(usefil1, O_RDWR | O_NDELAY)) == -1) {
#endif
					fprintf(stderr, "tar: cannot open %s, %s\n",
						usefile, er_strng);
					fprintf(stderr, "tar: cannot open %s, %s\n",
						usefil1, sys_errlist[errno]);
					done(1);
				}
			}
		}

		if (cflag == 0 && nblock == 0)
			nblock = 1;
		dorep(argv);
	}
	else if (xflag)  {
		if (strcmp(usefile, "-") == 0) {
			mt = dup(0);
			nblock = 1;
		}
		else
#if m68k
		if ((mt = open(usefile,O_RDONLY)) < 0 && (mt = open(usefil1,O_RDONLY)) < 0) {
#else
		if ((mt = open(usefile,O_RDONLY | O_NDELAY)) < 0 && (mt = open(usefil1,O_RDONLY | O_NDELAY)) < 0) {
#endif
			fprintf(stderr, "tar: cannot open %s\n", usefile);
			done(1);
		}
		doxtract(argv);
	}
	else if (tflag) {
		if (strcmp(usefile, "-") == 0) {
			mt = dup(0);
			nblock = 1;
		}
		else
#if m68k
		if ((mt = open(usefile,O_RDONLY)) < 0 && (mt = open(usefil1,O_RDONLY)) < 0) {
#else
		if ((mt = open(usefile,O_RDONLY | O_NDELAY)) < 0 && (mt = open(usefil1,O_RDONLY | O_NDELAY)) < 0) {
#endif
			fprintf(stderr, "tar: cannot open %s\n", usefile);
			done(1);
		}
		dotable();
	}
	else
		usage();
	done(0);
		/*NOTREACHED*/
}

usage()
{
/*	fprintf(stderr, "tar: usage  tar -{txruc}[vwfblmh] [tapefile] [blocksize] file1 file2...\n");	*/
	fprintf(stderr, "tar: usage  tar [-]{txruc}[vwfblm][0-7[lmh]] [tapefile] [blocksize] file1 file2...\n");	/* For version 1.2 of tar */
	done(1);
}

dorep(argv)
char	*argv[];
{
	register char *cp, *cp2;
	char wdir[60];

	if (!cflag) {
		getdir();
		do {
			passtape();
			if (term)
				done(0);
			getdir();
		} while (!endtape());
		if (tfile != NULL) {
			char buf[200];

			strcat(buf, "sort +0 -1 +1nr ");
			strcat(buf, tname);
			strcat(buf, " -o ");
			strcat(buf, tname);
			(void) sprintf(buf, "sort +0 -1 +1nr %s -o %s; awk '$1 != prev {print; prev=$1}' %s >%sX;mv %sX %s",
				tname, tname, tname, tname, tname, tname);
			fflush(tfile);
			system(buf);
			freopen(tname, "r", tfile);
			fstat(fileno(tfile), &stbuf);
			high = stbuf.st_size;
		}
	}

	getcwd(wdir, 50);
	while (*argv && ! term) {
		cp2 = *argv;
		for (cp = *argv; *cp; cp++)
			if (*cp == '/')
				cp2 = cp;
		if (cp2 != *argv) {
			*cp2 = '\0';
			chdir(*argv);
			*cp2 = '/';
			cp2++;
		}
		putfile(*argv++, cp2);
		chdir(wdir);
	}
	putempty();
	putempty();
	flushtape();
	if (linkerrok == 1)
		for (; ihead != NULL; ihead = ihead->nextp)
			if (ihead->count != 0)
				fprintf(stderr, "Missing links to %s\n", ihead->pathname);
}

endtape()
{
	if (dblock.dbuf.name[0] == '\0') {
		backtape();
		return(1);
	}
	else
		return(0);
}

getdir()
{
	register struct stat *sp;
	int i;

	readtape( (char *) &dblock);
	if (dblock.dbuf.name[0] == '\0')
		return;
	sp = &stbuf;
	sscanf(dblock.dbuf.mode, "%o", &i);
	sp->st_mode = i;
	sscanf(dblock.dbuf.uid, "%o", &i);
	sp->st_uid = i;
	sscanf(dblock.dbuf.gid, "%o", &i);
	sp->st_gid = i;
	sscanf(dblock.dbuf.size, "%lo", &sp->st_size);
	sscanf(dblock.dbuf.mtime, "%lo", &sp->st_mtime);
	sscanf(dblock.dbuf.chksum, "%o", &chksum);
	if (chksum != checksum()) {
		fprintf(stderr, "directory checksum error\n");
		done(2);
	}
	if (tfile != NULL)
		fprintf(tfile, "%s %s\n", dblock.dbuf.name, dblock.dbuf.mtime);
}

passtape()
{
	long blocks;
	char buf[TBLOCK];

	if (dblock.dbuf.linkflag == '1')
		return;
	blocks = stbuf.st_size;
	blocks += TBLOCK-1;
	blocks /= TBLOCK;

	while (blocks-- > 0)
		readtape(buf);
}

putfile(longname, shortname)
char *longname;
char *shortname;
{
	int infile;
	long blocks;
	char buf[TBLOCK];
	register char *cp, *cp2;
	struct direct dbuf;
	int i, j;

	infile = open(shortname,O_RDONLY | O_NDELAY);
	if (infile < 0) {
		fprintf(stderr, "tar: %s: cannot open file\n", longname);
		return;
	}

	fstat(infile, &stbuf);

	if (tfile != NULL && checkupdate(longname) == 0) {
		close(infile);
		return;
	}
	if (checkw('r', longname) == 0) {
		close(infile);
		return;
	}

	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		for (i = 0, cp = buf; *cp++ = longname[i++];);
		*--cp = '/';
		cp++;
		i = 0;
		chdir(shortname);
		while (read(infile, (char *)&dbuf, sizeof(dbuf)) > 0 && !term) {
			if (dbuf.d_ino == 0) {
				i++;
				continue;
			}
			if (strcmp(".", dbuf.d_name) == 0 || strcmp("..", dbuf.d_name) == 0) {
				i++;
				continue;
			}
			cp2 = cp;
			for (j=0; j < DIRSIZ; j++)
				*cp2++ = dbuf.d_name[j];
			*cp2 = '\0';
			close(infile);
			putfile(buf, cp);
			infile = open(".",O_RDONLY);
			i++;
			lseek(infile, (long) (sizeof(dbuf) * i), 0);
		}
		close(infile);
		chdir("..");
		return;
	}
	if ((stbuf.st_mode & S_IFMT) != S_IFREG) {
		fprintf(stderr, "tar: %s is not a regular file. Not archived\n", longname);
		close(infile);
		return;
	}

	tomodes(&stbuf);

	cp2 = longname;
	for (cp = dblock.dbuf.name, i=0; (*cp++ = *cp2++) && i < NAMSIZ; i++);
	if (i >= NAMSIZ) {
		fprintf(stderr, "%s: file name too long\n", longname);
		close(infile);
		return;
	}

	if (stbuf.st_nlink > 1) {
		struct linkbuf *lp;
		int found = 0;

		for (lp = ihead; lp != NULL; lp = lp->nextp) {
			if (lp->inum == stbuf.st_ino && lp->devnum == stbuf.st_dev) {
				found++;
				break;
			}
		}
		if (found) {
			strcpy(dblock.dbuf.linkname, lp->pathname);
			dblock.dbuf.linkflag = '1';
			(void) sprintf(dblock.dbuf.chksum, "%6o", checksum());
			writetape( (char *) &dblock);
			if (vflag) {
				fprintf(stderr, "a %s ", longname);
				fprintf(stderr, "link to %s\n", lp->pathname);
			}
			lp->count--;
			close(infile);
			return;
		}
		else {
			lp = (struct linkbuf *) malloc(sizeof(*lp));
			if (lp == NULL) {
				if (freemem) {
					fprintf(stderr, "Out of memory. Link information lost\n");
					freemem = 0;
				}
			}
			else {
				lp->nextp = ihead;
				ihead = lp;
				lp->inum = stbuf.st_ino;
				lp->devnum = stbuf.st_dev;
				lp->count = stbuf.st_nlink - 1;
				strcpy(lp->pathname, longname);
			}
		}
	}

	blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
	if (vflag) {
		fprintf(stderr, "a %s ", longname);
		fprintf(stderr, "%ld blocks\n", blocks);
	}
	(void) sprintf(dblock.dbuf.chksum, "%6o", checksum());
	writetape( (char *) &dblock);

	while ((i = read(infile, buf, TBLOCK)) > 0 && blocks > 0) {
		writetape(buf);
		blocks--;
	}
	close(infile);
	if (blocks != 0 || i != 0)
		fprintf(stderr, "%s: file changed size\n", longname);
	while (blocks-- >  0)
		putempty();
}



doxtract(argv)
char	*argv[];
{
	long blocks, bytes;
	char buf[TBLOCK];
	char **cp;
	int ofile;

	for (;;) {
		getdir();
		if (endtape())
			break;

		if (*argv == 0)
			goto gotit;

		for (cp = argv; *cp; cp++)
			if (prefix(*cp, dblock.dbuf.name))
				goto gotit;
		passtape();
		continue;

gotit:
		if (checkw('x', dblock.dbuf.name) == 0) {
			passtape();
			continue;
		}

		checkdir(dblock.dbuf.name);

		if (dblock.dbuf.linkflag == '1') {
			unlink(dblock.dbuf.name);
			if (link(dblock.dbuf.linkname, dblock.dbuf.name) < 0) {
				fprintf(stderr, "%s: cannot link\n", dblock.dbuf.name);
				continue;
			}
			if (vflag)
				fprintf(stderr, "%s linked to %s\n", dblock.dbuf.name, dblock.dbuf.linkname);
			continue;
		}
		if ((ofile = creat(dblock.dbuf.name, stbuf.st_mode & 07777)) < 0) {
			fprintf(stderr, "tar: %s - cannot create\n", dblock.dbuf.name);
			passtape();
			continue;
		}

		if (!oflag)
			chown(dblock.dbuf.name, stbuf.st_uid, stbuf.st_gid);

		blocks = ((bytes = stbuf.st_size) + TBLOCK-1)/TBLOCK;
		if (vflag)
			fprintf(stderr, "x %s, %ld bytes, %ld tape blocks\n", dblock.dbuf.name, bytes, blocks);
		while (blocks-- > 0) {
			readtape(buf);
			if (bytes > TBLOCK) {
				if (write(ofile, buf, TBLOCK) < 0) {
					fprintf(stderr, "tar: %s: HELP - extract write error\n", dblock.dbuf.name);
					done(2);
				}
			} else
				if (write(ofile, buf, (int) bytes) < 0) {
					fprintf(stderr, "tar: %s: HELP - extract write error\n", dblock.dbuf.name);
					done(2);
				}
			bytes -= TBLOCK;
		}
		close(ofile);
		if (mflag == 0) {
			time_t timep[2];

			timep[0] = time(NULL);
			timep[1] = stbuf.st_mtime;
			utime(dblock.dbuf.name, timep);
		}
	}
}

dotable()
{
	for (;;) {
		getdir();
		if (endtape())
			break;
		if (vflag)
			longt(&stbuf);
		printf("%s", dblock.dbuf.name);
		if (dblock.dbuf.linkflag == '1')
			printf(" linked to %s", dblock.dbuf.linkname);
		printf("\n");
		passtape();
	}
}

putempty()
{
	char buf[TBLOCK];
	char *cp;

	for (cp = buf; cp < &buf[TBLOCK]; )
		*cp++ = '\0';
	writetape(buf);
}

longt(st)
register struct stat *st;
{
	register char *cp;
	char *ctime();

	pmode(st);
	printf("%6u%6u", st->st_uid, st->st_gid);
	printf("%9lu", st->st_size);
	cp = ctime(&st->st_mtime);
	printf(" %-12.12s %-4.4s ", cp+4, cp+20);
}

#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	STXT	01000
int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 2, STXT, 't', XOTH, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

pmode(st)
register struct stat *st;
{
	register int **mp;

	for (mp = &m[0]; mp < &m[9];)
		select(*mp++, st);
}

select(pairp, st)
int *pairp;
struct stat *st;
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (st->st_mode&*ap++)==0)
		ap++;
	printf("%c", *ap);
}

checkdir(name)
register char *name;
{
	register char *cp=name;
	int i, pid;
	for (cp++; *cp; cp++) {	/* start at *name++ */
		if (*cp == '/') {
			*cp = '\0';
			if (access(name, 01) < 0) {
				pid = fork();			/* pid == 0 implies   */
				if (pid == 0) {			/*	child process */
					execl("/bin/mkdir", "mkdir", name, 0);
					execl("/usr/bin/mkdir", "mkdir", name, 0);
					fprintf(stderr, "tar: cannot find mkdir!\n");
					done(0);
				}
								/* pid != 0 implies   */
				if (pid == -1) {		/* 	parent process*/
					printf(stderr,
					"tar: fork from checkdir(%s) failed\ntar: %s\n",
					name, sys_errlist[errno]);
					exit(1);
				}
				while (wait(&i) != pid);
				if (!oflag)
					chown(name, stbuf.st_uid, stbuf.st_gid);
			}
			*cp = '/';
		}
	}
}

void
onintr()
{
	(void) signal(SIGINT, SIG_IGN);
	term++;
}

void
onquit()
{
	(void) signal(SIGQUIT, SIG_IGN);
	term++;
}

void
onhup()
{
	(void) signal(SIGHUP, SIG_IGN);
	term++;
}

tomodes(sp)
register struct stat *sp;
{
	register char *cp;

	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		*cp = '\0';
	(void) sprintf(dblock.dbuf.mode, "%6o ", sp->st_mode & 07777);
	(void) sprintf(dblock.dbuf.uid, "%6o ", sp->st_uid);
	(void) sprintf(dblock.dbuf.gid, "%6o ", sp->st_gid);
	(void) sprintf(dblock.dbuf.size, "%11lo ", sp->st_size);
	(void) sprintf(dblock.dbuf.mtime, "%11lo ", sp->st_mtime);
}

checksum()
{
	register i;
	register char *cp;

	for (cp = dblock.dbuf.chksum; cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return(i);
}

checkw(c, name)
char *name;
{
	if (wflag) {
		printf("%c ", c);
		if (vflag)
			longt(&stbuf);
		printf("%s: ", name);
		if (response() == 'y'){
			return(1);
		}
		return(0);
	}
	return(1);
}

response()
{
	int c;

	c = getchar();
	if (c != '\n')
		while (getchar() != '\n');
	else c = 'n';
	return(c);
}

checkupdate(arg)
char	*arg;
{
	char name[100];
	long	mtime;
	daddr_t seekp;
	daddr_t	lookup();

	rewind(tfile);
	for (;;) {
		if ((seekp = lookup(arg)) < 0)
			return(1);
		fseek(tfile, seekp, 0);
		fscanf(tfile, "%s %lo", name, &mtime);
		if (stbuf.st_mtime > mtime)
			return(1);
		else
			return(0);
	}
}

done(n)
{
	unlink(tname);
	exit(n);
}

prefix(s1, s2)
register char *s1, *s2;
{
	while (*s1)
		if (*s1++ != *s2++)
			return(0);
	if (*s2)
		return(*s2 == '/');
	return(1);
}

#define	N	200
int	njab;
daddr_t
lookup(s)
char *s;
{
	register i;
	daddr_t a;

	for(i=0; s[i]; i++)
		if(s[i] == ' ')
			break;
	a = bsrch(s, i, low, high);
	return(a);
}

daddr_t
bsrch(s, n, l, h)
daddr_t l, h;
char *s;
{
	register i, j;
	char b[N];
	daddr_t offset, offset1;

	njab = 0;

loop:
	if(l >= h)
		return((daddr_t)-1);
	offset = l + (h-l)/2 - N/2;
	if(offset < l)
		offset = l;
	fseek(tfile, offset, 0);
	fread(b, 1, N, tfile);
	njab++;
	for(i=0; i<N; i++) {
		if(b[i] == '\n')
			break;
		offset++;
	}
	if(offset >= h)
		return((daddr_t)-1);
	offset1 = offset;
	j = i;
	for(i++; i<N; i++) {
		offset1++;
		if(b[i] == '\n')
			break;
	}
	i = cmp(b+j, s, n);
	if(i < 0) {
		h = offset;
		goto loop;
	}
	if(i > 0) {
		l = offset1;
		goto loop;
	}
	return(offset);
}

cmp(b, s, n)
char *b, *s;
{
	register i;

	if(b[0] != '\n')
		exit(2);
	for(i=0; i<n; i++) {
		if(b[i+1] > s[i])
			return(-1);
		if(b[i+1] < s[i])
			return(1);
	}
	return(b[i+1] == ' '? 0 : -1);
}

readtape(buffer)
char *buffer;
{
	int i, j;

	if (recno >= nblock || first == 0) {
		if (first == 0 && nblock == 0)
			j = NBLOCK;
		else
			j = nblock;
		if ((i = read(mt, tbuf, TBLOCK*j)) < 0) {
			fprintf(stderr, "Tar: tape read error, %s\n", sys_errlist[errno]);
			if (chrflg)
				fprintf(stderr, "Tar: warning, blocksize not specified, defaulted to %d\n", nblock);
			done(3);
		}
		if (first == 0) {
			if ((i % TBLOCK) != 0) {
				fprintf(stderr, "Tar: tape blocksize error\n");
				done(3);
			}
			i /= TBLOCK;
			if (rflag && i != 1) {
				fprintf(stderr, "Tar: Cannot update blocked tapes (yet)\n");
				done(4);
			}
			if (i != nblock && i != 1) {
				fprintf(stderr, "Tar: blocksize = %d\n", i);
				nblock = i;
			}
		}
		recno = 0;
	}
	first = 1;
	copy(buffer, &tbuf[recno++]);
	return(TBLOCK);
}

writetape(buffer)
char *buffer;
{
	first = 1;
	if (nblock == 0)
		nblock = 1;
	if (recno >= nblock) {
		if (write(mt, tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "Tar: tape write error\n");
			done(2);
		}
		recno = 0;
	}
	copy(&tbuf[recno++], buffer);
	if (recno >= nblock) {
		if (write(mt, tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "Tar: tape write error\n");
			done(2);
		}
		recno = 0;
	}
	return(TBLOCK);
}

backtape()
{
	lseek(mt, (long) -TBLOCK, 1);
	if (recno >= nblock) {
		recno = nblock - 1;
		if (read(mt, tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "Tar: tape read error after seek, %s\n", sys_errlist[errno]);
			done(4);
		}
		lseek(mt, (long) -TBLOCK, 1);
	}
}

flushtape()
{
	write(mt, tbuf, TBLOCK*nblock);
}

copy(to, from)
register char *to, *from;
{
	register i;

	i = TBLOCK;
	do {
		*to++ = *from++;
	} while (--i);
}
