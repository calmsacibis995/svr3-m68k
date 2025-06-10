h44348
s 00020/00000/00662
d D 1.7 86/09/16 15:28:38 fnf 7 6
c Changes to make ident directive work.
e
s 00005/00103/00657
d D 1.6 86/09/15 14:01:38 fnf 6 5
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00007/00006/00753
d D 1.5 86/09/11 13:02:22 fnf 5 4
c Finish work for shared libraries, checkpoint changes.
e
s 00001/00000/00758
d D 1.4 86/08/18 08:48:19 fnf 4 3
c Checkpoint changes at first working shared library version
e
s 00059/00013/00699
d D 1.3 86/08/07 08:03:25 fnf 3 2
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00115/00005/00597
d D 1.2 86/08/04 15:45:22 fnf 2 1
c Initial changes to support "init" and "lib" coff sections for
c shared libraries.
e
s 00602/00000/00000
d D 1.1 86/08/04 09:38:05 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)pass1.c	6.1		*/
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <paths.h>
#include "systems.h"
#include "symbols.h"
#include "gendefs.h"
I 3
#include "dbug.h"
E 3

/*
 *
 *	"pass1.c" is a file containing the main routine for the first
 *	pass of the assembler.  It is invoked with the command:
 *
 *		as1 [flags] ifile ofile t1 t2 t3 t4 t5 t6
 *
 *	where {flags] are optional flags passed from pass 0,
 *	"ifile" is the name of the assembly language input file,
 *	"t1" through "t6" are the names of temporary files to be used
 *	by the assembler, and "ofile" is the file where the object code
 *	is to be written.  Pass 1 of the assembler reads "ifile" and
 *	writes the temporary text section to "t1", the temporary data
 *	section to "t2", and the symbol table to "t3".
 *
 *	The following things are done by this function:
 *
 *	1. Initialization. This consists of calling signal to catch
 *	   interrupt signals for hang-up, break, and terminate. Then
 *	   the argument list is processed by "getargs" followed by the
 *	   initialization of the symbol table with mnemonics for
 *	   instructions and pseudos-ops.
 *
 *	2. "yyparse" is called to do the actual first pass processing.
 *	   This is followed by a call to "cgsect". Normally this
 *	   function is used to change the section into which code
 *	   is generated. In this case, it is only called to make
D 2
 *	   sure that "dottxt", "dotdat", and "dotbss" contain the
 *	   proper values for the program counters for the respective
 *	   sections. The following symbols are then defined:
E 2
I 2
 *	   sure that "dottxt", "dotdat", "dotinit", "dotlib" and "dotbss"
 *	   contain the proper values for the program counters for the 
 *	   respective sections. The following symbols are then defined:
E 2
 *
 *		.text	This has a type of text and a value of zero.
 *			It is used to label the beginning of the text
 *			section, and later as a reference for relocation
 *			entries that are relative to the text section.
 *
 *		.data	This has a type of data and a value of zero.
 *			It is used to label the beginning of the data
 *			section, and later as a reference for relocation
 *			entries that are relative to the data section.
 *
 *		.bss	This has a type of bss and a value of zero.  It
 *			is used to label the beginning of the bss
 *			section, and later as a reference for relocation
 *			entries that are relative to the bss section.
 *
 *		(text)	This is a totally internal symbol used to
 *			remember the size of the text section.  It has
 *			characters in it that cannot legally be used in
 *			a symbol, and hence cannot be referenced or
 *			redefined by a user.
 *
 *		(data)	This is an internal symbol used to remember the
 *			size of the data section.
 *
 *		(bss)	This is an internal symbol used to remember the
 *			size of the bss section.
 *
 *		(sdicnt) This is the internal symbol used to remember
 *			the number of span dependent instructions
 *			on which optimizations were performed.
 *
 *	     3. The function "dmpstb" is called to dump the symbol
 *		table out to a temporary file to be used by pass 2 of
 *		the assembler.
 *
 *	4. The temporary files are closed and the next pass (if any)
 *	   is called.
 *
 */

#if ONEPROC
extern short passnbr;
#endif

extern char file[];

extern char *filenames[];

extern unsigned short
	line,
	sdicnt;


#if DEBUG
extern unsigned
	numcalls,
	numids,
	numcoll;
