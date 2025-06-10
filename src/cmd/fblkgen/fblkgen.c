#include <stdio.h>
#include <fcntl.h>
#include <sys/io/win.h>
#include <sys/io/smd.h>
#include <sys/types.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5filsys.h>

#define MAXPART 	8
#define SUBPART		MAXPART - 1	/* don't count last  partn(full disk) */
#define DISKDEFS  	"/etc/diskdefs"	/* where to get disk format data */
#define YES		1
#define NO		0
#define COMMENT 	'*'
#define LINELEN 	512
#define DO_TRACKS	1		/* file format of tracks */
#define	DO_HDCYL	2		/* file format of head, cyl */
#define	DO_DEVBLK	3		/* file format of log dev, block */
#define	SBSIZE		512		/* superblock size */
#define USAGE \
"usage: fsblkgen [-T] [-t file] type rawdev\n"
/* 	
	T	-	track info is in track number form.  Default head/cyl.
	t	-	take track input from a file 
*/

struct diskinfo {
	char dtype[20]; /* unique disk name string */
	int typeno; 	/* unique disk type identifying number */
	char formatter[80];	/* disk formatter name */
	char diag[20]; 	/* diagnostic tracks (y - yes or n - no) */
	char strat[20]; /* bad track strategy */
	int maxbad;	/* max number of bad tracks per disk */
	int rfs; 	/* root file sys offset from beginning of disk (in K bytes) */
	int nls; 	/* number of logical sectors */
	int pss; 	/* physical sector size */
	int spt; 	/* number of sectors per track */
	int cyl; 	/* number of cylinders per drive */
	int heads; 	/* number of heads  */
	int precomp; 	/* precompensation cylinder */
	int interl; 	/* interleave factor */
	int spiral; 	/* spiral offset */
	int step; 	/* stepping rate code */
	int starthead; 	/* starting head number of drive */
	int ecc; 	/* error correcting code data burst length */
	int attm; 	/* attributes mask (device specific) */
	int attw; 	/* attributes word (device specific) */
	int slice0; 	/* number of blocks in slice0 */
	int gap1; 	/* inter-sector gap1 */
	int gap2; 	/* inter-sector gap2 */
	int gap3; 	/* inter-sector gap3 */
	int gap4; 	/* inter-sector gap4 */
	int eattw; 	/* Extended attributes word */
};
struct diskinfo dinfo;

struct disktab {
	int pflg;	/* YES or NO - track falls in this partition */
	int fflg;	/* YES or NO - track falls in this file system */
	long dist;	/* # of blocks into file system (512 byte blks) */
	long size;	/* file system size, 512 byte blocks */
	long btype;	/* block size, Fs1b, Fs2b, etc */
	long pstart;	/* partition start block */
	long plength;	/* partition length (in 512 byte blocks)*/
};

/* set up a table to record lots of information about each partition */
struct disktab  dtable[SUBPART];	/* disk layout and data table */
			
struct list {		/* generic list type for bad track & alts lists */
	int count;
	int *value;
} tracks;

struct filecntl {
	int	rfdes;
	int	wfdes;
	int	mod;
};
struct filecntl	dfile;			/* file descriptors for filesys */

union bufarea {
		char b_buf[BSIZE];	/* buffer space */
		struct filsys b_fs;    	/* the superblock */
};
typedef union bufarea BUFAREA;

int wd15[16] = {WD15};			/* disk partition information */
int wd40[16] = {WD40};			/* disk partition information */
int wd70[16] = {WD70};			/* disk partition information */
int wd140[16] = {WD140};		/* disk partition information */
int fu337[16] = {FU337};		/* disk partition information */

int badform;		/* format of file containing badtrack */
char *type;		/* drive type, input parameter */
char *rdev;		/* raw device, input parameter */
char *devshort;		/* raw device name derivation  ex: /dev/rdsk/m320_0s */
char *badfile;		/* file containing bad track, input parameter */

