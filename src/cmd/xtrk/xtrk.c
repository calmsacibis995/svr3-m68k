#include <stdio.h>
#include <fcntl.h>
#include <sys/io/win.h>
#include <sys/io/smd.h>
#include <sys/io/esdi.h>
#include <sys/types.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5filsys.h>


/* exit codes:
	0 - success
	1 - internal failure, bad values and such
	2 - I/O failure, couldn't find files, couldn't read/write, etc
	3 - bad command usage
	4 - user asked to quit
*/
#define DISKDEFS  	"/etc/diskdefs" /* where to get disk format data */
#define YES		1
#define NO		0
#define COMMENT 	'*'
#define LINELEN 	512

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

struct filecntl {
	int	rfdes;
	int	wfdes;
	int	mod;
};
struct filecntl	dfile;			/* file descriptors for filesys */

/* This stuff will have to change if we do dynamic partition redefinition */
int wd15[16] = {WD15};			/* disk partition information */
int wd40[16] = {WD40};			/* disk partition information */
int wd70[16] = {WD70};			/* disk partition information */
int wd140[16] = {WD140};		/* disk partition information */
int fu337[16] = {FU337};		/* disk partition information */
int cdc182mb[8][3] = {CDC182MB};	/* disk partition information */

int xfflag = NO;	/* available if we need to single out tasks */
int fbflag = NO;
long *partkey;		/* key into the partition def'n file */
char cmd[15];		/* the name we're called by, for prints and branch */
char *device;		/* device name, m320_0, m360_1, etc */
char *type;		/* drive type, input parameter, m32070m, etc. */
int ecode;		/* all-purpose error code to check return values */

/* This stuff is specifically for the xformtrk part */
#define XUSAGE \
"usage: xformtrk -I {c|t|d} -O {c|t} [-t file] type dev\n \
	(formats:     c - Hd/Cyl,    t - Trk,    d - Dev/Blk\n \
	type: m32070m, m360337, etc     dev: m320_0, m360_2, etc)\n\n"

struct xlist {		/* generic list type for bad track list */
	int count;
	int *single;
	int *pair;
} outtable;

char in_fmt;		/* format of input file */
char out_fmt;		/* format of output file */
char *contlr;		/* controller type, m320, m360, etc */
char *in_file;		/* file containing bad track, input parameter */
char *out_file;		/* file containing bad track, output parameter */

int convfd;		/* file descriptor for conversion data */
char *outprfx = "/tmp/xf_";	/* prefix for conversion file */
/* end xformtrk stuff */

/* This stuff is specifically for the fblkgen part */
#define MAXPART 	8
#define SUBPART		MAXPART - 1	/* don't count last  partn(full disk) */
#define DO_TRACKS	1		/* file format of tracks */
#define	DO_HDCYL	2		/* file format of head, cyl */
#define	DO_DEVBLK	3		/* file format of log dev, block */
#define	SBSIZE		512		/* superblock size */
#define FUSAGE \
"usage: fblkgen [-T] [-t file] type dev\n \
 	 type: m32070m, m360337, etc\n \
	 dev:  m320_0,  m360_2,  etc)\n\n"

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
			
struct flist {		/* generic list type for bad track lists */
	int count;
	int *value;
} tracks;

union bufarea {
		char b_buf[BSIZE];	/* buffer space */
		struct filsys b_fs;    	/* the superblock */
};
typedef union bufarea BUFAREA;

int badform;		/* format of file containing badtrack */
char *badfile;		/* file containing bad track, input parameter */

int fsckfd;		/* file descriptor for fsck input */
char *fsckprfx = "/etc/badtracks/F.";	/* file prefix for fsck */
char *fsckinp;				/* write file for fsck */
/* end fblkgen stuff */

main(argc, argv)
char *argv[];
{
	char *cell;
	int i;

	/* get the name minus any path prefix */
	cell = argv[0] + (strlen(argv[0]));
	while ((cell >= argv[0]) && (*cell != '/'))
		cell--;
	strcpy(cmd, cell+1);
	if (strcmp(cmd, "xformtrk") == 0)
	{
		xfflag = YES;
		ecode = xform(argc, argv);
	}
	else if (strcmp(cmd, "fblkgen") == 0)
	{
		fbflag = YES;
		ecode = fblk(argc, argv);
	}
	else {
		fprintf(stderr, "Branches exist for 'fblkgen' and 'xformtrk' only\n");
		ecode = 1;
	}
	exit(ecode);
}

