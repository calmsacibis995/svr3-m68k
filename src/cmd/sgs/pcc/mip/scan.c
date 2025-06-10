/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
static char SCCSID[] = "	@(#)scan.c	7.2		";


/*	MC68000 optimizer - File scan.c		*/

/* Routines to scan the input and find relevant pieces. */

/* All of these routines assume an input scan pointer, scanp.
** Scanp only moves over characters that have been successfully
** scanned.
*/

#include <ctype.h>
#include "optim.h"
/* #include "defs" is implicit */

extern	char * scanp;		/* scan pointer */

#define	swhite() while (*scanp == '\t' || *scanp == ' ') scanp++
/* sexpr -- scan across expression
**
** This routine scans across an expression, which can consist of
** terms (numbers or names) separated by + or -.  There may be a
** leading -.  Return true if we scan just a single number, false
** if we see a name or any non-leading + or -.
*/

static boolean
sexpr()
{
    boolean onenum = true;	/* true if scanned just one numeric value */
    boolean snum();
    boolean sname();

    if (*scanp == '-')		/* leading - */
	scanp++;		/* skip it */
    
    while (true)		/* loop until we don't find +/- separator */
    {
	if (! snum()) 		/* check for number */
	    if (sname())	/* see name? */
		onenum = false;	/* yes.  expression is not single number */
	    else
		serr("Bad expression");
				/* no.  Neither number nor name */
	
	if (*scanp != '-' && *scanp != '+')
				/* check for operator */
	    return(onenum);	/* none there.  We're done. */
	
	scanp++;		/* skip over operator */
    }
}
/* sline -- scan one input line
**
** This routine scans an input line.  It takes care of labels,
** opcode, and operands.
*/

void
sline()
{
    char * first;		/* remembered beginning pointer */
    void dolab();
    boolean doop();
    boolean sname();
    void dorand();
    RAND * srand();
    RAND * r;

    swhite();			/* skip initial white space */

    /* check for label */

    first = scanp;		/* remember where we are */
    if (sname() && *scanp == ':')
    {
	dolab(first, scanp-first); /* take care of label */
	scanp++;		/* skip : */
	swhite();		/* skip to next non-white space */
    }
    else
	scanp = first;		/* back up in case we saw name */

    /* check for null operation */

    if (*scanp == '\n' || *scanp == CC)
	return;			/* no op code.  Done. */
    
    /* find op mnemonic */

    first = scanp;		/* remember starting point */
    while (islower(*scanp) || *scanp == '.')
	scanp++;		/* scan across likely candidate chars */
    
    if (scanp == first)		/* did we move? */
	serr("Bad op name");
    /* Do operation-specific things */

    if (! doop(first,scanp-first)) /* true if rest of line was processed */
    {
	/* do normal operand processing */
	int randno = 1;		/* current operand number */
	
	do
	{
	    r = srand();	/* scan next operand */
	    dorand(r, randno);	/* do other operand processing */
	    randno++;
	} while (*scanp++ == ','); /* until we don't see a , */
	scanp--;		/* back up over non-, */
	
	/* Must find newline or comment start */

	if (*scanp != '\n' && *scanp != CC)
	    serr("Bad end of instruction");
    }
    return;
}
/* sname -- scan across a "name"
**
** This routine scans across a "name".  In this context, a name is
** a leading ~, or it consists of a leading letter or _,
** followed by letters, digits, _, ~, or %.
** Return a boolean success/failure value.
*/

static boolean
sname()
{
    register char c;		/* current character */
    boolean first = true;	/* whether we're working on the first char */

    if ( *scanp == '~' )	/* ~ as first character is single name */
    {
	scanp++;		/* skip ~ */
	return(true);		/* saw a name */
    }

    while ( (c = *scanp),
	       isalpha(c)	/* letter */
	    || c == '_'		/* underline */
	    || ((! first) && (isdigit(c) || c == '%' || c == '~'))
				/* non-first digit or % or ~ */
	    )
    {
	scanp++;		/* bump scan pointer */
	first = false;		/* no longer on first char */
    }

    return (! first);		/* saw something if not doing first char */
}
/* snum -- scan over number
**
** This routine scans over a number, which can be decimal, octal
** (with preceding 0), or hexadecimal (with preceding 0x or 0X).
** Allow a leading - sign for numbers.
** A success/failure code gets returned.
*/

