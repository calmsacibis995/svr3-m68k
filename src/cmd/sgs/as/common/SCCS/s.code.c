h57983
s 00065/00000/00376
d D 1.7 86/09/16 15:28:01 fnf 7 6
c Changes to make ident directive work.
e
s 00003/00169/00373
d D 1.6 86/09/15 14:01:05 fnf 6 5
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00542
d D 1.5 86/09/11 13:01:42 fnf 5 4
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00542
d D 1.4 86/08/18 08:47:56 fnf 4 3
c Checkpoint changes at first working shared library version
e
s 00191/00010/00351
d D 1.3 86/08/07 08:02:54 fnf 3 2
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00036/00000/00325
d D 1.2 86/08/04 15:45:18 fnf 2 1
c Initial changes to support "init" and "lib" coff sections for
c shared libraries.
e
s 00325/00000/00000
d D 1.1 86/08/04 09:37:57 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)code.c	6.1			*/
#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "codeout.h"
#include "gendefs.h"
I 3
#include "dbug.h"
E 3

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
I 3
D 6
	init1buf[TBUFSIZE],
	init2buf[TBUFSIZE],
	init3buf[TBUFSIZE],
	lib1buf[TBUFSIZE],
	lib2buf[TBUFSIZE],
	lib3buf[TBUFSIZE],
E 6
E 3
#endif
I 3
	initbuf[TBUFSIZE],
	libbuf[TBUFSIZE],
I 7
	cmtbuf[TBUFSIZE],
E 7
E 3
	databuf[TBUFSIZE];

#if MULTSECT
extern char	*filenames[];

static short	t1bufrot = 0;
static short	t2bufrot = 0;
static short	t3bufrot = 0;
static short	d1bufrot = 0;
static short	d2bufrot = 0;
static short	d3bufrot = 0;
I 3
D 6
static short	i1bufrot = 0;
static short	i2bufrot = 0;
static short	i3bufrot = 0;
static short	l1bufrot = 0;
static short	l2bufrot = 0;
static short	l3bufrot = 0;
E 6
E 3
#endif

static short dbufrot=0;
static short tbufrot=0;
D 3
   
E 3
I 3
static short ibufrot=0;
static short lbufrot=0;   
I 7
static short cbufrot=0;
E 7
E 3

extern FILE *fdtext,
#if MULTSECT
	*fdtxt1,
	*fdtxt2,
	*fdtxt3,
	*fddat1,
	*fddat2,
	*fddat3,
I 3
D 6
	*fdinit1,
	*fdinit2,
	*fdinit3,
	*fdlib1,
	*fdlib2,
	*fdlib3,
E 6
E 3
#endif
I 3
	*fdinit,
	*fdlib,
I 7
	*fdcmt,
E 7
E 3
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

I 3
	DBUG_ENTER ("generate");
E 3
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
D 3
				return;
E 3
I 3
				DBUG_VOID_RETURN;
E 3
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
I 3
		case INIT:{
D 6
#if MULTSECT
			switch (dot->sectnum)
			{
			case 0:	fd = fdinit;
				bufptr2 = initbuf;
				bufptr1 = &initbuf[ibufrot++];
				count = &ibufrot;
				break;
			case 1:	fd = fdinit1;
				bufptr2 = init1buf;
				bufptr1 = &init1buf[i1bufrot++];
				count = &i1bufrot;
				break;
			case 2:	fd = fdinit2;
				bufptr2 = init2buf;
				bufptr1 = &init2buf[i2bufrot++];
				count = &i2bufrot;
				break;
			case 3:	fd = fdinit3;
				bufptr2 = init3buf;
				bufptr1 = &init3buf[i3bufrot++];
				count = &i3bufrot;
				break;
			}
#else
E 6
			fd=fdinit;
			bufptr2 = initbuf;
			bufptr1 = &initbuf[ibufrot++];
			count = &ibufrot;
D 6
#endif
E 6
			break;
		}
		case LIB:{
D 6
#if MULTSECT
			switch (dot->sectnum)
			{
			case 0:	fd = fdlib;
				bufptr2 = libbuf;
				bufptr1 = &libbuf[lbufrot++];
				count = &lbufrot;
				break;
			case 1:	fd = fdlib1;
				bufptr2 = lib1buf;
				bufptr1 = &lib1buf[l1bufrot++];
				count = &l1bufrot;
				break;
			case 2:	fd = fdlib2;
				bufptr2 = lib2buf;
				bufptr1 = &lib2buf[l2bufrot++];
				count = &l1bufrot;
				break;
			case 3:	fd = fdlib3;
				bufptr2 = lib3buf;
				bufptr1 = &lib3buf[l3bufrot++];
				count = &l1bufrot;
				break;
			}
#else
E 6
			fd=fdlib;
			bufptr2 = libbuf;
			bufptr1 = &libbuf[lbufrot++];
			count = &lbufrot;
D 6
#endif
E 6
			break;
		}
