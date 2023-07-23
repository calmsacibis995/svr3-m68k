/*
 * File name:	csoffset.c
 *
 * Description: 
 *	Filter program to translate characters, expand tabs, insert 
 *	carriage returns, and offset text.
 *
 * Copyright 1985 by ISG, Motorola Inc.
 *
 * Revision history:
 */

#define PROGID	"@(#)csoffset.c	1.13 - 7/16/85"
#define COPYRT	"@(#)Copyright (C) 1985  by  Four Phase Systems, Inc."

/*
#define DEBUG 1
 */

#define CSMAXSIZ	1
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cstty.h"
#include "cs.h"

#define TRUE 1
#define FALSE 0
#define LOCAL_MODE	5

/* Structure to define translation steps.
 */
static struct {		int	siz;
			int	mode;
			char	*ttname;
			int	opt;
			CSDATP	datp;
} d[] = {
		/* specify the translation steps */
	{1024,	CS_XS2I16, NULL,	0},	/* Xerox string to int */
						/*   opt is changed if Ifmt7 */
	{2048,	CS_I162I16, "cs_tostd",	0},	/* int to std int */
	{2048,	LOCAL_MODE, NULL,	0},	/* offset and tab processing */
	{1024,	CS_I162XS, NULL, 	0},	/* int to Xerox string */
						/*   changed if t=... param */
	{0} };

static char	Progid[] = PROGID;
static char	Copyrt[] = COPYRT;
char	*Prog;

struct csbuf8	*Ibuf;
struct csbuf8	*Obuf;

int	Exptabs = FALSE;	/* Expand tabs to every 8 chars after offset */
int	Onlcr = FALSE;		/* Output CR before each NL */
int	Ifmt7 = FALSE;		/* Allow 7-bit format of Xerox input strings */
int	Offval = 0;		/* Number of chars to offset the output */

/* Error Messages.
 */
#define BADFNAME	"%s:  can't access character set translation file %s\n"
#define BADFILE		"%s:  %s is not a valid character set translation file \n"
#define BADMALLOC	"%s:  unable to allocate space to build translation \
table\n"
#define BADOP		"%s:  invalid option %s\n"
#define OVRFLO		"%s:  array overflow while building translation table\n"
#define USAGE		"usage:\n\tcsoffset [-tabs] [onlcr] [<offset>] [-if7] \
[t=<filename>]\n"


CSDATP buildat (size, prev, mode, ttname, opt)
    int		size;
    CSDATP	prev;
    int		mode;
    char	*ttname;
    int		opt;
{
	register CSDATP		datp;
	int			rslt;
	struct csttbl		*ttbl;

	datp = (CSDATP) malloc (sizeof (*datp));
	if (prev == NULL) {
	    int			firstsize = 1024;

	    datp->ibuf = (CSBUFP) malloc (sizeof (struct csbuf8));
	    datp->ibuf->ch.base = (unsigned char *) malloc (firstsize);
	    datp->ibuf->ch.out = datp->ibuf->ch.in = datp->ibuf->ch.base;
	    datp->ibuf->ch.lim = datp->ibuf->ch.base + firstsize;
	    Ibuf = &datp->ibuf->ch;
	} else {
	    datp->ibuf = prev->obuf;
	}

	datp->obuf = (CSBUFP) malloc (sizeof (struct csbuf8));
	datp->obuf->ch.base = (unsigned char *) malloc (size);
	datp->obuf->ch.out = datp->obuf->ch.in = datp->obuf->ch.base;
	datp->obuf->ch.lim = datp->obuf->ch.base + size;

	datp->mode = mode;
	if (ttname == NULL || *ttname == '\0') 
	    datp->tt = NULL;
	else {
	    if (0 == strcmp(ttname,"cs_tostd")) datp->tt = &cs_tostd;
	    else
	    if (0 == strcmp(ttname,"cs_topri")) datp->tt = &cs_topri;
	    else {
		ttbl = csinit(ttname, 0, &rslt);
		if (rslt != 0) {
		    if (rslt == CI_BADFILE)
			fprintf(stderr, BADFILE, Prog, ttname);
		    else if (rslt == CI_BADMALLOC)
			fprintf(stderr, BADMALLOC, Prog);
		    else if (rslt == CI_OVRFLO)
			fprintf(stderr, OVRFLO, Prog);
		    exit(0 - rslt);
		}
		if (ttbl == NULL) exit(2);
		datp->tt = &ttbl->cs_u.cs_hdr;
	    }
	}
	datp->opt = opt;

	return (datp);
}

