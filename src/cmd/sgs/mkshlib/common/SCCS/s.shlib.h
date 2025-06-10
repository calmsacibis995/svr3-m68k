h10076
s 00000/00000/00061
d D 1.3 86/08/18 08:49:21 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00002/00000/00059
d D 1.2 86/08/11 09:54:47 fnf 2 1
c Checkpoint changes to date.
e
s 00059/00000/00000
d D 1.1 86/07/31 14:39:25 fnf 1 0
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

#ident	"@(#)mkshlib:common/shlib.h	1.3"

#define FALSE	0
#define TRUE	1

extern char	*specname;
extern char	*trgname;
extern char	*hstname;
extern char	*idstr,
		*progname,
		*prefix,
		*ldname,
		*asname;
extern char	**trgobjects;
extern long	numobjs;
extern int	maketarg,
		makehost,
I 2
		dflag,		/* Special debugging flag */
		vflag,		/* Verbose mode (show exec'd lines) */
E 2
		qflag;


/* System supplied functions */
extern FILE     *fopen();
extern char     *strcat(), *strcpy(), *strncpy(), *strchr(), *strrchr(), *strtok();
extern char     *malloc(), *realloc(), *calloc(), *ldgetname();
extern long     time(), strtol();
extern void	exit(), free(), perror();
extern LDFILE	*ldopen();


/* User supplied functions */
extern void	catchsigs();
extern void	cmdline();
extern char	*concat();
extern int	copy();
extern void	dexit();
extern void	directive();
extern int	execute();
extern void	fatal();
extern char	*getbase();
extern char	*getident();
extern char	*getname();
extern char	*getpref();
extern char	*gettok();
extern long	hash();
extern char	*makename();
extern void	mktmps();
extern void	rdspec();
extern void	rm();
extern void	rmtmps();
extern void	spec();
extern char	*stralloc();
extern void	usagerr();
E 1