/* convert a file of one badtrack data type to another */
xform(argc, argv)
int argc;
char *argv[];
{
	char *itoa();
	char *bufsingle, *bufpair;
	char retn = '\n';
	char blank = ' ';
	int index;
	int length;

	alloc_glbl();
	scanopts(argc, argv);
	strncpy(contlr, type, 4);
	contlr[4] = '\0';
	if ((ecode = readdefs()) != 0) 		/* read disk parameters */
		return(ecode);
	if ((partkey = (long *)mapdisk(type)) == 0) {
		fprintf(stderr,"%s: No match of key for %s\n", cmd, type);
		return(1);
	}
	if ((ecode = xreadlist(&outtable, in_file, 3 * dinfo.maxbad, in_fmt, out_fmt)) != 0)
		return(ecode);
        strcpy(out_file, outprfx);
        strcat(out_file, device);
        if ((convfd = open(out_file, O_WRONLY | O_TRUNC )) == -1)
        	if ((convfd = creat(out_file, 0666)) == -1) {
			fprintf(stderr, "%s: can't create file %s\n", cmd,
			out_file);
			return(2);
		}
	for (index=0; index<outtable.count; index++) {
		bufsingle = itoa(&outtable.single[index]);
		length = strlen(bufsingle);
		if (write(convfd, bufsingle, length) != length) {
			fprintf(stderr, "%s: write error\n", cmd);
			return(2);
		}
		if (out_fmt == 'c') {
			write(convfd, &blank, sizeof(char));
			bufpair = itoa(&outtable.pair[index]);
			length = strlen(bufpair);
			if (write(convfd, bufpair, length) != length) {
				fprintf(stderr, "%s: write error\n", cmd);
				return(2);
			}
		}
		write(convfd, &retn, sizeof(char));
	}
        close(convfd);
	return(0);
}

