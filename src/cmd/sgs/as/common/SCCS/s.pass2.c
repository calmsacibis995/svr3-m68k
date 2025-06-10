h43055
s 00047/00008/00762
d D 1.6 86/09/16 15:28:45 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00770
d D 1.5 86/09/15 14:01:45 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00015/00000/00755
d D 1.4 86/09/11 13:02:27 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00045/00002/00710
d D 1.3 86/08/18 08:48:23 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00021/00016/00691
d D 1.2 86/08/07 08:03:30 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00707/00000/00000
d D 1.1 86/08/04 09:38:06 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)pass2.c	6.1		*/
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include "systems.h"
#include "gendefs.h"
#include "symbols.h"
I 2
#include "dbug.h"
E 2

/*
 *
 *	"pass2.c" is a file that contains the main routine for the
 *	final pass of the assembler.  This includes the primary control
 *	(function "main.c") and "getargs", the flag handling routine.
 *	This pass of the assembler is invoked with the command:
 *
 *		as2 [flags] ifile ofile t1 t2 t3 t4 t5 t6
 *
 *	where [flags] are optional flags passed from pass 1,
 *	"ifile" is the name of the assembly language source
 *	program, "t1" through "t6" are the names of temporary files,
 *	and "ofile" is the name of the file where the object program
 *	is to be written.  Since all of the information about the
 *	program is contained in temporary files at the time that the
 *	second pass is executed, the name "ifile" is only used in
 *	printing error messages.  The intermediate form of the text
 *	section appears in the temporary file "t1", the intermediate
 *	data section appears in "t2", and the symbol table appears in
 *	"t3".  Each of the temporary files "t1" through "t6" is used in
 *	a variety of ways during this pass.  See the commentary in
 *	function "main.c" for the exact sequence of events.  The file
 *	"ofile" is used as the output from this pass.
 *
 */

/*
 *
 *	The following are declarations for the major global variables
 *	used in the second pass of the assembler.  The variables and
 *	their meainings are as follows:
 *
 *	fderr	This contains the file desciptor to which error messages
 *		are to be written.
 *
 *	fdout	This is used to contain the file descriptor for the
 *		object file.
 *
 *	fdsect	This is the file descriptor where temporary section
 *		information is to be written.  This will refer to
 *		several different temporary files throughout execution
 *		of this pass.
 *
 *	fdline	This is the file descriptor where the completed line
 *		number entries are always written. Both the line number
 *		entries for the text section and the data section are
 *		placed in this file. Currently the file used for this
 *		is temporary file "t3".
 *
 *	fdrel	This is the file descriptor where preliminary relocation
 *		entries are to be written.  As it happens, this will
 *		always refer to the temporary file "t4" during execution
 *		of this program.
 *
 *	fdsymb	This is the file descriptor where symbol table entries
 *		will be written.  These are symbol table entries from
 *		".def" pseudo operations, not to be confused with the
 *		internal symbol table entries that are read from "t3"
 *		when this pass begins execution.  This file descriptor
 *		will refer to the temporary file "t6" during execution
 *		of this program.
 *
 *	fdgsymb	This is the file descriptor where symbol table entries
 *		for global symbols whose declarations appear in
 *		the intermediate file for the "data" section will be
 *		written.  These are written to a file other than that
 *		referred to by "fdsymb" so that declarations for
 *		external static symbols can find their way ahead of the
 *		global symbol declarations.
 *
 *	file	This is a pointer to the string which is the input file
 *		name ("ifile", as described at the beginning of this
 *		file).  This will be used if necessary to report the
 *		file name in an error message (see "errors.c").
 *
 *	filhead	This is a structure that is used to build the header for
 *		the object file.  It is of type "FILHDR", defined in
 *		"ldfmt.h".
 *
 *	txthead	This is a structure that is used to build the header
 *		for the text section.  It is of type "SCNHDR", defined
 *		in "ldfmt.h".
 *
 *	dathead	This is a structure that is used to build the header
 *		for the data section.  It is of type "SCNHDR", defined
 *		in "ldfmt.h".
 *
 *	bsshead	This is a structure that is used to build the header
 *		for the bss section.  It is of type "SCNHDR", defined
 *		in "ldfmt.h".
 *
 *	symhead	This is a structure that is used to build the dummy
 *		section header whose offset points to the symbol table.
 *		It is of type "SCNHDR", defined in "ldfmt.h".
 *
 *	uptxt	This is a flag that is set if the size of the text
 *		section needs to be rounded up to make it an even
 *		number of bytes.
 *
 *	updat	This is a flag that is set if the size of the data
 *		section needs to be rounded up to make it an even
 *		number of bytes.
 *
 *	symbent	A count of the number of entries in the symbol table
 *		whose file is referred to be "fdsymb".
 *
 *	gsymbent A count of the number of entries in the symbol table
 *		whose file is referred to by "fdgsymb".
 *
 *	txtsiz	Gives the size of the text section, in bytes.
 *
 *	datsiz	Gives the size of the data section, in bytes.
 *
 *	bsssiz	Gives the size of the bss section, in bytes.
 *
 *	txtlin	A count of the number of line number entries for the
 *		text section.
 *
 *	datlin	A count of the number of line number entries for the
 *		data section.
 *
 *	txtrel	A count of the number of relocation entries for the
 *		text section.
 *
 *	datrel	A count of the number of relocation entries for the
 *		data section.
 *
 */