#endif

extern short
	anyerrs;

extern int
	aerror(),
	delexit(),
#if !ONEPROC
	dmpstb(),
#endif
#if MC68
	ckalign(),	/* force even section alignment */
#endif
	fixsyms(),
	flags(),
	flushbuf(),
	onintr();

extern FILE
	*fderr;

#if !ONEPROC
extern FILE
	*fdstab;
#endif

extern upsymins
	*lookup();

I 6
extern long	dotinit;
extern long	dotlib;
I 7
extern long	dotcmt;
E 7

E 6
extern long
#if MULTSECT
	dottxt[4],
	dotdat[4],
I 2
D 6
	dotinit[4],
	dotlib[4],
E 6
E 2
#else
	dottxt,
	dotdat,
I 2
D 6
	dotinit,
	dotlib,
E 6
E 2
#endif
	dotbss;

#if !ONEPROC
char	*xargp[15];
#endif

short	opt = YES,
	Oflag = NO;

#if M4ON
extern short rflag;
#endif

#if M32RSTFIX
short rstflag = YES;
#endif	/* M32RSTFIX */
#if ONEPROC
extern short
	transvec;
#else
short
	transvec = NO,
	argindex = 1;
#endif

#if ONEPROC
extern long newdot;
extern symbol *dot;
#else
long	newdot;
symbol	*dot;
#endif

FILE	*fdin,
#if !ONEPROC
	*fdstring,
	*fdlong,
#endif
	*fdtext,
I 3
	*fdinit,
	*fdlib,
I 7
	*fdcmt,
E 7
E 3
#if MULTSECT
	*fdtxt1,
	*fdtxt2,
	*fdtxt3,
	*fddat1,
	*fddat2,
	*fddat3,
I 3
D 6
	*fdinit1,
	*fdinit2,
	*fdinit3,
	*fdlib1,
	*fdlib2,
	*fdlib3,
E 6
E 3
#endif
#if DEBUG
	*perfile,	/* performance data file descriptor */
#endif
	*fddata;

#if MULTSECT
add1text(ptr)
symbol	*ptr;
{
	addsect(ptr,TXT,1);
}	/* add1text()	*/


add2text(ptr)
symbol	*ptr;
{
	addsect(ptr,TXT,2);
}	/* add2text()	*/


add3text(ptr)
symbol	*ptr;
{
	addsect(ptr,TXT,3);
}	/* add3text()	*/


add1data(ptr)
symbol	*ptr;
{
	addsect(ptr,DAT,1);
}	/* add1data()	*/


add2data(ptr)
symbol	*ptr;
{
	addsect(ptr,DAT,2);
}	/* add2data()	*/


add3data(ptr)
symbol	*ptr;
{
	addsect(ptr,DAT,3);
}	/* add3data()	*/

I 2
D 6
add1init(ptr)
symbol	*ptr;
{
	addsect(ptr,INIT,1);
}	/* add1init()	*/


add2init(ptr)
symbol	*ptr;
{
	addsect(ptr,INIT,2);
}	/* add2init()	*/


add3init(ptr)
symbol	*ptr;
{
	addsect(ptr,INIT,3);
}	/* add3init()	*/

add1lib(ptr)
symbol	*ptr;
{
	addsect(ptr,LIB,1);
}	/* add1lib()	*/


add2lib(ptr)
symbol	*ptr;
{
	addsect(ptr,LIB,2);
}	/* add2lib()	*/


add3lib(ptr)
symbol	*ptr;
{
	addsect(ptr,LIB,3);
}	/* add3lib()	*/

E 6
E 2
addsect(ptr,sectclass,sectnum)
register symbol	*ptr;
short	sectclass,
	sectnum;
{
I 3
	DBUG_ENTER ("addsect");
E 3
D 2
	if (((ptr->styp & TYPE) == sectclass) && (ptr->sectnum == sectnum))
		ptr->value += (sectclass == TXT) ? dottxt[0] : dotdat[0];
E 2
I 2
	if (((ptr->styp & TYPE) == sectclass) && (ptr->sectnum == sectnum)) {
		switch (sectclass) {
			case TXT:
		    		ptr->value += dottxt[0];
				break;
			case DAT:
				ptr->value += dotdat[0];
				break;
			case INIT:
D 6
				ptr->value += dotinit[0];
E 6
I 6
				ptr->value += dotinit;
E 6
				break;
			case LIB:
D 6
				ptr->value += dotlib[0];
E 6
I 6
				ptr->value += dotlib;
E 6
				break;
I 7
			case CMT:
				ptr->value += dotcmt;
				break;
E 7
		}
	}
I 3
	DBUG_VOID_RETURN;
E 3
E 2
}	/* addsect()	*/