I 7
		case CMT:{
			fd=fdcmt;
			bufptr2 = cmtbuf;
			bufptr1 = &cmtbuf[cbufrot++];
			count = &cbufrot;
			break;
		}
E 7
E 3
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
I 3
	DBUG_VOID_RETURN;
E 3
}
    
flushbuf()
{
I 3
	DBUG_ENTER ("flushbuf");
E 3
	fwrite((char *)textbuf, sizeof(*textbuf), tbufrot, fdtext);
	fwrite((char *)databuf, sizeof(*databuf), dbufrot, fddata);
I 3
	fwrite((char *)initbuf, sizeof(*initbuf), ibufrot, fdinit);
	fwrite((char *)libbuf, sizeof(*libbuf), lbufrot, fdlib);
I 7
	fwrite((char *)cmtbuf, sizeof(*cmtbuf), cbufrot, fdcmt);
E 7
E 3
#if MULTSECT
	fwrite((char *)text1buf,sizeof(*text1buf),t1bufrot,fdtxt1);
	fwrite((char *)text2buf,sizeof(*text2buf),t2bufrot,fdtxt2);
	fwrite((char *)text3buf,sizeof(*text3buf),t3bufrot,fdtxt3);
	fwrite((char *)data1buf,sizeof(*data1buf),d1bufrot,fddat1);
	fwrite((char *)data2buf,sizeof(*data2buf),d2bufrot,fddat2);
	fwrite((char *)data3buf,sizeof(*data3buf),d3bufrot,fddat3);
I 3
D 6
	fwrite((char *)init1buf,sizeof(*init1buf),i1bufrot,fdinit1);
	fwrite((char *)init2buf,sizeof(*init2buf),i2bufrot,fdinit2);
	fwrite((char *)init3buf,sizeof(*init3buf),i3bufrot,fdinit3);
	fwrite((char *)lib1buf,sizeof(*lib1buf),l1bufrot,fdlib1);
	fwrite((char *)lib2buf,sizeof(*lib2buf),l2bufrot,fdlib2);
	fwrite((char *)lib3buf,sizeof(*lib3buf),l3bufrot,fdlib3);
E 6
E 3

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
I 3

E 3
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

D 3
	if ((fdtxt1 = fopen(filenames[8],"r")) == NULL)
E 3
I 3
D 6
	fflush(fdinit1);
	if (ferror(fdinit1))
		aerror("trouble writing (init 1); probably out of temp-file space");
	fclose(fdinit1);
	fflush(fdinit2);
	if (ferror(fdinit2))
		aerror("trouble writing (init 2); probably out of temp-file space");
	fclose(fdinit2);
	fflush(fdinit3);
	if (ferror(fdinit3))
		aerror("trouble writing (init3); probably out of temp-file space");
	fclose(fdinit3);

	fflush(fdlib1);
	if (ferror(fdlib1))
		aerror("trouble writing (lib 1); probably out of temp-file space");
	fclose(fdlib1);
	fflush(fdlib2);
	if (ferror(fdlib2))
		aerror("trouble writing (lib 2); probably out of temp-file space");
	fclose(fdlib2);
	fflush(fdlib3);
	if (ferror(fdlib3))
		aerror("trouble writing (lib3); probably out of temp-file space");
	fclose(fdlib3);

E 6
	if ((fdtxt1 = fopen(filenames[FDTXT1],"r")) == NULL)
E 3
		aerror("Unable to open temporary (text 1) file");

	while ((t1bufrot = fread((char *)text1buf,sizeof(*text1buf),TBUFSIZE,fdtxt1)) == TBUFSIZE)
		fwrite((char *)text1buf,sizeof(*text1buf),TBUFSIZE,fdtext);
	fwrite((char *)text1buf,sizeof(*text1buf),t1bufrot,fdtext);
	t1bufrot = 0;

D 3
	if ((fdtxt2 = fopen(filenames[9],"r")) == NULL)
E 3
I 3
	if ((fdtxt2 = fopen(filenames[FDTXT2],"r")) == NULL)
E 3
		aerror("Unable to open temporary (text 2) file");

	while ((t2bufrot = fread((char *)text2buf,sizeof(*text2buf),TBUFSIZE,fdtxt2)) == TBUFSIZE)
		fwrite((char *)text2buf,sizeof(*text2buf),TBUFSIZE,fdtext);
	fwrite((char *)text2buf,sizeof(*text2buf),t2bufrot,fdtext);
	t2bufrot = 0;

D 3
	if ((fdtxt3 = fopen(filenames[10],"r")) == NULL)
E 3
I 3
	if ((fdtxt3 = fopen(filenames[FDTXT3],"r")) == NULL)
E 3
		aerror("Unable to open temporary (text 3) file");

	while ((t3bufrot = fread((char *)text3buf,sizeof(*text3buf),TBUFSIZE,fdtxt3)) == TBUFSIZE)
		fwrite((char *)text3buf,sizeof(*text3buf),TBUFSIZE,fdtext);
	fwrite((char *)text3buf,sizeof(*text3buf),t3bufrot,fdtext);
	t3bufrot = 0;

D 3
	if ((fddat1 = fopen(filenames[11],"r")) == NULL)
E 3
I 3
	if ((fddat1 = fopen(filenames[FDDAT1],"r")) == NULL)
E 3
		aerror("Unable to open temporary (data 1) file");

	while ((d1bufrot = fread((char *)data1buf,sizeof(*data1buf),TBUFSIZE,fddat1)) == TBUFSIZE)
		fwrite((char *)data1buf,sizeof(*data1buf),TBUFSIZE,fddata);
	fwrite((char *)data1buf,sizeof(*data1buf),d1bufrot,fddata);
	d1bufrot = 0;

D 3
	if ((fddat2 = fopen(filenames[12],"r")) == NULL)
E 3
I 3
	if ((fddat2 = fopen(filenames[FDDAT2],"r")) == NULL)
E 3
		aerror("Unable to open temporary (data 2) file");

	while ((d2bufrot = fread((char *)data2buf,sizeof(*data2buf),TBUFSIZE,fddat2)) == TBUFSIZE)
		fwrite((char *)data2buf,sizeof(*data2buf),TBUFSIZE,fddata);
	fwrite((char *)data2buf,sizeof(*data2buf),d2bufrot,fddata);
	d2bufrot = 0;

D 3
	if ((fddat3 = fopen(filenames[13],"r")) == NULL)
E 3
I 3
	if ((fddat3 = fopen(filenames[FDDAT3],"r")) == NULL)
E 3
		aerror("Unable to open temporary (data 3) file");

	while ((d3bufrot = fread((char *)data3buf,sizeof(*data3buf),TBUFSIZE,fddat3)) == TBUFSIZE)
		fwrite((char *)data3buf,sizeof(*data3buf),TBUFSIZE,fddata);
	fwrite((char *)data3buf,sizeof(*data3buf),d3bufrot,fddata);
	d3bufrot = 0;
I 3

D 6
	if ((fdinit1 = fopen(filenames[FDINIT1],"r")) == NULL)
		aerror("Unable to open temporary (init 1) file");

	while ((i1bufrot = fread((char *)init1buf,sizeof(*init1buf),TBUFSIZE,fdinit1)) == TBUFSIZE)
		fwrite((char *)init1buf,sizeof(*init1buf),TBUFSIZE,fdinit);
	fwrite((char *)init1buf,sizeof(*init1buf),i1bufrot,fdinit);
	i1bufrot = 0;

	if ((fdinit2 = fopen(filenames[FDINIT2],"r")) == NULL)
		aerror("Unable to open temporary (init 2) file");

	while ((i2bufrot = fread((char *)init2buf,sizeof(*init2buf),TBUFSIZE,fdinit2)) == TBUFSIZE)
		fwrite((char *)init2buf,sizeof(*init2buf),TBUFSIZE,fdinit);
	fwrite((char *)init2buf,sizeof(*init2buf),i2bufrot,fdinit);
	i2bufrot = 0;

	if ((fdinit3 = fopen(filenames[FDINIT3],"r")) == NULL)
		aerror("Unable to open temporary (init 3) file");

	while ((i3bufrot = fread((char *)init3buf,sizeof(*init3buf),TBUFSIZE,fdinit3)) == TBUFSIZE)
		fwrite((char *)init3buf,sizeof(*init3buf),TBUFSIZE,fdinit);
	fwrite((char *)init3buf,sizeof(*init3buf),i3bufrot,fdinit);
	i3bufrot = 0;

	if ((fdlib1 = fopen(filenames[FDLIB1],"r")) == NULL)
		aerror("Unable to open temporary (lib 1) file");

	while ((l1bufrot = fread((char *)lib1buf,sizeof(*lib1buf),TBUFSIZE,fdlib1)) == TBUFSIZE)
		fwrite((char *)lib1buf,sizeof(*lib1buf),TBUFSIZE,fdlib);
	fwrite((char *)lib1buf,sizeof(*lib1buf),l1bufrot,fdlib);
	l1bufrot = 0;

	if ((fdlib2 = fopen(filenames[FDLIB2],"r")) == NULL)
		aerror("Unable to open temporary (lib 2) file");

	while ((l2bufrot = fread((char *)lib2buf,sizeof(*lib2buf),TBUFSIZE,fdlib2)) == TBUFSIZE)
		fwrite((char *)lib2buf,sizeof(*lib2buf),TBUFSIZE,fdlib);
	fwrite((char *)lib2buf,sizeof(*lib2buf),l2bufrot,fdlib);
	l2bufrot = 0;

	if ((fdlib3 = fopen(filenames[FDLIB3],"r")) == NULL)
		aerror("Unable to open temporary (lib 3) file");

	while ((l3bufrot = fread((char *)lib3buf,sizeof(*lib3buf),TBUFSIZE,fdlib3)) == TBUFSIZE)
		fwrite((char *)lib3buf,sizeof(*lib3buf),TBUFSIZE,fdlib);
	fwrite((char *)lib3buf,sizeof(*lib3buf),l3bufrot,fdlib);
	l3bufrot = 0;
E 6
E 3
#endif
D 3
	tbufrot = 0;
	dbufrot = 0;
E 3

I 3
 	tbufrot = 0;
	dbufrot = 0;
	ibufrot = 0;
	lbufrot = 0;
	DBUG_VOID_RETURN;
E 3
}

