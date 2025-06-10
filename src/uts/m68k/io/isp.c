#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/tty.h"
#include "sys/ttold.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/termio.h"
#include "sys/ioctl.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/cmn_err.h"

#define ISP
#define KERNEL 1


#define  defined_io	1
#include "sys/cstty.h"
#include "sys/csintern.h"

extern struct cstthdr cs_topri;
extern struct cstthdr cs_tostd;

extern struct map csmap[];

extern unsigned char 	csoff[CANBSIZ];

extern int greek;

#define GET_NEXT ( (*nc)-- == 0 ? -1 : *((*cp)++) )

/*
 * All following routines are added to support ISP, none of these routines
 * were used by the orignal line discipline code
 */


/*
 * compare a string of characters making a translation table name
 */
ttmatch (name1, name2)
    register char *name1, *name2;
{
	register int len = sizeof(csttname);

	for ( ; len-- > 0; )
	    if (*name1++ != *name2++)
		return(0);
	return(1);
}

/*
 * Function:	getcsip

 * Description:	Get a pointer to the char set terminal info structure for
		the given tty structure.  Because there are several groups
		of tty structures in the kernel, this is not a simple index
		operation.  We use a table sorted to point to tty addresses
		in corresponding csttinf structures.  For fast access to
		the most recently used one, we check it first and update it
		when a new csttinf structure is referenced.

 * Parameters:	tp - address of a tty structure.  This is expected to be the
		    only tty structure for an RS-232 terminal or one of several
		    for the windows of an RS-422 terminal.  An attached serial
		    port on an RS-422 terminal uses one of the windows.

 * Globals:	cs_info - contains most recently used element, maximum 
		    number and currently used number of elements in the
		    cs_term table.
 
		cs_term - contains the char set tty info structures.

		cs_index - contains addresses of the allocated cs_term
		    elements.

 * Input:	None

 * Output:	Panic message if bad tp address or if not enough elements
		in cs_term.

 * Calls:	cmn_err

 * Returns:	Address of the cs_term entry containing the csttinf structure
		corresponding to the given tty structure.

 * Status:	Unit-tested.

 * Pseudo-code:
		Ensure the tty address is reasonable.
		If the most recently used cs_term entry is the right one,
		    return its address.
		Search cs_index until an equal or smaller entry is found.
		(Note: this ends at the end of the table because of a null,
		 also, the table entries are sorted from big to small.)
		If not equal,
		    Prevent interrupts.
		    If no correct entry (interrupts were possible)
			Build a new cs_term element.
			Add its address to the cs_index array.
		    Re-enable interrupts.
		Update the most recently used entry.
		Return the correct value.
 */

struct csttinf *
getcsip (tp)
    register struct tty *tp;
{
	register struct csttinf *csip;
	register struct csttinf **indp;
	struct csttinf *inf2;
	int lvl;

	if (tp <= NULL)
		cmn_err (CE_PANIC,"getcsip: bad tp");

	  /* see if it is the same csip as the last one uses */
	if ((csip = cs_info.cs_imru) && csip->cs_tp == tp)
	    return (csip);

	  /* Rats, well look it up via cs_index */
	indp = cs_index;
	while ((csip = *indp++) && csip->cs_tp < tp) /* nothing */;

	lvl = spltty( );

	/*
	 * Make sure it hasn't been moved down
	 */
	while (csip && csip->cs_tp < tp)
	    csip = *indp++;
	if (csip == NULL || csip->cs_tp != tp)
	{
	      /* Oops, the first reference ever to this tty */
	    /* optimized for fast normal use, not this rare loop */
	    indp--;
	    csip = cs_term + cs_info.cs_iuse;
	    cs_info.cs_imru = csip;

	      /* insert the new entry in cs_index, sorted by tp addr */
	    do
	    {
		    inf2 = *indp;
		    *indp++ = csip;
	    } while (csip = inf2);
		    
	    if (cs_info.cs_iuse++ >= cs_info.cs_itty)
		    cmn_err (CE_PANIC,"getcsip: cs_term overflow");
	    *indp = NULL;
	    csip = cs_info.cs_imru;
	    csip->cs_tp = tp;
	}

	splx(lvl);

	cs_info.cs_imru = csip;
	return (csip);
}


/*
 * Local Function:	ttxfcs

 * Description:	Search for a match using a character set and a possible
		accent character prefix.

 * Parameters:	cstt - address of a translation table header.

		accp - address of an accent character (assumed to be
			0x00C1..0x00CF) or an absent char (0xffff).
		NOTE:  *accp=0xffff; if the accent is successfully translated.

		ch16 - the 16-bit character to translate.

 * Globals:	None

 * Input:	None

 * Output:	None

 * Calls:	None

 * Returns:	0	if no match for this character
		n>0	if CSTTF(struct csttent, cstt, n) is the desired translate
			table entry

		Also see returned value through parameter accp above.

 * Status:	Unit-tested.

 * Pseudo-code:
		Check once for a match using a possible accent and
		again without it if no match the first time
 */