#endif

#if !ONEPROC
static char
	nextpass[80];

static char
	teststr[4] = {'-','t','\0'};
#endif

short tstlookup = NO;

static short
#if !ONEPROC
	filecnt = 0,
#endif
	testas = TESTVAL;

#if DEBUG
/*
 *	Performance data structure
 */
	long	ttime;
	struct	tbuffer {
		long	proc_user_time;
		long	proc_system_time;
		long	child_user_time;
		long	child_system_time;
		} ptimes;
	extern	long	times();

#endif
#if !ONEPROC
/*
 *
 *	"getargs" is a general purpose argument parsing routine.
 *	It locates flags (identified by a preceding minus sign ('-'))
 *	and initializes any associated flags for the assembler.
 *	If there are any file names in the argument list, then a
 *	pointer to the name is stored in the array "filenames" for
 *	later use.
 *
 */

getargs(xargc,xargv)
	register int xargc;
	register char **xargv;
{
	register char ch;
	
I 3
	DBUG_ENTER ("getargs");
E 3
	while (xargc-- > 0) {
		if (**xargv == '-') {
			while ((ch = *++*xargv) != '\0')
				switch (ch) {
					case 'n':
						opt = NO;
						break;
#if DEBUG
					case 'O':
						Oflag = YES;
						break;
#endif
#if M4ON
					case 'R':
						rflag = YES;
						xargp[argindex++] = "-R";
						break;
#endif
					case 'd':
						if (*++*xargv == 'l')
						      xargp[argindex++] = "-dl";
						break;
					case 't': {
						++*xargv;
#if TRANVEC
						if (**xargv == 'v'){
							transvec = YES;
							xargp[argindex++]="-tv";
							break;
						}
#endif
						if (isdigit(**xargv)) {
							testas = **xargv - '0' -1;
							if (testas > TESTVAL + 1) {
								teststr[2] = (char)(testas + '0');
							}
						}
						else {
							--*xargv;
							testas += 2;
						}
						xargp[argindex++] = teststr;
						break;
					}
#if DEBUG
					case 'T': {
						switch (*++*xargv) {
							case 'L': {
								tstlookup = YES;
								break;
							}
						}
						break;
					}
#endif
#if M32RSTFIX
					case 'r':
						rstflag = NO;
						break;
#endif	/* M32RSTFIX */
					default: {
						/* installation dependent flag? */
						flags(ch);
						break;
					}
				}
			xargv++;
		}
		else {
			filenames[filecnt++] = *xargv++;
		}
	}
I 3
	DBUG_VOID_RETURN;
E 3
}

main(argc,argv)
	int argc;
	char **argv;
#else

aspass1()