fblk(argc, argv)
int argc;
char *argv[];
{
	int index;		/* a counter */
	BUFAREA ib;		/* buf area for superblock */
	char dev[30];		/* device to get superblock for */
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

	alloc_glbl();
	badform = DO_HDCYL;
	scanopts(argc, argv);
	if ((ecode = readdefs()) != 0)	/* read disk parameters */
		return(ecode);
	if ((partkey = (long *)mapdisk(type)) == 0) {
		fprintf(stderr,"%s: No match of key for %s\n", cmd, type);
		return(1);
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
		else {
		    if (partkey == fu337) {
			bpc=(dinfo.pss*dinfo.spt*dinfo.heads)/BSIZE;
          		dtable[index].plength = *(partkey+(2*index));
          		dtable[index].pstart = (*(partkey+(2*index)+1))*bpc;
		    }
		    else {
			if (partkey == cdc182mb) {
          		    dtable[index].pstart = (*(partkey+(3*index)+1));
          		    dtable[index].plength = (*(partkey+(3*index)+2));
			}
			else {
          		    fprintf(stderr, "%s: BAD partition option.\n", cmd);
			    return(1);
		        }
		    }
		}
        }
	if ((ecode = freadlist(&tracks, badfile, 3 * dinfo.maxbad, badform))!=0)
		return(ecode);

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
          	strcpy(dev, "/dev/rdsk/");
          	strcat(dev, device);
          	strcat(dev, "s");
          	strcat(dev, suffix);
          	if(getsblk(dev, &ib) == NO) {
          		fprintf(stderr,"%s: SUPERBLOCK (or equiv) not found\n", cmd);
          		return(2);
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
          	/* determine if track falls within the file system here */
          	if (dtable[index].size != 0) {
          		/* fill in fflg if track falls within the filsys */
          		fsblk = pblk - dtable[index].pstart; /* relative */
          		if (fsblk < dtable[index].size) {
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
          /* Find out if block occurs in multiple file systems - that's bad */
          if (fscount > 1) {
          	fspartn = queryusr(fscount);
		fscount = 1;
	  }
          if (fscount != 0) {
          	/* write the blocks to the corresponding file */
          	strcpy(fsckinp, fsckprfx);
		strcat(fsckinp, device);
		strcat(fsckinp, "s");
          	suffix = itoa(&fspartn);
          	strcat(fsckinp, suffix);
          	if ((fsckfd = open(fsckinp, O_WRONLY | O_APPEND)) == -1)
          		if ((fsckfd = creat(fsckinp, 0666)) == -1) {
				fprintf(stderr, "%s: can't create file %s\n", cmd, fsckinp);
				return(2);
			}
          	if ((ecode = conv_wr(fsckfd, fspartn)) != 0)
			return(ecode);
          	close(fsckfd);
          }
    	}
	return(0);
}

/* do allocation of space */
alloc_glbl()
{
	type = (char *)malloc(15);	/* get space for options */
	device = (char *)malloc(5);
	out_file = (char *)malloc(120);
	contlr = (char *)malloc(5);
	fsckinp = (char *)malloc(120);
}

/* map disk device to partition key */
/* return the address of the array to use as parms */
long 
mapdisk(type)
char *type;
{
	if (strcmp(type, "m32040m") == 0)
		return((long)wd40);
	if (strcmp(type, "m32040t") == 0)
		return((long)wd40);
	if (strcmp(type, "m32040p") == 0)
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
	if (strcmp(type, "m360337") == 0)
		return((long)fu337);
	if (strcmp(type, "m323182") == 0)
		return((long)cdc182mb);
	return(0);
}


/* scan the command line options */

scanopts(argc, argv)
int argc;
char *argv[];
{
	int c;
	extern int optind;
	extern char *optarg;

	if (xfflag) {
		while ((c = getopt(argc, argv, "I:O:t:")) !=EOF)
		   switch(c)
		   {
			case 'I':
				in_fmt = *optarg;
				if (in_fmt != 'c' && in_fmt != 'd' && in_fmt != 't') {
					fprintf(stderr, "\n%s: Bad input format option.\n", cmd);
					fprintf(stderr, XUSAGE);
					exit(3);
				}
				break;
			case 'O':
				out_fmt = *optarg;
				if (out_fmt != 'c' && out_fmt != 't') {
					fprintf(stderr, "\n%s: Bad output format option.\n", cmd);
					fprintf(stderr, XUSAGE);
					exit(3);
				}
				break;
			case 't':
				in_file = optarg;
				break;
			case '?':
				fprintf(stderr, XUSAGE);
				exit(3);
		   }
		if (optind + 2 != argc)
		{
			fprintf(stderr, XUSAGE);
			exit(3);
		}
	}
	else {
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
					fprintf(stderr, FUSAGE);
					exit(3);
			}
		if (optind + 2 != argc)
		{
			fprintf(stderr, FUSAGE);
			exit(3);
		}
	}
	type = argv[optind++];
	device = argv[optind];
}

/* read the diskdef information for the disk type */

readdefs()
{
	FILE *deffile;

	if ((deffile = fopen(DISKDEFS, "r")) == NULL)
	{
		fprintf(stderr, "%s: can't open file '%s'\n", cmd,DISKDEFS);
		return(2);
	}
	if (!lookup(deffile))
	{
		fprintf(stderr, "%s: disk type '%s' not found\n", cmd,type);
		return(1);
	}
	fclose(deffile);
	return(0);
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
				fprintf(stderr, "track: diskdef line too long\n");
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
					fprintf(stderr, "track: diskdef line too short\n");
				else
					return(1);
			}
			}
		}
	return(0);
}

/* convert track to head/cyl */

t2c(track, o_head, o_cyl)
int track;
int *o_head, *o_cyl;	/* output parms */
{
	*o_cyl = (int)(track / dinfo.heads);
	*o_head = track - (*o_cyl * dinfo.heads);
}

/* convert head/cyl to track */

c2t(head, cyl, o_track)
int head, cyl;	
int *o_track; 	/* output parms */
{
	*o_track = (cyl * dinfo.heads) + head;
}

/* convert dev/blk to track */

d2t(dev, blk, o_track)
int dev, blk;
int *o_track;	/* the output parameter */
{
	int bpt;		/* blocks per track */
	int bpc;		/* blocks per cylinder */
	int track;		/* track offset into disk */
	int block;		/* block offset into disk */
	
	bpt=(dinfo.pss*dinfo.spt)/BSIZE;
	if (strcmp(contlr, "m320") == 0) {
          	block = *(partkey+(2*(dev % 8)+1)) + blk;
		*o_track = (int)(block / bpt);
	}
	else {
		if (strcmp(contlr, "m360") == 0) {
			bpc=(dinfo.pss*dinfo.spt*dinfo.heads)/BSIZE;
          		block = (*(partkey+(2*(dev % 8)+1)) + blk) * bpc;
			*o_track = (int)(block / bpt);
		}
		else {
			if (strcmp(contlr, "m323") == 0) {
          			block = *(partkey+(3*(dev % 8)+1)) + blk;
				*o_track = (int)(block / bpt);
			}
			else {
				fprintf(stderr, "unknown controller type\n");
				exit(1);
			}
		}
	}
}