ttxfcs (cstt, accp, ch16)
    register struct cstthdr	*cstt;
    ushort			*accp;
    register ushort		 ch16;
{
	register struct cscsix	*csix;
	register int		 compar = 0;	/* comparison result */
	register int		 acc;
	register int		 n;
	register unsigned char	 cset = ch16 >> 8;
	unsigned char		*listp;

	if ((acc = *accp) == 0xffff) acc = 0;
	ch16 &= 0xff;
	/* Try to match it with any accent first; if not,
	   discard the accent and try again */
	do
	{
	    if (compar != 0) acc = 0;
	    n = cstt->cs_nchset;
	    csix = CSTTF(struct cscsix, cstt, cstt->cs_pchset);

	    /* exit this loop with a match (compar == 0) or when
	       the table entries exceed the characters we have */
	    for ( ; --n >= 0; csix++)
	    {
		if ((compar = *(unsigned char *)csix - cset) < 0) continue;
		if (compar > 0) break;
		if ((compar = ((unsigned char *)csix)[1] - acc) < 0) continue;
		if (compar > 0) break;

		  /* We found a table for the char set and [ accent ]. */
		  /* If we find a match in the list or range, we set n
		     to indicate which translation table entry applies */
		if (csix->cs_nlist)
		{
		      /* See if the char is in the list. */
		      /* There should only be one list, so always break */
		    n = csix->cs_nlist;
		    listp = CSTTF(unsigned char, cstt, csix->cs_plist);
		    while (--n >= 0)
			if ((compar = ch16 - listp[n]) >= 0)
			    break;	/* out of this while */
		    break;	/* with n set if compar == 0 */
		}
		else
		{
		      /* See if the char is in the range. */
		      /* There may be multiple ranges, but they are sorted
			 so we break on a match or if char < low bound */
		    if ((compar = cs_cshi(csix) - ch16) >= 0)
		    {
			if ((n = ch16 - cs_cslo(csix)) >= 0)
			    compar = 0;
			break;
		    }
		}
	    }
	} while (acc && compar);

	if (compar != 0)
	    return (0);
	else
	{
	    if (acc) *accp = 0xffff;
	    return (csix->cs_cstt + n * sizeof(struct csttent));
	}
}


/*
 * Function:	ttrxput

 * Description:	Call ttxput with the appropriate characters.
		This handles the SO/SI representation of 8-bit chars.

 * Parameters:	csip - pointer to the char set info structure.
		    It modifies the CS_4EXTSO flag bit in cs_state.

		ch8 - an 8-bit character to output

 * Globals:	None

 * Input:	None

 * Output:	Uses ttxput to place chars into the tty output queue.

 * Calls:	ttxput

 * Returns:	None

 * Status:	Unit-tested.

 * Pseudo-code:
		If reset is set, reset the shift-out state.

		If using SO/SI on external device codes,
		    If char bit 7 is clear and in SO state,
			Change to SI state.
		    Else if char bit 7 is set and in SI state,
			Change to SO state.
		    Clear bit 7 in the character.
		Output the character.
 */

ttrxput (csip, ch8)
    register struct csttinf	*csip;
    register unsigned char		 ch8;
{
	register struct cblock *cb = csip->cs_cb;
	register int ch7;

	if (CSTATE&CS_4RESET)
	{
	    CSTATE &= ~ CS_4RESET;
	    if (CSTATE&CSEXTSO)
	    {
		CSTATE &= ~ CS_4EXTSO;
		if (cb == NULL)
		    ttxput(csip->cs_tp, '\017', 0);
		else
		{
		    if(cb->c_last>= CLSIZE)
			    cmn_err(CE_PANIC,"ttrxput: Over the top 3");
		    cb->c_data[cb->c_last++] = '\017';
		}
	    }
	}

	if (CSTATE&CSEXTSO)
	{
	    ch7 = ch8 & 0x7f;
	    if (CSTATE&CS_4EXTSO)
	    {
		if (ch7 == ch8)
		{
		    if (cb == NULL)
		        ttxput(csip->cs_tp, '\017', 0 );
		    else
		    {
			    if(cb->c_last>= CLSIZE)
				    cmn_err(CE_PANIC,"ttrxput: Over the top 4");
			    cb->c_data[cb->c_last++] = '\017';
		    }
		    CSTATE &= ~CS_4EXTSO;
		}
	    }
	    else
	    {
		if (ch7 != ch8)
		{
		    if (cb == NULL)
		        ttxput(csip->cs_tp, '\016', 0 );
		    else
		    {
			if(cb->c_last>= CLSIZE)
				cmn_err(CE_PANIC,"ttrxput: Over the top 5");
			cb->c_data[cb->c_last++] = '\016';
		    }
		    CSTATE |= CS_4EXTSO;
		}
	    }
	    ch8 = ch7;
	}
	if (cb == NULL)
	    ttxput(csip->cs_tp, ch8, 0);
	else
	{
		if(cb->c_last>= CLSIZE)
			cmn_err(CE_PANIC,"ttrxput: Over the top 6");
	        cb->c_data[cb->c_last++] = ch8;
	}
}