#endif
{
	register short i;
	register symbol *ptr;
	
I 3
	DBUG_ENTER ("aspass1");
E 3
#if ONEPROC
	passnbr = 1;
#endif
	if (signal(SIGHUP,SIG_IGN) == SIG_DFL)
		signal(SIGHUP,onintr);
	if (signal(SIGINT,SIG_IGN) == SIG_DFL)
		signal(SIGINT,onintr);
	if (signal(SIGTERM,SIG_IGN) == SIG_DFL)
		signal(SIGTERM,onintr);
	fderr = stderr;

#if DEBUG
/*	Performance data collected	*/
	ttime = times(&ptimes);
#endif

#if !ONEPROC
	strcpy(nextpass,argv[0]);
	argv++;
	argc--;
	getargs(argc,argv);
	if (filecnt < NFILES)
		aerror("Illegal number of temporary files");
D 3
	strcpy(file,filenames[0]);
E 3
I 3
	strcpy(file,filenames[FDIN]);
E 3
#endif
	if ((fdin = fopen(file, "r")) == NULL)
		aerror("Unable to open input file");
D 3
	if ((fdtext = fopen(filenames[2], "w")) == NULL)
E 3
I 3
	if ((fdtext = fopen(filenames[FDTEXT], "w")) == NULL)
E 3
		aerror("Unable to open temporary (text) file");
I 3
	if ((fdinit = fopen(filenames[FDINIT], "w")) == NULL)
		aerror("Unable to open temporary (init) file");
	if ((fdlib = fopen(filenames[FDLIB], "w")) == NULL)
		aerror("Unable to open temporary (lib) file");
I 7
	if ((fdcmt = fopen(filenames[FDCMT], "w")) == NULL)
		aerror("Unable to open temporary (cmt) file");
E 7
E 3
#if MULTSECT
D 3
	if ((fdtxt1 = fopen(filenames[8],"w")) == NULL)
E 3
I 3
	if ((fdtxt1 = fopen(filenames[FDTXT1],"w")) == NULL)
E 3
		aerror("Unable to open temporary (text 1) file");
D 3
	if ((fdtxt2 = fopen(filenames[9],"w")) == NULL)
E 3
I 3
	if ((fdtxt2 = fopen(filenames[FDTXT2],"w")) == NULL)
E 3
		aerror("Unable to open temporary (text 2) file");
D 3
	if ((fdtxt3 = fopen(filenames[10],"w")) == NULL)
E 3
I 3
	if ((fdtxt3 = fopen(filenames[FDTXT3],"w")) == NULL)
E 3
		aerror("Unable to open temporary (text 3) file");
D 3
	if ((fddat1 = fopen(filenames[11],"w")) == NULL)
E 3
I 3
	if ((fddat1 = fopen(filenames[FDDAT1],"w")) == NULL)
E 3
		aerror("Unable to open temporary (data 1) file");
D 3
	if ((fddat2 = fopen(filenames[12],"w")) == NULL)
E 3
I 3
	if ((fddat2 = fopen(filenames[FDDAT2],"w")) == NULL)
E 3
		aerror("Unable to open temporary (data 2) file");
D 3
	if ((fddat3 = fopen(filenames[13],"w")) == NULL)
E 3
I 3
	if ((fddat3 = fopen(filenames[FDDAT3],"w")) == NULL)
E 3
		aerror("Unable to open temporary (data 3) file");
I 3
D 6
	if ((fdinit1 = fopen(filenames[FDINIT1],"w")) == NULL)
		aerror("Unable to open temporary (init 1) file");
	if ((fdinit2 = fopen(filenames[FDINIT2],"w")) == NULL)
		aerror("Unable to open temporary (init 2) file");
	if ((fdinit3 = fopen(filenames[FDINIT3],"w")) == NULL)
		aerror("Unable to open temporary (init 3) file");
	if ((fdlib1 = fopen(filenames[FDLIB1],"w")) == NULL)
		aerror("Unable to open temporary (lib 1) file");
	if ((fdlib2 = fopen(filenames[FDLIB2],"w")) == NULL)
		aerror("Unable to open temporary (lib 2) file");
	if ((fdlib3 = fopen(filenames[FDLIB3],"w")) == NULL)
		aerror("Unable to open temporary (lib 3) file");
E 6
E 3
#endif
D 3
	if ((fddata = fopen(filenames[3], "w")) == NULL)
E 3
I 3
	if ((fddata = fopen(filenames[FDDATA], "w")) == NULL)
E 3
		aerror("Unable to open temporary (data) file");
#if !ONEPROC
D 3
	if ((fdstring = fopen(filenames[6], "w")) == NULL)
E 3
I 3
	if ((fdstring = fopen(filenames[FDSTRING], "w")) == NULL)
E 3
		aerror("Unable to open temporary (string) file");
#endif
#if FLEXNAMES
	strtabinit();
#endif
	creasyms();
	dot = (*lookup(".",INSTALL, USRNAME)).stp;
	dot->styp = TXT;
	dot->value = newdot = 0L;
#if MULTSECT
	dot->sectnum = 0;
#endif
	yyparse();	/* pass 1 */
	fclose(fdin);
#if !ONEPROC
	fflush(fdstring);
	if (ferror(fdstring))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdstring);