#if ONEPROC
extern short passnbr;
extern unsigned short sdicnt;
#endif

extern char
	file[];

extern char
	*filenames[];

extern char
	*malloc(),
	*strcpy();

extern short
	anyerrs;

extern unsigned short
	lineent;

extern int
	aerror(),
	delexit(),
	deltemps(),
	flags(),
	onintr();

extern upsymins
	*lookup();

#if FLEXNAMES
extern long	currindex;
extern char	*strtab;
#endif

extern FILE
	*fderr;

extern long
	codestrt;

I 4
extern void initcodestrt ();

extern unsigned short nsects;

E 4
BYTE	*longsdi;

#if ONEPROC
extern char islongsdi[];
#endif

unsigned short
	relent,
	txtlin,
	datlin,
I 3
	initlin,
	liblin,
I 6
	cmtlin,
E 6
E 3
	txtrel,
D 3
	datrel;
E 3
I 3
	datrel,
	initrel,
D 6
	librel;
E 6
I 6
	librel,
	cmtrel;
E 6
E 3

#if ONEPROC
extern short transvec;
#else
short transvec = NO; /* indicates transfer vector program addressing */
#endif

short
	dlflag = NO;

#if M4ON
extern short rflag;
#endif

FILE
	*fdout,
	*fdsect,
	*fdline,
	*fdrel,
	*fdsymb,
	*fdgsymb
#if !ONEPROC
	,
	*fdstring
#endif
	;
#if DEBUG
extern FILE
	*perfile;	/* performance data file descriptor */
#endif

long	uptxt,
	updat,
I 3
	upinit,
	uplib,
I 6
	upcmt,
E 6
E 3
	upbss,
	txtsiz,
	datsiz,
I 3
	initsiz,
D 6
	libsiz;
E 6
I 6
	libsiz,
	cmtsiz,
E 6
E 3
	bsssiz;

long	symbent = 0L,
	gsymbent = 0L;

#if !ONEPROC
static char
	teststr[4] = {'-','t','\0','\0'};
#endif

static short
#if !ONEPROC
	filecnt,
#endif
	testas = TESTVAL;

#if DEBUG
/*
 *	Performance data variables
 */
