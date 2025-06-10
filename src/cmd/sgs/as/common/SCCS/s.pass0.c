h23725
s 00007/00007/00595
d D 1.6 86/09/16 15:28:32 fnf 6 5
c Changes to make ident directive work.
e
s 00008/00030/00594
d D 1.5 86/09/15 14:01:33 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00624
d D 1.4 86/09/11 13:02:19 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00624
d D 1.3 86/08/18 08:48:17 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00075/00023/00549
d D 1.2 86/08/07 08:03:20 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00572/00000/00000
d D 1.1 86/08/04 09:38:04 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)pass0.c	6.1		*/

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <paths.h>
#include "systems.h"
#include "pass0.h"
#include "sgs.h"
I 2
#include "dbug.h"
#include "gendefs.h"
E 2

/*
 *
 *	"pass0.c" is a file containing the source for the setup program
 *	for the SGS Assemblers.  Routines in this file parse the
 *	arguments, determining the source and object file names, invent
 *	names for temporary files, and execute the first pass of the
 *	assembler.
 *
 *	This program can be invoked with the command:
 *
 *			as [flags] ifile [-o ofile]
 *
 *	where [flags] are optional user-specified flags,
 *	"ifile" is the name of the assembly languge source file
 *	and "ofile" is the name of a file where the object program is
 *	to be written.  If "ofile" is not specified on the command line,
 *	it is created from "ifile" by the following algorithm:
 *
 *	     1.	If the name "ifile" ends with the two characters ".s",
 *		these are replaced with the two characters ".o".
 *
 *	     2.	If the name "ifile" does not end with the two
 *		characters ".s" and is no more than 12 characters in
 *		length, the name "ofile" is created by appending the
 *		characters ".o" to the name "ifile".
 *
 *	     3.	If the name "ifile" does not end with the two
 *		characters ".s" and is greater than 12 characters in
 *		length, the name "ofile" is created by appending the
 *		characters ".o" to the first 12 characters of "ifile".
 *
 *	The global variables "name1" through "name6" are used to store
 *	the six temporary file names.  The variable "pid" is used to
 *	store a character version of the process ID number.  This is
 *	used to make temporary file names unique.
 *
 */

#define NO		0
#define YES		1
#define	MAXFLAGS	15 - 9	/* sizeof(xargp)-num of args required for pass1 */
#ifndef P_tmpdir
#define P_tmpdir	"/usr/tmp"
#endif

#if ONEPROC
short passnbr = 0;

extern short opt;

extern short
	tstlookup,
	Oflag,
	dlflag;

extern char file[];

extern char *filenames[];

#if M4ON
extern short rflag;
#endif

#if M32RSTFIX
extern short rstflag;	/* Don't generate workaround for RESTORE chip bug */
#endif

#endif

char	*infile;

char	outfile[80];

char	*tempnam();

char
	*name0,
	*name1,
	*name2,
	*name3,
	*name4,
	*name5,
D 6
#if MULTSECT
E 6
	*name6,
I 6
#if MULTSECT
E 6
	*name7,
	*name8,
	*name9,
	*name10,
	*name11,
D 2
	*name12;
E 2
I 2
	*name12,
D 6
	*name13,
D 5
	*name14,
	*name15,
	*name16,
	*name17,
	*name18,
	*name19,
	*name20;
E 5
I 5
	*name14;
E 5
E 2
#else
D 5
	*name6;
E 5
I 5
	*name6,
	*name13,
	*name14;
E 6
E 5
#endif
I 6
	*name13,
	*name14,
	*name15;
E 6

short	transvec = NO,
	argindex = 1,
	flagindex = 0;

#if !ONEPROC
static char
	nextpass[15];

static char
	teststr[4] = {'-','t','\0','\0'};

static char
	*xargp[15];
#endif

static char
	flagstr[8][3];

#if M4ON
static short
	macro = MACRO;
#endif

#if !ONEPROC
static short
	testas = -2;
#endif

extern char
	*strcpy(),
	*strcat(),
	*strncat();

#define max(A,B) (((A)<(B))?(B):(A))

extern char *malloc(), *getenv(), *mktemp();
extern int access();

static char *pcopy(), *seed="AAA";

/*
 *
 *	"tempnam" is the routine that allocates the temporary files needed
 *	by the assembler.  It searches directories for enough space for the
 *	files, then assigns a (guaranteed) unique filename using the prefix
 *	in the function call.
 *
 */