#endif
#if MULTSECT
	cgsect(TXT,0);
#else
	cgsect(TXT);
#endif

#if !ONEPROC
D 3
	if ((fdlong = fopen(filenames[5], "w")) == NULL)
E 3
I 3
	if ((fdlong = fopen(filenames[FDLONG], "w")) == NULL)
E 3
		aerror("Unable to open temporary (sdi) file");
#endif
	fixsyms();
#if MULTSECT
	cgsect(TXT,0);
#if MC68
	 ckalign(2);		/* force even section alignment */
	 cgsect(TXT,1);
	 ckalign(2);
	 cgsect(TXT,2);
	 ckalign(2);
	 cgsect(TXT,3);
	 ckalign(2);
	 cgsect(DAT,0);
	 ckalign(2);
	 cgsect(DAT,1);
	 ckalign(2);
	 cgsect(DAT,2);
	 ckalign(2);
	 cgsect(DAT,3);
	 ckalign(2);
I 2
	 cgsect(INIT,0);
	 ckalign(2);
D 6
	 cgsect(INIT,1);
	 ckalign(2);
	 cgsect(INIT,2);
	 ckalign(2);
	 cgsect(INIT,3);
	 ckalign(2);
E 6
	 cgsect(LIB,0);
	 ckalign(2);
I 7
	 cgsect(CMT,0);
	 ckalign(2);
E 7
D 6
	 cgsect(LIB,1);
	 ckalign(2);
	 cgsect(LIB,2);
	 ckalign(2);
	 cgsect(LIB,3);
	 ckalign(2);
E 6
E 2
	 cgsect(TXT,0);
#endif
#else
	cgsect(TXT); /* update "dottxt" */
#endif
#if !ONEPROC
	fflush(fdlong);
	if (ferror(fdlong))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdlong);
#endif
	flushbuf();

#if MULTSECT
	fclose(fdtxt1);
	fclose(fdtxt2);
	fclose(fdtxt3);
	fclose(fddat1);
	fclose(fddat2);
	fclose(fddat3);
I 3
D 6
	fclose(fdinit1);
	fclose(fdinit2);
	fclose(fdinit3);
	fclose(fdlib1);
	fclose(fdlib2);
	fclose(fdlib3);
E 6
E 3

	traverse(add1text);
	dottxt[0] += dottxt[1];
	traverse(add2text);
	dottxt[0] += dottxt[2];
	traverse(add3text);
	dottxt[0] += dottxt[3];

	traverse(add1data);
	dotdat[0] += dotdat[1];
	traverse(add2data);
	dotdat[0] += dotdat[2];
	traverse(add3data);
	dotdat[0] += dotdat[3];
I 2

D 6
	traverse(add1init);
D 5
	dotdat[0] += dotdat[1];
E 5
I 5
	dotinit[0] += dotinit[1];
E 5
	traverse(add2init);
D 5
	dotdat[0] += dotdat[2];
E 5
I 5
	dotinit[0] += dotinit[2];
E 5
	traverse(add3init);
D 5
	dotdat[0] += dotdat[3];
E 5
I 5
	dotinit[0] += dotinit[3];
E 5
I 3

E 3
	traverse(add1lib);
D 5
	dotdat[0] += dotdat[1];
E 5
I 5
	dotlib[0] += dotlib[1];
E 5
	traverse(add2lib);
D 5
	dotdat[0] += dotdat[2];
E 5
I 5
	dotlib[0] += dotlib[2];
E 5
	traverse(add3lib);
D 5
	dotdat[0] += dotdat[3];
E 5
I 5
	dotlib[0] += dotlib[3];
E 6
E 5
E 2
#endif
	fflush(fdtext);
	if (ferror(fdtext))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdtext);
	fflush(fddata);
	if (ferror(fddata))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fddata);
I 3
	fflush(fdinit);
	if (ferror(fdinit))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdinit);
	fflush(fdlib);
	if (ferror(fdlib))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdlib);
I 7
	fflush(fdcmt);
	if (ferror(fdcmt))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdcmt);
E 7
E 3

	ptr = (*lookup(".text", INSTALL, USRNAME)).stp;
	ptr->styp = TXT;
	ptr->value = 0L;
	ptr = (*lookup("(text)", INSTALL, USRNAME)).stp;
	ptr->styp = TXT;
