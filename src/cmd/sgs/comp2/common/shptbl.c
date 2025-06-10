/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*	@(#)shptbl.c	7.2		*/
# ifndef NSTYSHPS
# define NSTYSHPS 15000
# endif

typedef struct {
	int	sop,
		sleft,
		sright,
		ssh,
		scost,
		scnt;
	char	shname[8];
} STYSHP;

STYSHP	shp[NSTYSHPS];