/* convert dev/blk to head/cyl */

d2c(dev, blk, o_head, o_cyl)
int dev, blk;
int *o_head, *o_cyl;	/* output parms */
{
	char *ctlr;		/* controller type, m320, m360, etc */
	int bpt;		/* blocks per track */
	int bpc;		/* blocks per cylinder */
	int track;		/* track offset into disk */
	int block;		/* block offset into disk */
	
	ctlr = (char *)malloc(5);
	strncpy(ctlr, type, 4);
	ctlr[4] = '\0';
	bpt=(dinfo.pss*dinfo.spt)/BSIZE;
	if (strcmp(ctlr, "m320") == 0) {
          	block = *(partkey+(2*(dev % 8)+1)) + blk;
		track = (int)(block / bpt);
	}
	else {
		if (strcmp(ctlr, "m360") == 0) {
			bpc=(dinfo.pss*dinfo.spt*dinfo.heads)/BSIZE;
          		block = (*(partkey+(2*(dev % 8)+1)) + blk) * bpc;
			track = (int)(block / bpt);
		}
		else {
			if (strcmp(ctlr, "m323") == 0) {
          			block = *(partkey+(3*(dev % 8)+1)) + blk;
				track = (int)(block / bpt);
			}
			else {
				fprintf(stderr, "unknown controller type\n");
				exit(1);
			}
		}
	}
	*o_cyl = (int)(track / dinfo.heads);
	*o_head = track - (*o_cyl * dinfo.heads);
}

getsblk(dev, bp)
char *dev;
BUFAREA *bp;
{
	register struct filecntl *fcp;

	if((dfile.rfdes = open(dev,0)) < 0) {
		fprintf(stderr,"%s: Can't open %s\n",cmd, dev);
		return(NO);
	}
	if((dfile.wfdes = open(dev,1)) < 0)
		dfile.wfdes = -1;
	fcp = &dfile;
	if(lseek(fcp->rfdes,(long)SUPERBOFF,0) < 0)
		fprintf(stderr,"%s: error on superblock SEEK \n", cmd);
	else if(read(fcp->rfdes,bp->b_buf,SBSIZE) == SBSIZE) {
		close(dfile.rfdes);
		return(YES);
	}
	fprintf(stderr,"%s: error on superblock READ \n", cmd);
	close(dfile.rfdes);
	return(NO);
}


/* convert logical device / block offset to a track number */