char *
tempnam(dir, pfx)
char *dir;		/* use this directory please (if non-NULL) */
char *pfx;		/* use this (if non-NULL) as filename prefix */
{
	register char *p, *q, *tdir;
	int x=0, y=0, z;

I 2
	DBUG_ENTER ("tempnam");
E 2
	z=strlen(P_tmpdir);
	if((tdir = getenv("TMPDIR")) != NULL) {
		x = strlen(tdir);
	}
	if(dir != NULL) {
		y=strlen(dir);
	}
	if((p=malloc((unsigned)(max(max(x,y),z)+16))) == NULL)
D 2
		return(NULL);
E 2
I 2
		DBUG_RETURN((char *)NULL);
E 2
	if(x > 0 && access(pcopy(p, tdir), 3) == 0)
		goto OK;
	if(y > 0 && access(pcopy(p, dir), 3) == 0)
		goto OK;
	if(access(pcopy(p, P_tmpdir), 3) == 0)
		goto OK;
	if(access(pcopy(p, "/tmp"), 3) != 0)
D 2
		return(NULL);
E 2
I 2
		DBUG_RETURN((char *)NULL);
E 2
OK:
	(void)strcat(p, "/");
	if(pfx) {
		*(p+strlen(p)+5) = '\0';
		(void)strncat(p, pfx, 5);
	}
	(void)strcat(p, seed);
	(void)strcat(p, "XXXXXX");
	q = seed;
	while(*q == 'Z')
		*q++ = 'A';
	++*q;
	(void)mktemp(p);
	if (*p == '\0')
D 2
		return(NULL);
	return(p);
E 2
I 2
		DBUG_RETURN((char *)NULL);
	DBUG_RETURN(p);
E 2
}

static char*
pcopy(space, arg)
register char *space, *arg;
{
	register char *p;

I 2
	DBUG_ENTER ("pcopy");
E 2
	if(arg) {
		(void)strcpy(space, arg);
		p = space-1+strlen(space);
		if(*p == '/')
			*p = '\0';
	}
D 2
	return(space);
E 2
I 2
	DBUG_RETURN(space);
E 2
}

/*
 *
 *	"getargs" is a general purpose argument parsing routine.
 *	It locates flags (identified by a preceding minus sign ('-'))
 *	and initializes any associated flags for the assembler.
 *	If there are any file names in the argument list, then a
 *	pointer to the name is stored in a global variable for
 *	later use.
 *
 */

