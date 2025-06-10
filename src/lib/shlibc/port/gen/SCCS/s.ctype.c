h28385
s 00048/00000/00000
d D 1.1 86/07/31 10:25:00 fnf 1 0
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

#ident	"@(#)libc-port:gen/ctype.c	1.4"
/*LINTLIBRARY*/
#include <ctype.h>

char	_ctype[] = { 0,

/*	 0	 1	 2	 3	 4	 5	 6	 7  */

/* 0*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 10*/	_C,	_S|_C,	_S|_C,	_S|_C,	_S|_C,	_S|_C,	_C,	_C,
/* 20*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 30*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 40*/	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 50*/	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 60*/	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,
/* 70*/	_N|_X,	_N|_X,	_P,	_P,	_P,	_P,	_P,	_P,
/*100*/	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,
/*110*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*120*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*130*/	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P,
/*140*/	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,
/*150*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*160*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*170*/	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C,
/*200*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0
};
E 1