static boolean
snum()
{
    boolean hex = false;	/* not doing hex value initially */
    register char * p = scanp;	/* use this for scanning */

    if (*p == '-')
	p++;			/* skip over leading sign */

    if (! isdigit(*p))		/* check for digit */
	return(false);		/* return with scanp unaffected */
    
    if (*p++ == '0')		/* check for 0x, 0X */
	if (*p == 'x' || *p == 'X')
	{
	    p++;		/* skip x/X */
	    hex = true;		/* have a hex number */
	}
    
    /* scan digits, being lazy about octal */

    while (isdigit(*p) || (hex && isxdigit(*p)))
	p++;			/* move to next char */
    
    scanp = p;			/* reset scan pointer */
    return(true);		/* say we saw a number */
}
/* srand -- scan across complete operand
**
** This routine scans across a complete normal operand.
** The operand is classified by type, and the RAND structure
** is built and returned.  The basic operand types we look for
** are keyed on the first character or two:
**
**	%	reg				RT_REG
**	&	number				RT_IMMNUM
**	&	expression			RT_IMMED
**	(	%reg)				RT_IREG
**	(	%reg)+				RT_INCR
**	-	(%reg)				RT_DECR
**	[-]	expression			RT_MEM
**	[-]	expression(%reg)		RT_OREG
**	[-]	expression(%reg,%reg.wl)	RT_DBLIND
*/

static RAND *
srand()
{
    REG regs = REG_NONE;	/* registers used */
    RTYPE rtype;		/* operand type */
    char * first;		/* starting point for operand */
    RAND * operand;		/* pointer to result structure */

    boolean sexpr();
    REG sreg();

    swhite();			/* skip leading white space */
    first = scanp;		/* remember this point */

/* Begin processing based on first non-space character. */

    switch(*scanp)
    {
    case '%':			/* register */
	regs = sreg();		/* get register code */
	rtype = RT_REG;		/* set operand type */
	break;

    case '&':			/* immediate */
	scanp++;		/* skip & */
	rtype = RT_IMMNUM;	/* assume single number */
	if (! sexpr())		/* single numeric value? */
	    rtype = RT_IMMED;	/* no. */
	break;
    
    case '(':			/* register indirect and auto-increment */
	scanp++;		/* skip ( */
	regs = sreg();		/* get register information */
	if (*scanp++ != ')')	/* must see trailing ) */
	    serr("Bad register indirect");
	
	rtype = RT_IREG;	/* assume register indirect */
	if (*scanp == '+')	/* check for auto-increment */
	{
	    scanp++;		/* have it.  Skip + */
	    rtype = RT_INCR;
	}
	break;
    
    case '-':			/* predecrement and others */
	scanp++;		/* skip - */
	if (*scanp == '(')	/* start predecrement */
	{
	    scanp++;		/* skip ( */
	    regs = sreg();	/* get register information */
	    rtype = RT_DECR;	/* set operand type */
	    if (*scanp++ != ')') /* check for closing ) */
		serr("Bad predecrement");
	    break;		/* done */
	}
	/* fall through to default case */
/* Handle other cases of operand type */

    default:
	(void) sexpr();		/* scan across expression */
	if (*scanp != '(')	/* register offset or double index? */
	{
	    rtype = RT_MEM;	/* no, straight memory reference */
	    break;
	}

	scanp++;		/* skip ( */
	regs = sreg();		/* get register information */
	if (*scanp == ')')	/* ) means register ind., ',' means DBLIND */
	{
	    scanp++;		/* skip ) */
	    rtype = RT_OREG;	/* register offset */

	    /* If the register is %fp or %sp, this is really the same as
	    ** a memory reference.
	    */

	    if ((regs & (REG_FP | REG_SP)) != 0)
		rtype = RT_MEM;	/* call this memory */
	    break;
	}
	if (*scanp++ == ',')	/* double check double indirect */
	{
	    regs |= sreg();	/* get second register information */
	    rtype = RT_DBLIND;	/* double indexing operand */

	    /* check for trailing .[wl] ) */

	    if (*scanp++ == '.')
	    {
		if (*scanp == 'w' || *scanp == 'l')
		    if (*++scanp == ')')
		    {
			scanp++; /* skip ) */
			break;
		    }
	    }
	    serr("Bad double index syntax");
	}
	/* Reach here on some error */

	serr("Bad operand");
    } /* end switch */

    operand = saverand(first, scanp-first, rtype, regs, true);
				/* build RAND structure */
    
    swhite();			/* skip trailing white space */
    return(operand);		/* return new operand */
}
/* Tables for sreg.  They give the correspondence between register
** names and REG value.  There are two parallel tables, one with the
** packed representation for the name, the other with the value.
** Since sreg() uses a linear search, the names are ordered by
** frequency of use, according to several large code samples.
*/