int fsckfd;		/* file descriptor for fsck input */
char *fsckprfx = "/etc/badtracks/F.";	/* file prefix for fsck */
char *fsckinp;				/* write file for fsck */

main(argc, argv)
char *argv[];
{
	long *partkey;		/* key into the partition def'n file */
	int index;		/* a counter */
	BUFAREA ib;		/* buf area for superblock */
	char dev[30];		/* device to get superblock for */
	char *malloc();
	char *suffix;
	char *itoa();
	char *buf;
	char retn = '\n';
	long pblk;		/* block count into the disk (512 byte) */
	long fsblk;
	int fspartn;		/* partition # containing fil sys which
				   contains the track */
	register int fscount;	/* count of fil sys which include the track */
	int loop;		/* loop once for each entry in badlist */
	int bpc;		/* blocks per cylinder, for smd.h conversion */
	int length;

	type = (char *)malloc(15);	/* get space for options */
	rdev = (char *)malloc(30);
	devshort = (char *)malloc(30);
	fsckinp = (char *)malloc(120);
	badform = DO_HDCYL;
	scanopts(argc, argv);
	readdefs();		/* read disk parameters */
	/* dumpdef(); */
	if ((partkey = (long *)mapdisk(type)) == 0) {
		fprintf(stderr,"fblkgen: No match of key for %s\n", type);
		exit(1);
	}
	initdtab(YES);    /* yes means clear partition fields too */
        for (index=0; index<SUBPART; index++) {   /* partitions 0 to 6 */
		/* things out of win.h are in block number format, those in 
		smd.h are in a cylinder format.  Be sure to get the correct 
		entry into the table.  We want the number of 512 byte blocks 
		within the partition */
		if ((partkey == wd15) || (partkey == wd40) ||
		(partkey == wd70) || (partkey == wd140)) {
          			dtable[index].plength = *(partkey+(2*index));
          			dtable[index].pstart = *(partkey+(2*index)+1);
		}
		else
			if (partkey == fu337) {
				bpc=(dinfo.pss*dinfo.spt*dinfo.heads)/BSIZE;
          			dtable[index].plength = *(partkey+(2*index));
          			dtable[index].pstart = (*(partkey+(2*index)+1))*bpc;
			}
			else {
          			fprintf(stderr, "fblkgen: BAD partition option.\n");
				exit(1);
			}
        }
	readlist(&tracks, badfile, 3 * dinfo.maxbad, badform);

	for (loop=0; loop<tracks.count; loop++)
	{
	  initdtab(NO);	/* no means don't clear partition fields */
          pblk = tracks.value[loop] * ((dinfo.pss * dinfo.spt) / BSIZE);
          for (index=0; index<SUBPART; index++) {   /* partitions 0 to 6 */
          /* fill in pflg if this track falls within the partition */
          	if ((pblk >= dtable[index].pstart) &&
          	(pblk < (dtable[index].pstart + dtable[index].plength)))
          		dtable[index].pflg = YES;
	  }
          /* for partitions with YES, get superblock info */
          for (index=0; index<SUBPART; index++) {   /* partitions 0 to 6 */
             if (dtable[index].pflg == YES) {
          	suffix = itoa(&index);
          	strcpy(dev, devshort);
          	strcat(dev, suffix);
          	if(getsblk(dev, &ib) == NO) {
          		fprintf(stderr,"fblkgen: SUPERBLOCK not found\n");
          		exit(2);
          	}
          	dtable[index].btype = ib.b_fs.s_type;
          	switch (ib.b_fs.s_type) {
          		case Fs1b: 	dtable[index].size = ib.b_fs.s_fsize;
          				break;
          		case Fs2b: 	dtable[index].size = ib.b_fs.s_fsize*2;
          				break;
          		case Fs4b: 	dtable[index].size = ib.b_fs.s_fsize*4;
          				break;
          		case Fs8b: 	dtable[index].size = ib.b_fs.s_fsize*8;
          				break;
          		case Fs16b: 	dtable[index].size = ib.b_fs.s_fsize*16;
          				break;
          		default: 	dtable[index].btype = 0;
          				dtable[index].size = 0;
             				dtable[index].pflg = NO;
             				dtable[index].fflg = NO;
          				break;
          	}
		/* double check if valid by looking at the stat too */
		if ((ib.b_fs.s_state != FsOKAY)&&(ib.b_fs.s_state != FsACTIVE)
		&&(ib.b_fs.s_state != FsBAD)&&(ib.b_fs.s_state != FsBADBLK)) {
          		dtable[index].btype = 0;
          		dtable[index].size = 0;
             		dtable[index].pflg = NO;
             		dtable[index].fflg = NO;
		}
          	/* determine if track falls within the file system here */
          	if (dtable[index].size != 0) {
          		/* fill in fflg if track falls within the filsys */
          		fsblk = pblk - dtable[index].pstart; /* relative */
          		if (fsblk <= dtable[index].size) {
          			dtable[index].fflg = YES;
          			dtable[index].dist = fsblk;
          		}
          	}
             }
          }
	  fscount = 0;
          for (index=0; index<SUBPART; index++) {
          	if (dtable[index].fflg == YES) {
          		fspartn = index;
          		fscount++;
          	}
          }
	  /* dumptable(); */
          /* Find out if block occurs in multiple file systems - that's bad */
          if (fscount > 1) {
          	fspartn = queryusr(fscount);
		fscount = 1;
	  }
          if (fscount != 0) {
          	/* write the blocks to the corresponding file */
          	strcpy(fsckinp, fsckprfx);
		strncpy(suffix, rdev+10, (strlen(rdev) - 11));
		strcat(fsckinp, suffix);
          	suffix = itoa(&fspartn);
          	strcat(fsckinp, suffix);
          	if ((fsckfd = open(fsckinp, O_WRONLY | O_APPEND)) == -1)
          		if ((fsckfd = creat(fsckinp, 0666)) == -1) {
				fprintf(stderr, "fblkgen: can't create file %s\n", fsckinp);
				exit(2);
			}
          	conv_wr(fsckfd, fspartn);
          	close(fsckfd);
          }
    	}
  	exit(0);
}

