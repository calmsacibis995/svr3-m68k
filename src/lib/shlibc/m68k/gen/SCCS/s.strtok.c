h13872
s 00042/00000/00000
d D 1.1 86/07/31 12:17:09 fnf 1 0
c Initial copy from 5.2 Generic M68000 distribution.
e
u
U
t
T
I 1
/*	@(#)strtok.c	6.1	
/*	strtok.c	1.1 UNIX System V/68	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * uses strpbrk and strspn to break string into tokens on
 * sequentially subsequent calls.  returns NULL when no
 * non-separator characters remain.
 * `subsequent' calls are calls with first argument NULL.
 */

#define	NULL	(char*)0

extern int strspn();
extern char *strpbrk();

char *
strtok(string, sepset)
char	*string, *sepset;
{
	register char	*p, *q, *r;
	static char	*savept;

	/*first or subsequent call*/
	p = (string == NULL)? savept: string;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, sepset);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = strpbrk(q, sepset)) == NULL)	/* move past token */
		savept = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept = ++r;
	}
	return(q);
}
E 1