extern long	ttime;
extern struct	tbuffer {
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
	
I 2
	DBUG_ENTER ("getargs");
E 2
	while (xargc-- > 0) {
		if (**xargv == '-') {
			while ((ch = *++*xargv) != '\0')
				switch (ch) {
#if M4ON
					case 'R': {
					/* set flag to remove input file */
						rflag = YES;
						break;
					}
#endif
					case 'd':
						if (*++*xargv == 'l')
							dlflag = YES;
						break;
					case 't': {
						++*xargv;
#if TRANVEC
						if (**xargv == 'v') {
							transvec = YES;
							break;
						}
#endif
						if (isdigit(**xargv)) {
							testas = **xargv - '0' -1;
							if (testas > TESTVAL + 1)
								teststr[2] = (char)(testas + '0');
						}
						else {
							--*xargv;
							testas += 2;
						}
						break;
					}
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
I 2
	DBUG_VOID_RETURN;
E 2
}

/*
 *
 *	"main" is the main function for the second pass of the assembler.
 *	It is invoked as described at the beginning of this file.  It
 *	performs initialization by setting the variable "file" to point
 *	to the source file name, setting "arglist" to point to the
 *	argument list (this is so temporary files can be removed easily
 *	in case of error), checking the argument count, and calling
 *	"getstab" to get the symbol table from temporary file t3.  It
 *	then performs the following sequence of steps:
 *
 *	     1. The special symbol "(text)" is looked up in the symbol
 *		table to determine the length of the text section.  If
 *		necessary, this is rounded up to make it an even number
 *		of bytes and the flag "uptxt" is set to indicate that
 *		this was done.  The symbol "(data)" is looked up to
 *		determine the length of the data section.  If necessary
 *		this is rounded up to make it an even number of bytes
 *		and the flag "updat" is set to indicate that this was
 *		done.  The symbol "(bss)" is looked up to determine the
 *		length of the bss section and, if necessary, rounded up
 *		to an even number of bytes.  The result of this step is
 *		that the sizes of the three sections appear in "txtsiz",
 *		"datsiz", and "bsssiz".  The function "fixstab" is
 *		called to convert data and bss symbol values from
 *		offsets to addresses.
 *
 *	     2. The following file descriptors are opened:
 *
 *		fdsect	This is opened for writing on the output file
 *			ofile.  The section information for the text
 *			section will be written to this file.
 *
 *		fdsymb	This is opened for writing on the temporary file
 *			t6.  The symbol table entries will be written
 *			to this file.
 *
 *		fdline	This is opened for writing on the temporary file
 *			t3.  The preliminary line number entries for
 *			the text section will be written to this file.
 *
 *		fdrel	This is opened for writing on the temporary file
 *			t4.  The preliminary relocation information for
 *			the text section will be written to this file.
 *
 *		The following variables are initialized:
 *
 *		lineent	This is used to count the number of line number
 *			entries written to "fdline".  The value of this
 *			variable will be copied to "txtlin" after the
 *			text section is processed.
 *
 *		relent	This is used to count the number of relocation
 *			entries written to "fdrel".  The value of this
 *			variable will be copied to "txtrel" after the
 *			text section is processed.
 *
 *		symbent	This is used to keep count of the number of
 *			symbol table entries written to "fdsymb".
 *
 *		gsymbent This is used to keep count of the number of
 *			symbol table entries written to "fdgsymb".  This
 *			will not be used until the data section is
 *			processed.
 *
 *		The function "codout" (from codout.c) is called to
 *		process the text section.  If the flag "uptxt" is set,
 *		an extra byte having the value hex "f4" is written to
 *		the end of "fdsect".  This rounds the text section out
 *		to an even number of bytes by adding a "hlt"
 *		instruction.  The contents of the intermediate files
 *		at this point are:
 *
 *		ofile	Contains the final code for the text section.
 *
 *		t2	Still the intermediate data file from pass 2.
 *
 *		t3	Processed line number entries from the text
 *			section.
 *
 *
 *		t4	Contains the preliminary relocation entries for
 *			the text section.
 *
 *		t5	Contains any strings found in instructions by
 *			pass one.
 *
 *		t6	Contains the symbol table entries from ".def"s
 *			in the text section.
 *
 *	     3. This step processes the data section.  The following
 *		file descriptors are involved:
 *
 *		fdline	The line number entries for the data section
 *			will be appended after the line number entries
 *			for the text section.
 *
 *		fdsect	This remains open pointing at the output file
 *			ofile.  The data section code will be written
 *			here, following the code for the text section.
 *
 *		fdrel	This remains open for writing on the temporary
 *			file t4.  The preliminary relocation entries
 *			for the data section will follow the preliminary
 *			relocation information for the text section on
 *			this file.
 *
 *		fdsymb	This remains open for writing on the temporary
 *			file t6.  During data section processing, only
 *			static symbols will be written to this file,
 *			following all ".def"s from the text section.
 *
 *		fdgsymb	This points to the temporary file t1.  Entries
 *			are written to this file for global symbol
 *			".def"s.
 *
 *		The variables "lineent" and "relent" are re-cleared to
 *		count line number and relocation entries for the data
 *		section.  These will be copied to "datlin" and "datrel"
 *		after processing of the data section is complete.  The
 *		function "codout" is then called to process the data
 *		section.  The file descriptors "fdgsymb", "fdline",
 *		"fdrel", and "fdsect" are closed. The function
 *		"symout" is then called to create all symbol table
 *		entries not created by ".def"s and copy the re-
 *		maining entries from "fdgsymb" to fdsymb". This
 *		leaves the complete symbol table on t6 and leaves
 *		files t1 and t2 empty. The file descriptors "fdsymb"
 *		and "fdgsymb" are closed. The contents of the temp-
 *		orary files are as follows:
 *
 *		ofile	This contains the generated code for the
 *			text section followed by the code for the
 *			data section.
 *
 *		t1	empty
 *
 *		t2	empty
 *
 *		t3	This contains the completed line number
 *			entries for both text and data.
 *
 *		t4	This file contains the preliminary relocation
 *			entries for both the text and data sections.
 *
 *		t5	No longer used.
 *
 *		t6	This contains the now completed symbol table.
 *
 *	     4. The file descriptor "fdout" is initialized and is
 *		set to point to the beginning of the object file to
 *		be created.  The function "headers" is called to
 *		create the file headers and all section headers for
 *		the object file.
 *
 *	     5. The file descriptor "fdrel" is opened to read from t4,
 *		and "reloc" is called to process the text section
 *		and data section relocation entries and write them
 *		to the object file.
 *
 *	     6. The file t3 containing the text and data line number
 *		entries is appended to the object file by calling
 *		"copysect".
 *
 *	     7. The file t6 containing the complete symbol table is
 *		copied to the object file using "copysect".  Following
 *		this, the file descriptor "fdout" can be closed, since
 *		the object file is complete.
 *
 *	    8. If the assembler has not been called for testing
 *		the temporary file are removed by calling "deltemps".
 *
 */

main(argc,argv)
	int argc;
	char **argv;
#else

aspass2()

#endif
{
	register symbol *ptr;
	long home;
#if !ONEPROC
	unsigned int sdicnt = 0;
	char *savptr;
#endif

I 2
	DBUG_ENTER ("aspass2");
E 2
#if ONEPROC
	passnbr = 2;
#else
	if (signal(SIGHUP,SIG_IGN) == SIG_DFL)
		signal(SIGHUP,onintr);
	if (signal(SIGINT,SIG_IGN) == SIG_DFL)
		signal(SIGINT,onintr);
	if (signal(SIGTERM,SIG_IGN) == SIG_DFL)
		signal(SIGTERM,onintr);
	fderr = stderr;
#endif

#if DEBUG
/*	Performance data collected here	*/
	ttime = times(&ptimes);
#endif

#if !ONEPROC
	argv++;
	argc--;
	getargs(argc,argv);
D 2
	strcpy(file,filenames[0]);
E 2
I 2
	strcpy(file,filenames[FDIN]);
E 2
	if (filecnt < NFILES)
		aerror("Illegal Argument Count");
D 2
	getstab(filenames[4]);
E 2
I 2
	getstab(filenames[FDSTAB]);
E 2
	ptr = (*lookup("(sdicnt)",N_INSTALL,USRNAME)).stp;
	if (ptr != NULLSYM && (sdicnt = (unsigned)(ptr->value)) != 0) {
D 2
		if ((fdsect = fopen(filenames[5],"r")) == NULL)
E 2
I 2
		if ((fdsect = fopen(filenames[FDLONG],"r")) == NULL)
E 2
			aerror("Cannot open temporary (sdi) file");
		longsdi = (BYTE *)malloc(sdicnt);
		savptr = longsdi;
		fread(longsdi,sizeof(*longsdi),(int)sdicnt,fdsect);
		fclose(fdsect);
		}
#else
	longsdi = (BYTE *)islongsdi;
#endif
	ptr = (*lookup("(text)",N_INSTALL,USRNAME)).stp;
	txtsiz=ptr->value;
I 4
	DBUG_3 ("txtsiz", "txtsiz = %ld", txtsiz);
E 4
	if (uptxt = txtsiz % SCTALIGN) {
		uptxt = SCTALIGN - uptxt;
		txtsiz += uptxt;
I 4
		DBUG_3 ("txtsiz", "txtsiz = %ld", txtsiz);
E 4
	}
I 6
	DBUG_4 ("txt", "txtsiz = %ld, uptxt = %ld", txtsiz, uptxt);
E 6
	ptr = (*lookup("(data)",N_INSTALL,USRNAME)).stp;
	datsiz=ptr->value;
I 4
	DBUG_3 ("datsiz", "datsiz = %ld", datsiz);
E 4
	if (updat = datsiz % SCTALIGN) {
		updat = SCTALIGN - updat;
		datsiz += updat;
I 4
		DBUG_3 ("datsiz", "datsiz = %ld", datsiz);
E 4
	}
I 6
	DBUG_4 ("dat", "datsiz = %ld, updat = %ld", datsiz, updat);
E 6
	ptr = (*lookup("(bss)",N_INSTALL,USRNAME)).stp;
	bsssiz = ptr->value;
	if (upbss = bsssiz % SCTALIGN) {
		upbss = SCTALIGN - upbss;
		bsssiz += upbss;
	}
I 6
	DBUG_4 ("bss", "bsssiz = %ld, upbss = %ld", bsssiz, upbss);
E 6
I 3
	ptr = (*lookup("(init)",N_INSTALL,USRNAME)).stp;
	initsiz=ptr->value;
	if (upinit = initsiz % SCTALIGN) {
		upinit = SCTALIGN - upinit;
		initsiz += upinit;
	}
I 4
	if (initsiz > 0) {
	    nsects++;
	}
I 6
	DBUG_4 ("init", "initsiz = %ld, upinit = %ld", initsiz, upinit);
E 6
E 4
	ptr = (*lookup("(lib)",N_INSTALL,USRNAME)).stp;
	libsiz=ptr->value;
	if (uplib = libsiz % SCTALIGN) {
		uplib = SCTALIGN - uplib;
		libsiz += uplib;
	}
I 4
	if (libsiz > 0) {
	    nsects++;
	}
I 6
	DBUG_4 ("lib", "libsiz = %ld, uplib = %ld", libsiz, uplib);
	ptr = (*lookup("(cmt)",N_INSTALL,USRNAME)).stp;
	cmtsiz=ptr->value;
	if (upcmt = cmtsiz % SCTALIGN) {
		upcmt = SCTALIGN - upcmt;
		cmtsiz += upcmt;
	}
	if (cmtsiz > 0) {
	    nsects++;
	}
	DBUG_4 ("cmt", "cmtsiz = %ld, upcmt = %ld", cmtsiz, upcmt);
E 6
	initcodestrt ();
E 4
E 3

	fixstab(0L,txtsiz,DAT);
	fixstab(0L,txtsiz+datsiz,BSS);
I 3
	fixstab(0L,txtsiz+datsiz,INIT);		/* No bsssiz */
	fixstab(0L,txtsiz+datsiz+initsiz,LIB);
I 6
	fixstab(0L,txtsiz+datsiz+initsiz+libsiz,CMT);
E 6
E 3

	lineent = 0;
	relent = 0;
D 2
	if((fdsect = fopen(filenames[1],"w"))==NULL)
E 2
I 2
	if((fdsect = fopen(filenames[FDOUT],"w"))==NULL)
E 2
		aerror("Cannot Open Output File");
	fseek(fdsect,codestrt,0);	/* position pointer past area for headers */
#if !ONEPROC
D 2
	if((fdstring = fopen(filenames[6],"r"))==NULL)
E 2
I 2
	if((fdstring = fopen(filenames[FDSTRING],"r"))==NULL)
E 2
		aerror("Cannot Open Temporary (string) File");
#endif
D 2
	if((fdsymb = fopen(filenames[7],"w"))==NULL)
E 2
I 2
	if((fdsymb = fopen(filenames[FDSYMB],"w"))==NULL)
E 2
		aerror("Cannot Open Temporary (symbol) File");
D 2
	if((fdline = fopen(filenames[4],"w"))==NULL)
E 2
I 2
	if((fdline = fopen(filenames[FDLINE],"w"))==NULL)
E 2
		aerror("Cannot Open Temporary (line no) File");
D 2
	if((fdrel = fopen(filenames[5],"w"))==NULL)
E 2
I 2
	if((fdrel = fopen(filenames[FDREL],"w"))==NULL)
E 2
		aerror("Cannot Open Temporary (rel) File");
D 2
	codout(filenames[2],0L,TXT);
E 2
I 2
	codout(filenames[FDTEXT],0L,TXT);
E 2
#if !ONEPROC
	if ( sdicnt != 0 )
		free(savptr);	/* free space no longer needed */
#endif
I 6
	DBUG_3 ("uptxt", "uptxt = %ld", uptxt);
E 6
#if MC68
	if (uptxt & 01)
	{
		uptxt--;
		codgen(BITSPBY,FILL);
	}
	while (uptxt > 0)
	{
		codgen(2*BITSPBY,TXTFILL);
		uptxt -= 2;
	}
#else
D 6
	while (uptxt--)
E 6
I 6
	while (uptxt-- > 0)
E 6
		codgen(BITSPBY,TXTFILL);
#endif
	txtlin = lineent;
	txtrel = relent;
I 6
	DBUG_3 ("rel", "txtlin = lineent = %ld", lineent);
	DBUG_3 ("rel", "txtrel = relent = %ld", relent);
E 6
D 2
	if((fdgsymb = fopen(filenames[2],"w"))==NULL)
E 2
I 2
	if((fdgsymb = fopen(filenames[FDGSYMB],"w"))==NULL)
E 2
		aerror("Cannot Open Temporary (gbl sym) File");

	lineent = 0;
	relent = 0;
D 2
	codout(filenames[3],txtsiz,DAT);
E 2
I 2
	codout(filenames[FDDATA],txtsiz,DAT);
E 2
D 6
	while (updat--)
E 6
I 6
	while (updat-- > 0)
E 6
		codgen(BITSPBY,FILL);
		/* raw code for text and data is complete */
	datlin = lineent;
	datrel = relent;
I 6
	DBUG_3 ("rel", "datlin = lineent = %ld", lineent);
	DBUG_3 ("rel", "datrel = relent = %ld", relent);
E 6
I 3

	lineent = 0;
	relent = 0;
	codout(filenames[FDINIT],datsiz,INIT);
D 6
	while (upinit--)
E 6
I 6
	while (upinit-- > 0)
E 6
		codgen(BITSPBY,FILL);
		/* raw code for text data, and init is complete */
	initlin = lineent;
	initrel = relent;
I 6
	DBUG_3 ("rel", "initlin = lineent = %ld", lineent);
	DBUG_3 ("rel", "initrel = relent = %ld", relent);
E 6

	lineent = 0;
	relent = 0;
	codout(filenames[FDLIB],initsiz,LIB);
D 6
	while (uplib--)
E 6
I 6
	while (uplib-- > 0)
E 6
		codgen(BITSPBY,FILL);
D 6
		/* raw code for text and data is complete */
E 6
	liblin = lineent;
	librel = relent;
I 6
	DBUG_3 ("rel", "liblin = lineent = %ld", lineent);
	DBUG_3 ("rel", "librel = relent = %ld", relent);
E 6

I 6
	lineent = 0;
	relent = 0;
	codout(filenames[FDCMT],libsiz,CMT);
	while (upcmt-- > 0)
		codgen(BITSPBY,FILL);
	cmtlin = lineent;
	cmtrel = relent;
	DBUG_3 ("rel", "cmtlin = lineent = %ld", lineent);
	DBUG_3 ("rel", "cmtrel = relent = %ld", relent);

E 6
E 3
	fixst(fdsymb);
#if !ONEPROC
	fclose(fdstring);	/* file no longer needed */
#endif
	fflush(fdline);
	if (ferror(fdline))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdline);	/* line numbers are complete */
	fflush(fdrel);
	if (ferror(fdrel))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdrel);	/* flush the buffer */
	fflush(fdgsymb);
	if (ferror(fdgsymb))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdgsymb);	/* flush the buffer */

D 2
	if ((fdgsymb = fopen(filenames[2],"r")) == NULL)
E 2
I 2
	if ((fdgsymb = fopen(filenames[FDGSYMB],"r")) == NULL)
E 2
		aerror("Cannot Open Temporary (gbl sym) File");
	symout();
	fclose(fdgsymb);	/* preliminary global symbols are processed */
	fflush(fdsymb);
	if (ferror(fdsymb))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdsymb);	/* symbol table is complete */
	fdout = fdsect;
	home = ftell(fdout);	/* remember where we are */
	fseek(fdout,0L,0);
	if (dlflag) {
		txtlin = 0;
		datlin = 0;
I 3
		initlin = 0;
		liblin = 0;
I 6
		cmtlin = 0;
E 6
E 3
	}
	headers();
	fseek(fdout,home,0);	/* return for more work */

