/*	@(#)code.c	6.1			*/
#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "codeout.h"
#include "gendefs.h"
#include "dbug.h"

short bitpos = 0;
extern symbol symtab[];
extern short Oflag;
extern long
	newdot;		/* up-to-date value of "." */
extern symbol
	*dot;
extern unsigned short line;
   
codebuf textbuf[TBUFSIZE],
#if MULTSECT
	text1buf[TBUFSIZE],
	text2buf[TBUFSIZE],
	text3buf[TBUFSIZE],
	data1buf[TBUFSIZE],
	data2buf[TBUFSIZE],
	data3buf[TBUFSIZE],
#endif
	initbuf[TBUFSIZE],
	libbuf[TBUFSIZE],
	cmtbuf[TBUFSIZE],
	databuf[TBUFSIZE];

#if MULTSECT
extern char	*filenames[];

static short	t1bufrot = 0;
static short	t2bufrot = 0;
static short	t3bufrot = 0;
static short	d1bufrot = 0;
static short	d2bufrot = 0;
static short	d3bufrot = 0;
#endif

static short dbufrot=0;
static short tbufrot=0;
static short ibufrot=0;
static short lbufrot=0;   
static short cbufrot=0;

extern FILE *fdtext,
#if MULTSECT
	*fdtxt1,
	*fdtxt2,
	*fdtxt3,
	*fddat1,
	*fddat2,
	*fddat3,
#endif
	*fdinit,
	*fdlib,
	*fdcmt,
	*fddata;

generate (nbits,action,value,sym)
		BYTE nbits;
		unsigned short action;
		long value;
		symbol *sym;
{
	FILE  *fd;
	register codebuf *bufptr1,
		*bufptr2;
	register short *count;

	DBUG_ENTER ("generate");
	switch (dot->styp){
		case TXT:{
#if MULTSECT
			switch (dot->sectnum)
			{
			case 0:	fd = fdtext;
				bufptr2 = textbuf;
				bufptr1 = &textbuf[tbufrot++];
				count = &tbufrot;
				break;
			case 1:	fd = fdtxt1;
				bufptr2 = text1buf;
				bufptr1 = &text1buf[t1bufrot++];
				count = &t1bufrot;
				break;
			case 2:	fd = fdtxt2;
				bufptr2 = text2buf;
				bufptr1 = &text2buf[t2bufrot++];
				count = &t2bufrot;
				break;
			case 3:	fd = fdtxt3;
				bufptr2 = text3buf;
				bufptr1 = &text3buf[t3bufrot++];
				count = &t3bufrot;
				break;
			}
#else
			fd = fdtext;
			bufptr2 = textbuf;
			bufptr1 =  &textbuf[tbufrot++];
			count = &tbufrot;
#endif
			break;
		}
		case BSS: {
			if (nbits != 0) {
				yyerror("Attempt to initialize bss");
				DBUG_VOID_RETURN;
			}
			/* otherwise, can write on any file, so treat as DAT */
		}
		case DAT:{
#if MULTSECT
			switch (dot->sectnum)
			{
			case 0:	fd = fddata;
				bufptr2 = databuf;
				bufptr1 = &databuf[dbufrot++];
				count = &dbufrot;
				break;
			case 1:	fd = fddat1;
				bufptr2 = data1buf;
				bufptr1 = &data1buf[d1bufrot++];
				count = &d1bufrot;
				break;
			case 2:	fd = fddat2;
				bufptr2 = data2buf;
				bufptr1 = &data2buf[d2bufrot++];
				count = &d2bufrot;
				break;
			case 3:	fd = fddat3;
				bufptr2 = data3buf;
				bufptr1 = &data3buf[d3bufrot++];
				count = &d3bufrot;
				break;
			}
#else
			fd=fddata;
			bufptr2 = databuf;
			bufptr1 = &databuf[dbufrot++];
			count = &dbufrot;
#endif
			break;
		}
		case INIT:{
			fd=fdinit;
			bufptr2 = initbuf;
			bufptr1 = &initbuf[ibufrot++];
			count = &ibufrot;
			break;
		}
		case LIB:{
			fd=fdlib;
			bufptr2 = libbuf;
			bufptr1 = &libbuf[lbufrot++];
			count = &lbufrot;
			break;
		}
		case CMT:{
			fd=fdcmt;
			bufptr2 = cmtbuf;
			bufptr1 = &cmtbuf[cbufrot++];
			count = &cbufrot;
			break;
		}
	}
	bufptr1->caction = action;
	bufptr1->cnbits = nbits;
	bufptr1->cvalue = value;
#if VAX
	bufptr1->cindex = (int)sym;
#else
	bufptr1->cindex = sym==NULL ? 0 : 1+(sym-symtab);
#endif
	if (*count == TBUFSIZE) {
		fwrite((char *)bufptr2, sizeof(*bufptr2), TBUFSIZE, fd);
		(*count)= 0;
	}
#if DEBUG
 		if (Oflag) {
 			if (bitpos == 0)
 				printf("(%d:%6lx)	",dot->styp,newdot);
 			else
 				printf("		");
 			printf("%hd	%d	%hd	%.13lx	%hd\n",
 				line,(short)nbits,action,value,bufptr1->cindex);
 		}
#endif
	bitpos += nbits;
	newdot += bitpos/BITSPBY;
	bitpos %= BITSPBY;
	DBUG_VOID_RETURN;
}
    