/* initialize the disk table to avoid accidents */

initdtab(clearpartn)
int clearpartn;
{
	int i;
	for (i=0; i<SUBPART; i++) {
		dtable[i].pflg = NO;
		dtable[i].fflg = NO;
		dtable[i].dist = 0;
		dtable[i].size = 0;
		dtable[i].btype = 0;
		if (clearpartn) {
			dtable[i].pstart = 0;
			dtable[i].plength = 0;
		}
	}
}

/* compute the affected block numbers, write to the file */

conv_wr(fd, partn)
int fd, partn;
{
	char *buf, *suffix;
	int bpt;
	int factor;
	int fsblk;	/* (file system block size) offset into file system */
	int i;
	int length;
	char retn = '\n';
	
	switch (dtable[partn].btype) {
		case Fs1b: 	bpt = (dinfo.pss * dinfo.spt)/ 512;
				factor = 1;
				break;
		case Fs2b: 	bpt = (dinfo.pss * dinfo.spt)/ 1024;
				factor = 2;
				break;
		case Fs4b: 	bpt = (dinfo.pss * dinfo.spt)/ 2048;
				factor = 4;
				break;
		case Fs8b: 	bpt = (dinfo.pss * dinfo.spt)/ 4096;
				factor = 8;
				break;
		case Fs16b: 	bpt = (dinfo.pss * dinfo.spt)/ 8192;
				factor = 16;
				break;
		default: 	fprintf(stderr, "fblkgen: bad file system block size\n");
				exit(2);
				break;
	}
	fsblk = dtable[partn].dist / factor;
	if (bpt < 1) 	/* in case of blocks larger than the track, */
		bpt = 1;	/* count that one whole block as bad */
	for (i = fsblk; i < (fsblk+bpt); i++) {
		buf = itoa(&i);
		length = strlen(buf);
		if (write(fd, buf, length) != length) {
			fprintf(stderr, "fblkgen: write error\n");
			return(-1);
		}
		write(fd, &retn, sizeof(char));
	}
	return(0);
}

