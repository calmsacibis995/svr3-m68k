/*	@(#)dinit.c	7.5		*/
/*
 *	Copyright (c) 1985	Motorola Inc.  
 *		All Rights Reserved.
 */

/* Disk formatting and initialization utility */
/* (C) Copyright 1984 by Motorola Inc. */
/*
	options:
		-f	- format the disk
		-o	- override previous information on disk
		-c	- check the disk for additional bad tracks
		-e	- substitute "EXORMACS" for "MOTOROLA" in vid_mac
		-r	- do nothing, read/print badtrack list; set Readonly
		-a	- print alternates along with bad tracks
		-T	- accept/print badtracks in track number format
			- default is <head> <cylinder> format
		-d desc	- put "desc" into vid_vd (volume description) field
		-b file - insert contents of file (a.out format) as bootloader
		-t file	- take bad track numbers from file (instead of user)
		-g file - map new bad track file entries to available alternate
			  tracks.
*/
/*  	options from "dinit" to the device's "format" routine:
		
		-F	- set-up the configuration block, but don't format
			  the entire disk.
		-A	- read the bad track table off the disk and
			  map bad tracks to the alternates.
		-M	- map a bad track to an alternate track.

*/



#include "stdio.h"
#include "sys/dk.h"
#include "fcntl.h"
#include "filehdr.h"
#include "scnhdr.h"
#include "ldfcn.h"
#include "string.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/errno.h"

extern int errno;

#define CHGBTIOCTL 3000		/* ioctl command for updating the disk driver
				   bad track list in core. */
#define YES	1
#define NO	0
#define BIG	((1 << 31) - 1)
#define USAGE \
"usage: dinit [-r] [-T] [-focea] [-d desc] [-b file] [-t file] [-g file] \
type rawdev\n"
#define FLAG (0xffff)
#define HIGHNUMBER (0x7fff)
#define DO_TRACKS	1
#define	DO_HDCYL	2

#define FTYPE(x)	(x & 0170000)	/* file type (from stat(2)) */
#define CHAR		0020000		/* char (raw) type device */
#define BLOCK		0060000		/* block type device */
#define MINOR(x)	(x & 0xff)	/* minor device number */
#define RW		06		/* read/write for access(2) */

#define LINELEN 512
#define COMMENT '*'

#define MAXBOOT	(16 * 1024)	/* maximum bootloader size = 16K */
#define LSS	256		/* logical sector size */
#define BLOCKSZ	512		/* largest supported block size */

/* bad track strategies */

#define PERFECT 	0
#define HARDWARE	1
#define SOFTWARE	2

#ifdef DEBUG
#define DISKDEFS	diskdefs
char *diskdefs = "diskdefs";
char diskalts[40] = "diskalts/";
#else
#define DISKDEFS	"/etc/diskdefs"
char diskalts[40] = "/etc/diskalts/";
#endif



char *bootloader;		/* bootloader file name */
char *tracklist;		/* bad tracks file name */
char *altlist;			/* alternate tracks file name */
char *type;			/* disk type name */
char *rdev;			/* raw device name (slice 7) */ 
char bdev[80];			/* block device name (slice 7) */ 
char formatter[80];		/* disk formatter name */

FILE	*alt,
	*track,
	*boot;

char	override = NO,		/* override any current disk contents */
	readonly = 0,		/* don't touch the disk, readonly */
	prtalts = 0,		/* print alternate tracks along with bads */
	badform = DO_HDCYL,	/* accept/print bad list as <head> <cyl> */
	mkfs = NO,		/* perform 'mkfs(1M)' on slice o */
	hasboot = NO,		/* copy in the bootloader */
	hasdiag = NO,		/* create diagnostic tracks */
	check = NO,		/* scan the disk for bad tracks */
	exor = NO,		/* -e option flag */
	doformat = NO;		/* format the disk */
	morebadtrks = NO;	/* map bad track file changes to available
				   alternate tracks. */

char	*desc;			/* argument to -d */
char	btstrat;		/* bad track strategy */

int	slice0;			/* #blocks in slice 0 for 'mkfs(1M)' */
int	maxbad;			/* maximum number of bad tracks per disk */

struct badent *bttable;		/* bad track table */

struct diagent *diagdir;	/* diagnostic test area directory */

int diaglo;		/* the starting track of the first diagnostic area */
int diaghi;		/* the starting track of the second diagnostic area */

unsigned char hash[HASHSIZE];	/* bad track hash table */

struct list {		/* generic list type for bad track & alts lists */
	int count;
	int *value;
	} tracks, alts, tmpalts;

int *altno;		/* the assigned alternates for the bad tracks */

struct volumeid v;	/* prototype Volume ID Block */
struct dkconfig c;	/* prototype Configuration Block */

char *malloc(), *realloc();

unsigned char diagpat[] = {	/* diagnostic pattern for vid */
	0x0f,	0x1e,	0x2d,	0x3c,	0x4b,	0x5a,	0x69,	0x78,
	0x87,	0x96,	0xa5,	0xb4,	0xc3,	0xd2,	0xe1,	0xf0,	
	0x0f,	0x1e,	0x2d,	0x3c,	0x4b,	0x5a,	0x69,	0x78,
	0x87,	0x96,	0xa5,	0xb4,	0xc3,	0xd2,	0xe1,	0xf0,	
	0xf1,	0xf2,	0xf4,	0xf8,	0xf9,	0xfa,	0xfc,	0xfe,
	0xff,	0x7f,	0xbf,	0xdf,	0xef,	0x6f,	0xaf,	0xcf,
	0x4f,	0x8f,	0x0f,	0x07,	0x0b,	0x0d,	0x0e,	0x06,
	0x0a,	0x0c,	0x04,	0x08,	0x04,	0x02,	0x01,	0x00
	};

