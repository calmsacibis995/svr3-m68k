h61111
s 00043/00009/00295
d D 1.3 86/08/18 08:48:52 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00027/00001/00277
d D 1.2 86/08/11 09:54:41 fnf 2 1
c Checkpoint changes to date.
e
s 00278/00000/00000
d D 1.1 86/07/31 14:39:13 fnf 1 0
c Initial copy from 5.3 distribution for AT&T 3b2.
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/fcns.c	1.3"

#include <stdio.h>
#include <signal.h>
#include <varargs.h>
#include "paths.h"
#include "filehdr.h"
#include "ldfcn.h"
#include "syms.h"
#include "reloc.h"
#include "scnhdr.h"
#include "shlib.h"
#include "hst.h"
#include "trg.h"
I 3
#include "dbug.h"
E 3

#define MAXARGS	500	/* maxinum number of args to a call to execute() */

/*
 * getpref() gets the prefix of the mkshlib command and returns a pointer to a string
 * holding the prefix
 */
char *
getpref(cmd)
char	*cmd;
{
	char	*pref;
	char	*pptr;
	int	cmdlen;

I 3
	DBUG_ENTER ("getpref");
E 3
	pptr= getbase(cmd);
	cmdlen= strlen(pptr);
	if (cmdlen < 7 || strncmp(pptr+(cmdlen-7),"mkshlib",7) != 0)
		fatal("command name must end in \"mkshlib\"");
	pref= stralloc(pptr);
	pref[cmdlen-7]='\0';
D 3
	return(pref);
E 3
I 3
	DBUG_3 ("pref", "command prefix is '%s'", pref);
	DBUG_RETURN(pref);
E 3
}


/* getbase() gets the base filename of the supplied pathname */
char *
getbase(fnam)
char	*fnam;
{
	char	*pptr;

I 3
	DBUG_ENTER ("getbase");
E 3
	if ((pptr= strrchr(fnam,'/')) == NULL)
		pptr= fnam;
	else
		pptr++;
D 3
	return(pptr);
E 3
I 3
	DBUG_4 ("base", "base filename of pathname '%s' is '%s'", fnam, pptr);
	DBUG_RETURN(pptr);
E 3
}

/* concat() allocates space to concatenate pref and name and returns a pointer
 * to the concatenated string
 */
char *
concat(pref, name)
char	*pref,
	*name;
{
	char	*p;
I 3
	DBUG_ENTER ("concat");
	DBUG_4 ("cat", "concatenate '%s' and '%s'", pref, name);
E 3
	if ((p = malloc((unsigned)(strlen(prefix) + strlen(name) + 1))) == NULL)
		fatal("Out of space");
	(void)strcpy(p, pref);
	(void)strcat(p, name);
D 3
	return(p);
E 3
I 3
	DBUG_3 ("cat", "result is '%s'", p);
	DBUG_RETURN(p);
E 3
}

/* Hash function for all hash tables */
long
hash(name, tabsize)
char *name;
int	tabsize;
{
	char	*tstr;
	long	i;
	unsigned long	hashval=0;

	for (tstr=name, i=1; *tstr != '\0'; tstr++, i++)
		hashval+= (*tstr) * i;

	return(hashval % tabsize);
}


/* Copy size bytes from the object file, ldfil, to the file, ffil. */
int
copy(ldfil, ffil,  size )
	LDFILE	*ldfil;
	FILE	*ffil;
	register long	size;
{
	char	buffer[BUFSIZ];
	int	n;

I 3
	DBUG_ENTER ("copy");
	DBUG_3 ("copy", "copy %ld bytes from ldfil to ffil", size);
E 3
	while (size && (n = FREAD(buffer,sizeof(char),BUFSIZ,ldfil)) > 0) {
		if (size/n == 0)
			n= size;
		size-= n;
		if (fwrite(buffer,sizeof(char),n,ffil) != n)
D 3
			return(FAILURE);
E 3
I 3
			DBUG_RETURN(FAILURE);
E 3
	}
	if (size != 0)
D 3
		return(FAILURE);
	return(SUCCESS);
E 3
I 3
		DBUG_RETURN(FAILURE);
	DBUG_RETURN(SUCCESS);
E 3
}


