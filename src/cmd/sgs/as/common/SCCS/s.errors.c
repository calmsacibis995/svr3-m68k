h23963
s 00000/00000/00136
d D 1.6 86/09/16 15:28:12 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00136
d D 1.5 86/09/15 14:01:17 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00136
d D 1.4 86/09/11 13:01:49 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00136
d D 1.3 86/08/18 08:48:01 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00017/00002/00119
d D 1.2 86/08/07 08:03:06 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00121/00000/00000
d D 1.1 86/08/04 09:37:59 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)errors.c	6.1		*/
/*
 *
 *	"errors.c" is a file containing a routine "aerror" that prints
 *	out  error messages and (currently) terminates execution when
 *	an error is encountered.  It prints out  the  file  and  line
 *	number  where  the  error  was  encountered and uses a switch
 *	statement to select a diagnostic message based on  the  error
 *	code.   If  this  error  was  reported by anything other than
 *	"makeinst" (indicated by "pass" being non-zero), the  command
 *	"rm" is "exec"ed to remove all temporary files.   The following
 *	global variables are used in this file:
 *
 *	line	Gives the line  number  in  the  current  file  being
 *		assembled.
 *
 *	file	A character  array that contains the name of the
 *		file being assembled.
 *
 *	fderr	The file descriptor for error output.
 *
 *	filenames  This is an array of pointers to character strings
 *		that contains pointers to all of the file names
 *		obtained from the argument list. This array is used
 *		to obtain the names of the temporary files to be
 *		removed.
 *
 */

#include <stdio.h>
#include <signal.h>
#include "systems.h"
#include "gendefs.h"
I 2
#include "dbug.h"
E 2

#define MAXERRS	30

unsigned short
	line = 1,
	cline = 0; /* `c' line number (from `.ln') */
short	anyerrs = 0;
char	file[134];
char	cfile[15] = { "" }; /* name of `c' source file */

FILE	*fderr;

D 2
char	*filenames[15];
E 2
I 2
char	*filenames[NFILES];
E 2

#if M4ON
short	rflag = NO; /* if set, remove (unlink) input when through */
#endif

onintr()
{
I 2
	DBUG_ENTER ("onintr");
E 2
	signal(SIGINT,onintr);
	signal(SIGTERM,onintr);
	delexit();
I 2
	DBUG_VOID_RETURN;
E 2
}

aerror(str)
	register char *str;
{
I 2
	DBUG_ENTER ("aerror");
E 2
	errmsg("",str);
	delexit();
I 2
	DBUG_VOID_RETURN;
E 2
}

yyerror(str)
	char *str;
{
I 2
	DBUG_ENTER ("yyerror");
E 2
	errmsg("",str);
	if (++anyerrs > MAXERRS) {
		fprintf(stderr,"Too many errors - Goodbye\n");
		delexit();
	}
I 2
	DBUG_VOID_RETURN;
E 2
}

werror(str)
	char *str;
{
I 2
	DBUG_ENTER ("werror");
E 2
	errmsg("Warning: ",str);
I 2
	DBUG_VOID_RETURN;
E 2
}

errmsg(str1,str2)
	char *str1,*str2;
{
	char *msgfile;
	static short firsterr = 1;

I 2
	DBUG_ENTER ("errmsg");
E 2
	if (firsterr) {
		if (cfile[0] == '\0')
			msgfile = file;
		else
			msgfile = cfile;
		fprintf(stderr,"Assembler: %s\n",msgfile);
		firsterr = 0;
	}
	fprintf(stderr,"\taline %u",line);
	if (cline != 0)
		fprintf(stderr,"(cline %u)",cline);
	else
		fprintf(stderr,"\t");
	fprintf(stderr,": %s%s\n",str1,str2);
I 2
	DBUG_VOID_RETURN;
E 2
}

delexit()
{
D 2
	unlink(filenames[1]); /* unlink object (.o) file */
E 2
I 2
	DBUG_ENTER ("delexit");
	unlink(filenames[FDOUT]); /* unlink object (.o) file */
E 2
	deltemps();
	exit(127);
I 2
	DBUG_VOID_RETURN;
E 2
}

deltemps()
{
	register short i;

I 2
	DBUG_ENTER ("deltemps");
E 2
#if M4ON
	if (rflag)
		unlink(file);
#endif
	for (i = 2; i < NFILES; ++i)
		unlink(filenames[i]);
I 2
	DBUG_VOID_RETURN;
E 2
}
E 1