main(argc, argv)
	int argc;
	char *argv[];
	{
	char name[40];
	char command[80];
	int fid;
	char *s, *t;

	memcpy(v.vid_mac, "MOTOROLA", 8);
	memcpy(v.vid_dtp, diagpat, sizeof(diagpat));
	v.vid_cas = 1;
	v.vid_cal = 1;

	scanopts(argc, argv);
	readdefs(DISKDEFS, type);
	if (btstrat != PERFECT)
		{
		tracks.value = (int *) malloc(3 * maxbad * sizeof(int));
		alts.value = (int *) malloc(maxbad * sizeof(int));
		tmpalts.value = (int *) malloc(maxbad * sizeof(int));
		}

	if(morebadtrks){
		map_new_bads();
		exit(0);
	}

	if (doformat || override)
		{
		fid = open(bdev, O_WRONLY);
		if (strchr(formatter, '#') != NULL)
	/* If this form is used, the -F option must be accepted at the end */
			{
			t = formatter;
			while ((s = strchr(t, '#')) != NULL)
				{
				strncat(command, t, s - t);
				strcat(command, rdev);
				t = s + 1;
				}
			strcat(command, t);
			if (override && !doformat)
				strcat(command, " -F");
			}
		else    {
			strcpy(command, formatter);
			if (override && !doformat)
				strcat(command, " -F");
			strcat( command, " ");
			strcat(command, rdev);
			}
		if (!readonly)
		    system(command);

	/* set up (incomplete) header to make disk usable */

		lseek(fid, 0L, 0);
		if (write(fid, &v, sizeof(struct volumeid)) == -1 )
			fprintf(stderr,
				 "dinit: error on write to %s, probably not formatted\n",bdev);
		lseek(fid, c.ios_rec, 0);
		write(fid, &c, sizeof(struct dkconfig));
		close(fid);
		}
	else	     /* read current vid/config structs and bad track table */
		readhead(bdev);	

	/* Overwrite requested fields in vid */

	if ( exor == YES )
	    memcpy(v.vid_mac, "EXORMACS", 8);
	if ( *desc )
	    strncpy(v.vid_vd, desc, 20);

	if (check)
		checkdisk(rdev);
	if (hasdiag)
		initdiag();
	/* 
	 * For Hardware/Software, prompt only if changing the bad track table.
	 * (Used to always prompt if btstrat == SOFTWARE.  This is dangerous...
	 * MMD - wjc - 09/06/86)
	 */
	if (!readonly && (btstrat != PERFECT) && (doformat || override) )
		{
		readlist(&tracks, tracklist, "bad track numbers", 3 * maxbad,
			badform);
		if ( tracks.count != 0)
			readlist(&alts, altlist, "alternate track numbers", 
				maxbad, DO_TRACKS);
		assignalt();
		}
	else if (tracks.count && btstrat == PERFECT )
		{
		fprintf(stderr, "dinit: bad tracks on 'perfect' disk\n");
		exit(2);
		}
	if (!readonly)
	    setup(bdev, rdev);
	/*  
	 *  For Hardware badtrack handling set up a second call 
	 *  to the format routine with a -A option.
	 */
	if  (  btstrat == HARDWARE && ( doformat || override )  )  {
		fid = open(bdev, O_WRONLY);
		if (strchr(formatter, '#') != NULL)
	/* If this form is used, the -F option must be accepted at the end */
			{
			t = formatter;
			while ((s = strchr(t, '#')) != NULL)
				{
				strncat(command, t, s - t);
				strcat(command, " -A ");
				strcat(command, rdev);
				t = s + 1;
				}
			strcat(command, t);
			if (override && !doformat)
				strcat(command, " -F");
		}
		else    {
			strcpy(command, formatter);
			if (override && !doformat)
				strcat(command, " -F");
			strcat(command, " -A ");
			strcat(command, rdev);
			}
		if (!readonly)
		    system( command );
		close( fid );
	}
	if (mkfs && !readonly)
		{
		strcpy(name, rdev);
		name[strlen(name) - 1] = '0';
		sprintf(command, "/etc/mkfs %s %d", name, slice0);
		system(command);
		}
	}