/*
 * Function:	ttre2i

 * Description:	Translate external character sequences to internal Xerox
		standard 16-bit values:  <charset><char>.  The translation
		table should generate values avoiding char sets 040 and 361.

 * Parameters:	csip - address of the char set info structure.

 * Globals:	None

 * Input:	Gets characters from the raw tty queue.

 * Output:	None

 * Calls:	getc - to get a character from the raw tty queue.

 * Returns:	Returns a 16-bit value:  <charset><char>
		or -1 if no character available yet.

 * Status:	Unit-tested.

 * Pseudo-code:
		Get current input state.
		While no chars are selected for output,
		    Get an input character.
		    If no data, break.
		    If at the length of the current table prefix,
			If the range matches,
			    Select the translation for output.
			Else select the original for output.
		    Else scan forward looking for a table match.
		    If a match,
			Increment the length and continue.
		    Else
			Select the original for output.

		If we have selected any chars for output.
		    Determine the current selected char.
		    Remember if more chars were selected.
		Remember the current state.
		Return the current selected character.
 */


ttre2i (csip,nc,cp)
    ushort *nc;
    unsigned char **cp;
    register struct csttinf *csip;
{
	register struct cstthdr	*cstt = csip->cs_tt;
	register struct csescix	*eix;
	register int		 len = csip->cs_1len;
	register int		 pos = csip->cs_1pos;
	register int		 compar;
	register int		 ch;
	register int		 j;

	  /* Search the translate tables for a match to a sequence
	     of characters until it is imposible for a match to be
	     found.
	     
	     We don't explicitly remember the previous chars we have
	     matched; it is implicit in the values of pos and len
	     that we have already matched the len first characters
	     of the pos-th escape sequence index.  */

	if ( ! (CSTATE&CS_1SAVE))
	{

	    if ( (greek && ((ch= GET_NEXT ) == -1)) ||

	    (!greek && ((ch=getc(&csip->cs_tp->t_rawq)) < 0)))

		    goto cleanup;
	    if (pos == 0)
	    {
		pos = cstt->cs_nesc;
		len = 0;
	    }
	    eix = CSTTF(struct csescix, cstt, cstt->cs_pesc) + pos - 1;

	      /* We are searching for an escape sequence translate entry
		 which matches the input we have and will be getting.
		 We search from the back to the front in order to optimize
		 for the normal cases.  This way it usually exits at once.

		 The loop has two main sections:  the first checks the
		 current table (which matches for len characters) against
		 the new character.  It breaks out on a final match, or
		 sets compar to indicate the match status:  >0 fail, 0
		 good so far, <0 need to go to another table (they are
		 sorted in increasing order).  It advances len when a
		 match is found so far.

		 The second section moves back when the current table
		 can't match, looking for a table entry which does match
		 the first len characters of the input.
	     */
	    do
	    {
		if (len >= sizeof(eix->cs_esc) ||
		    (j = eix->cs_esc[len]) == 0)
		{
		    if ((compar = ch - eix->cs_eschi) <= 0)
		    {
			if ((compar = ch - eix->cs_esclo) >= 0)
			{
			    pos = sizeof(struct csttent)*compar +
				eix->cs_esctt;
			    CSTATE |= CS_1XLAT | CS_1SAVE;
			    compar = 0;
			    break;			/* success */
			}
		    }
		}
		else compar = ch - j;
		if (compar == 0)
		{
		    len++;		/* match so far */

		    if ( (greek && ((ch= GET_NEXT ) == -1)) ||

		    ( !greek && ((ch=getc(&csip->cs_tp->t_rawq)) < 0)))

			goto cleanup;
		}

		while (compar < 0)
		{
		    if (--pos <= 0)
		    {
			compar = 1;
			break;				/* failure */
		    }
		    eix--;
		    compar = 0;
		    for (j=0; j<len && compar==0; j++)
		    {
			compar = eix[1].cs_esc[j] - eix->cs_esc[j];
		    }
		}
	    } while (compar == 0);

	    if (compar != 0)
	    {
		if (!(csip->cs_tiopt&CSFMT7) && len == 0 && 
		   (csip->cs_2set == 0 || csip->cs_2set == CSMOTPRI))
		{
		    csip->cs_1char = -1;
		    pos = 0;
		    CSTATE &= ~(CS_1SAVE|CS_1XLAT);
		    csip->cs_tiopt |= CS_OFF;
		    goto cleanup;
		}
		csip->cs_1char = ch;
		pos = (char *)eix - (char *)cstt;
		CSTATE |= CS_1SAVE;
	    }
	    /* Exit the loop above by any of these conditions:
		- success:  CS_1SAVE and CS_1XLAT are true, other values
		  are also in pos, eix and ch.
		- failure: CS_1SAVE is true, CS_1XLAT is false, other 
		  values in pos, len, and csip->cs_1char.
	       Also exit via goto cleanup when we need more characters.
	     */
	}

	/* if any output chars are saved, assign the next char to ch */
	/* set pos to zero when the last char has been assigned */
	if (CSTATE&CS_1SAVE)
	{
	    if (CSTATE&CS_1XLAT)
	    {
		register struct csttent *tte;

		/* translated input is saved */
		tte = CSTTF(struct csttent, cstt, pos);
		switch (j = tte->cs_tttyp)
		{
		    case CS_NOCHG:
			csip->cs_1char = ch;
			pos = (char *)eix - (char *)cstt;
			CSTATE &= ~ CS_1XLAT;
			goto nochange;
		    case CS_CS0:
			ch = tte->cs_ttval;
			pos = 0;
			break;
		    case CS_CS40:
			ch = tte->cs_ttval + (CSMOTPRI<<8);
			pos = 0;
			break;
		    case CS_ACC:
			ch = 0xC0 + (tte->cs_ttval >> 8);
			csip->cs_1char = tte->cs_ttval & 0xff;
			len = 0;
			CSTATE &= ~ CS_1XLAT;
			break;
		    case CS_ERR:
			ch = '?';
			pos = 0;
			break;
		    default:
			/* input tables always use <char set><char> form */
			/* len is the current 16-bit char position */
			if (! (CSTATE&CS_1CNT))
			{
			    CSTATE |= CS_1CNT;
			    len = 0;
			}
			ch = CSTTF(ushort, cstt, cstt->cs_ptrchar)
				[len++ + tte->cs_ttval];
			if (len >= tte->cs_tttyp)
			{
			    pos = 0;
			    CSTATE &= ~ CS_1CNT;
			}
			break;
		}
	    }
	    else
	    {

		/* untranslatable input is saved */
		/* pos scans through the unrecognized input.
		   csip->cs_1char contains the unmatched char or -1 */
nochange:
		if (--len >= 0) 
		    ch = *CSTTF(unsigned char, cstt, pos++);
		else 
		{
		    ch = (unsigned char) csip->cs_1char;
		    csip->cs_1char = -1;
		    pos = 0;
		}

	    }

	    if (pos == 0)
	    {
		CSTATE &= ~(CS_1SAVE|CS_1XLAT);
		len = 0;
	    }
	}

    cleanup:
	csip->cs_1len = len;
	csip->cs_1pos = pos;
	return (ch);
}