/*
 * Function:	main
 *
 * Description:
 *	Main driver for csoffset.
 *
 * Parameters:	argc, argv.
 *
 * Globals:	Prog, d, Obuf, Ibuf.
 *
 * Inputs:
 *	Reads standard input for characters to translate.
 *
 * Outputs:
 *	Writes translated characters on standard output.
 *
 * Calls:
 *	buildat		Build translation data structures.
 *	cstrans		Perform character set translation.
 *	trlocal		Perform local translation (tabs, carriage returns,
 *			and page offset).
 *
 * Returns:
 *	0	Success.
 *	< 0	System error.
 *	> 0	User error.
 *
 * Status:	Unit-tested.
 *
 * Pseudo-code:
 *	Get command line arguments
 *	Call buildat to build translation data structures
 *	While input
 *		Read a buffer in input from standard input
 *		Perform translation
 *		Write translated characters of standard out
 *
 *
 */
main (argc, argv)
    int		argc;
    char	*argv[];
{
	register int 		 j;
	register CSDATP		 datp;
	int			 rslt;
	short			 endfile;
	int			 ifile;
	int			 ofile;
	int			 c, i;
	struct stat		 statbuf;
	char			*ttable;
	int			 badop;

	Prog = *argv;
	badop = 0;
	while (--argc > 0)
	{
	    argv++;
	    if (0 == strncmp(*argv, "t=", 2)) {
		ttable = (*argv) + 2;
		if (0 == stat(ttable, &statbuf)) {
		    d[3].mode = CS_I162EDC;
		    d[3].ttname = ttable;
		}
		else {
		    fprintf (stderr, BADFNAME, Prog, ttable);
		    exit(1);
		}
	    }
	    else if (0 == strcmp(*argv, "-tabs"))
		Exptabs = TRUE;
	    else if (0 == strcmp(*argv, "onlcr"))
		Onlcr = TRUE;
	    else if (0 == strcmp(*argv, "-if7"))
		Ifmt7 = TRUE;
	    else if ('0' <= **argv && **argv <= '9')
		Offval = atoi (*argv);
	    else {
		fprintf(stderr, BADOP, Prog, *argv);
		badop = 1;
	    }
	}

	if (badop) {
	    fprintf(stderr, USAGE);
	    exit(1);
	}

	if (Ifmt7)
	    d[0].opt |= CSFMT7;

	datp = NULL;
	for (j=0; d[j].siz; j++) {
	    datp = buildat (d[j].siz, datp, d[j].mode, d[j].ttname, d[j].opt);
	    d[j].datp = datp;
	}
	Obuf = &datp->obuf->ch;
	d[j].datp = NULL;

	ifile = fileno(stdin);
	ofile = fileno(stdout);

	do {
	    
	    if (ifile != -1 && Ibuf->out == Ibuf->in) {
		rslt = read (ifile, Ibuf->base, Ibuf->lim - Ibuf->base);
		if (rslt < 0) {
		    perror ("csoffset:  read");
		    return (-1);
		} else if (rslt == 0) {
		    close (ifile);
		    ifile = -1;
		}
		Ibuf->out = Ibuf->base;
		Ibuf->in = Ibuf->base + rslt;
	    }
	    
	    endfile = (ifile == -1);
	    for (j=0; datp = d[j].datp; j++) {
		if (datp->mode == LOCAL_MODE)
		    trlocal(datp);
		else
		    cstrans(datp);
		if (datp->ibuf->ch.out < datp->ibuf->ch.in)
		    endfile = 0;
	    }

	    if (Obuf->in == Obuf->lim || endfile) {
		rslt = write (ofile, Obuf->out, Obuf->in - Obuf->out);
		if (rslt < 0) {
		    perror ("csoffset:  write");
		    return (-1);
		}
		
		Obuf->out += rslt;
		if (Obuf->in == Obuf->out) {
		    Obuf->out = Obuf->base;
		    Obuf->in = Obuf->base;
		}
	    }
	    if (Obuf->out < Obuf->in)
		endfile = 0;
	} while (endfile == 0);
	return (0);
}