flushbuf()
{
	DBUG_ENTER ("flushbuf");
	fwrite((char *)textbuf, sizeof(*textbuf), tbufrot, fdtext);
	fwrite((char *)databuf, sizeof(*databuf), dbufrot, fddata);
	fwrite((char *)initbuf, sizeof(*initbuf), ibufrot, fdinit);
	fwrite((char *)libbuf, sizeof(*libbuf), lbufrot, fdlib);
	fwrite((char *)cmtbuf, sizeof(*cmtbuf), cbufrot, fdcmt);
#if MULTSECT
	fwrite((char *)text1buf,sizeof(*text1buf),t1bufrot,fdtxt1);
	fwrite((char *)text2buf,sizeof(*text2buf),t2bufrot,fdtxt2);
	fwrite((char *)text3buf,sizeof(*text3buf),t3bufrot,fdtxt3);
	fwrite((char *)data1buf,sizeof(*data1buf),d1bufrot,fddat1);
	fwrite((char *)data2buf,sizeof(*data2buf),d2bufrot,fddat2);
	fwrite((char *)data3buf,sizeof(*data3buf),d3bufrot,fddat3);

	fflush(fdtxt1);
	if (ferror(fdtxt1))
		aerror("trouble writing (text 1); probably out of temp-file space");
	fclose(fdtxt1);
	fflush(fdtxt2);
	if (ferror(fdtxt2))
		aerror("trouble writing (text 2); probably out of temp-file space");
	fclose(fdtxt2);
	fflush(fdtxt3);
	if (ferror(fdtxt3))
		aerror("trouble writing (text 3); probably out of temp-file space");
	fclose(fdtxt3);

	fflush(fddat1);
	if (ferror(fddat1))
		aerror("trouble writing (data 1); probably out of temp-file space");
	fclose(fddat1);
	fflush(fddat2);
	if (ferror(fddat2))
		aerror("trouble writing (data 2); probably out of temp-file space");
	fclose(fddat2);
	fflush(fddat3);
	if (ferror(fddat3))
		aerror("trouble writing (data3); probably out of temp-file space");
	fclose(fddat3);

	if ((fdtxt1 = fopen(filenames[FDTXT1],"r")) == NULL)
		aerror("Unable to open temporary (text 1) file");

	while ((t1bufrot = fread((char *)text1buf,sizeof(*text1buf),TBUFSIZE,fdtxt1)) == TBUFSIZE)
		fwrite((char *)text1buf,sizeof(*text1buf),TBUFSIZE,fdtext);
	fwrite((char *)text1buf,sizeof(*text1buf),t1bufrot,fdtext);
	t1bufrot = 0;

	if ((fdtxt2 = fopen(filenames[FDTXT2],"r")) == NULL)
		aerror("Unable to open temporary (text 2) file");

	while ((t2bufrot = fread((char *)text2buf,sizeof(*text2buf),TBUFSIZE,fdtxt2)) == TBUFSIZE)
		fwrite((char *)text2buf,sizeof(*text2buf),TBUFSIZE,fdtext);
	fwrite((char *)text2buf,sizeof(*text2buf),t2bufrot,fdtext);
	t2bufrot = 0;

	if ((fdtxt3 = fopen(filenames[FDTXT3],"r")) == NULL)
		aerror("Unable to open temporary (text 3) file");

	while ((t3bufrot = fread((char *)text3buf,sizeof(*text3buf),TBUFSIZE,fdtxt3)) == TBUFSIZE)
		fwrite((char *)text3buf,sizeof(*text3buf),TBUFSIZE,fdtext);
	fwrite((char *)text3buf,sizeof(*text3buf),t3bufrot,fdtext);
	t3bufrot = 0;

	if ((fddat1 = fopen(filenames[FDDAT1],"r")) == NULL)
		aerror("Unable to open temporary (data 1) file");

	while ((d1bufrot = fread((char *)data1buf,sizeof(*data1buf),TBUFSIZE,fddat1)) == TBUFSIZE)
		fwrite((char *)data1buf,sizeof(*data1buf),TBUFSIZE,fddata);
	fwrite((char *)data1buf,sizeof(*data1buf),d1bufrot,fddata);
	d1bufrot = 0;

	if ((fddat2 = fopen(filenames[FDDAT2],"r")) == NULL)
		aerror("Unable to open temporary (data 2) file");

	while ((d2bufrot = fread((char *)data2buf,sizeof(*data2buf),TBUFSIZE,fddat2)) == TBUFSIZE)
		fwrite((char *)data2buf,sizeof(*data2buf),TBUFSIZE,fddata);
	fwrite((char *)data2buf,sizeof(*data2buf),d2bufrot,fddata);
	d2bufrot = 0;

	if ((fddat3 = fopen(filenames[FDDAT3],"r")) == NULL)
		aerror("Unable to open temporary (data 3) file");

	while ((d3bufrot = fread((char *)data3buf,sizeof(*data3buf),TBUFSIZE,fddat3)) == TBUFSIZE)
		fwrite((char *)data3buf,sizeof(*data3buf),TBUFSIZE,fddata);
	fwrite((char *)data3buf,sizeof(*data3buf),d3bufrot,fddata);
	d3bufrot = 0;

#endif

 	tbufrot = 0;
	dbufrot = 0;
	ibufrot = 0;
	lbufrot = 0;
	DBUG_VOID_RETURN;
}