#if MULTSECT
	ptr->value = dottxt[0];
#else
	ptr->value = dottxt;
#endif
I 2

E 2
	ptr = (*lookup(".data", INSTALL, USRNAME)).stp;
	ptr->styp = DAT;
	ptr->value = 0L;
	ptr = (*lookup("(data)", INSTALL, USRNAME)).stp;
	ptr->styp = DAT;
#if MULTSECT
	ptr->value = dotdat[0];
#else
	ptr->value = dotdat;
#endif
I 5
	DBUG_3 ("datsiz", "datsiz (pass 1) = %ld", ptr->value);
E 5
I 2

	ptr = (*lookup(".init", INSTALL, USRNAME)).stp;
	ptr->styp = INIT;
	ptr->value = 0L;
	ptr = (*lookup("(init)", INSTALL, USRNAME)).stp;
	ptr->styp = INIT;
D 6
#if MULTSECT
	ptr->value = dotinit[0];
#else
E 6
	ptr->value = dotinit;
D 6
#endif
E 6
I 4

E 4
	ptr = (*lookup(".lib", INSTALL, USRNAME)).stp;
	ptr->styp = LIB;
	ptr->value = 0L;
	ptr = (*lookup("(lib)", INSTALL, USRNAME)).stp;
	ptr->styp = LIB;
D 6
#if MULTSECT
	ptr->value = dotlib[0];
#else
E 6
	ptr->value = dotlib;
I 7

	ptr = (*lookup(".comment", INSTALL, USRNAME)).stp;
	ptr->styp = CMT;
	ptr->value = 0L;
	ptr = (*lookup("(cmt)", INSTALL, USRNAME)).stp;
	ptr->styp = CMT;
	ptr->value = dotcmt;
E 7
D 6
#endif
E 6

E 2
	ptr = (*lookup(".bss", INSTALL, USRNAME)).stp;
	ptr->styp = BSS;
	ptr->value = 0L;
	ptr = (*lookup("(bss)", INSTALL, USRNAME)).stp;
	ptr->styp = BSS;
	ptr->value = dotbss;

#if !ONEPROC
	ptr = (*lookup("(sdicnt)",INSTALL,USRNAME)).stp;
	ptr->value = (long)sdicnt; /* has to be set after fixsyms is called */
	ptr->styp = ABS;

D 3
	if ((fdstab = fopen(filenames[4], "w")) == NULL)
E 3
I 3
	if ((fdstab = fopen(filenames[FDSTAB], "w")) == NULL)
E 3
		aerror("Unable to open temporary (symtab) file");
	dmpstb();	/* dump the symbol table for the next pass */
	fflush(fdstab);
	if (ferror(fdstab))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdstab);
#endif

#if DEBUG
	if (tstlookup) {
		printf("Number of calls to lookup: %u\n",numcalls);
		printf("Number of identifiers: %u\n",numids);
		printf("Number of identifier collisions: %u\n",numcoll);
		fflush(stdout);
	}
/*
 *	Performance data collected and written out here
 */

	ttime = times(&ptimes) - ttime;
	if ((perfile = fopen("as.info", "r")) != NULL ) {
		fclose(perfile);
		if ((perfile = fopen("as.info", "a")) != NULL ) {
			fprintf(perfile,
			   "as1\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\tpass 1\n",
			    ttime, ptimes);
			fclose(perfile);
		}
	}

#endif

	if (!anyerrs) {
#if ONEPROC
D 3
		return(aspass2());
E 3
I 3
		DBUG_RETURN(aspass2());
E 3
#else
		nextpass[strlen(nextpass) - 1] = '2';
		xargp[0] = nextpass;
		for (i=0; i < filecnt; ++i)
			xargp[argindex++] = filenames[i];
		if (testas != TESTVAL + 1) {
			if (testas > TESTVAL + 1){
				execv(NAS2,xargp);
			}
			else
				execv(AS2,xargp);
			aerror("Unable to exec pass 2");
		}
#endif
	}
	else {
		delexit();
	}
I 3
	DBUG_VOID_RETURN;
E 3
}
E 1