/*
 * Function:	ttri2xs

 * Description:	Translate internal 16-bit <charset><char> values to Xerox
		strings according to the cstty options.  This finishes the
		processing of input from a terminal.

 * Parameters:	csip - address of the char set info structure.

		ch16 - the next Xerox 16-bit char to process.

 * Globals:	None

 * Input:	None

 * Output:	Places zero or more chars in the canonical tty queue.  These
		are then available for input to the user program.

 * Calls:	putc - to put characters in the canonical tty queue.

 * Returns:	None

 * Status:	Unit-tested.

 * Pseudo-code:
		If reset is selected,
		    Ensure the output state is unambiguous.
		If char set 040 selected,
		    Try to translate to char set 040.
		Discard an unused accent.
		If translation was possible,
		    Perform the translation.

		If a 16-bit char is ready to output,
		    If 16-bit stringlet mode, handle it.
		    If enforced char set 040, 
			Substitute a '?' if not in char set 040.
		    Else if change in char set,
			Handle the new char set number.
		    Buffer the 8-bit char.

		    If 7-bit format selected,
			For each buffered char,
			    For 000, 377 and bit 7,
			    use the SUB and SO/SI representations.
			    Output 7-bit char.
		    Else output each buffered char.
 */

/* State variables:

	csip->cs_tiopt bits:
		CSFMT7		Use SO .. SI around 7-bits char to show bit 8
				and use SUB @ for a \000 and SUB / for \377
		CST16		Output 16-bit stringlets:
				    \377 \377 \000 { <cs> <ch> } ...
				instead of 8-bit stringlets:
				    \377 <cs> <ch> ...
		CS040		Enforce the Motorola Private char set 040,
				with no char set stringlet prefixes.

	csip->cs_state or CSTATE bits:
		CS_2RESET	Reset any output sequences
		CS_2INTSO	The Shift-out or shift-in state for the Xerox
				string.

	csip->cs_2acc:		Holds an untranslated accent (or other char)

	csip->cs_2set:		Holds the current Xerox string char set

 */