D 2
	if ((fdrel = fopen(filenames[5],"r")) == NULL)
E 2
I 2
	if ((fdrel = fopen(filenames[FDREL],"r")) == NULL)
E 2
		aerror("Cannot Open Temporary (rel) File");
D 3
	reloc((long)txtrel + (long)datrel);
E 3
I 3
D 6
	reloc((long)txtrel + (long)datrel + (long)initrel + (long)librel);
E 6
I 6
	reloc((long)txtrel + (long)datrel + (long)initrel + (long)librel +(long)cmtrel);
E 6
E 3
	fclose(fdrel);	/* relocation info is complete and appended */

#if DEBUG
/*
 *	Performance data collected and written out here
 */

	ttime = times(&ptimes) - ttime;
	if ((perfile = fopen("as.info", "r")) != NULL ) {
		fclose(perfile);
		if ((perfile = fopen("as.info", "a")) != NULL ) {
			fprintf(perfile,
			   "as2\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\tpass 2\n",
			    ttime, ptimes);
			fclose(perfile);
		}
	}

#endif

	if (!anyerrs) {
		if (!dlflag)
D 2
			copysect(filenames[4]);	/* append line number info */
		copysect(filenames[7]);	/* append symbol table */
E 2
I 2
			copysect(filenames[FDLINE]);	/* append line number info */
		copysect(filenames[FDSYMB]);	/* append symbol table */
E 2
#if FLEXNAMES
		if (currindex > 4)	/* Does a string table exist?	*/
		{
			fwrite((char *)&currindex,sizeof(long),sizeof(char),fdout);
			fwrite(&strtab[4],sizeof(char),currindex-4,fdout);
		}
#endif
		fflush(fdout);
		if (ferror(fdout))
			aerror("trouble writing; probably out of temp-file space");
		fclose(fdout);

#if DEBUG
		if (testas != TESTVAL + 1)
#endif
			deltemps();
#if ONEPROC
D 2
		return(0);
E 2
I 2
		DBUG_RETURN(0);
E 2
#else
		exit(0);
#endif
	}
	else
		delexit();
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