/* ask the user which partition is the usable, latest one */

queryusr(count)
int count;
{
	int index;
	char line[80];
	int reply;

	line[0] = '\0';
	fprintf(stderr, "\n%d fblkgen: FILE SYSTEMS ARE SUPERIMPOSED\n", count);
	fprintf(stderr, "	STARTING PARTITIONS:  ");
	for (index=0; index<SUBPART; index++)
		if (dtable[index].fflg == YES)
			fprintf(stderr, "%d, ", index);
	fprintf(stderr,"\n");
	fprintf(stderr,"WHICH IS CURRENT?  (enter partition) : ");
	while (line[0] == '\0') {
		if(getline(stdin,line,sizeof(line)) == EOF)
			fprintf(stderr,"no number, quit\n");
		fprintf(stderr,"\n");
		reply = atoi(line);
		if(reply >= 0 && reply < SUBPART) {
			if (dtable[reply].fflg == YES)
				return(reply);
		}
		fprintf(stderr,"You must enter a number from the list: ");
		line[0] = '\0';
	}
}

/* map disk device to partition key */
/* return the address of the array to use as parms */
long 
mapdisk(type)
char *type;
{
	if (strcmp(type, "m32015") == 0)
		return((long)wd15);
	if (strcmp(type, "m32040m") == 0)
		return((long)wd40);
	if (strcmp(type, "m32040v") == 0)
		return((long)wd40);
	if (strcmp(type, "m32040t") == 0)
		return((long)wd40);
	if (strcmp(type, "m32040n") == 0)
		return((long)wd40);
	if (strcmp(type, "m32040s") == 0)
		return((long)wd40);
	if (strcmp(type, "m32070m") == 0)
		return((long)wd70);
	if (strcmp(type, "m32070s") == 0)
		return((long)wd70);
	if (strcmp(type, "m32070p") == 0)
		return((long)wd70);
	if (strcmp(type, "m32070t") == 0)
		return((long)wd70);
	if (strcmp(type, "m320140") == 0)
		return((long)wd140);
	if (strcmp(type, "m360337") == 0)
		return((long)fu337);
	if (strcmp(type, "m360cmd337") == 0)
		return((long)fu337);
	return(0);
}

getsblk(dev, bp)
char *dev;
BUFAREA *bp;
{
	register struct filecntl *fcp;

	if((dfile.rfdes = open(dev,0)) < 0) {
		fprintf(stderr,"fblkgen: Can't open %s\n",dev);
		return(NO);
	}
	if((dfile.wfdes = open(dev,1)) < 0)
		dfile.wfdes = -1;
	fcp = &dfile;
	if(lseek(fcp->rfdes,(long)SUPERBOFF,0) < 0)
		fprintf(stderr,"fblkgen: error on superblock SEEK \n");
	else if(read(fcp->rfdes,bp->b_buf,SBSIZE) == SBSIZE) {
		close(dfile.rfdes);
		return(YES);
	}
	fprintf(stderr,"fblkgen: error on superblock READ \n");
	close(dfile.rfdes);
	return(NO);
}


/* read the diskdef information for the disk type */

readdefs()
	{
	FILE *deffile;

	if ((deffile = fopen(DISKDEFS, "r")) == NULL)
		{
		fprintf(stderr, "fblkgen: can't open file '%s'\n", DISKDEFS);
		exit(2);
		}
	if (!lookup(deffile))
		{
		fprintf(stderr, "fblkgen: disk type '%s' not found\n", type);
		exit(2);
		}
	fclose(deffile);
	}