/* Table of packed names */

#define	PACK(a,b)	(((a) << 8 ) + (b))

static unsigned long rname[] =
{
	PACK('d','0'),		/* %d0 */
	PACK('f','p'),		/* %fp */
	PACK('s','p'),		/* %sp */
	PACK('a','0'),		/* %a0 */
	PACK('a','2'),		/* %a2 */
	PACK('a','3'),		/* %a3 */
	PACK('d','1'),		/* %d1 */
	PACK('d','2'),		/* %d2 */
	PACK('a','1'),		/* %a1 */
	PACK('a','4'),		/* %a4 */
	PACK('d','3'),		/* %d3 */
	PACK('a','5'),		/* %a5 */
	PACK('p','c'),		/* %pc */
	PACK('d','4'),		/* %d4 */
	PACK('d','6'),		/* %d6 */
	PACK('d','5'),		/* %d5 */
	PACK('d','7'),		/* %d7 */
	PACK('c','c'),		/* %cc */
	PACK('s','r'),		/* %sr */
	PACK('a','6'),		/* %a6 */
	PACK('a','7'),		/* %a7 */
	PACK(PACK('u','s'),'p'),/* %usp */
	0			/* end of table */
};


/* Table of encode register values.  These must correspond, one-to-one
** with the encoded names above.
*/

static REG rcode[] =
{
	REG_D0,		/* %d0 */
	REG_FP,		/* %fp */
	REG_SP,		/* %sp */
	REG_A0,		/* %a0 */
	REG_A2,		/* %a2 */
	REG_A3,		/* %a3 */
	REG_D1,		/* %d1 */
	REG_D2,		/* %d2 */
	REG_A1,		/* %a1 */
	REG_A4,		/* %a4 */
	REG_D3,		/* %d3 */
	REG_A5,		/* %a5 */
	REG_PC,		/* %pc */
	REG_D4,		/* %d4 */
	REG_D6,		/* %d6 */
	REG_D5,		/* %d5 */
	REG_D7,		/* %d7 */
	REG_CC,		/* %cc */
	REG_SR,		/* %sr */
	REG_A6,		/* %a6 */
	REG_A7,		/* %a7 */
	REG_USP,	/* %usp */
	REG_NONE	/* no register */
};
/* sreg -- scan register name, return encoded value. */

static REG
sreg()
{
    unsigned long name = 0;	/* encoded name */
    unsigned long * lp;		/* pointer into table */

    if (*scanp++ == '%')	/* check for start of register */
    {
	name = PACK(scanp[0],scanp[1]);
	scanp += 2;		/* encode first two chars */

	if (islower(*scanp))	/* check for third letter */
	    name = PACK(name,*scanp++);
	
	/* search for register name in list */

	for (lp = rname; *lp != 0; lp++)
	    if (*lp == name)
		return(rcode[lp-rname]);
				/* return encoded value on match */
    }

    /* On any failure, produce error message. */

    serr("Bad register name");
/*NOTREACHED*/
}