/* 
 * Function:	trlocal

 * Description:	Handle CR/newline, offset and tab expansion on the
		internal 16-bit characters, where it can be cone
		correctly.

 * Parameters:	csdp - a CSDATP pointer to the buffers to use and 
			state save information.

 * Globals:	Exptabs - select expand tabs
		Onlcr - select output CR before NL
		Offval - number of spaces to offset each line

 * Status:	Unit-tested.

 * Pseudo-code:	Get the saved information.
		Reset the output buffer if empty.
		Move as many characters as possible, while
		    Inserting CR before line feeds if selected
		    Adding offset spaces at the front of non-empty
			lines (pushes tab stops over)
		    Expand tabs to every 8 columns after the offset.
		Quit on input buffer empty of output buffer full.
		Reset the input buffer if empty.
		Save any information for next time through.
 */
trlocal (csdp)
    register CSDATP	csdp;
{
	register struct csbuf16	*ibuf = &csdp->ibuf->sh;
	register struct csbuf16	*obuf = &csdp->obuf->sh;
	register int		 ch = csdp->ch16;
	register int		 cnt = csdp->cnt;

	if (obuf->out == obuf->in)
	    obuf->out = obuf->in = obuf->base;

#ifdef DEBUG
printf ("Initial cnt = %d\n", cnt);
#endif

	while (obuf->in < obuf->lim) {

#ifdef DEBUG
printf ("cnt=%d ch=%3o obuf->in=%d ibuf->out=%d csdp->pos=%d\n", cnt, ch, obuf->in, ibuf->out, csdp->pos);
#endif

	    if (cnt > 0) {
		if (--cnt > 0)
		    *obuf->in++ = *csdp->cbuf;
		else {
		    if (ch == '\t') {
			ibuf->out--;
			*obuf->in++ = *csdp->cbuf;
		    }
		    else
		    	*obuf->in++ = ch;
		}
	    } else {
		if (ibuf->out >= ibuf->in)
		    goto cleanup;
		ch = *ibuf->out++;
		switch (ch)
		{
		    case '\n':
			csdp->pos = 0;
			if (Onlcr) {
			    *csdp->cbuf = '\r';
			    cnt = 2;
			    continue;
			}
			break;

		    case '\f':
			break;

		    default:
			if (csdp->pos == 0)
			{
			      /* we don't count the offset blanks
				 in the column position to make the
				 tabs work correctly. */
			    *csdp->cbuf = ' ';
			    cnt = Offval + 1;
			    csdp->pos = 1;
			    continue;
			} else if (ch == '\t' && Exptabs) {
			    *csdp->cbuf = ch = ' ';
			    cnt = 8 - ((csdp->pos) & 7);
			    csdp->pos += cnt;
			    continue;
			} else if (ch == '\b') {
			      /* decrement column position for backspace,
				 but note that we don't bother to try to
				 accommodate BS over TABs */
			    csdp->pos--;
			} else {
			    if (ch > 0x1f && ! (0xc1 <= ch && ch <= 0xcf))
				csdp->pos++;
			}
		}
		*obuf->in++ = ch;
	    }
	}
    cleanup:
	if (ibuf->out == ibuf->in)
	    ibuf->out = ibuf->in = ibuf->base;

	csdp->cnt = cnt;
	csdp->ch16 = ch;
}