I 6
long	dotinit = 0L;
long	dotlib = 0L;
I 7
long	dotcmt = 0L;
E 7

E 6
#if MULTSECT
long	dottxt[4],
	dotdat[4],
I 2
D 6
	dotinit[4],
	dotlib[4],
E 6
E 2
#else
long dottxt = 0L,
	dotdat = 0L,
I 2
D 6
	dotinit = 0L,
	dotlib = 0L,
E 6
E 2
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
I 3
	DBUG_ENTER ("cgsect");
E 3
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
I 2
		case INIT:{
D 6
#if MULTSECT
			dotinit[dot->sectnum] = newdot;
#else
E 6
			dotinit = newdot;
D 6
#endif
E 6
			break;
		}
		case LIB:{
D 6
#if MULTSECT
			dotlib[dot->sectnum] = newdot;
#else
E 6
			dotlib = newdot;
D 6
#endif
E 6
			break;
		}
I 7
		case CMT:{
			dotcmt = newdot;
			break;
		}
E 7
E 2
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
I 2
#endif
			break;
		}
		case INIT:{
D 6
#if MULTSECT
			newdot = dotinit[newsectnum];
#else
E 6
			newdot = dotinit;
D 6
#endif
E 6
			break;
		}
		case LIB:{
D 6
#if MULTSECT
			newdot = dotlib[newsectnum];
#else
E 6
			newdot = dotlib;
E 2
D 6
#endif
E 6
			break;
		}
I 7
		case CMT:{
			newdot = dotcmt;
			break;
		}
E 7
		case BSS:{
			newdot = dotbss;
			break;
		}
	}
	dot->value = newdot;
I 3
	DBUG_VOID_RETURN;
E 3
}
I 7

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

E 7
E 1