getargs(xargc,xargv)
	register int xargc;
	register char **xargv;
{
	register char ch;

I 2
	DBUG_ENTER ("getargs");
E 2
	while (xargc-- > 0) {
		if (**xargv == '-') {
			while ((ch = *++*xargv) != '\0')
				switch (ch) {
I 2
				case '#':
					if (*++*xargv == '\000') {
						xargv++;
						xargc--;
					}
					DBUG_PUSH (*xargv);
					while (*++*xargv != '\0') ;
					--*xargv;
					break;
E 2
#if M4ON
				case 'm':
					macro = ! macro;
					break;
#endif
				case 'o':
					strcpy(outfile,*(++xargv));
					--xargc;
					while (*++*xargv != '\0') ;
					--*xargv;
					break;
				case 'd':
					if (*++*xargv == 'l')
#if !ONEPROC
						xargp[argindex++] = "-dl";
#else
						dlflag = YES;
#endif
					break;
#if TRANVEC || !ONEPROC
				case 't':
					++*xargv;
#endif
#if TRANVEC
					if (**xargv == 'v'){
						transvec = YES;
#if !ONEPROC
						xargp[argindex++] = "-tv";
#endif
						break;
					}
#endif
#if !ONEPROC
					if (isdigit(**xargv)) {
						testas = **xargv - '0' - 1;
						if (testas > -1) {
							teststr[2] = (char)(testas + '0');
						}
					}
					else {
						--*xargv;
						testas += 2;
					}
					xargp[argindex++] = teststr;
					break;
#endif
				case 'V':
					/* version flag */
					fprintf(stderr,"%s: assembler - %s\n",
						SGSNAME,RELEASE);
					break;
#if ONEPROC || MC68
#if MC68
				case 'j':	/* m68k ljas: no optimization */
#endif
				case 'n':
					opt = NO;
					break;
#endif
#if ONEPROC
#if DEBUG
				case 'O':
					Oflag = YES;
					break;
				case 'T': {
					switch (*++*xargv) {
						case 'L' : {
							tstlookup = YES;
							break;
						}
					}
					break;
				}
#endif
#if M4ON
				case 'R':
					rflag = YES;
					break;
#endif
#if M32RSTFIX
				case 'r': /* Don't generate the workaround
					   * for the RESTORE chip bug. 
                                           */
					rstflag = NO;
					break;
#endif	/* M32RSTFIX */
#endif
#if VAX
				case 'j': /* invoke ljas instead of as */
					if (strcmp(xargv[0],"ljas")) {
						execvp("ljas",xargv);
						aerror("cannot exec ljas");
						};
					break;
#endif
				default:
#if ONEPROC
					flags(ch);
#else
					flagstr[flagindex][0] = '-';
					flagstr[flagindex][1] = ch;
					xargp[argindex++] = &(flagstr[flagindex++][0]);
#endif
					break;
				}
			xargv++;
		}
		else {
			infile = *xargv++;
		}
	}
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"main" is the main driver for the assembler. It calls "getargs"
 *	to parse the argument list, set flags, and stores pointers
 *	to any file names in the argument list .
 *	If the output file was not specified in the argument list
 *	then the output file name is generated. Next the temporary
 *	file names are generated and the first pass of the assembler
 *	is invoked.
 *
 */

main(argc,argv)
	int argc;
	char **argv;
{
	register short index, count;
#if MC68 || !ONEPROC
	register char *temp;
#endif
	FILE	*fd;

I 2
	DBUG_ENTER ("main");
E 2
	argc--;
	if (argc <= 0) {
		fprintf(stderr,"Usage: %sas [options] file\n",SGS);
		exit(1);
	}
#if MC68 || !ONEPROC
	temp = *argv;
	index = -1;
	while (temp[++index] != '\0') ;
	while (--index >= 0 && temp[index] != '/') ;
#if MC68	/* check if this is "ljas" */
	if (strcmp(&temp[++index],"ljas") == 0)
		opt = NO;
#else
	strcpy(nextpass,&temp[++index]);
	nextpass[strlen(nextpass)] = '1';
#endif
#endif
	argv++;
	getargs(argc, argv);
/*	Check to see if input file exits */
	if ((fd = fopen(infile,"r")) != NULL)
		fclose(fd);
	else {
		fprintf(stderr,"Nonexistent file\n");
		exit(1);
	}
	if (outfile[0] == '\0') {
		for(index=0,count=0;infile[index]!='\0';index++,count++)
			if(infile[index]=='/')
				count = -1;
		strcpy(outfile,infile+index-count);
		if(outfile[count-2]=='.' &&
		   outfile[count-1]=='s')
			outfile[count-1]='o';
		else
		{
			if(count>12)
				count = 12;
			strcpy(outfile+count,".o");
		}
	}

	name1 = tempnam(TMPDIR,TMPFILE1);
	name2 = tempnam(TMPDIR,TMPFILE2);
	name3 = tempnam(TMPDIR,TMPFILE3);
	name4 = tempnam(TMPDIR,TMPFILE4);
	name5 = tempnam(TMPDIR,TMPFILE5);
	name6 = tempnam(TMPDIR,TMPFILE6);
#if MULTSECT
	name7 = tempnam(TMPDIR,TMPFILE7);
	name8 = tempnam(TMPDIR,TMPFILE8);
	name9 = tempnam(TMPDIR,TMPFILE9);
	name10 = tempnam(TMPDIR,TMPFILEA);
	name11 = tempnam(TMPDIR,TMPFILEB);
	name12 = tempnam(TMPDIR,TMPFILEC);
I 5
#endif
E 5
I 2
	name13 = tempnam(TMPDIR,TMPFILED);
	name14 = tempnam(TMPDIR,TMPFILEE);
I 6
	name15 = tempnam(TMPDIR,TMPFILEF);
E 6
D 5
	name15 = tempnam(TMPDIR,TMPFILEF);
	name16 = tempnam(TMPDIR,TMPFILEG);
	name17 = tempnam(TMPDIR,TMPFILEH);
	name18 = tempnam(TMPDIR,TMPFILEI);
	name19 = tempnam(TMPDIR,TMPFILEJ);
	name20 = tempnam(TMPDIR,TMPFILEK);
E 2
#endif
E 5
#if !ONEPROC
	xargp[0] = nextpass;
#endif
#if M4ON
	if (macro) {
		/* tell pass1 to unlink its input when through */
#if ONEPROC
		rflag = YES;
#else	
		xargp[argindex++] = "-R";
#endif
		callm4();
	}
#endif

#if ONEPROC
	strcpy(file,infile);
D 2
	filenames[0] = infile;
	filenames[1] = outfile;
	filenames[2] = name1;
	filenames[3] = name2;
	filenames[4] = name3;
	filenames[5] = name4;
	filenames[6] = name5;
	filenames[7] = name6;
E 2
I 2
	filenames[FDIN] = infile;
	filenames[FDOUT] = outfile;
	filenames[FDTEXT] = name1;
	filenames[FDDATA] = name2;
	filenames[FDSTAB] = name3;
	filenames[FDLONG] = name4;
	filenames[FDSTRING] = name5;
	filenames[FDSYMB] = name6;
E 2
#if MULTSECT
D 2
	filenames[8] = name7;
	filenames[9] = name8;
	filenames[10] = name9;
	filenames[11] = name10;
	filenames[12] = name11;
	filenames[13] = name12;
E 2
I 2
	filenames[FDTXT1] = name7;
	filenames[FDTXT2] = name8;
	filenames[FDTXT3] = name9;
	filenames[FDDAT1] = name10;
	filenames[FDDAT2] = name11;
	filenames[FDDAT3] = name12;
D 5
	filenames[FDINIT1] = name13;
	filenames[FDINIT2] = name14;
	filenames[FDINIT3] = name15;
	filenames[FDLIB1] = name16;
	filenames[FDLIB2] = name17;
	filenames[FDLIB3] = name18;
	filenames[FDINIT] = name19;
	filenames[FDLIB] = name20;
E 5
E 2
#endif
I 5
	filenames[FDINIT] = name13;
	filenames[FDLIB] = name14;
I 6
	filenames[FDCMT] = name15;
E 6
E 5
	exit(aspass1());
#else
	if (argindex > MAXFLAGS) {
		fprintf(stderr,"Too many flags\n");
		exit (1);
	}

	xargp[argindex++] = infile;
	xargp[argindex++] = outfile;
	xargp[argindex++] = name1;
	xargp[argindex++] = name2;
	xargp[argindex++] = name3;
	xargp[argindex++] = name4;
	xargp[argindex++] = name5;
	xargp[argindex++] = name6;
#if MULTSECT
	xargp[argindex++] = name7;
	xargp[argindex++] = name8;
	xargp[argindex++] = name9;
	xargp[argindex++] = name10;
	xargp[argindex++] = name11;
	xargp[argindex++] = name12;
I 5
#endif
E 5
I 2
	xargp[argindex++] = name13;
	xargp[argindex++] = name14;
I 6
	xargp[argindex++] = name15;
E 6
D 5
	xargp[argindex++] = name15;
	xargp[argindex++] = name16;
	xargp[argindex++] = name17;
	xargp[argindex++] = name18;
	xargp[argindex++] = name19;
	xargp[argindex++] = name20;
E 2
#endif
E 5
	xargp[argindex] = 0;
	if (testas != -1) {
		if (testas > -1) {
			execv(NAS1,xargp);
		}
		else
			execv(AS1,xargp);
	fprintf(stderr,"Assembler Error - Cannot Exec Pass 1\n");
	exit(1);
	}
#endif
I 2
	DBUG_VOID_RETURN;
E 2
} /* main */

#if M4ON
callm4()
{
	static char
		*av[] = { "m4", 0, 0, 0};

	/*
	*	The following code had to be added with a 'u370' ifdef due
	*	to a MAXI bug concerning static pointers. We think it's fixed
	*	that is why the code is commented out. It can be deleted
	*	when this fact is verified.
	*
	*	char *regdef, *tvdef;
	*	regdef = CM4DEFS;
	*	tvdef = CM4TVDEFS;
	*/

#if !NODEFS
	static char
		*regdef = CM4DEFS
#if TRANVEC
		,
		*tvdef  = CM4TVDEFS
#endif
		;

I 2
	DBUG_ENTER ("callm4");
E 2
#if TRANVEC
	av[1] = (transvec) ? tvdef : regdef;
	av[2] = infile;
#else
	av[1] = regdef;
	av[2] = infile;
#endif
#else
I 2
	DBUG_ENTER ("callm4");
E 2
	av[1] = infile;
#endif
	name0 = tempnam(TMPDIR,TMPFILE0); 		/* m4 output file */
	if (callsys(M4, av, name0) != 0) {
		unlink(name0);
		fprintf(stderr,"Assembly inhibited\n");
		exit(100);
	}
	infile = name0;
D 2
	return;
E 2
I 2
	DBUG_VOID_RETURN;
E 2
} /* callm4 */

callsys(f,v,o)
	char f[], *v[];
	char *o;	/* file name, if any, for redirecting stdout */
{
	int t, status;

I 2
	DBUG_ENTER ("callsys");
E 2
	if ((t=fork())==0) {
		if (o != NULL) {	/* redirect stdout */
			if (freopen(o, "w", stdout) == NULL) {
				fprintf(stderr,"Can't open %s\n", o);
				exit(100);
			}
		}
		execv(f, v);
		fprintf(stderr,"Can't find %s\n", f);
		exit(100);
	} else
		if (t == -1) {
			fprintf(stderr,"Try again\n");
D 2
			return(100);
E 2
I 2
			DBUG_RETURN(100);
E 2
		}
	while(t!=wait(&status));
	if ((t=(status&0377)) != 0) {
		if (t!=SIGINT)		/* interrupt */
			{
			fprintf(stderr,"status %o\n",status);
			fprintf(stderr,"Fatal error in %s\n", f);
			}
		exit(100);
	}
D 2
	return((status>>8) & 0377);
E 2
I 2
	DBUG_RETURN((status>>8) & 0377);
E 2
} /* callsys */
#endif
E 1
