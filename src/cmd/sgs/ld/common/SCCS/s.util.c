h18833
s 00000/00000/00437
d D 1.3 86/08/18 08:45:34 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00028/00014/00409
d D 1.2 86/07/30 14:25:08 fnf 2 1
c Add macro based C debugging package macros.
e
s 00423/00000/00000
d D 1.1 86/07/29 14:32:00 fnf 1 0
c Baseline code from 5.3 release for 3b2
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

#ident	"@(#)ld:common/util.c	1.15"
#include "system.h"

#include <stdio.h>
#include <signal.h>
#include "structs.h"
#include "extrns.h"
#include "sgs.h"
#include "sgsmacros.h"
#include "y.tab.h"
#include "reloc.h"
#include "slotvec.h"
I 2
#include "dbug.h"		/* Special macro based debugging package */
E 2

D 2

E 2
/*eject*/
ACTITEM *
dfnscngrp(type, bondaddr, align, block)
int type;
ENODE *bondaddr, *align;
ADDRESS *block;
{

/*
 * Build a AIDFNSCN/AIDFNGRP action item
 */

	ACTITEM *p;

I 2
	DBUG_ENTER ("dfnscngrp");
E 2
	p = (ACTITEM *) mycalloc(sizeof(ACTITEM));

	p->dfnscn.aiinflnm = curfilnm;
	p->dfnscn.aiinlnno = lineno;
	p->dfnscn.aitype = type;
	p->dfnscn.aibndadr = NULL;
	p->dfnscn.aialign  = NULL;
	p->dfnscn.aiblock  =  0L;

	if ( bondaddr != NULL ) 	/* there is a bond */
		p->dfnscn.aibndadr = bondaddr;
	if (align != NULL) 		/* there is alignment */
		p->dfnscn.aialign = align;

	if (block) {
		p->dfnscn.aiblock = *block;
		chkpower2(*block);
		}

D 2
	return(p);
E 2
I 2
	DBUG_RETURN(p);
E 2
}
/*eject*/
chkpower2(n)
ADDRESS n;
{

/*
 * Determine if 'n' is a power of two
 */

	ADDRESS c;

I 2
	DBUG_ENTER ("chkpower2");
E 2
	for( c = n; c > 2; c >>= 1 )
		if( (c % 2) == 1 ) {
			lderror(0, lineno, curfilnm, "%.1lx is not a power of 2", n );
			break;
			}
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
char *
zero(ptr, cnt)
char *ptr;
int cnt;
{

/*
 * Zero "cnt" bytes starting at the address "ptr".
 * Return "ptr".
 */

	register char *p, *w;

I 2
	DBUG_ENTER ("zero");
E 2
	if( cnt > 0 ) {
		if( ((short) (p = ptr)) & 1 ) {
			*p++ = '\0';
			--cnt;
			}

		w = p + (cnt & (~1));
		while (p < w) {
			*((short *) p) = 0;
			p += sizeof(short);
			}

		if (cnt & 1)
			*p = '\0';
		}
	
D 2
	return( ptr );
E 2
I 2
	DBUG_RETURN( ptr );
E 2
}
/*eject*/
zerofile(fildes,bptr,bsiz)
	FILE		*fildes;	/* write opened file */
	unsigned	*bptr;		/* zero filled buffer */
	int		bsiz;		/* size of buffer */
{
	long	file_end;
	OUTSECT	*osptr;
/*
 * Insure that fildes is zero filled up to the point
 * of symbol table origin, or up to the last section if the symbol
 * table is stripped. Under TS, we simply seek to the point
 * and write one unsigned. Under RT or DMERT we must write 0's
 * throughout the entire space.
 */
I 2

	DBUG_ENTER ("zerofile");
E 2
	if (!sflag)
		file_end = symtborg - 4;
	else
	{
		file_end = 0L;
		for (osptr = (OUTSECT *) outsclst.head; osptr; osptr = osptr->osnext)
			file_end = max( file_end, osptr->oshdr.s_scnptr );

	}

#if RT
	rewind(fildes);
	while ( file_end > ftell(fildes) ) 
		fwrite( (char*) bptr, bsiz, 1, fildes);
#else
	fseek(fildes, file_end, 0);
	fwrite( (char *) bptr, sizeof(long), 1, fildes);
#endif
	rewind(fildes);		/* leave everythiing as we found it */

D 2
	return;
E 2
I 2
	DBUG_VOID_RETURN;
E 2
}
/*eject*/
char *
sname(s)
char *s;
{

/*
 * Returns pointer to "simple" name of path name; that is,
 * pointer to first character after last '/'.  If no slashes,
 * returns pointer to first char of arg.
 *
 * If the string ends in a slash:
 *	1. replace the terminating slash with '\0'
 *	2. return a pointer to the first character after the last
 *		'/', or the first character in the string
 */

	register char *p;

I 2
	DBUG_ENTER ("sname");
E 2
	p = s;
	while( *p )
		if(*p++ == '/')
			if( *p == '\0' ) {
				*(p-1) = '\0';
				break;
				}
			else
				s = p;

D 2
	return(s);
E 2
I 2
	DBUG_RETURN(s);
E 2
}




char *
myalloc(nbytes)
int nbytes;
{

/*
 * Allocate "nbytes" of memory, and exit from ld
 * upon failure.
 */

	register char *mem;
	extern char *malloc();

I 2
	DBUG_ENTER ("myalloc");
E 2
	if ((mem = malloc( nbytes )) == NULL )
	{
		lderror(0,0,NULL,
			"memory allocation failure on %d-byte 'malloc' call",
			nbytes);
		lderror(2,0,NULL, "%s run is too large and complex", SGS );
	}

#if PERF_DATA
	allocspc += (long) nbytes;
#endif
D 2
	return(mem);
E 2
I 2
	DBUG_RETURN(mem);
E 2
}


char *
mycalloc(nbytes)
int nbytes;
{

/*
 * Allocate "nbytes" of ZERO'D OUT memory, and exit from ld
 * upon failure.
 */

	register char *mem;
	extern char *calloc();

I 2
	DBUG_ENTER ("mycalloc");
E 2
	if( (mem = calloc(1,nbytes)) == NULL )
	{
		lderror(0,0,NULL,
			"memory allocation failure on %d-byte 'calloc' call",
			nbytes);
		lderror(2,0,NULL, "%s run is too large and complex", SGS );
	}

#if PERF_DATA
	allocspc += (long) nbytes;
#endif
D 2
	return(mem);
E 2
I 2
	DBUG_RETURN(mem);
E 2
}


stoi(p)
char *p;
{

/*
 * Given a string of digits, perhaps beginning with "0" or "0x",
 * convert to an integer
 *
 * Return 0x8000 on any type of error
 */

	register int base, num;
	register char limit;

I 2
	DBUG_ENTER ("stoi");
E 2
	num = 0;
	
	if (*p == '0') {
		if (*++p == 'x' || *p == 'X') {		/* string is hex */
			for (p++; *p; p++) {
				num <<= 4;
				if (*p >= '0' && *p <= '9')
					num += *p - '0';
				else if (*p >= 'a' && *p <= 'f')
					num += *p - 'a' + 10;
				else if (*p >= 'A' && *p <= 'F')
					num += *p - 'A' + 10;
D 2
				else return(0x8000);
E 2
I 2
				else DBUG_RETURN(0x8000);
E 2
				}
D 2
			return (num);
E 2
I 2
			DBUG_RETURN (num);
E 2
			}
		else {
			base = 8;
			limit = '7';
			}
		}
	else {
		base = 10;
		limit = '9';
		}

	for ( ; *p; p++)
		if( (*p >= '0')  &&  (*p <= limit) )
			num = num * base + *p - '0';
		else
D 2
			return(0x8000);
E 2
I 2
			DBUG_RETURN(0x8000);
E 2

D 2
	return(num);
E 2
I 2
	DBUG_RETURN(num);
E 2
}



/*eject*/
/*VARARGS*/
lderror(lvl, ln,fln,strng, a1,a2,a3,a4,a5,a6,a7,a8,a9,aa)
int lvl;		/* error severity level */
int ln;			/* line in ifile containing the error */
char *fln;		/* name of the ifile containing the error */
char *strng;		/* error message format */
int a1,a2,a3,a4,a5,	/* error message format arguments */
    a6,a7,a8,a9,aa;
{

/*
 *  Prepend a filename and a line number to an error message.
 *  used for printing all parsing-related errors.
 */

I 2
	DBUG_ENTER ("lderror");
E 2
	errlev = (errlev < lvl ) ? lvl : errlev;

	if( Sflag  &&  (lvl != 2) )
D 2
		return;
E 2
I 2
		DBUG_VOID_RETURN;
E 2

	fprintf(stderr, "%sld", SGS);
	if (fln != NULL)
		fprintf(stderr, " %s", sname(fln));
	if ( ln > 0 )
		fprintf(stderr, " %d", ln) ;

	switch(lvl) {
	case 0:
		fprintf(stderr, " warning: ");
		break;
	case 2:
		fprintf(stderr, " fatal: ");
		break;
	default:
		fprintf(stderr, ": ");
		break;
	}

	fprintf(stderr, strng, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa);
	fprintf(stderr, "\n");
	fflush(stderr);

	if(lvl > 1)
		ldexit();
I 2
	DBUG_VOID_RETURN;
E 2
}




ENODE *
cnstnode(val)
long val;
{

/*
 * Build an expression node for a constant
 */

	register ENODE *p;

I 2
	DBUG_ENTER ("cnstnode");
E 2
	p = (ENODE *) myalloc(sizeof(ENODE));

	p->vnode.exop = INT;
	p->vnode.exvalue = val;

D 2
	return(p);
E 2
I 2
	DBUG_RETURN(p);
E 2
}




long
copy_section( infile, infl, isp, osp, fdes, buffer, buf_size )
	register FILE	*infile;
	INFILE	*infl;
	INSECT	*isp;
	OUTSECT	*osp;
	register FILE	*fdes;
	char	*buffer;
	long	buf_size;
{
	register long	more;
	register long	num_bytes;
	int relcnt, offset;
	long shlibsize;
	RELOC rentry;
	long vaddiff, numshlibs = 0;
	SLOTVEC	*svp;

I 2
	DBUG_ENTER ("copy_section");
E 2
	more = isp->ishdr.s_size;
	while ( more )
	{
		num_bytes = min( buf_size, more );
		if (fread( buffer, (int) num_bytes, 1, infile ) != 1
			|| fwrite( buffer, (int) num_bytes, 1, fdes ) != 1)
			lderror( 2, 0, NULL, "cannot copy section %.8s of file %s", isp->ishdr.s_name, infl->flname);
		more -= num_bytes;
		if (osp->oshdr.s_flags & STYP_LIB) {
			if (numshlibs == 0) {
				shlibsize = ((long *) buffer)[0];
				numshlibs++;
				}
			offset = 0;
			while (num_bytes) {
				if (num_bytes > (shlibsize * sizeof(long))) {
					numshlibs++;
					offset += shlibsize;
					num_bytes -= (shlibsize * sizeof(long));
					shlibsize = ((long *) buffer)[offset];
					}
				else {
					shlibsize -= (num_bytes/sizeof(long));
					num_bytes = 0;
					}
				}
			}
	}
	if (rflag && (osp->oshdr.s_flags & STYP_COPY)) {
		vaddiff = isp->isnewvad - isp->ishdr.s_vaddr;
		fseek(infile, isp->ishdr.s_relptr + infl->flfiloff, 0);
		fseek(fdes, osp->oshdr.s_relptr + isp->isrldisp, 0);
		for ( relcnt = 1; relcnt <= isp->ishdr.s_nreloc; relcnt++ ) {
			if (fread( &rentry, RELSZ, 1, infile ) != 1)
				lderror( 2, 0, NULL, "cannot read relocation entries of section %.8s of %s",
					isp->ishdr.s_name, infl->flname );

			if ((svp = svread( rentry.r_symndx)) == NULL) {
				lderror(1, 0, NULL, "relocation entry found for non-relocatable symbol in %s",
					infl->flname);
				continue;
			}
				rentry.r_vaddr += vaddiff;
				rentry.r_symndx = svp->svnewndx;
				fwrite(&rentry, RELSZ, 1, fdes);
		}
	}
D 2
	return(numshlibs);
E 2
I 2
	DBUG_RETURN(numshlibs);
E 2
}
E 1
