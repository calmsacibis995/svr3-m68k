h51291
s 00098/00000/00000
d D 1.1 86/07/31 10:25:36 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
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

#ident	"@(#)libc-port:gen/putenv.c	1.4"
/*	LINTLIBRARY	*/
/*	putenv - change environment variables

	input - char *change = a pointer to a string of the form
			       "name=value"

	output - 0, if successful
		 1, otherwise
*/
#define NULL 0
extern char **environ;		/* pointer to enviroment */
static reall = 0;		/* flag to reallocate space, if putenv is called
				   more than once */

int
putenv(change)
char *change;
{
	char **newenv;		    /* points to new environment */
	register int which;	    /* index of variable to replace */
	char *realloc(), *malloc(); /* memory alloc routines */

	if ((which = find(change)) < 0)  {
		/* if a new variable */
		/* which is negative of table size, so invert and
		   count new element */
		which = (-which) + 1;
		if (reall)  {
			/* we have expanded environ before */
			newenv = (char **)realloc(environ,
				  which*sizeof(char *));
			if (newenv == NULL)  return -1;
			/* now that we have space, change environ */
			environ = newenv;
		} else {
			/* environ points to the original space */
			reall++;
			newenv = (char **)malloc(which*sizeof(char *));
			if (newenv == NULL)  return -1;
			(void)memcpy((char *)newenv, (char *)environ,
 				(int)(which*sizeof(char *)));
			environ = newenv;
		}
		environ[which-2] = change;
		environ[which-1] = NULL;
	}  else  {
		/* we are replacing an old variable */
		environ[which] = change;
	}
	return 0;
}

/*	find - find where s2 is in environ
 *
 *	input - str = string of form name=value
 *
 *	output - index of name in environ that matches "name"
 *		 -size of table, if none exists
*/
static
find(str)
register char *str;
{
	register int ct = 0;	/* index into environ */

	while(environ[ct] != NULL)   {
		if (match(environ[ct], str)  != 0)
			return ct;
		ct++;
	}
	return -(++ct);
}
/*
 *	s1 is either name, or name=value
 *	s2 is name=value
 *	if names match, return value of 1,
 *	else return 0
 */

static
match(s1, s2)
register char *s1, *s2;
{
	while(*s1 == *s2++)  {
		if (*s1 == '=')
			return 1;
		s1++;
	}
	return 0;
}
E 1