ttri2xs (csip, ch16)
    register struct csttinf	*csip;
    register int		 ch16;
{
	register int		 n;
	register int		 c;
	char			 buf[6];
	register char		*bp = buf;
	ushort			 xtrach = 0xffff;

	if (CSTATE&CS_2RESET)
	{
	    register struct tty	*tp = csip->cs_tp;

	    CSTATE &= ~ CS_2RESET;
	    if (csip->cs_tiopt&CSFMT7)
	    {
		if(greek)
			putc('\017', &tp->t_rawq);
		else
			putc('\017', &tp->t_canq);
	    }
	    CSTATE &= ~ CS_2INTSO;

	    if (csip->cs_tiopt&CST16)
	    {
		if (csip->cs_tiopt&CSFMT7)
		{
		    if(greek)
		    {
			    putc('\032', &tp->t_rawq);
			    putc('/', &tp->t_rawq);
			    putc('\032', &tp->t_rawq);
			    putc('/', &tp->t_rawq);
			    putc('\032', &tp->t_rawq);
			    putc('@', &tp->t_rawq);
		    }
		    else
		    {
			    putc('\032', &tp->t_canq);
			    putc('/', &tp->t_canq);
			    putc('\032', &tp->t_canq);
			    putc('/', &tp->t_canq);
			    putc('\032', &tp->t_canq);
			    putc('@', &tp->t_canq);
		    }
		}
		else
		{
		    if(greek)
		    {
			    putc(0xff, &tp->t_rawq);
			    putc(0xff, &tp->t_rawq);
			    putc(0, &tp->t_rawq);
		    }
		    else
		    {
			    putc(0xff, &tp->t_canq);
			    putc(0xff, &tp->t_canq);
			    putc(0, &tp->t_canq);
		    }
		}
	    }
	
	}


	do
	{			/* at least once, and again if we have
				   to translate an unused accent */
	    if (csip->cs_tiopt&CS040)
	    {
		register struct cstthdr	*cstt;

		if (xtrach != 0xffff)
		{
		    ch16 = xtrach;
		    xtrach = 0xffff;
		}
		
		if (CS_ACCENT(ch16))
		{
		    if (csip->cs_2acc != 0xffff)
		    {
			xtrach = ch16;
			ch16 = csip->cs_2acc;
			csip->cs_2acc = 0xffff;
		    }
		    else
		    {
			csip->cs_2acc = ch16;
			return;
		    }
		}

		/* see if there is a match */
		cstt = &cs_topri;
		n = ttxfcs(cstt, &csip->cs_2acc, ch16);
		if (csip->cs_2acc != 0xffff)
		{
		    xtrach = ch16;
		    ch16 = csip->cs_2acc;
		    csip->cs_2acc = 0xffff;
		    n = ttxfcs(cstt, &csip->cs_2acc, ch16);
		}

		if (n)
		{
		    register struct csttent *tte = CSTTF(struct csttent, cstt, n);

		    /* we found a match and its position is n */
		    switch (tte->cs_tttyp)
		    {

			case CS_ACC:
			    *bp++ = cs_ttnib(tte) + 0xC0;
			    ch16 = cs_ttchar(tte) + (CSMOTPRI << 8);
			    break;

			case CS_NOCHG:
			    break;

			case CS_CS40:
			    ch16 = tte->cs_ttval + (CSMOTPRI << 8);
			    break;

			/* we don't expect to get other values 1-7 or CS_CS0 */
			case CS_CS0:
			case CS_ERR:
			default:
			    ch16 = '?';
		    }
		}
	    }

	    /* We have a possibly translated character to output as part of
	       a Xerox string. */
	    c = ch16 >> 8;
	    ch16 &= 0xff;		/* truncating to 8 bits */
	    if (csip->cs_tiopt&CST16)
	    {
		*bp++ = c;
	    }
	    else if (ch16 <= ' ')
	    {
		if (csip->cs_2set != 0 && csip->cs_2set != CSMOTPRI)
		{
		    *bp++ = 0xff;
		    c = csip->cs_tiopt&CS040 ? CSMOTPRI : 0;
		    *bp++ = c;
		    csip->cs_2set = c;
	   	}
	    }
	    else if (ch16 < 0x7f || (0xa0 < ch16 && ch16 < 0xff))
	    {
		if (csip->cs_tiopt&CS040)
		{
		    if (c != CSMOTPRI) ch16 = '?';
		}
		else if (c != csip->cs_2set)
		{
		    *bp++ = 0xff;
		    *bp++ = c;
		    csip->cs_2set = c;
		}
	    }
	    *bp++ = ch16;

	    {
		register struct tty	*tp = csip->cs_tp;

		n = bp - buf;
		bp = buf;
		if (csip->cs_tiopt&CSFMT7)
		{
		    while (n--)
			switch (ch16 = *bp++)
			{
			    case 0:
				if(greek)
				{
					putc('\032', &tp->t_rawq);
					putc('@', &tp->t_rawq);
				}
				else
				{
					putc('\032', &tp->t_canq);
					putc('@', &tp->t_canq);
				}
				break;
			    case 0377:
				if(greek)
				{
					putc('\032', &tp->t_rawq);
					putc('/', &tp->t_rawq);
				}
				else
				{
					putc('\032', &tp->t_canq);
					putc('/', &tp->t_canq);
				}
				break;
			    default:
				c = ch16 & 0x7f;
				if (CSTATE&CS_2INTSO)
				{
				    if (c == ch16)
				    {
					if(greek)
						putc('\017', &tp->t_rawq);
					else
						putc('\017', &tp->t_canq);
					CSTATE &= ~ CS_2INTSO;
				    }
				}
				else 
				{
				    if (c != ch16)
				    {
					if(greek)
						putc('\016', &tp->t_rawq);
					else
						putc('\016', &tp->t_canq);
					CSTATE |= CS_2INTSO;
				    }
				}
				if(greek)
					putc(c, &tp->t_rawq);
				else
					putc(c, &tp->t_canq);
			}
		}
		else while (n--)
		{
			if(greek)
			    putc(*bp++, &tp->t_rawq);
			else
			    putc(*bp++, &tp->t_canq);
		}
	    }
	} while (xtrach != 0xffff);

	return;
}