long	dotinit = 0L;
long	dotlib = 0L;
long	dotcmt = 0L;

#if MULTSECT
long	dottxt[4],
	dotdat[4],
#else
long dottxt = 0L,
	dotdat = 0L,
#endif
	dotbss = 0L;

#if MULTSECT
cgsect(newtype,newsectnum)
	register short	newtype;
	register short	newsectnum;
#else
cgsect(newtype)
	register short newtype;
#endif
{
	DBUG_ENTER ("cgsect");
	switch(dot->styp){
		case TXT:{
#if MULTSECT
			dottxt[dot->sectnum] = newdot;
#else
			dottxt = newdot;
#endif
			break;
		}
		case DAT:{
#if MULTSECT
			dotdat[dot->sectnum] = newdot;
#else
			dotdat = newdot;
#endif
			break;
		}
		case INIT:{
			dotinit = newdot;
			break;
		}
		case LIB:{
			dotlib = newdot;
			break;
		}
		case CMT:{
			dotcmt = newdot;
			break;
		}
		case BSS:{
			dotbss = newdot;
			break;
		}
	}

	dot->styp = newtype;
#if MULTSECT
	dot->sectnum = newsectnum;
#endif
	switch(newtype){
		case TXT:{
#if MULTSECT
			newdot = dottxt[newsectnum];
#else
			newdot = dottxt;
#endif
			break;
		}
		case DAT:{
#if MULTSECT
			newdot = dotdat[newsectnum];
#else
			newdot = dotdat;
#endif
			break;
		}
		case INIT:{
			newdot = dotinit;
			break;
		}
		case LIB:{
			newdot = dotlib;
			break;
		}
		case CMT:{
			newdot = dotcmt;
			break;
		}
		case BSS:{
			newdot = dotbss;
			break;
		}
	}
	dot->value = newdot;
	DBUG_VOID_RETURN;
}

void comment (string)
char *string;
{
    int i;
    int j;
    int k;
    int oldstyp;
    int oldsectnum;
    int size;
    union {
	long lng;
	char chr[sizeof (long) / sizeof (char)];
    } value;

    DBUG_ENTER ("comment");
    DBUG_3 ("ident", "comment '%s'", string);
    oldstyp = dot -> styp;
#ifdef MULTSECT
    oldsectnum = dot -> sectnum;
    cgsect (CMT, 0);
#else
    cgsect (CMT);
#endif
    size = strlen (string) + 1;
    for (i = 0; i < size / 4; i++) {
	for (j = 0; j < sizeof (long) / sizeof (char); j++) {
	    value.chr[j] = *(string + j + i * 4);
	}
	generate (BITSPW, NOACTION, value, NULLSYM);
    }
    if (size % 4) {
	for (k = 0, j = sizeof (long) / sizeof (char) - size % 4; j < sizeof (long) / sizeof (char); j++, k++) {
	    value.chr[j] = *(string + k + i * 4);
	}
	generate ((size % 4) * BITSPBY, NOACTION, value, NULLSYM);
    }
#ifdef MULTSECT
    cgsect (oldstyp, oldsectnum);
#else
    cgsect (oldstyp);
#endif
    DBUG_VOID_RETURN;
}

