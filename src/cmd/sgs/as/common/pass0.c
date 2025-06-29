/*	@(#)pass0.c	6.1		*/

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <paths.h>
#include "systems.h"
#include "pass0.h"
#include "sgs.h"
#include "dbug.h"
#include "gendefs.h"

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
	*name6,
#if MULTSECT
	*name7,
	*name8,
	*name9,
	*name10,
	*name11,
	*name12,
#endif
	*name13,
	*name14,
	*name15;

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

	DBUG_ENTER ("tempnam");
	z=strlen(P_tmpdir);
	if((tdir = getenv("TMPDIR")) != NULL) {
		x = strlen(tdir);
	}
	if(dir != NULL) {
		y=strlen(dir);
	}
	if((p=malloc((unsigned)(max(max(x,y),z)+16))) == NULL)
		DBUG_RETURN((char *)NULL);
	if(x > 0 && access(pcopy(p, tdir), 3) == 0)
		goto OK;
	if(y > 0 && access(pcopy(p, dir), 3) == 0)
		goto OK;
	if(access(pcopy(p, P_tmpdir), 3) == 0)
		goto OK;
	if(access(pcopy(p, "/tmp"), 3) != 0)
		DBUG_RETURN((char *)NULL);
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
		DBUG_RETURN((char *)NULL);
	DBUG_RETURN(p);
}

static char*
pcopy(space, arg)
register char *space, *arg;
{
	register char *p;

	DBUG_ENTER ("pcopy");
	if(arg) {
		(void)strcpy(space, arg);
		p = space-1+strlen(space);
		if(*p == '/')
			*p = '\0';
	}
	DBUG_RETURN(space);
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

	DBUG_ENTER ("getargs");
	while (xargc-- > 0) {
		if (**xargv == '-') {
			while ((ch = *++*xargv) != '\0')
				switch (ch) {
				case '#':
					if (*++*xargv == '\000') {
						xargv++;
						xargc--;
					}
					DBUG_PUSH (*xargv);
					while (*++*xargv != '\0') ;
					--*xargv;
					break;
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
	DBUG_VOID_RETURN;
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

	DBUG_ENTER ("main");
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
#endif
	name13 = tempnam(TMPDIR,TMPFILED);
	name14 = tempnam(TMPDIR,TMPFILEE);
	name15 = tempnam(TMPDIR,TMPFILEF);
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
	filenames[FDIN] = infile;
	filenames[FDOUT] = outfile;
	filenames[FDTEXT] = name1;
	filenames[FDDATA] = name2;
	filenames[FDSTAB] = name3;
	filenames[FDLONG] = name4;
	filenames[FDSTRING] = name5;
	filenames[FDSYMB] = name6;
#if MULTSECT
	filenames[FDTXT1] = name7;
	filenames[FDTXT2] = name8;
	filenames[FDTXT3] = name9;
	filenames[FDDAT1] = name10;
	filenames[FDDAT2] = name11;
	filenames[FDDAT3] = name12;
#endif
	filenames[FDINIT] = name13;
	filenames[FDLIB] = name14;
	filenames[FDCMT] = name15;
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
#endif
	xargp[argindex++] = name13;
	xargp[argindex++] = name14;
	xargp[argindex++] = name15;
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
	DBUG_VOID_RETURN;
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

	DBUG_ENTER ("callm4");
#if TRANVEC
	av[1] = (transvec) ? tvdef : regdef;
	av[2] = infile;
#else
	av[1] = regdef;
	av[2] = infile;
#endif
#else
	DBUG_ENTER ("callm4");
	av[1] = infile;
#endif
	name0 = tempnam(TMPDIR,TMPFILE0); 		/* m4 output file */
	if (callsys(M4, av, name0) != 0) {
		unlink(name0);
		fprintf(stderr,"Assembly inhibited\n");
		exit(100);
	}
	infile = name0;
	DBUG_VOID_RETURN;
} /* callm4 */

callsys(f,v,o)
	char f[], *v[];
	char *o;	/* file name, if any, for redirecting stdout */
{
	int t, status;

	DBUG_ENTER ("callsys");
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
			DBUG_RETURN(100);
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
	DBUG_RETURN((status>>8) & 0377);
} /* callsys */
#endif