/* stralloc() returns a pointer to a copy of str */
char *
stralloc(str)
char	*str;
{
	char	*tptr;

I 3
	DBUG_ENTER ("stralloc");
	DBUG_3 ("scopy", "make duplicate copy of '%s'", str);
E 3
	if ((tptr= malloc((unsigned)(strlen(str)+1))) == NULL)
		fatal("Out of space");

	(void)strcpy(tptr, str);
D 3
	return(tptr);
E 3
I 3
	DBUG_RETURN(tptr);
E 3
}


/* This function gets the names of all necessary temporary files */
void
mktmps()
{
	extern int	errno;
I 3
	DBUG_ENTER ("mktmps");
E 3
	if (maketarg == TRUE) {
		ifil1name=tempnam(TMPDIR,"trg5"); /* name of ifile for initial 
						   * load */
I 3
		DBUG_3 ("ifil1name", "'%s': ifile for initial load", ifil1name);
E 3
		ifil2name=tempnam(TMPDIR,"trg1"); /* name of ifile for final load */
I 3
		DBUG_3 ("ifil2name", "'%s': ifile for final load", ifil2name);
E 3
		tpltnam=  tempnam(TMPDIR,"trg2"); /* name of intermediate partially
						   * loaded target */
I 3
		DBUG_3 ("tpltnam", "'%s': intermediate partially loaded target", tpltnam);
E 3
		pltname=  tempnam(TMPDIR,"trg3"); /* name of partially loaded 
						   * target */
I 3
		DBUG_3 ("pltname", "'%s': partially loaded target", pltname);
E 3
		assemnam= tempnam(TMPDIR,"trg4"); /* name of assembly file to 
						   * hold branch table */
I 3
		DBUG_3 ("assemnam", "'%s': assembly file for branch table", assemnam);
E 3
		btname=   tempnam(TMPDIR,"trg6"); /* name of object to hold branch 
						   * table */
I 3
		DBUG_3 ("btname", "'%s': object file for branch table", btname);
E 3
		if (!(ifil1name && ifil2name && tpltnam && pltname && assemnam &&
			btname))
			fatal("Cannot create temp file name");
	}

	if (makehost == TRUE) {
		moddir=  tempnam(TMPDIR,"hst1"); /* name of temp directory for
						  * holding modified objects */
I 3
		DBUG_3 ("moddir", "'%s': temp directory for modified objects", moddir);
E 3
		defname= tempnam(TMPDIR,"hst2"); /* name of lib. definition file */
I 3
		DBUG_3 ("defname", "'%s': name of lib. definition file", defname);
E 3
		ainit=   tempnam(TMPDIR,"hst3"); /* name of aseembly file for 
						  * creating .init sections */
I 3
		DBUG_3 ("ainit", "'%s': name of assembly file for .init sections", ainit);
E 3
		if (!(moddir && defname && ainit))
			fatal("Cannot create temp file name");
	}
	errno=0; /* reset errno to prevent erroneous perror() messages */
I 3
	DBUG_VOID_RETURN;
E 3
}


/* This function enables signal catching so that temp files will be removed
 * before an abnormal termination of the program */
void
catchsigs()
{
I 3
	DBUG_ENTER ("catchsigs");
E 3
	if (signal(SIGHUP, SIG_IGN) == SIG_DFL)
		(void)signal(SIGHUP, (int (*)())dexit);
	if (signal(SIGINT, SIG_IGN) == SIG_DFL)
		(void)signal(SIGINT, (int (*)())dexit);
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
		(void)signal(SIGTERM, (int (*)())dexit);
I 3
	DBUG_VOID_RETURN;
E 3
}