/*
 * Function:	ttrxs2i

 * Description:	Translate Xerox strings to internal Xerox 16-bit char values:
		<charset><char>.  This handles output destined for a terminal.
		Recognize SO/SI and SUB protocol if present.  Handle Xerox
		stringlets.  Don't change control characters.  Translate data
		values to avoid char sets 040, 360 and 361.

 * Parameters:	csip - address of the char set info structure.

 * Globals:	u.u_base and u.u_count are used and modified.

 * Input:	Is from calls to fubyte.

 * Output:	None

 * Calls:	fubyte - to get characters from the user process buffer.

 * Returns:	A Xerox 16-bit char value or
		-1 if no more input is available or
		-2 if an error is detected (from fubyte).

 * Status:	Unit-tested.

 * Pseudo-code:
		If any saved chars,
		    Return the next one.

		Repeat
		    If no more data in user buffer, return -1.
		    Get a char from the user buffer.
		    If an error, return -2.
		    Update the buffer pointers.
		    If we had a SUB the last time,
			Clear the SUB flag.
			If this is an '@' or '/',
			    Convert the char to 000 or 377.
			Else
			    Back up the buffer pointers.
			    Return a SUB char.

		    If this is an SI, SO or SUB, and CSFMT7 is set,
			Set the appropriate flag correctly.
			Continue.
		    If in SO mode and the char > 040,
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
			Return the character.

		    If we are in 16-bit mode,
			If we expect a char set,
			    Store the char in the current char set.
			    Clear the expect a char set flag.
			    Continue.
			Else
			    Set the expect a char set flag.

		    Add the current char set to the char.
		    Check if the current char should be translated
		    (to avoid char sets 040, 360 and 361) and if so,
			Try to translate it.
			If it can be translated,
			    Translate the character.
			    Save any more chars.
		    Return the character.
 */

ttrxs2i (csip, ch)
    register struct csttinf *csip;
    register int             ch;
{
	register struct csttent *tte;
	struct cstthdr		*cstt = &cs_tostd;
	register int		 n;
	static ushort		 nullacc = 0xffff;


	if (CSTATE&CS_3FSUB)
	{
            CSTATE &= ~ CS_3FSUB;
	    if (ch == '@') ch = '\000';
	    else if (ch == '/') ch = 0xff;
	    else
	    {
		u.u_base--;
		u.u_count++;
		return ('\032');		/* a SUB char */
            }
	}

	if (ch > ' ' && ch < CINTR)
	{
	    if (CSTATE&CS_3INTSO) 
	        ch |= 0x80;
	}
	else if (csip->cs_tiopt&CSFMT7)
	{
	    switch (ch)
	    {
		case '\032':			/* a SUB char */
		    CSTATE |= CS_3FSUB;
		    return(-1);
		case '\016':			/* a SO char */
		    CSTATE |= CS_3INTSO;
		    return(-1);
		case '\017':			/* a SI char */
		    CSTATE &= ~ CS_3INTSO;
		    return(-1);
            }
        }

        if (CSTATE&CS_3F377)
	{
	    if (CSTATE&CS_3FXCS)
	    {
	        CSTATE &= ~ CS_3FXCS;
	        if (ch == 0xff)
		    return(-1);
		csip->cs_3set = ch;
		cselect = ch;
		if (ch == CSMOTPRI)
		    cselect = 0;
		CSTATE &= ~ (CS_3F377|CS_3DS16);
		return(-1);
	    }
	    CSTATE &= ~ CS_3F377;
	    CSTATE |= CS_3DS16;
	    CSTATE |= CS_3FXCS;
	    return(-1);
	}

	if (ch == 0xff)
	{
	    CSTATE |= CS_3F377;
	    CSTATE |= CS_3FXCS;
	    return(-1);
	}

	if (CSTATE&CS_3DS16)
	{
	    if (CSTATE&CS_3FXCS)
	    {
	        csip->cs_3set = ch;
	        CSTATE &= ~ CS_3FXCS;
	        return(-1);
	    }
	    else 
	        CSTATE |= CS_3FXCS;
	}

	if (ch < ' ')
	    return (ch);

	if (ch > ' ')
	{
	    n = csip->cs_3set;
	    ch += n << 8;
	}
	else n = 0;
    
	if (n == 040 || n == 0360 || n == 0361)
	{
	    if (csip->cs_3pos = ttxfcs(cstt, &nullacc, ch))
	    {
	        tte = CSTTF(struct csttent, cstt, csip->cs_3pos);
	        switch (n = tte->cs_tttyp)
		{
	            case CS_NOCHG:
		        break;
		    case CS_CS0:
		        ch = tte->cs_ttval;
		        break;
		    case CS_CS40:
		        ch = tte->cs_ttval + (CSMOTPRI << 8);
		        break;
		    case CS_ACC:
		        ch = cs_ttnib(tte) + 0xc0;
		        csip->cs_3save = 1;
			CSTATE |= CS_3ACC;
		        u.u_count++;
		        break;
		    case CS_ERR:
		        ch = '?';
		        break;
		    default:
		        csip->cs_3pos = sizeof(ushort)*tte->cs_ttval +
		            cstt->cs_ptrchar;
			ch = *CSTTF(ushort, cstt, csip->cs_3pos);
			csip->cs_3pos += 2;
			csip->cs_3save = n - 1;
		}
	    }
	}
	return (ch);
}


