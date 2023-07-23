#define PROGID	"@(#)cstrans.c	1.20 - 4/10/85"
/*
	TBD - unit-test tre2i
 */
/*
 * File name:	cstrans.c

 * Description: International char set translation subroutine.

 * @(#) Copyright 1985 by Four Phase (ISG) of Motorola, Inc.

 * Contents:

	cstrans - main subroutine which calls others to do the work.
	
	ttxfcs - search for a matching char set translation table entry.

	tre2i - translate external input to internal 16-bit chars.

	tri2xs - translate internal 16-bit chars to Xerox string form.

	trxs2i - translate Xerox string form to internal 16-bit chars.

	tri2e - translate internal 16-bit char to external output form.

	tri2i - translate internal 16-bit chars to internal 16-bit chars.

 * Revision history: See SCCS delta commentary.


   This subroutine package handles translation of data which may be 
   represented as Xerox strings, external device codes or internal 16-bit
   characters.  Input data is in a buffer of unsigned char or short.
   The output is placed into a similar buffer.

   There are five translation modes, all of which use an internal 16-bit
   char input or output buffer, with the other buffer being either 8-bit
   chars or internal 16-bit chars:

	0 - translate from internal 16-bit to internal 16-bit using an
	    internal translate table.
	    (This is used to enforce the Motorola Private char set, or
	    to avoid the char sets for Motorola Private, ligatures and
	    accented chars.)
	1 - translate from external device char code to internal 16-bit
	    chars using a external device translate table.
	2 - translate from internal 16-bit chars to Xerox strings with
	    options for 16-bit stringlets or for 7-bit representations.
	3 - translate from Xerox strings to internal 16-bit chars.
	4 - translate from internal 16-bit chars to external device char
	    codes using a external device translate table.

   As an output filter, three translations would be applied in sequence:

	Mode 3 -> Mode 0 using cs_tostd -> Mode 4 using a device-specific
	translation table.

   To reformat Xerox strings, e.g. to Motorola Private, four translations
   could be applied:

	Mode 3 -> Mode 0 using cs_tostd -> Mode 0 using cs_topri -> Mode 2.

   Other combinations of translation modes may be used.  The only requirement
   is that each output buffer must be in the form expected for the next
   translation's input buffer.

   The external device translation input sections are expected to provide
   characters in the standard internal char sets, avoiding 040, 360 and 361.
   They may assume that their input is coming from that same standard form.
   The cs_tostd translation table is applied to input strings to ensure the
   standard input form.  

   This convention means that output translation tables do not have to handle
   all the different forms that are legal.  For example, the A umlaut symbol
   can be represented in three different internal forms:
	<000><310> <000><101>		standard form:  umlaut and 'A'
	<361><047>			the accented character rendering
	<040><241>			the Motorola private form
   Also, for devices that accept the ISO forms, no translation is required;
   and for some hardcopy devices that don't accept the ISO form, the accents
   can still be mapped to <accent> and <backspace>.
 */

#define  defined_io  1

#include <sys/types.h>

#include "cstty.h"
/* #include "csintern.h" */
#include "cs.h"

#ifndef STATIC
#define STATIC		/* nothing */
#endif STATIC

#ifndef REGISTER
#define REGISTER	register
#endif REGISTER

static char Progid[] = PROGID;

#ifndef NOCHECK
#include <stdio.h>
#include <sys/signal.h>
#define ASSERT(Exp,Fmt,Val)	if (!(Exp)) { fprintf(stderr, \
	"At line %d FAILED Exp Fmt\n", __LINE__, Val); \
	kill(getpid(),SIGSEGV); }
#else
#define ASSERT(exp,f,v)		/* nothing at all */
#endif

#define CSTATE	csdp->state
#define CSTTF(Typ,Pos)	( (Typ *) (((char *)cstt) + Pos) )
#define CS_ACCENT(c)	(0x00c1 <= (c) && (c) <= 0x00cf)

STATIC int tri2i(), tre2i(), tri2xs(), trxs2i(), tri2e();

static int (*trsub[]) () = {
	tri2i,
	tre2i,
	tri2xs,
	trxs2i,
	tri2e
	};


