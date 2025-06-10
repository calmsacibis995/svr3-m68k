h38459
s 00018/00000/00000
d D 1.1 86/07/31 10:25:35 fnf 1 0
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

#ident	"@(#)libc-port:gen/opt_data.c	1.2"
/*
 * Global variables
 * used in getopt
 */
#if SHLIB
int	opterr = 1;
int	optind = 1;
int	optopt = 0;
char	*optarg = 0;
#endif
E 1