/* scan the diskdef file for the disk type, remember its parameters */

lookup(file)
	FILE *file;
	{
	char line[LINELEN];
	char message = NO;
	int count;

	while (fgets(line, LINELEN, file))
		{
		if (line[strlen(line)-1] != '\n')
			{
			if (!message && *line != COMMENT)
				{
				fprintf(stderr, 
				 "track: diskdef line too long\n");
				message = YES;
				}
			while (fgets(line, LINELEN, file) && 
				line[strlen(line)-1] != '\n')
				;
			continue;
			}
		if (*line != COMMENT)
			{
			count = sscanf(line, 
			  "%s %d '%[^']' %s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			  dinfo.dtype, &dinfo.typeno, dinfo.formatter, 
			  dinfo.diag, dinfo.strat, &dinfo.maxbad, 
			  &dinfo.rfs, &dinfo.nls, &dinfo.pss, &dinfo.spt, 
			  &dinfo.cyl, &dinfo.heads, &dinfo.precomp, 
			  &dinfo.interl, &dinfo.spiral, &dinfo.step, 
			  &dinfo.starthead, &dinfo.ecc, &dinfo.attm, 
			  &dinfo.attw, &dinfo.slice0, &dinfo.gap1, 
			  &dinfo.gap2, &dinfo.gap3, &dinfo.gap4, &dinfo.eattw);
			if (strcmp(dinfo.dtype, type) == 0) {
				if (count < 20)
					fprintf(stderr, 
					 "track: diskdef line too short\n");
				else
					return(1);
			}
			}
		}
	return(0);
	}

/* read a bad track or alternate list */

readlist(l, file, max, format)
	struct list *l;
	char *file;
	int max, format;
	{
	FILE *infile;
	int data;
	register int i;
	int head, cyl;
	int logdev, blkoff;
 
	l->value = (int *) malloc(max * sizeof(int));
	infile = stdin;
	if (file && (infile = fopen(file, "r")) == NULL)
		{
		fprintf(stderr, "fblkgen: can't open file '%s'\n", file);
		exit(2);
		}
	/*
	if (infile == stdin && isatty(0))  {
	*/
	if (infile == stdin)  {
		printf("Enter bad track numbers");
		if (format == DO_HDCYL)
		    printf (" as   <Head> <Cylinder>  ");
		if (format == DO_DEVBLK)
		    printf (" as   <logical dev> <block>  ");
		printf ("; end with a period\n");
	}
	if (format == DO_TRACKS)  {
	    while (fscanf(infile, " %d", &data) > 0 && l->count < max)
		l->value[l->count++] = data;
	} else {
	    if (format == DO_HDCYL) {
	    	while (((i = fscanf(infile, " %d%d", &head, &cyl)) > 1)  &&
		    	l->count < max)
			l->value[l->count++] = (dinfo.heads * cyl) + head;
	    }
	    else {
	    	while (((i = fscanf(infile, " %d%d", &logdev, &blkoff)) > 1)  &&
		    	l->count < max)
			l->value[l->count++] = gettrack(logdev, blkoff);
	    }
	}
	    
	fscanf(infile, " .");
	if (infile != stdin)
		fclose(infile);
	}

/* convert logical device / block offset to a track number */

gettrack(logdev, blkoff)
int logdev, blkoff;
{
	char *contlr;		/* controller type, m320, m360, etc */
	int bpt;		/* blocks per track */
	
	contlr = (char *)malloc(5);
	strncpy(contlr, type, 4);
	contlr[4] = '\0';
	bpt=(dinfo.pss*dinfo.spt)/BSIZE;
	if (strcmp(contlr, "m320") == 0) {
		return( (dtable[(logdev % 8)].pstart + blkoff) / bpt);
	}
	else {
		if (strcmp(contlr, "m360") == 0) {
			return( (dtable[(logdev % 16)].pstart + blkoff) / bpt);
		}
		else {
			printf("unknown controller type\n");
			exit(1);
		}
	}
}