gettrack(logdev, blkoff)
int logdev, blkoff;
{
	char *ctlr;		/* controller type, m320, m360, etc */
	int bpt;		/* blocks per track */
	
	ctlr = (char *)malloc(5);
	strncpy(ctlr, type, 4);
	ctlr[4] = '\0';
	bpt=(dinfo.pss*dinfo.spt)/BSIZE;
	if (strcmp(ctlr, "m320") == 0) {
		return( (dtable[(logdev % 8)].pstart + blkoff) / bpt);
	}
	else
		if (strcmp(ctlr, "m360") == 0)
			return( (dtable[(logdev % 8)].pstart + blkoff) / bpt);
		else {
			if (strcmp(ctlr, "m323") == 0)
				return( (dtable[(logdev % 8)].pstart + blkoff) / bpt);
			else {
				fprintf(stderr, "unknown controller type\n");
				exit(1);
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
		default: 	fprintf(stderr, "%s: bad file system block size\n", cmd);
				return(1);
				break;
	}
	fsblk = dtable[partn].dist / factor;
	if (bpt < 1) 	/* in case of blocks larger than the track, */
		bpt = 1;	/* count that one whole block as bad */
	for (i = fsblk; i < (fsblk+bpt); i++) {
		buf = itoa(&i);
		length = strlen(buf);
		if (write(fd, buf, length) != length) {
			fprintf(stderr, "%s: write error\n", cmd);
			return(2);
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
	fprintf(stdout, "\n%s: %d FILE SYSTEMS ARE SUPERIMPOSED\n", cmd, count);
	fprintf(stdout, "	STARTING PARTITIONS:  ");
	for (index=0; index<SUBPART; index++)
		if (dtable[index].fflg == YES)
			fprintf(stdout, "%d, ", index);
	fprintf(stdout,"\n");
	fprintf(stdout,"WHICH IS CURRENT? (enter partition, cntl d to quit): ");
	for (;;) {
		if(getline(stdin,line,sizeof(line)) == EOF)
			exit(4);
		fprintf(stdout,"\n");
		reply = atoi(line);
		if(reply >= 0 && reply < SUBPART) {
			if (dtable[reply].fflg == YES)
				return(reply);
		}
		fprintf(stdout,"You must enter a number from the list: ");
		line[0] = '\0';
	}
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

/* read a bad track list */

freadlist(l, file, max, format)
struct flist *l;
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
		fprintf(stderr, "%s: can't open file '%s'\n", cmd, file);
		return(2);
	}
	/*
	if (infile == stdin && isatty(0))  {
	*/
	if (infile == stdin)  {
		fprintf(stdout, "Enter bad track numbers");
		if (format == DO_HDCYL)
		    fprintf (stdout, " as   <Head> <Cylinder>  ");
		if (format == DO_DEVBLK)
		    fprintf (stdout, " as   <logical dev> <block>  ");
		fprintf (stdout, "; end with a period\n");
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
	else 		/* get rid of the last <cr> sitting out there */
		getc(stdin);
	return(0);
}

/* read a bad track list */

xreadlist(lout, file, max, in_fmt, out_fmt)
struct xlist *lout;
char *file;
int max;
char in_fmt, out_fmt;
{
	FILE *infile;
	register int i;
	int head, cyl, dev, blk, trk;
	int o_trk, o_head, o_cyl, o_dev, o_blk;
	int max_tracks;
 
	lout->single = (int *) malloc(max * sizeof(int));
	lout->pair = (int *) malloc(max * sizeof(int));
	max_tracks = dinfo.cyl * dinfo.heads;
	infile = stdin;
	if (file && (infile = fopen(file, "r")) == NULL)
		{
		fprintf(stderr, "%s: can't open file '%s'\n", cmd, file);
		return(2);
		}
	/*
	if (infile == stdin && isatty(0))  {
	*/
	if (infile == stdin)  {
		fprintf(stdout, "Enter bad track numbers");
		if (in_fmt == 'c')
		    fprintf (stdout, " as   <Head> <Cylinder>  ");
		if (in_fmt == 'd')
		    fprintf (stdout, " as   <logical dev> <block>  ");
		fprintf (stdout, "; end with a period\n");
	}
	if (in_fmt == 't')  {
	    	while (fscanf(infile, " %d", &trk) > 0  &&
		    	lout->count < max) {
			if (trk >= 0 && trk < max_tracks) {
			    switch (out_fmt) {
				case 't':	/* do nothing */
					lout->single[lout->count++] = trk;
					break;
				case 'c':
					t2c(trk, &o_head, &o_cyl);
					lout->single[lout->count] = o_head;
					lout->pair[lout->count++] = o_cyl;
					break;
			    }
			}
			else
				fprintf(stdout,"BAD INPUT - %d\n", trk);
		}
	} else {
	    if (in_fmt == 'c') {
	    	while (((i = fscanf(infile, " %d%d", &head, &cyl)) > 1)  &&
		    	lout->count < max)
			if ((head >= 0 && head < dinfo.heads) && (cyl >= 0 && cyl < dinfo.cyl)) {
			    switch (out_fmt) {
				case 't':
					c2t(head, cyl, &o_trk);
					lout->single[lout->count++] = o_trk;
					break;
				case 'c':	/* do nothing */
					lout->single[lout->count] = head;
					lout->pair[lout->count++] = cyl;
					break;
			    }
			}
			else
				fprintf(stdout,"BAD INPUT - %d, %d\n", head, cyl);
	    }
	    else {
	    	while (((i = fscanf(infile, " %d%d", &dev, &blk)) > 1)  &&
		    	lout->count < max)
			switch (out_fmt) {
				case 't':
					d2t(dev, blk, &o_trk);
					if (o_trk >= 0 && o_trk < max_tracks)
						lout->single[lout->count++] = o_trk;
					else
						fprintf(stdout,"BAD INPUT - %d, %d\n", dev, blk);
					break;
				case 'c':
					d2c(dev, blk, &o_head, &o_cyl);
					if ((o_head >= 0 && o_head < dinfo.heads) && (o_cyl >= 0 && o_cyl < dinfo.cyl)) {
						lout->single[lout->count] = o_head;
						lout->pair[lout->count++] = o_cyl;
					}
					else
						fprintf(stdout,"BAD INPUT - %d, %d\n", dev, blk);
					break;
			}
	    }
	}
	fscanf(infile, " .");
	if (infile != stdin)
		fclose(infile);
	else 		/* get rid of the last <cr> sitting out there */
		getc(stdin);
	return(0);
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

#ifdef DEBUG
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
#endif

