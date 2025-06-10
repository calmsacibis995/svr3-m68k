h62055
s 00034/00000/00000
d D 1.1 86/07/31 10:25:45 fnf 1 0
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

#ident	"@(#)libc-port:gen/strdup.c	1.7"
/*LINTLIBRARY*/
/* string duplication
   returns pointer to a new string which is the duplicate of string
   pointed to by s1
   NULL is returned if new string can't be created
*/

#include <string.h>
#ifndef NULL
#define NULL	0
#endif

extern int strlen();
extern char *malloc();

char *
strdup(s1) 

   char * s1;

{  
   char * s2;

   s2 = malloc((unsigned) strlen(s1)+1) ;
   return(s2==NULL ? NULL : strcpy(s2,s1) );
}
E 1