/*
 * Function:	ttri2e

 * Description:	Translate internal Xerox 16-bit <charset><char> values to
		external sequences.  This finishes processing of output to
		a terminal.  It uses the output half of the translation
		table to convert to device-specific codes.

 * Parameters:	csip - address of the char set info structure.

 * Globals:	None

 * Input:	From calls to ttrxs2i to get chars from the user buffer.

 * Output:	By calls to ttrxput to put chars into the output queue.

 * Calls:	ttrxs2i - to get chars from the user buffer.
		ttrxput - put chars into the tty output queue.

 * Returns:	Zero if normal completion, or
		-1 if an error was encountered.

 * Status:	Unit-tested.

 * Pseudo-code:
		If no translation table,
		    Get a character.
		    Return if an error or no more chars.
		    Replace it by an error indicator if not char set 0.
		    Output the char via ttrxput.
		Else
		    Get a char via trxs2i unless we have a saved non-accent.
		    Return if an error or no more chars.
		    Try to translate the char(s) we have.
		    If we have an accent left over,
			Try to translate just the (first) accent.
			Save the other character for the next time.

		    If it was a successful translation,
			Determine the translation type and the first char.
			Set any state variables needed to do any more.

		    For all the chars to output,
		    	Handle a possible change in char set number.
			Output the escape sequence prefix if needed.
			If the char is not in char set 0 
			Replace it by an error indicator if not char set 0.
			Output the char via ttrxput.
			Set up for the next character.

		Return.
 */

ttri2e (csip, ch, csave)
    register struct csttinf *csip;
    register int             ch;
{
	register struct cstthdr	*cstt = csip->cs_tt;
	register struct cstthdr *cstd = &cs_tostd;
	register struct csttent *tte;
	register int		 n;
	register int		 pos;
	register int		 cnt;
	register int         	 len;

	/* get some input char */
        if (csave)
	{
	    ch = *CSTTF(ushort, cstd, csip->cs_3pos);
	    if (CSTATE&CS_3ACC)
	    {
		/* it was an accented char */
	        CSTATE &= ~ CS_3ACC;
	        ch &= 0xff;
	    }
	    csip->cs_3pos += 2;
	    csip->cs_3save--;
	    u.u_count--;
	}
	    
	if (cstt == NULL)
	{
	    if (ch > 0xff) ch = '?';
	    ttrxput(csip, ch);
	    return(1);
	} 

        /* see if there is a match */
	if (CS_ACCENT(ch) && csip->cs_4acc == 0xffff)
	{
	    csip->cs_4acc = ch;
	    return (1);
	}
	pos = ttxfcs(cstt, &csip->cs_4acc, ch);
	if (csip->cs_4acc != 0xffff)
	{
            n = ch;
	    ch = csip->cs_4acc;
	    csip->cs_4acc = 0xffff;
	    pos = ttxfcs(cstt, &csip->cs_4acc, ch);
	    csip->cs_4acc = n;
        }

        cnt = 0;
        if (pos == 0)
	{
            if (ch > 0xff) ch = '?';
        }
	else
	{
            tte = CSTTF(struct csttent, cstt, pos);
            /* we found a match and its position is n */
            switch (n = tte->cs_tttyp)
	    {
	        case CS_NOCHG:
	            break;

		case CS_CS0:
		    ch = tte->cs_ttval;
		    break;

	        case CS_CS40:
		    ch = tte->cs_ttval + (CSMOTPRI << 8);
		    break;

		case CS_ACC:
		    ch = cs_ttnib(tte) + 0xC0;
		    cnt = 1;
		    CSTATE |= CS_4ACC;
		    break;

		case CS_ERR:
		    ch = '?';
		    break;

	        default:
	            pos = sizeof(ushort)*tte->cs_ttval + cstt->cs_ptrchar;
		    ch = *CSTTF(ushort, cstt, pos);
		    pos += 2;
		    cnt = n - 1;
	    }
	} 

	/* output the current value of 'ch', and more chars if needed
	   until cnt == 0 */
	do
	{
	    /* if a char set change, handle it */
	    n = ch >> 8;
	    ch &= 0xff;
	    if (n < cstt->cs_nextcs)
	    {
	    /* it is an external char set number */
		if (n == csip->cs_4set)
		{
	            goto outputch;
		}
		else
		{
		    csip->cs_4set = n;	/*we have a new ext char set */
		    /* and we will output its char set sequence now */
		}
            }

	    /* output any escape sequence needed */
	    len = CSTTF(ushort, cstt, cstt->cs_pextcs)[n];
	    while (0 != (n = *(CSTTF(unsigned char, cstt, len++))))
	        ttrxput(csip, n);

    outputch:
            /* output a character */
	    if (ch > 0xff) ch = '?';
	    ttrxput(csip, ch);

	    /* set up for the next char, if any */
	    ch = *CSTTF(ushort, cstt, pos);
	    pos += 2;
	    if (CSTATE&CS_4ACC)
	    {
	        ch &= 0xff;
	        CSTATE &= ~ CS_4ACC;
	    }

	}  while (cnt-- > 0);

	return (1);
}