/* scan the command line options */

scanopts(argc, argv)
	int argc;
	char *argv[];
	{
	int c;
	extern int optind;
	extern char *optarg;

	while ((c = getopt(argc, argv, "Tt:")) !=EOF)
		switch(c)
			{
			case 'T':
				badform = DO_TRACKS;
				break;
			case 't':
				badfile = optarg;
				break;
			case '?':
				fprintf(stderr, USAGE);
				exit(2);
			}
	if (optind + 2 != argc)
		{
		fprintf(stderr, USAGE);
		exit(2);
		}
	type = argv[optind++];
	rdev = argv[optind];
	strncpy(devshort, rdev, (strlen(rdev) - 1));
	}

/* dump contents of diskinfo structure */

dumpdef()
{
fprintf(stderr,"diskinfo DUMP:\n");
fprintf(stderr,"type=%s, no=%d, for=%s\n", dinfo.dtype, dinfo.typeno, dinfo.formatter);
fprintf(stderr,"diag=%s, strat=%s,  maxb=%d\n", dinfo.diag, dinfo.strat, dinfo.maxbad);
fprintf(stderr,"rfs=%d, nls=%d, pss=%d, spt=%d \n", dinfo.rfs, dinfo.nls, dinfo.pss, dinfo.spt);
fprintf(stderr,"cyl=%d, head=%d, prec=%d \n", dinfo.cyl, dinfo.heads, dinfo.precomp);
fprintf(stderr,"intl=%d, spir=%d, step=%d \n", dinfo.interl, dinfo.spiral, dinfo.step);
fprintf(stderr,"sh=%d, ecc=%d, attm=%d \n", dinfo.starthead, dinfo.ecc, dinfo.attm);
fprintf(stderr,"attw=%d, sl0=%d, gap1=%d \n", dinfo.attw, dinfo.slice0, dinfo.gap1);
fprintf(stderr,"gap2=%d, gap3=%d, gap4=%d, eattw=%d \n\n", dinfo.gap2, dinfo.gap3, dinfo.gap4, dinfo.eattw);
}

/* dump contents of superblock buffer */

dumpsblk(ib)
BUFAREA ib;
{
	fprintf(stderr,"fsiz=%d, free=%d,  ninod=%d,  freeino=%d\n", ib.b_fs.s_fsize, 
	ib.b_fs.s_nfree, ib.b_fs.s_ninode, ib.b_fs.s_tinode);
	fprintf(stderr,"state=x%x, magic=x%x, type=%d\n",
	ib.b_fs.s_state, ib.b_fs.s_magic, ib.b_fs.s_type);
}

dumptable()
{
	int i;
	for (i=0; i<SUBPART; i++) {
	      fprintf(stderr,"%d:pfl=%1s, ffl=%1s, di=%6d, sz=%6d, btyp=%1d, st=%6d, ln=%6d\n",
	      i, (dtable[i].pflg==YES)?"Y":"N", 
	      (dtable[i].fflg==YES)?"Y":"N", dtable[i].dist,
	      dtable[i].size, dtable[i].btype, dtable[i].pstart, 
	      dtable[i].plength);
	}
}

char *
itoa(intp)
int *intp;
{
	int value;
	static char abuf[16];

	value = *((int *)intp);
	sprintf(abuf,"%ld", value);
	return(abuf);
}

getline(fp,loc,maxlen)
FILE *fp;
char *loc;
{
	register n, ignore_rest = 0;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[maxlen-1];
	while((n = getc(fp)) != '\n') {
		if(n == EOF)
			return(EOF);
		if(n == ' ' || n == '	'){
			if(p > loc){
				ignore_rest = 1;
			}
		}else{
			if(ignore_rest == 0 && p < lastloc){
				*p++ = n;
			}
		}
	}
	*p = 0;
	return(p - loc);
}