/*
 * Function:	cstrans

 * Description:	Translate chars from one buffer to another using a translate
		table.  It translates chars until either the output buffer
		becomes full or the input buffer is empty.

 * Parameters:	csdp - address of a char set data structure which points to
		an input buffer, a translate table and an output buffer.
		It also contains information describing the current state
		of the translation.  

 * Globals:	Static trsub[] - see Calls below for usage.

 * Input:	None

 * Output:	None if NOCHECK is defined.
		Otherwise, messages to standard error in case of
		detected errors in data strucutre contents.

 * Calls:	Translation functions in trsub[]:
		- tri2i
		- tre2i
		- tri2xs
		- trxs2i
		- tri2e

		fprintf - in case of errors
		getpid - in case of errors
		kill - in case of errors

 * Returns:	None

 * Status:	Unit-tested.

 * Pseudo-code:
		Determine which type of translation is desired and call a
		subroutine to handle it.  Also reset buffer in and out
		pointers to base when empty.  And if the CS_INIT bit is
		not set in the option word, reset all the state variables
		to intial values.
 */

cstrans (csdp)
    REGISTER CSDATP	csdp;
{
	REGISTER struct csbuf8	*ibuf;
	REGISTER struct csbuf8	*obuf;

	ASSERT(csdp != NULL, ,0)
	ASSERT(csdp->ibuf != NULL, ,0)
	ASSERT(csdp->obuf != NULL, ,0)
	ASSERT(0 <= csdp->mode && csdp->mode <= 4, mode=%d, csdp->mode)
	if (csdp->mode != 2 && csdp->mode != 3)
	    ASSERT(csdp->tt != NULL, mode=%d, csdp->mode)
	if (csdp->mode == 0)
	    ASSERT(csdp->tt->cs_ttflag&CSINTERN, state=%#o, csdp->tt->cs_ttflag)

	/* assume byte pointer comparisons are ok for short pointers */
	ibuf = &csdp->ibuf->ch;
	obuf = &csdp->obuf->ch;

	ASSERT(ibuf->base <= ibuf->out, ,0)
	ASSERT(ibuf->out <= ibuf->in, ,0)
	ASSERT(ibuf->in <= ibuf->lim, ,0)
	ASSERT(obuf->base <= obuf->out, ,0)
	ASSERT(obuf->out <= obuf->in, ,0)
	ASSERT(obuf->in <= obuf->lim, ,0)

	if (obuf->in == obuf->out)
	    obuf->in = obuf->out = obuf->base;

	if (! (csdp->opt&CS_INIT)) {
	    csdp->opt |= CS_INIT;
	    csdp->state = 0;
	    csdp->cset = 0;
	    csdp->len = 0;
	    csdp->cnt = 0;
	    csdp->pos = 0;
	    csdp->ch16 = 0xffff;
	    csdp->state |= CS_RESET;
	}

	(*trsub[csdp->mode])(csdp);

	ASSERT(ibuf->out == ibuf->in || obuf->in == obuf->lim, ,0)
	ASSERT(ibuf->base <= ibuf->out, ,0)
	ASSERT(ibuf->out <= ibuf->in, ,0)
	ASSERT(ibuf->in <= ibuf->lim, ,0)
	ASSERT(obuf->base <= obuf->out, ,0)
	ASSERT(obuf->out <= obuf->in, ,0)
	ASSERT(obuf->in <= obuf->lim, ,0)

	if(ibuf->in == ibuf->out) ibuf->in = ibuf->out = ibuf->base;

	return;
}

/*
 * Local Function:	ttxfcs

 * Description:	Search for a match using a character set and a possible
		accent character prefix.

 * Parameters:	cstt - address of a translation table header.

		accp - address of an accent character stored as an 8-bit char
		    (assumed to be 0x00C1..0x00CF).
		NOTE:  *accp=0; if the accent is successfully translated.

		ch16 - the 16-bit character to translate.

 * Globals:	None

 * Input:	None

 * Output:	None

 * Calls:	None

 * Returns:	0	if no match for this character
		n>0	if CSTTF(struct csttent, n) is the desired translate
			table entry

		Also see returned value through parameter accp above.

 * Status:	Unit-tested.

 * Pseudo-code:
		Check once for a match using a possible accent and
		again without it if no match the first time
 */

STATIC ttxfcs (cstt, accp, ch16)
    REGISTER struct cstthdr	*cstt;
    unsigned char		*accp;
    REGISTER ushort		 ch16;
{
	REGISTER struct cscsix	*csix;
	REGISTER int		 compar = 0;	/* comparison result */
	REGISTER int		 acc = (accp) ? *accp : '\0';
	REGISTER int		 n;
	REGISTER unsigned char	 cset = ch16 >> 8;
	unsigned char		*listp;

	ch16 &= 0xff;
	/* Try to match it with any accent first; if not,
	   discard the accent and try again */
	do {
	    if (compar != 0) acc = 0;
	    n = cstt->cs_nchset;
	    csix = CSTTF(struct cscsix, cstt->cs_pchset);

	    /* exit this loop with a match (compar == 0) or when
	       the table entries exceed the characters we have */
	    for ( ; --n >= 0; csix++) {
		if ((compar = *(unsigned char *)csix - cset) < 0) continue;
		if (compar > 0) break;
		if ((compar = ((unsigned char *)csix)[1] - acc) < 0) continue;
		if (compar > 0) break;

		  /* We found a table for the char set and [ accent ]. */
		  /* If we find a match in the list or range, we set n
		     to indicate which translation table entry applies */
		if (csix->cs_nlist) {
		      /* See if the char is in the list. */
		      /* There should only be one list, so always break */
		    n = csix->cs_nlist;
		    listp = CSTTF(unsigned char, csix->cs_plist);
		    while (--n >= 0)
			if ((compar = ch16 - listp[n]) >= 0)
			    break;	/* out of this while */
		    break;	/* with n set if compar == 0 */
		} else {
		      /* See if the char is in the range. */
		      /* There may be multiple ranges, but they are sorted
			 so we break on a match or if char < low bound */
		    if ((compar = cs_cshi(csix) - ch16) >= 0) {
			if ((n = ch16 - cs_cslo(csix)) >= 0)
			    compar = 0;
			break;
		    }
		}
	    }
	} while (acc && compar);

	if (compar != 0)
	    return (0);
	else {
	    if (acc) *accp = '\0';
	    return (csix->cs_cstt + n * sizeof(struct csttent));
	}
}


/*
 * Local Function:	tre2i

 * Description:	Translate external character sequences to internal Xerox
		standard 16-bit values:  <charset><char>.  The translation
		table should generate values avoiding char sets 040 and 361.

 * Parameters:	csdp - address of the char set data structure.

 * Globals:	None

 * Input:	None

 * Output:	None

 * Calls:	None

 * Returns:	Returns a 16-bit value:  <charset><char>
		or -1 if no character available yet.

 * Status:	Coded.  Not unit-tested.

 * Pseudo-code:
		Get current state.
		While the output buffer is not full,
		    If we have selected any chars for output.
			Move a selected to to the output buffer.
			Continue if more were selected.
		    While no chars are selected for output,
			If the input buffer is empty,
			    Goto cleanup.
			Get an input character.
			If at the length of the current table prefix,
			    If the range matches,
				Select the translation for output.
			    Else select the original for output.
			Else scan forward looking for a table match.
			If a match,
			    Increment the length and continue.
			Else
			    Select the original for output.

	Cleanup:
		Remember the current state.
 */

STATIC tre2i (csdp)
    REGISTER CSDATP	 csdp;
{
	REGISTER struct cstthdr *cstt = csdp->tt;
	REGISTER struct csescix *eix;
	REGISTER ushort		 len = csdp->len;
	REGISTER ushort		 pos = csdp->pos;
	REGISTER int		 compar;
	REGISTER int		 c;

	while (csdp->obuf->ch.in < csdp->obuf->ch.lim) {

	    /* if any output chars are saved, assign the next char to c */
	    /* set pos to zero when the last char has been assigned */
	    if (CSTATE&CS_SAVE) {
		if (CSTATE&CS_XLAT) {
		    REGISTER struct csttent *tte;

		    /* translated input is saved */
		    tte = CSTTF(struct csttent, pos);
		    switch (tte->cs_tttyp) {
			case CS_NOCHG:
			    csdp->cnt = 0;
			    csdp->ch16 = 0xffff;
			    pos = (char *)eix - (char *)cstt;
			    CSTATE &= ~ CS_XLAT;
			    goto nochange;
			case CS_CS0:
			    c = tte->cs_ttval;
			    pos = 0;
			    break;
			case CS_CS40:
			    c = tte->cs_ttval + (CSMOTPRI<<8);
			    pos = 0;
			    break;
			case CS_ACC:
			    if (! len) {
				c = 0xC0 + (tte->cs_ttval >> 8);
				len++;
			    } else {
				c = tte->cs_ttval & 0xff;
				pos = 0;
			    }
			    break;
			case CS_ERR:
			    c = '?';
			    pos = 0;
			    break;
			default:
			    /* input tables always use <char set><char> form */
			    /* len is the current 16-bit char position */
			    c = CSTTF(ushort,cstt->cs_ptrchar)[len++];
			    if (len >= tte->cs_tttyp)
				pos = 0;
			    break;
		    }
		} else {

		    /* untranslatable input is saved */
		    /* csdp->cnt scans through the recognized input and
		       csdp->ch16 contains the unmatched char or -1 */
    nochange:
		    eix = CSTTF(struct csescix, pos);
		    if (csdp->cnt < len) {
			c = eix->cs_esc[csdp->cnt++];
			if (csdp->cnt >= len && csdp->ch16 == 0xffff)
			    pos = 0;
		    } else {
			c = csdp->ch16;
			pos = 0;
		    }
		}

		*csdp->obuf->sh.in++ = c;
		if (pos)
		    continue;
		else {
		    CSTATE &= ~(CS_SAVE|CS_XLAT);
		}
	    }

	    /* now that we have output everything we had gotten,
	       let's go get some more input */
	    while ( ! (CSTATE&CS_SAVE)) {
		REGISTER struct csescix *limix;
		REGISTER int j;

		if (csdp->ibuf->ch.out >= csdp->ibuf->ch.in)
		    goto cleanup;
		c = *csdp->ibuf->ch.out++;
 
		if (pos == 0) {
		    pos = cstt->cs_pesc;
		    len = 0;
		}
		eix = CSTTF(struct csescix, pos);
		limix = CSTTF(struct csescix,cstt->cs_pesc) + cstt->cs_nesc;

		  /* search from the current table position for a match */
		  /* other possibilities are definitely no match or need
		     more data.  compar is set to zero on exit to indicate
		     a match, <0 to indicate failure or >0 for need more */
		for (;;) {
		    /* Check the current table */
		    if (eix->cs_esc[len] == 0 ||
			len == sizeof(eix->cs_esc)) {

			  /* this is right if the range matches */
			if ((compar = c - eix->cs_eschi) <= 0)
			    if ((compar = c - eix->cs_esclo) >= 0) {
				pos = compar;
				compar = 0;
				CSTATE |= CS_XLAT;
			    }
		    } else
			compar = c - eix->cs_esc[len];

		      /* See if we matched it or went too far */
		    if (compar <= 0)
			break;

		    /* scan forward through the sorted tables for a match */
		    while (++eix < limix || ! (compar = -1)) {
				/* the second clause sets compar on exit */
		    	pos += sizeof(struct csescix);
			/* ensure match on any previous chars */
			compar = 0;
			for (j=0; j < len; j++) {
			    compar = eix[-1].cs_esc[j] - eix->cs_esc[j];
			    if (compar != 0) break;
			}
			if (compar == 0)
			    compar = c - eix->cs_esc[j];
			if (compar <= 0) break;
		    }	
		} while (compar > 0);

		if (compar > 0)
		    len++;
		else if (compar == 0)
		    CSTATE |= CS_SAVE;
		else {
		    csdp->ch16 = c;
		    csdp->cnt = 0;
		}
	    }
	}

    cleanup:
	csdp->len = len;
	csdp->pos = pos;
}


/*
 * Local Function:	tri2xs

 * Description:	Translate internal 16-bit <charset><char> values to Xerox
		strings according to the cstty options.

 * Parameters:	csdp - address of the char set data structure.

 * Globals:	None

 * Input:	None

 * Output:	Places zero or more chars in the canonical tty queue.  These
		are then available for input to the user program.

 * Calls:	None

 * Returns:	None

 * Status:	Unit-tested.

 * Pseudo-code:
		NOTE:  This uses csdp->cnt to count how many chars are
		    saved.  The chars are kept in csdp->cbuf.
		    If csdp->cnt is non-zero on entry, csdp->pos indicates
		    the first remaining position in cbuf.

		If the first-time flag is set, handle it.
		This may result in setting up some saved chars to specify
		the current stringlet form.

		If the output is full, return.
		Repeat

		    While there are any saved characters,
			If 7-bit format is selected,
			    If char is 000,
				SUB -> buffer, @ -> char, goto test.
			    Else if char is 377,
				SUB -> buffer, / -> char, goto test.
			    Else if (bit 7 in char) != SO flag,
				Bit 7 in char -> SO flag.
				SO/SI -> buffer, clear bit 7 in char, goto test.
			Move char -> buffer and decrement the saved count.
	test:		If the output is full, return.

		    If the input is empty, return.
		    Get a 16-bit char.
		    If 16-bit stringlet mode, save the char set and char.
		    Else if enforced char set 040, 
			Substitute a '?' if not in char set 040.
		    Else if change in char set,
			Save the char set change sequence.
		    Save the char.
 */

STATIC tri2xs (csdp)
    REGISTER CSDATP	csdp;
{
	REGISTER struct csbuf16	*ibuf = &csdp->ibuf->sh;
	REGISTER struct csbuf8	*obuf = &csdp->obuf->ch;
	REGISTER int		 n;
	REGISTER int		 cs;
	REGISTER unsigned char	*bp = csdp->cbuf + csdp->pos;
	REGISTER int		 ch;

	if (obuf->in >= obuf->lim) return;

	if (CSTATE&CS_RESET) {
	    CSTATE &= ~ CS_RESET;
	    csdp->ch16 = 0;
	    if (csdp->opt&CST16) {
		bp[0] = 0xff;
		bp[1] = 0xff;
		bp[2] = '\000';
		csdp->cnt = 3;
	    } else if (csdp->opt&CST8) {
		bp[0] = 0xff;
		bp[1] = '\000';
		csdp->cnt = 2;
	    }
	}

	for (;;) {

	    while (csdp->cnt > 0) {
		ch = *bp;
		if (csdp->opt&CSFMT7) {
		    switch (ch) {
			case 0:
			    *obuf->in++ = '\032';
			    *bp = '@';
			    if (CSTATE&CS_SO)
				*bp += 0x80;
			    goto test;

			case 0377:
			    *obuf->in++ = '\032';
			    *bp = '/';
			    if (CSTATE&CS_SO)
				*bp += 0x80;
			    goto test;

			default:
			    if (ch & 0x80) {
				if (! (CSTATE&CS_SO)) {
				    CSTATE |= CS_SO;
				    *obuf->in++ = '\016';
				    goto test;
				}
			    } else {
				if (CSTATE&CS_SO) {
				    CSTATE &= ~ CS_SO;
				    *obuf->in++ = '\017';
				    goto test;
				}
			    }
		    }	/* end switch */
		    ch &= 0x7f;
		}
		*obuf->in++ = ch;
		csdp->cnt--;
		bp++;

	test:
		if (obuf->in >= obuf->lim) {
		    csdp->pos = bp - csdp->cbuf;
		    return;
		}
	    }

	    if (ibuf->out >= ibuf->in) return;

	    bp = csdp->cbuf;
	    ch = *ibuf->out++;		/* 16-bit value */
	    cs = ch >> 8;
	    ch &= 0xff;			/* truncating to 8 bits */
	    if (csdp->opt&CST16) {
		*bp++ = cs;
	    } else if (ch > ' ' && (ch < 0x7f || (0xa0 < ch && ch < 0xff))) {
		if (csdp->opt&CS040) {
		    if (cs != CSMOTPRI) ch = '?';
		} else if (cs != csdp->cset) {
		    *bp++ = 0xff;
		    *bp++ = cs;
		    csdp->cset = cs;
		}
	    }
	    *bp++ = ch;
	    csdp->cnt = bp - csdp->cbuf;
	    bp = csdp->cbuf;
	}
}


/*
 * Local Function:	trxs2i

 * Description:	Translate Xerox strings to internal Xerox 16-bit char values:
		<charset><char>.
		Recognize SO/SI and SUB protocol if present.  Handle Xerox
		stringlets.  Recognize if starting in char set 040.  
		Don't change control characters.

 * Parameters:	csdp - address of the char set data structure.

 * Globals:	None

 * Input:	None

 * Output:	None

 * Calls:	None

 * Returns:	None

 * Status:	Unit-tested.

 * Pseudo-code:
		If first-time flag is set, handle it.

		While ibuf isn't empty and obuf isn't full,
		    Get next input character.
		    If we had a SUB the last time,
			Clear the SUB flag.
			If this is an '@' or '/',
			    Convert the char to 000 or 377.
			Else
			    Back up in the input buffer.
			    Put a SUB char into the output buffer.
			    Continue.

		    If this is an SI, SO or SUB, and CSFMT7 is set.
			Set the appropriate flag correctly.
			Continue.
		    If in SO mode and the char > 040 and < 177,
			Set bit 7 in the char.
		    If we had a 377 last time,
			If we expect a char set,
			    Clear the expect a char set flag.
			    If we have 377,
				Continue (we expect 000 next).
			    Change the current char set to the char.
			    Clear the had a 377 flag and 16-bit mode flag.
			    Continue.
			(We expect 000 here, but we accept anything,
			instead of deciding how to handle an error.)
			Clear the had a 377 flag.
			Set the expect a char set flag.
			Discard the char and continue.
		    If this char is a 377,
			Set the had a 377 flag.
			Continue.
		    If we have a control char (i.e. < 040),
			Put the char in the output buffer and continue.

		    If we are in 16-bit mode,
			If we expect a char set,
			    Store the char in the current char set.
			    Clear the expect a char set flag.
			    Continue.
			Else
			    Set the expect a char set flag.

		    Add the current char set to the char.
		    Put the char into the output buffer.

 */

STATIC trxs2i (csdp)
    REGISTER CSDATP	csdp;
{
	struct csbuf8	*ibuf = &csdp->ibuf->ch;
	struct csbuf16	*obuf = &csdp->obuf->sh;
	REGISTER int	 ch;

	if (CSTATE&CS_RESET) {
	    CSTATE &= ~ CS_RESET;
	    if (csdp->opt&CS040)
		csdp->cset = 040;
	}

	for ( ; ibuf->out < ibuf->in && obuf->in < obuf->lim; ) {
	    ch = *ibuf->out++;

	    if (CSTATE&CS_FSUB) {
		CSTATE &= ~ CS_FSUB;
		if (ch == '@') ch = '\000';
		else if (ch == '/') ch = 0xff;
		else {
		    ibuf->out--;
		    *obuf->in++ = '\032';	/* a SUB char */
		    continue;
		}
	    }

	    if (CSTATE&CS_SO && ch > ' ' && ch < 0177)
		ch |= 0x80;
	    else if (csdp->opt&CSFMT7) {
		switch (ch) {
		    case '\032':	/* a SUB char */
			CSTATE |= CS_FSUB;
			continue;
		    case '\016':	/* a SO char */
			CSTATE |= CS_SO;
			continue;
		    case '\017':	/* a SI char */
			CSTATE &= ~ CS_SO;
			continue;
		}
	    }

	    if (CSTATE&CS_F377) {
		if (CSTATE&CS_FXCS) {
		    CSTATE &= ~ CS_FXCS;
		    if (ch == 0xff)
			continue;
		    csdp->cset = ch;
		    CSTATE &= ~ (CS_F377|CS_DS16);
		    continue;
		}
		CSTATE &= ~ CS_F377;
		CSTATE |= CS_DS16;
		CSTATE |= CS_FXCS;
		continue;
	    }

	    if (ch == 0xff) {
		CSTATE |= CS_F377;
		CSTATE |= CS_FXCS;
		continue;
	    }

	    if (CSTATE&CS_DS16) {
		if (CSTATE&CS_FXCS) {
		    csdp->cset = ch;
		    CSTATE &= ~ CS_FXCS;
		    continue;
		} else {
		    CSTATE |= CS_FXCS;
		}
	    } else if (ch <= ' ') {
		*obuf->in++ = ch;
		continue;
	    }

	    ch += csdp->cset << 8;
	    *obuf->in++ = ch;
	}
}


/*
 * Local Function:	tri2e

 * Description:	Translate internal Xerox 16-bit <charset><char> values to
		external sequences.  It uses the output half (the char set
		indexes) of the translation table to convert to
		device-specific codes.

 * Parameters:	csdp - address of the char set data structure.

 * Globals:	None

 * Input:	None

 * Output:	None

 * Calls:	None

 * Returns:	None

 * Status:	Unit-tested.

 * Pseudo-code:
		While the output buffer is not full,
		    If there are any saved chars,
			Check first for a saved char in ch16,
			then the current len in a char set or escape sequence
			and then to handle an external char set + char.

			Handle the 7-bit output SO/SI state.
			Put a char into the output buffer.
			Continue.

		    Get a char from the input buffer:
			If empty buffer, return.
			If an accent, remember it and try again.
			But if there are now two accents, 
			    Put back the second and go on.

		    Try to translate the current character
		    (with the possible accent)
		    If the accent was not translated,
			Backup in the input buffer.
			Try to translate the accent itself.

		    If translation was possible,
		    	Determine which type of translation entry was found.
			Set the char to the first char in the translation.
			If any more, save them for later.
		    Store the current char in the output buffer.
 */

STATIC tri2e (csdp)
    REGISTER CSDATP	csdp;
{
	REGISTER struct cstthdr	*cstt = csdp->tt;
	REGISTER struct csttent	*tte;
	REGISTER struct csbuf8	*obuf = &csdp->obuf->ch;
	REGISTER ushort		 ch;
	REGISTER int		 n;
	REGISTER ushort		 cnt = csdp->cnt;
	REGISTER ushort		 pos = csdp->pos;

	while (obuf->in < obuf->lim) {

	      /* if any saved chars, output them */

	    /*	The following state variables are used in this section:
		cnt > 0 means there are that many groups of characters
		to output.  A group of characters is any of:

		A)  if csdp->ch16 is not 0xffff, then its low 8 bits as a 
		    single character.

		B)  if csdp->len is not zero, then each of the 8-bit chars
		    it points to within the translate table, up to NULL.

		C)  if CS_XLAT is set in CSTATE, the character defined by
		    the low 8-bits at the ushort specified by pos, within
		    the character set 0.

		D)  otherwise the next ushort specified by pos, considered as
		    a character set plus character value.

		In the case C or D, if the char set >= the number of 
		external char sets, it specifies an escape sequence prefix
		number, which is to be output via case B before the 8-bit
		char is output.

		Similarly, if the char set number < number of external char
		sets, it requires output of the char set select sequence
		if the last specified char set was different from this one.
	     */
		    
	    if (cnt > 0) {
		if (csdp->ch16 != 0xffff) {
		    ch = csdp->ch16 & 0xff;
		    csdp->ch16 = 0xffff;
		    cnt--;
		} else if (csdp->len > 0) {
		  /* if in a char set or escape sequence, get next char */
		    if (0 == (ch = *(CSTTF(unsigned char, csdp->len++)))) {
			  /* we got the null terminator of the sequence */
			if (pos == 0) ch = csdp->cbuf[1];
			else ch = *CSTTF(ushort, pos);
			goto endoutput;
		    }

		} else {

		      /* locate the next char of the sequence */
		    ch = *CSTTF(ushort, pos);
		    if (CSTATE&CS_XLAT) {
			  /* it was an accented char */
			CSTATE &= ~ CS_XLAT;
			ch &= 0xff;
		    }

	checkcset:
		      /* now check if char set or escape sequence */
		    n = ch >> 8;
		    if (n < cstt->cs_nextcs) {
			  /* it is a char set number */
			if (n == csdp->cset) {
			    goto endoutput;
			} else
			    csdp->cset = n;	/* new char set */
			    /* and we will output its escape sequence now */
		    }

		      /* n here selects the escape sequence prefix string */
		    csdp->len = CSTTF(ushort, cstt->cs_pextcs)[n];

		      /* csdp->len is the position of the current char in
			 the escape sequence (offset within *csdp) */
		      /* NOTE:  This assumes a zero position value at 
				CSTTF(ushort,cstt->cs_nextcs) */
		    if (0 != (n = *CSTTF(unsigned char, csdp->len++))) {
			ch = n;
		    } else {
	endoutput:
			pos += 2;
			csdp->len = 0;
			cnt--;
		    }
	outputch:
		    ch = ch & 0xff;
		}

		if (csdp->opt&CSFMT7) {
		    if (ch & 0x80) {
			if (! (CSTATE&CS_SO)) {
			    CSTATE |= CS_SO;
			    csdp->ch16 = ch;
			    cnt++;
			    ch = '\016';		/* an SO character */
			}
		    } else {
			if (CSTATE&CS_SO) {
			    CSTATE &= ~ CS_SO;
			    csdp->ch16 = ch;
			    cnt++;
			    ch = '\017';		/* an SI character */
			}
		    }
		    ch &= 0x7f;
		}
		*obuf->in++ = ch;
		continue;		/* and test for buffer full */
	    }

	      /* get a character (not an accent unless adjacent accents) */
	    do {
		if (csdp->ibuf->sh.out >= csdp->ibuf->sh.in)
		    goto cleanup;
		ch = *csdp->ibuf->sh.out++;
	    } while (CS_ACCENT(ch) && (*csdp->cbuf = ch));
			/* the second clause saves the accent char */

	      /* see if there is a translation for the character */
	    pos = ttxfcs(cstt, csdp->cbuf, ch);

	      /* if an accent was left after translation */
	    if (*csdp->cbuf) {
		csdp->ibuf->sh.out--;
		ch = *csdp->cbuf;
		*csdp->cbuf = 0;
	    	pos = ttxfcs(cstt, csdp->cbuf, ch);
	    }

	      /* apply the translation if we found one */
	    if (pos == 0) {
		if (ch >> 8) ch = '?';
		csdp->cbuf[1] = ch;		/* in case char set change */
	    } else {
		tte = CSTTF(struct csttent, pos);
		switch (n = tte->cs_tttyp) {
		    case CS_NOCHG:
			break;		/* This would lose an accent */

		    case CS_CS0:
			ch = tte->cs_ttval;
			break;

		    case CS_CS40:
			ch = tte->cs_ttval + (CSMOTPRI << 8);
			break;

		    case CS_ACC:	/* to an accented character */
			ch = cs_ttnib(tte) + 0xc0;
			cnt = 1;
			CSTATE |= CS_XLAT;
			break;

		    case CS_ERR:
			ch = '?';
			break;

		    default:
			ASSERT (1 <= n && n <= 7, pos %x, pos)
			pos = sizeof(ushort)*tte->cs_ttval + cstt->cs_ptrchar;
			ch = *CSTTF(ushort, pos);
			cnt = n - 1;
		}
		/* we will handle char set or escape sequence
		   info from the top eight bits */
	    }
	    cnt++;
	    goto checkcset;
	}

    cleanup:
	csdp->cnt = cnt;
	csdp->pos = pos;
	return;
}


/*
 * Local Function:	tri2i

 * Description:	Translate internal 16-bit chars to another internal 16-bit
		char representation.  This is typically used to avoid use
		of the 040, 360 and 361 char sets (a standard representation)
		or to enforce use of the Motorola Private char set 040.

 * Parameters:	csdp - address of the char set translate data structure.

 * Globals:	None

 * Input:	None

 * Output:	None

 * Calls:	None

 * Returns:	None

 * Status:	Unit-tested.

 * Pseudo-code:
		While the output buffer is not full,
		    If there are any saved chars,
			Put next saved char into the output buffer.
			Continue.

		    Get a non-accent char from the input buffer:
			If empty buffer, return.
			If an accent, remember it and try again.
			(This discards an accent before another accent)

		    Try to translate the current character
		    (with the possible accent)
		    If the accent was not translated,
			Backup in the input buffer.
			Try to translate the accent itself.

		    If translation was possible,
		    	Determine which type of translation entry was found.
			Set the char to the first char in the translation.
			If any more, save them for later.
		    Store the current char in the output buffer.
 */

STATIC tri2i (csdp)
    REGISTER CSDATP 	csdp;
{
	REGISTER struct cstthdr	*cstt = csdp->tt;
	REGISTER struct csttent	*tte;
	REGISTER struct csbuf16	*obuf = &csdp->obuf->sh;
	REGISTER ushort		 ch;
	REGISTER int		 n;
	REGISTER int		 cnt = csdp->cnt;
	REGISTER int		 pos = csdp->pos;

	while (obuf->in < obuf->lim) {

	      /* if any saved chars, output them */
	    if (cnt > 0) {
		if (csdp->ch16 != 0xffff) {
		    ch = csdp->ch16 & 0xff;
		    csdp->ch16 = 0xffff;
		}
		else {
		      /* otherwise output next char of the sequence */
		    ch = *CSTTF(ushort, pos);
		    pos += 2;
		}

		*obuf->in++ = ch;
		cnt--;
		continue;
	    }

	      /* get a character to translate */
	    do {
		if (csdp->ibuf->sh.out >= csdp->ibuf->sh.in)
		    goto cleanup;
		ch = *csdp->ibuf->sh.out++;
	    } while (CS_ACCENT(ch) && *csdp->cbuf == 0 && (*csdp->cbuf = ch));
			/* the second clause saves the accent char */

	      /* see if there is a translation for the character */
	    pos = ttxfcs(cstt, csdp->cbuf, ch);

	      /* if an accent was left after translation */
	    if (*csdp->cbuf) {
		csdp->ibuf->sh.out--;
		ch = *csdp->cbuf;
		*csdp->cbuf = 0;
	    	pos = ttxfcs(cstt, csdp->cbuf, ch);
	    }

	      /* apply the translation if we found one */
	    if (pos) {
		tte = CSTTF(struct csttent, pos);
		switch (n = tte->cs_tttyp) {
		    case CS_NOCHG:
			break;		/* This would lose an accent */

		    case CS_CS0:
			ch = tte->cs_ttval;
			break;

		    case CS_CS40:
			ch = tte->cs_ttval + (CSMOTPRI << 8);
			break;

		    case CS_ACC:	/* to an accented character */
			ch = cs_ttnib(tte) + 0xc0;
			csdp->ch16 = cs_ttchar(tte);
			cnt = 1;
			break;

		    case CS_ERR:
			ch = '?';
			break;

		    default:
			ASSERT (1 <= n && n <= 7, pos 0x%x, pos)
			pos = sizeof(ushort)*tte->cs_ttval + cstt->cs_ptrchar;
			ch = *CSTTF(ushort, pos);
			pos += 2;
			cnt = n - 1;
		}
	    }
	    *obuf->in++ = ch;
	}

    cleanup:
	csdp->cnt = cnt;
	csdp->pos = pos;
	return;
}