/* map new bad track file entries to available alternate tracks */
map_new_bads()
{
	int	comp();		/* compare function used by qsort */
	register int *p, *q;
	int	fid, i, j, found;
	struct	list bads;      /* all bad tracks in bad track file */
	struct  list newbads;	/* new bad tracks derived from bad track file */
	struct  list availalts; /* available alternate tracks */

	bads.value = (int *) malloc(3 * maxbad * sizeof(int));
	newbads.value = (int *) malloc(3 * maxbad * sizeof(int));
	availalts.value = (int *) malloc(maxbad * sizeof(int));
	bads.count = newbads.count = availalts.count = 0;

	if(btstrat == PERFECT){
		fprintf(stderr,"dinit: -g option invalid for perfect media.\n");
		exit(2);
	}
	if(doformat || override ){
		fprintf(stderr, "dinit: -g option invalid with -f or -o.\n");
		exit(2);
	}

	/* This option is not intended to work with strange format utilities.*/
	if (strchr(formatter, '#') != NULL)
	{
		fprintf(stderr, "dinit:  No track map support for %s.\n",
			formatter);
		exit(2);
	}	

	readhead(bdev);		/* read current vid/config structs and
				   bad track table. */

	readlist(&bads, tracklist,"bad track numbers", 3 * maxbad,
		 badform);

	if(bads.count == 0){
		fprintf(stderr,"The file %s contained no bad track entries.\n", tracklist);
		exit(0);
	}

	if (! readonly)
		readlist(&alts, altlist,"alternate track numbers", maxbad,
		 DO_TRACKS);

	/* sort and remove duplicate entries in bad track list */
	/* This list is expected to contain some new bad track entries
	   in addition to the original bad track entries. The list may
	   also only contain new bad track entries. */
	qsort(bads.value, bads.count, sizeof(int), comp);
	for (i = 1, p = bads.value + 1, q = bads.value; i < bads.count; i++, p++)
		if (*p != *q)
			*++q = *p;
	bads.count = j = (q - bads.value) + 1;

	/* sort alternate track list */
	if (! readonly)
		qsort(alts.value, alts.count, sizeof(int), comp);

	/* compare bad track list against bad track table to determine 
	   which entries in the bad track list are new bad track entries
	   which need to be mapped to available alternate tracks. */ 
	for(newbads.count = 0, i = 0; i < bads.count; i++){
		found = 0;
		for(j = 0; j < tracks.count; j++){
			if((bads.value[i] == tracks.value[j])
			||(bads.value[i] == tmpalts.value[j])){
				found = 1;
				break;
			}
		}
		if(!found){
			if(bads.value[i] >= (c.ios_hds * c.ios_trk)){
				fprintf(stderr,"dinit: Track number in %s exceeds maximum range.\n", tracklist);
			}
			else
				newbads.value[newbads.count++] = bads.value[i];
		}
	}

	if(newbads.count){
		fprintf(stderr,"\nNew bad tracks to be mapped:\n");
		printtracks(&newbads);
		fprintf(stderr,"\n");
	}
	else
	{
		fprintf(stderr,"The file %s contained no new bad track entries.\n", tracklist);
		exit(0);
	}

	if (! readonly) {
		/* Build list of available alternate tracks */
		findalts(&newbads, &alts, &availalts);
	
		/* error and exit if not enough alternate tracks available */
		if( newbads.count >= availalts.count){
			fprintf(stderr, "dinit: not enough alternate tracks.\n");
			exit(2);
		}
				

		fid = open(bdev, O_WRONLY);

		/* append new bad tracks to "tracks" and insert  
	   	closest available alternate for each new bad track 
	   	in "tmpalts".
	   	Also perform hardware bad track mapping operations if
	   	applicable.  */

		appendbads(&newbads, &alts, &availalts);

		close( fid );
		free(bads.value);
		free(availalts.value);
		free(newbads.value);
		altno = tmpalts.value;	/* set this pointer up for "fillbads". */
		bttable = (struct badent *) malloc((tracks.count + 1) * 
			sizeof (struct badent));
	
		fillbads();		/* fill bad track table and hash table
				   	with proper values */
        	if(!readonly)    
        		setup(bdev, rdev);   	/* write bad track table to disk */
	
		free(bttable);
	
		/* If the bad track redirection is handled by the driver, then
	   	update the drivers bad track redirection table in memory  
	   	immediately. If the device is currently open and will never close
	   	(in the case of the root device) then this step has some real
	   	effect, otherwise there is no redirection table to update because
	   	it was discarded on the last close of the device.
		*/
		if(btstrat == SOFTWARE && ! readonly){
			fid = open(rdev, O_WRONLY);
			if(ioctl(fid, (int)CHGBTIOCTL, 0) == -1){
				if(errno == EINVAL)
					fprintf(stderr,"dinit: device driver is obsolete. It does not support ioctl command %d.\n", (int)CHGBTIOCTL);
	
				errno = 0; /* reset for next time errno is read */
				fprintf(stderr,"dinit: can not update the device drivers bad track table with the new mapping changes.\n");
			}
			close( fid );
		}
	}
}
	

/* Find available alternate tracks -
	   Eliminate all bad alternate tracks and previously mapped alternate
	   tracks from the list of all alternate tracks to derive a table of 
	   available alternate tracks ("availalts"). 
 */
findalts(newbads, alts, availalts)
register struct list *newbads;	/* new bad tracks */ 
register struct list *alts; 	/* all alternate tracks */
register struct list *availalts; /* available alternate tracks */
{
	int 	i, j, k, found;

	for(availalts->count = 0, i = 0; i < alts->count; i++){ 
		found = 0; 
		for(j = 0; j < tracks.count; j++){ 
			if(((tmpalts.value[j] == -1) 
			&&(alts->value[i] == tracks.value[j]))
			|| (alts->value[i] == tmpalts.value[j])){ 
				found = 1;
				break;
			}
		}
		for(k = 0; k < newbads->count; k++){
			if(alts->value[i] == newbads->value[k]){
				found = 1;
				break;		
			}
		}

		if(!found)
			availalts->value[availalts->count++] = alts->value[i];
	}
}

/* Append new bad tracks to bad track list -
	   Finds closest alternate track to the bad track number supplied
	   by the caller.
	   Zero the data on the closest alternate track.
	   Also enters bad track number and alternate track into the temporary
	   bad track lists ( "tracks" and "tmpalts" ) and sorts the lists.
*/
appendbads(newbads, alts, availalts) 
register struct list *newbads;	/* new bad tracks to append to "tracks" */
register struct list *alts;	/* entire list of alternate tracks */
register struct list *availalts; /* available alternate tracks */
{
	register i,j,temp;
	register diff;
	register bestdiff;
	register closest_alt;	/* closest alternate track to a bad track */
	register bestindex;
	char	altstring[10];	/* holds ascii conversion of alternate track */
	char    btstring[10];	/* holds ascii conversion of bad track */
	char command[80];
	unsigned char *trkbuffer;
	unsigned	trksize;	/* track size in bytes */
	int	fid;

	trksize = c.ios_psm * c.ios_spt;
	if((trkbuffer = (unsigned char *) calloc(1, trksize)) == NULL){
		fprintf(stderr,"dinit: cannot allocate track buffer.\n");
		exit(2);
	}

	/* Now map each bad track in "newbads" list to an alternate track. */
	for( i = 0; i < newbads->count; i++){

	
	   if(!foundtrk(alts, newbads->value[i])){
		/* The bad track is not an alternate track. Go ahead and
		   find the closest alternate. */
	   	for(bestdiff = HIGHNUMBER, j = 0; j < availalts->count; j++){
			if((diff = newbads->value[i] - availalts->value[j]) < 0) diff = -diff;
			if(diff < bestdiff){
				bestdiff = diff;
				bestindex = j;
			}
		}
		closest_alt = availalts->value[bestindex];

		/* remove the selected alternate from the available list */
		availalts->value[bestindex] = HIGHNUMBER;  

		/* clear the data on the alternate track */
		fid = open(rdev, O_WRONLY);
		lseek(fid, (long) (trksize * closest_alt), 0);
		if(write(fid, trkbuffer, trksize) != trksize)
			fprintf(stderr,"dinit: cannot clear data on track %d\n",closest_alt);
		close(fid);

  	        if(btstrat == HARDWARE){
			/* map the bad track to the alternate track */
			sprintf(btstring, "%d", newbads->value[i]);
			sprintf(altstring, "%d", closest_alt);
			strcpy(command, formatter);
			strcat(command, " -M ");
			strcat(command, btstring);
			strcat(command, " ");
			strcat(command, altstring);
			strcat(command, " ");
			strcat(command, rdev);
	  		if(!readonly)    
		  		system(command);   
	       }
	  }
	  else
		closest_alt = FLAG;  /* bad track is an alternate track*/
	
	  tracks.value[tracks.count] = newbads->value[i];
	  tmpalts.value[tracks.count++] = closest_alt;
       }	
	free(trkbuffer);

	/* Sort the newly appended list of bad tracks. */
	for( i = 0; i < tracks.count ; i++){
		for(j = tracks.count - 1; j > i; j--){
			if(tracks.value[j] < tracks.value[j-1]){
				temp = tracks.value[j-1];
				tracks.value[j-1] = tracks.value[j];
				tracks.value[j] = temp;
				temp = tmpalts.value[j-1];
				tmpalts.value[j-1] = tmpalts.value[j];
				tmpalts.value[j] = temp;
			}
		}
	}

}

