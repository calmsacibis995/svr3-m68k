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
#include "dbug.h"

#define MAXERRS	30

unsigned short
	line = 1,
	cline = 0; /* `c' line number (from `.ln') */
short	anyerrs = 0;
char	file[134];
char	cfile[15] = { "" }; /* name of `c' source file */

FILE	*fderr;

char	*filenames[NFILES];

#if M4ON
short	rflag = NO; /* if set, remove (unlink) input when through */
#endif

onintr()
{
	DBUG_ENTER ("onintr");
	signal(SIGINT,onintr);
	signal(SIGTERM,onintr);
	delexit();
	DBUG_VOID_RETURN;
}

aerror(str)
	register char *str;
{
	DBUG_ENTER ("aerror");
	errmsg("",str);
	delexit();
	DBUG_VOID_RETURN;
}

yyerror(str)
	char *str;
{
	DBUG_ENTER ("yyerror");
	errmsg("",str);
	if (++anyerrs > MAXERRS) {
		fprintf(stderr,"Too many errors - Goodbye\n");
		delexit();
	}
	DBUG_VOID_RETURN;
}

werror(str)
	char *str;
{
	DBUG_ENTER ("werror");
	errmsg("Warning: ",str);
	DBUG_VOID_RETURN;
}

errmsg(str1,str2)
	char *str1,*str2;
{
	char *msgfile;
	static short firsterr = 1;

	DBUG_ENTER ("errmsg");
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
	DBUG_VOID_RETURN;
}

delexit()
{
	DBUG_ENTER ("delexit");
	unlink(filenames[FDOUT]); /* unlink object (.o) file */
	deltemps();
	exit(127);
	DBUG_VOID_RETURN;
}

deltemps()
{
	register short i;

	DBUG_ENTER ("deltemps");
#if M4ON
	if (rflag)
		unlink(file);
#endif
	for (i = 2; i < NFILES; ++i)
		unlink(filenames[i]);
	DBUG_VOID_RETURN;
}