/*
 * Function:	ttcsinit

 * Description:	Initialize the char set data structures at boot time.

 * Parameters:	None

 * Globals:	csmap - resource map for unused translation table space.

		cs_info - data strucutre containing configuration params.

 * Input:	None

 * Output:	None

 * Calls:	None

 * Returns:	None

 * Status:	Unit-tested.

 * Pseudo-code:
		Initialize the resource map.
		The size of csmap actually allocated is 2 more than
		the number of ttmaps we can define.  The first entry
		holds map header information and the last one is 
		reserved for a zero entry to ensure rmalloc doesn't
		fail.
		CAUTION:  This assumes a resource map that holds full
		addresses.  AT+T System V only allows 15 bits.
		Conversion should use segment numbers instead of addresses.
		This affects the rmalloc and rmfree calls in ttiocom.
 */

ttcsinit ()
{
	register int i;

/* ISP remove the following as the 6600 does not have a wakeup
* bit in the map structure
	mapwant(csmap) = 0;
	rminit(csmap, cs_info.cs_iseg * cs_info.cs_isiz, cs_ttbuf, 
		"csmap", cs_info.cs_imap + 2);
*/

	mapinit( csmap, cs_info.cs_imap );
	csmap[1].m_addr = &cs_ttbuf[0];
/*Fix for 8000 to ensure map is properly initialiazed as on 8000 this appears to
be a byte map so lets go for it */
	csmap[1].m_size = cs_info.cs_iseg*cs_info.cs_isiz;
	/*mfree( csmap, cs_info.cs_iseg, 1 );*/

	*cs_index = NULL;
	for (i=0; i < CANBSIZ; i++) 
		csoff[i] = 0;
}

/* Procdure flip to flip translation table bottom bit by one to enable dual
simultaneous translation tables to allow multiple keyboards to be defined
*/
flip(csip)
struct csttinf *csip;
{
struct csopt opt;
register short i=0;
register char *ptr,*ptr1= opt.cs_name.dev;
register struct cstthdr *cstt;

	if(!csip->cs_tt) return(-1); /* No table so return */
/* Get current table name */
	for(ptr= csip->cs_tt->cs_tname.dev;i<sizeof(opt.cs_name);i++) *ptr1++ = *ptr++;
	opt.cs_name.lang[strlen(opt.cs_name.lang)-1] ^= 1;	
/* Flip bottom bit of language */
	/* Now try to set to it */
	for(i=0;i < cs_info.cs_imap;i++)
	{
		cstt = cs_ttmap[i];
		if (! ttmatch(&cstt->cs_tname,&opt.cs_name)) cstt=NULL;
		else break;
	}
	if ( cstt == NULL ) return(-1);
	/* Decrement reference counts reset state vars and flip */
	csip->cs_tt->cs_nref--;
	cstt->cs_nref++;
	csip->cs_tt = cstt;
	CSTATE |= cstt->cs_ttflag&(CSEXTSO|CS_4RESET);
	csip->cs_1pos = 0;
	csip->cs_1len = 0;
	csip->cs_1char = -1;
	if (csip->cs_tiopt&CS040)
	{
	    csip->cs_tiopt &= ~ CST16;
	    csip->cs_3set = CSMOTPRI;
	} else
	csip->cs_3set = 0;
	cselect = 0;
}