/*VARARGS0*/
void
fatal(va_alist)
va_dcl
{
	extern int	errno;

	va_list	args;
	char	*fmt;
	static int	first=TRUE;

I 3
	DBUG_ENTER ("va_dcl");
E 3
	va_start(args);
	(void)fprintf(stderr,"%s: ",progname);
	fmt= va_arg(args, char *);
	(void)vfprintf(stderr, fmt, args);
	va_end(args);
	if (errno != 0)
		perror(" ");
	else
		(void)putc('\n',stderr);

	if (first) {
		first= FALSE;
		dexit();
	} else
		exit(1);
I 3
	DBUG_VOID_RETURN;
E 3
}


/* dexit() deletes all temporary files and exits */
void
dexit()
{
I 3
	DBUG_ENTER ("dexit");
E 3
	rmtmps();
	exit(1);
I 3
	DBUG_VOID_RETURN;
E 3
}


/* This function removes all temporary files. */
void
rmtmps()
{
I 3
	DBUG_ENTER ("rmtmps");
E 3
I 2
	if (dflag == TRUE) {
		fprintf (stderr, "warning -- remove temporary files by hand!\n");
D 3
		return;
E 3
I 3
		DBUG_VOID_RETURN;
E 3
	}
E 2
	if (maketarg == TRUE) {
		(void)unlink(ifil1name);
		(void)unlink(ifil2name);
		(void)unlink(tpltnam);
		(void)unlink(pltname);
		(void)unlink(assemnam);
		(void)unlink(btname);
	}
	if (makehost == TRUE) {
		(void)unlink(defname);
		(void)unlink(ainit);
		if (execute("rm","rm","-r","-f",moddir,(char *)0))
			fatal("Internal rm invocation failed");
	}
I 3
	DBUG_VOID_RETURN;
E 3
}


/* This function invokes execvp to execute a command. */
/*VARARGS0*/
int
execute(va_alist)
va_dcl
{
	va_list	ap;
	char	*file;
	char	*args[MAXARGS];
	int	argno=0;
	int	pid,
		w,
		status;

I 3
	DBUG_ENTER ("execute");
E 3
	va_start(ap);
	file = va_arg(ap, char *);
D 2
	while ((args[argno++] = va_arg(ap,char *)) != (char *)0)
E 2
I 2
	while ((args[argno++] = va_arg(ap,char *)) != (char *)0) {
E 2
		if (argno >= MAXARGS)
			fatal("Too many args to %s for internal invocation",file);
I 2
		if (vflag) {
			printf ("%s ", args[argno-1]);
		}
	}
E 2
	va_end(ap);
I 2
	if (vflag) {
		printf("\n");
		fflush (stdout);
	}
E 2

	if ((pid = fork()) == 0) {
I 2
		closefiles();
E 2
		(void)execvp(file, args);
		fatal("Cannot exec %s", file);
	} else if (pid == -1)
		fatal("Process table full - try again later");

	while ((w = wait(&status)) != pid && w != -1) ;

	if (w == -1)
		fatal("Lost %d - No child process!",pid);
	else {
		if ((w = status & 0xff) != 0) {
			if (w != SIGINT)
				fatal("Fatal error in %s;  Status 0%o",file,status);
		}
	}
D 3
	return((status >> 8) & 0xff);
E 3
I 3
	DBUG_RETURN((status >> 8) & 0xff);
E 3
I 2
}

/* This is a hack to close files after fork but before exec in child
   so child will have enough available file descriptors.
   Skip 0 (stdin), 1 (stdout), and 2 (stderr) */

closefiles()
{
	register int fd;

I 3
	DBUG_ENTER ("closefiles");
E 3
	for (fd = 3; fd < 30; fd++) {
		(void) close (fd);
	}
I 3
	DBUG_VOID_RETURN;
E 3
E 2
}
E 1