foundtrk(trklist, trkno)
register struct list *trklist;	/* list of tracks */
register        trkno;		/* search key (track number)  */
{
	register i;

	for(i = 0; i < trklist->count; i++)
		if(trklist->value[i] == trkno)
			return(1);  /* track number found in list */

	return(0);	/* track number not found in list */
}

printtracks(trklist)
register struct	list	*trklist;	/* list of tracks to print */
{
	int	i, t;

	if(trklist->count > 0)
	{
		if (badform == DO_HDCYL) 
			fprintf (stderr, "\n<Head>\t<Cylinder>");

		fprintf (stderr, "\n");
		for (i = 0; i < trklist->count; i++) 
		{
			t = trklist->value[i];
			if (badform == DO_HDCYL) 
				printf ("%-6d\t%-6d", t%c.ios_hds, t/c.ios_hds); 
			else	/* -T */
				printf ("%d", t);

			printf ("\n");
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
	char *s;
	struct stat rstat, bstat;

	while ((c = getopt(argc, argv, "arRTcofed:b:t:D:g:")) !=EOF)
		switch(c)
			{
			case 'a':
				prtalts = YES;
				break;
			case 'r':
				readonly = YES;
				break;
			case 'T':
				badform = DO_TRACKS;
				break;
			case 'c':
				check = YES;
				break;
			case 'o':
				override = YES;
				break;
			case 'f':
				doformat = YES;
				break;
			case 'e':
				exor = YES;
				break;
			case 'd':
				desc = optarg;
				break;
			case 'b':
				bootloader = optarg;
				hasboot = YES;
				break;
			case 't':
				tracklist = optarg;
				break;
			case 'g':
				morebadtrks = YES;
				tracklist = optarg;
				break;
			case 'D':
#ifdef DEBUG
				diskdefs = optarg;
				break;
#endif
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

	if (stat(rdev = argv[optind], &rstat) == -1 || access(rdev, RW) == -1)
		{
		fprintf(stderr, "dinit: cannot open %s\n", rdev);
		exit(2);
		}
#ifdef DEBUG
	strcpy(bdev, rdev + 1);
#else
	if (FTYPE(rstat.st_mode) != CHAR)
		{
		fprintf(stderr, "dinit: must be raw device - %s\n", rdev);
		exit(2);
		}
	else if (strncmp(rdev, "/dev/r", 6) != 0)
		{
		fprintf(stderr, 
			"dinit: raw device must be \"/dev/r*\" - %s\n", rdev);
		exit(2);
		}
	strcpy(bdev, "/dev/");
	strcat(bdev, rdev+6);
	if (stat(bdev, &bstat) == -1 || access(bdev, RW) == -1)
		{
		fprintf(stderr, "dinit: cannot open block device - %s\n", bdev);
		exit(2);
		}
	if (FTYPE(bstat.st_mode) != BLOCK ||
		MINOR(rstat.st_rdev) != MINOR(bstat.st_rdev))
		{
		fprintf(stderr, "dinit: not corresponding block device - %s\n",
			bdev);
		exit(2);
		}
#endif

	strcat(diskalts, type);
	if (access(diskalts, RW) != -1)
		altlist = diskalts;
	}



/* read the diskdef information for the disk type */

readdefs(diskdefs, type)
	char *diskdefs;
	char *type;
	{
	FILE *deffile;

	if ((deffile = fopen(DISKDEFS, "r")) == NULL)
		{
		fprintf(stderr, "dinit: can't open file '%s'\n", DISKDEFS);
		exit(2);
		}
	if (!lookup(deffile, type))
		{
		fprintf(stderr, "dinit: disk type '%s' not found\n", type);
		exit(2);
		}
	fclose(deffile);
	}



/* scan the diskdef file for the disk type, remember its parameters */

lookup(file, type)
	FILE *file;
	char *type;
	{
	char line[LINELEN];
	char message = NO;
	int typeno, rfs, nls, pss, spt, cyl, heads, interl, precomp;
	int spiral, step, starthead, ecc, attm, attw;
	int gap1, gap2, gap3, gap4, eattw;
	int count;
	char dtype[20];
	char strat[20];
	char diag[20];

	while (fgets(line, LINELEN, file))
		{
		if (line[strlen(line)-1] != '\n')
			{
			if (!message && *line != COMMENT)
				{
				fprintf(stderr, 
				 "dinit: diskdef line too long\n");
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
			  dtype, &typeno, formatter, diag, strat, &maxbad, 
			  &rfs, &nls, &pss, &spt, &cyl, &heads, &precomp, 
			  &interl, &spiral, &step, &starthead, &ecc,
			  &attm, &attw, &slice0, 
			  &gap1, &gap2, &gap3, &gap4, &eattw );
			if (strcmp(dtype, type) == 0)
				{
				if (count < 20)
					{
					fprintf(stderr, 
					 "dinit: diskdef line too short\n");
					continue;
					}
				v.vid_typ = typeno;
				c.ios_rsz = nls;
				c.ios_spt = spt;
				c.ios_hds = heads;
				c.ios_trk = cyl;
				c.ios_ilv = interl;
				c.ios_psm = pss;
				c.ios_pcom = precomp;
				c.ios_sof = spiral;
				c.ios_sr = step;
				c.ios_shn = starthead;
				c.ios_ecc = ecc;
				c.ios_atm = attm;
				c.ios_atw = attw;
				v.vid_rfs = rfs;
				c.ios_rec = LSS;
				hasdiag = (*diag=='y' || *diag=='Y');
				if ( count > 20 ) mkfs = (slice0 != 0);
				
				/* Added for the m360 driver */
				if ( count == 26 ) {
				   c.ios_gpb1 = gap1;
				   c.ios_gpb2 = gap2;
				   c.ios_gpb3 = gap3;
				   c.ios_gpb4 = gap4;
				   c.ios_eatw = eattw;
				/*
				** Following needed by bootloader to allow
				** boot directly from SMD
				*/
				   c.ios_f1[0] = 0xff;  /* MCS  sbw - 4/24/86 */
				   c.ios_f1[1] = 0xff;  /* MCS  sbw - 4/24/86 */
				}

				switch (*strat)
					{
					case 'p':
					case 'P':
						btstrat = PERFECT;
						break;
					case 'h':
					case 'H':
						btstrat = HARDWARE;
						break;
					case 's':
					case 'S':
						btstrat = SOFTWARE;
						break;
					}
				return(1);
				}
			}
		}
	return(0);
	}

/* read in the old bad track table after validating volume ID & config block */

readhead(dev)
	char *dev;
	{
	struct volumeid dv;
	struct dkconfig dc;
	struct badent *badlist;
	int fid;

	fid = open(dev, O_RDONLY);
	if (read(fid, &dv, c.ios_rec) == -1 || read(fid, &dc, c.ios_rec) == -1)
		{
		fprintf(stderr, "dinit: cannot init unformatted disk\n");
		exit(2);
		}
	if (override)
		{
		close(fid);
		return;
		}
	/* SMD disks formatted before last change to default dkconfig may
	** fail memcmp test, unless we fix 'em up.  c.ios_f1[01] is only
	** used by 360 formatter.  See lookup().
	*/
	if (c.ios_f1[0] == 0xff) dc.ios_f1[0] = 0xff;
	if (c.ios_f1[1] == 0xff) dc.ios_f1[1] = 0xff;

	if ((dv.vid_vol || dv.vid_typ != v.vid_typ 
		|| memcmp(&dc, &c, sizeof(struct dkconfig)) != 0)
		&& !readonly)
		{
		fprintf(stderr, "dinit: attempt to change disk parameters:\n");
		fprintf(stderr, "\tdiskdefs may have changed since last dinit\n");
		fprintf(stderr, "\tProbably should use '-o'\n");
		exit(2);
		}

	/* Copy current contents of vid to our vid template */

	memcpy (&v, &dv, sizeof(struct volumeid));

	if (btstrat != PERFECT)
		{
		int i;
		register int t, a;
		int listlen = dv.vid_btl * sizeof(struct badent);

		badlist = (struct badent *) malloc(listlen);
		lseek(fid, 3 * c.ios_rec, 0);	/* skip hash */
		read(fid, badlist, listlen);
		for (i = 0; i < dv.vid_btl; i++)
			{
			if (badlist[i].b_bad != FLAG) 
				{
				tracks.value[tracks.count] = badlist[i].b_bad;
				tmpalts.value[tracks.count++]= badlist[i].b_alt;
				}
			}
		free(badlist);

		/* print bad tracks and (if requested) what each is
		** mapped to.  Only action, if -r specified.
		** If track is in alt area, it has no alt.  Print nothing.
		** MMD - wjc - 09/06/86
		*/
		if( ! morebadtrks)	/* set only for -g option  */
		{
		if (tracks.count > 0)
		    {
		    fprintf (stderr, "Current bad list from disk:");
		    if (badform == DO_HDCYL) 
		        {
			fprintf (stderr, "\n<Head>\t<Cylinder>");
			if (prtalts)  fprintf (stderr, "\t<Mapped to:>");
			}
		    fprintf (stderr, "\n");
		    for (i = 0; i < tracks.count; i++) 
			{
			t = tracks.value[i];
			a = tmpalts.value[i];
			if (a == FLAG) /* tracks.value[i] is in alt area */
			    a = 0;     /* assumes 0 is never a valid alt */
			if (badform == DO_HDCYL) 
			    {
			    printf ("%-6d\t%-6d", t%c.ios_hds, t/c.ios_hds); 
			    if (prtalts && a)   printf ("\t==>\t%-6d\t%-6d",
						    a%c.ios_hds, a/c.ios_hds);
			    }
			else	/* -T */
			    {
			    printf ("%d", t);
			    if (prtalts && a)  printf ("\t==>\t%d", a);
			    }
			printf ("\n");
			}
		    }
		}
		}
	close(fid);
	}

/* check the device for bad tracks */

checkdisk(dev)
	char *dev;
	{
	int fid;
	char *buffer;
	int ntracks = c.ios_hds * c.ios_trk;
	int tracksz = c.ios_psm * c.ios_spt;
	register int i;
	register int tc = tracks.count;
	register int *tv = tracks.value;
	register int count = 0;

	if (tracksz % BLOCKSZ)
		{
		fprintf(stderr, 
		  "dinit: cannot check - track not even block multiple\n");
		return;
		}
	buffer = malloc(tracksz);	/* one track */
	if (buffer == NULL)
		{
		fprintf(stderr, "dinit: cannot allocate track buffer\n");
		exit(2);
		}
	fid = open(dev, O_RDWR);
	for (i = 0; i < ntracks; i++)
		{
		lseek(fid, i * tracksz, 0);
		if (read(fid, buffer, tracksz) != tracksz)
			{
			if (count > maxbad)
				{
				fprintf(stderr, 
				   "dinit: too many bad tracks\n");
				exit(2);
				}
			tv[tc++] = i;
			count++;
			}
		else 
			{
			lseek(fid, i * tracksz, 0);
			if (write(fid, buffer, tracksz) != tracksz)
				{
				if (count >= maxbad)
					{
					fprintf(stderr, 
					   "dinit: too many bad tracks\n");
					exit(2);
					}
				tv[tc++] = i;
				count++;
				}
			}
		}
	close(fid);
	free(buffer);
	tracks.count = tc;
	if (btstrat == HARDWARE && count > 0)
		fprintf(stderr, "WARNING: additional bad tracks found\n");
	if (count > 0)  {
	    fprintf (stderr, "Found these additional bad tracks:");
	    if (badform == DO_HDCYL)
		fprintf (stderr, "\n<Head>\t<Cylinder>");
	    fprintf (stderr, "\n");
	    for (; count ; count--) {
		i = tv[tc-count];
		if (badform == DO_HDCYL)
		    printf ("%-6d\t%-6d\n", i%c.ios_hds, i/c.ios_hds); 
		else
		    printf ("%d\n", i);
	    }
	}
	}

/* read a bad track or alternate list */

readlist(l, file, prompt, max, format)
	struct list *l;
	char *file;
	char *prompt;
	int max, format;
	{
	FILE *infile;
	int data;
	register int i;
	int head, cyl;

	infile = stdin;
	if (file && (infile = fopen(file, "r")) == NULL)
		{
		fprintf(stderr, "dinit: can't open file '%s'\n", file);
		exit(2);
		}
	if (infile == stdin && isatty(0))  {
		printf("Enter %s", prompt);
		if (format == DO_HDCYL)
		    printf (" as   <Head> <Cylinder>  ");
		printf ("; end with a period\n");
	}
	if (format == DO_TRACKS)  {
	    while (fscanf(infile, " %d", &data) > 0 && l->count < max)
		l->value[l->count++] = data;
	} else {
	    while (((i = fscanf(infile, " %d%d", &head, &cyl)) > 1)  &&
		    l->count < max)  {
		l->value[l->count++] = (c.ios_hds * cyl) + head;
	    }
	}
	    
	fscanf(infile, " .");
	if (infile != stdin)
		fclose(infile);
	}

/* initialize diagnostic test area directory */

initdiag()
	{
	int lss = c.ios_rec;
	int tracksz = c.ios_psm * c.ios_spt;
	int cylsz = tracksz * c.ios_hds;
	int i, len, loc;
	register struct diagent *p;

	loc = lss * 2;
	if (btstrat != PERFECT)
		{
		loc += lss;
		loc += (maxbad*sizeof(struct badent) + lss - 1) / lss * lss;
		}
	diaglo = (loc + lss + MAXBOOT + (cylsz - 1)) / cylsz * c.ios_hds;
	diaghi = (c.ios_trk - 1) * c.ios_hds;
	diagdir = (struct diagent *) malloc((c.ios_hds + 1) * 
		sizeof(struct diagent));
	if (diagdir == NULL)
		{
		fprintf(stderr, "dinit: cannot allocate diagnostic dir.\n");
		exit(2);
		}
	len = tracksz / lss;
	
	for (i = 0, p = diagdir; i < c.ios_hds; i++, p++)
		{
		p->d_track1 = diaglo + i;
		p->d_track2 = diaghi + i;
		p->d_len1 = p->d_len2 = len;
		}
	p->d_track1 = p->d_track2 = p->d_len1 = p->d_len2 = 0;
	}

/* assign alternate tracks to the bad tracks */

assignalt()
	{
	int comp(), comp1();
	register int *p, *q;
	struct opt {		/* optimization structure (for heuristic) */
		int trackno;
		int dist;
		} *optarray;
	register int *av = alts.value;
	register int *tv = tracks.value;
	register int i, j, k;
	register int track;
	register int d, distance, pos;

	/* sort and remove any duplicates */

	qsort(tv, tracks.count, sizeof(int), comp);
	qsort(av, alts.count, sizeof(int), comp);


	if (tracks.count == 0)
		{
		v.vid_btl = 0;
		return;
		}

	for (i = 1, p = tv + 1, q = tv; i < tracks.count; i++, p++)
		if (*p != *q)
			*++q = *p;
	tracks.count = j = q - tv + 1;
	tv = tracks.value = (int *) realloc(tv, (j + 1) * sizeof(int));

	altno = (int *) malloc((tracks.count + 1) * sizeof(int));
	bttable = (struct badent *) malloc((tracks.count + 1) * 
		sizeof (struct badent));
	for (i = 0; i < tracks.count; i++)
		altno[i] = -1;

	if (tracks.count > alts.count)
		{
		fprintf(stderr, "dinit: not enough alternate tracks\n");
		exit(2);
		}

	/* compute hueristic for each track: distance to closest alternate */

	optarray = (struct opt *) malloc(j * sizeof(struct opt));
	for (i = 0; i < j; i++)
		{
		optarray[i].trackno = i;
		track = tv[i];
		distance = BIG;
		for (k = 0; k < alts.count; k++)
			{
			if ((d = track - av[k]) < 0)
				d = -d;
			if (d < distance)
				distance = d;
			else	/* distance increasing, can quit */
				break;
			}
		optarray[i].dist = distance;
		} 
	qsort(optarray, tracks.count, sizeof(struct opt), comp1);

	/* remove any bad alternate tracks */

	for (i = 0, j = 0; i < tracks.count && j < alts.count; )
		{
		if ((k = tv[i] - av[j]) < 0)
			i++;
		else if (k > 0)
			j++;
		else		/* a bad one */
			{
			av[j] = -1;	/* Remove it from alternate list */
			altno[i] = FLAG; /* preallocate its alt to FLAG */
			i++;
			j++;
			}
		}

	/* check for bad diagnostic tracks */

	if (hasdiag)
		{
		int tpc = c.ios_hds;
		int head;

		for (i = 0; i < tracks.count; i++)
			if (((track=tv[i]) >= diaglo && track < diaglo + tpc) ||
				(track >= diaghi && track < diaghi + tpc))
				{
				pos = distance = BIG;
				head = track % tpc;
				for (k = 0; k < alts.count; k++)
					{
					if (av[k] < 0) 
						continue;
					if (pos == BIG)
						pos = k;
					if ((d = track - av[k]) < 0)
						d = -d;
					if (d < distance && av[k] % tpc == head)
						{
						distance = d;
						pos = k;
						}
					}
				altno[i] = av[pos];
				av[pos] = -1;
				for (k = 0; k < tpc; k++)
					if (diagdir[k].d_track1 == track)
						diagdir[k].d_track1 = altno[i];
					else if (diagdir[k].d_track2 == track)
						diagdir[k].d_track2 = altno[i];
				}
		}

	/* allocate in heuristic order the closest available alternate */

	for (i = 0; i < tracks.count; i++)
		{
		j = optarray[i].trackno;
		if (altno[j] != -1)
			continue;	/* already handled */
		track = tv[j];
		distance = BIG;
		for (k = 0; k < alts.count; k++)
			{
			if (av[k] < 0) 
				continue;
			if ((d = track - av[k]) < 0)
				d = -d;
			if (d < distance)
				{
				distance = d;
				pos = k;
				}
			else
				break;
			}
		altno[j] = av[pos];
		av[pos] = -1;
		}
	free(optarray);
	fillbads();
	}

/* fill the bad track table and hash table with the proper values */

fillbads()
	{

	register int k;
	register int *tv = tracks.value;

	/* fill dummy entries */
	tv[tracks.count] = FLAG;
	altno[tracks.count] = tv[tracks.count];

	/* for HARDWARE handling simply write a list (not a tree) and return */
	if ( btstrat == HARDWARE )  {
	   for ( k = 0 ;  k <= tracks.count ;  k++ )  {
	       bttable[k].b_bad = tracks.value[k];
	       bttable[k].b_alt = altno[k];
           }
           return;
        }

	/*  For Software handling continue building tree and hash table */
	++tracks.count;
	mkhash(hash, tv, tracks.count - 1);

	for (k = 1; k <= tracks.count; k*=2) /* find order of problem */
		;
	process(0, k, tracks.count, 1);
	}

/* fill the hash table based on the bad track information */

mkhash(htab, badtrks, badcnt)
	char	htab[HASHSIZE];
	int *badtrks;
	int badcnt;
	{
	register i;
	register l;

	/* fill it with non-zero */
	for (i=0; i<HASHSIZE; htab[i++] = 255)
		;

	/* if no bad tracks then done */
	if (badcnt == 0)
		return;

	/* fill in the bad tracks */
	for (i = 0; i < badcnt; i++, badtrks++) 
		htab[*badtrks & 255] = 0;

	/* find last bad track */
	for (l = HASHSIZE-1; htab[l]; l--)
		;

	/* fill in entries from "l" down to 0 */
	for (i = l-1; i >= 0; i--)
		if (htab[i])
			htab[i] = htab[i + 1] + 1;

	/* wrap around */
	if (htab[HASHSIZE - 1])
		htab[HASHSIZE - 1] = htab[0] + 1;

	/* fill in entries from HASHSIZE-1 down to "l" */
	for (i = HASHSIZE - 2; i > l; i--)
		if (htab[i])
			htab[i] = htab[i + 1] + 1;

	/* Voila */
	}

/* create the packed tree for the bad track table */

process(base, step, size, loc)
	int base, step, size, loc;
	{
	int mid, midplus;

	if (size == 0)
		return;
	mid = step/2-1;		/* mid is middle for full balanced tree */
	midplus = 3*step/4-1;	/* midplus is 3/4 full, middle doesn't change */
	if (size<midplus)	/* if less than 3/4, adjust mid */
		mid -= midplus - size;
	bttable[loc-1].b_bad = tracks.value[base + mid];
	bttable[loc-1].b_alt = altno[base + mid];
	step /= 2;
	process(base, step, mid, loc * 2);
	if (step/2 > size-mid-1)
		step /= 2;
	process(base+mid+1, step, size-mid-1, loc*2 + 1);
	}

/* (finally) write it all to the disk */

setup(dev, rdev)
	char *dev;
	char *rdev;
	{
	struct volumeid dv;
	struct dkconfig dc;
	struct badent *badlist;
	int fid;

	fid = open(dev, O_WRONLY);

	if(!morebadtrks){ /*skip this if only mapping more bad tracks */
		if (hasboot)
			copyboot(fid);
		if (hasdiag)
			copydiag(fid, rdev);
	}
	/* 
	 * For Hardware/Software, write only if changing the bad track table.
	 * (Used to always write if btstrat == SOFTWARE.  This is dangerous...
	 * MMD - wjc - 09/06/86)
	 */
	if (btstrat != PERFECT && (doformat || override || morebadtrks) )
		{
		if (tracks.count)
			{
			lseek(fid, 2 * c.ios_rec, 0);
			write(fid, hash, 256);
			lseek(fid, 3 * c.ios_rec, 0);
			write(fid, bttable, tracks.count * 
				sizeof(struct badent));
			}
		v.vid_btl = tracks.count;
		}

	/* set flag for bootloader to indicate software bad track handling */
	if (btstrat == SOFTWARE)
		v.vid_strat = -1;
	if (btstrat == HARDWARE)
		v.vid_strat = 1;

	/* now put out the real header */
	/* 
	 * Be careful that this is not overriding 
	 * something when its not suppose to 
	 */
	lseek(fid, 0L, 0);
	write(fid, &v, sizeof(struct volumeid));
	lseek(fid, c.ios_rec, 0);
	write(fid, &c, sizeof(struct dkconfig));
	close(fid);
	}

/* copy the bootloader program onto the disk */

copyboot(fid)
	int fid;
	{
	int loc;
	int lss = c.ios_rec;
	char buffer[1024];
	LDFILE *ldptr;
	SCNHDR text, data;
	register int l, s;
	unsigned int length_sec;

	loc = lss * 2;
	if (btstrat != PERFECT)
		{
		loc += lss;
		loc += (maxbad*sizeof(struct badent) + lss - 1)/lss * lss;
		}
	if (hasdiag)
		loc += lss;
	ldptr = NULL;
	if ((ldptr = ldopen(bootloader, ldptr)) == NULL)
		{
		fprintf(stderr, "dinit: can't open bootloader file '%s'\n",
			bootloader);
		return;
		}
	if (ldnshread(ldptr, ".text", &text) == FAILURE)
		{
		fprintf(stderr,"dinit: can't seek .text header on '%s'\n",
			bootloader);
		ldclose(ldptr);
		return;
		}
	ldnshread(ldptr, ".data", &data);

	if (data.s_vaddr + data.s_size - text.s_vaddr > MAXBOOT)
		{
		fprintf(stderr,"dinit: bootloader '%s' too big\n",
			bootloader);
		ldclose(ldptr);
		return;
		}

	v.vid_osa = v.vid_ose = text.s_vaddr;
	ldnsseek(ldptr, ".text");

	  /* make sure bootloader is placed on a physical sector boundary. */
	if(loc & (c.ios_psm -1))  
		loc = (loc + c.ios_psm) & ~(c.ios_psm -1);

	lseek(fid, loc, 0);
	 for (s = text.s_size; s; s -= l)
		{
		l = (s > 1024) ? 1024 : s;
		FREAD(buffer, l, 1, ldptr);
		write(fid, buffer, l);
		}
	ldnsseek(ldptr, ".data");
	lseek(fid, loc + data.s_vaddr - text.s_vaddr, 0);
	for (s = data.s_size; s; s -= l)
		{
		l = (s > 1024) ? 1024 : s;
		FREAD(buffer, l, 1, ldptr);
		write(fid, buffer, l);
		}
	ldclose(ldptr);
	v.vid_oss = loc / lss;

	/* Make sure bootloader length (specified in logical sectors) translates
	   to an exact number of physical sectors.
	   Otherwise, the rom-based "130BUG" will attempt to read a partial
	   physical sector resulting in an error from the controller. 
	*/
	length_sec = (data.s_vaddr + data.s_size - text.s_vaddr + 
		c.ios_psm - 1) / c.ios_psm;  /* round to phys sectors */
	v.vid_osl = length_sec * c.ios_psm/lss; /* logical sectors */
	}

/* copy the diagnostic tracks to the disk */

copydiag(fid, rdev)
	int fid;
	char *rdev;
	{
	int loc;
	int lss = c.ios_rec;
	int tracksz = c.ios_psm * c.ios_spt;
	char *buffer;
	register struct diagent *p;
	int lspt;
	register int i;
	int rfid;

	loc = lss * 2;
	if (btstrat != PERFECT)
		{
		loc += lss;
		loc += (maxbad*sizeof(struct badent) + lss - 1)/lss * lss;
		}

	/* switch from track # to logical sector number */

	lspt = tracksz / lss;
	for (i = 0, p = diagdir; i < c.ios_hds; i++, p++)
		{
		p->d_track1 *= lspt;
		p->d_track2 *= lspt;
		}

	lseek(fid, loc, 0);
	write(fid, diagdir, (c.ios_hds + 1) * sizeof(struct diagent));
	v.vid_dta = loc / lss;

	buffer = malloc(tracksz);
	if (buffer == NULL)
		{
		fprintf(stderr, "dinit: cannot allocate track buffer\n");
		exit(2);
		}

	diagtrk(buffer, tracksz);

	/* if tracks are on block boundaries do raw I/O for speed */

	if (tracksz % BLOCKSZ)
		rfid = fid;
	else
		rfid = open(rdev, O_WRONLY);
	for (i = 0, p = diagdir; i < c.ios_hds; i++, p++)
		{
		lseek(rfid,  p->d_track1 * lss, 0);
		write(rfid, buffer, tracksz);
		lseek(rfid,  p->d_track2 * lss, 0);
		write(rfid, buffer, tracksz);
		}
	if (rfid != fid)
		close(rfid);
	free(buffer);
	}

/* compare two ints */

comp(p1, p2)
	int *p1, *p2;
	{
	return (*p1 - *p2);
	}

/* compare two ints (one past the pointers) */

comp1(p1, p2)
	int *p1, *p2;
	{
	return (p1[1] - p2[1]);
	}

/* generate diagnostic disk track pattern (based on VERSAdos assembly
   routine RANDOM in INIT utility but is hopefully machine independent now) */

static unsigned int seed = 0x00092855;

diagtrk(buffer, tracksize)
	register int tracksize;
	register char *buffer;
	{
	register int i;
	int r;

	for (i=0; i < tracksize - 3; i += 4)
		{
		r = random();
		*buffer++ = r >> 24;
		*buffer++ = r >> 16;
		*buffer++ = r >> 8;
		*buffer++ = r;
		}
	}

/* a (not very) random function for producing diagnostic data */

static int random()
	{
	register int xbit;

	if (seed == 0)
		seed = 0xff;
	xbit = ((((seed&0xff) >> 3) ^ (seed&0xff)) >> 1) & 1;
	seed = (seed >> 1) | xbit << 31;
	if (seed == 0xffffffff)
		seed++;
	return (seed);
	}
