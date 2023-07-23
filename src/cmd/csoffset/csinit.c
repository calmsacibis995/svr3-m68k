int Debug = 0;
/*
 * File name:	csinit.c
 *
 * Description: This is csinit(3) - Initialize a character set translation
 *		table.
 *
 * Author:	Gene P. Zombolas
 *
 * Copyright 1985 by ISG, Motorola Inc.
 *
 * Revision history:
 *	02/06/85 Changed the representation of outbound user defined character
 *		 sets (gpz)
 *	02/06/85 Added \m for internal translation tables (gpz)
 *	02/04/85 Added internal translation tables (gpz)
 *	01/17/85 Created (gpz)
 */
#define CSMAXSIZ	1

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>

#define defined_io 1

#include "cstty.h"
#include "cs.h"
#include "csintern.h"


static char	*Version = "@(#)csinit.c - 1.17   3/15/85";

/************************
 *	MACROS		*
 ************************/

/* Syntax error.
 */
#define SYNTAXERR(type)		{  if (!Silent)\
					fprintf(stderr, type, Curline);\
				   Syntaxerr = 1;  }

/* Internal errors.
 */
#define ERRALLOC(s)		{  Errno = CI_BADMALLOC;  \
		fprintf(stderr, "Badmalloc at line %d\n", Curline); \
		return(s);  }
#define ERROVRFLO(s)		{  Errno = CI_OVRFLO; \
		fprintf(stderr, "Overflow at line %d\n", Curline); \
		return(s);  }



/************************
 *	CONSTANTS	*
 ************************/

#define EOFILE		0		/* End of file.  */

/* Token definitions.
 */
#define EOL		0		/* End of line.  */
#define TOKENACCENT	1		/* Token 'accent'.  */
#define TOKENCSEL	2		/* Token 'cselect'.  */
#define TOKENFMT7	3		/* Token 'format7'.  */
#define TOKENINBND	4		/* Token 'inbound'.  */
#define TOKENITERN	5		/* Token 'internal'.  */
#define TOKENOUTBND	6		/* Token 'outbound'.  */
#define TOKENPRIME	7		/* Token 'primary'.  */
#define TOKENRANGE	8		/* Token 'range'.  */
#define TOKENTRANS	9		/* Token 'translate'.  */
#define VARACCENT	10		/* Accent variable.  */
#define VARCHAR		0xFF		/* Character variable only.  */
#define VARINSEQ	12		/* Internal character sequence.  */
#define CHARSEQ		13		/* Character sequence or value.  */


/****************************
 *	ERROR MESSAGES	    *
 ****************************/

/* Syntax errors.
 */
#define BADCHRSET	"line %d - redeclaration of character set %d\n"
#define BADCSNUM	"line %d - undefined character set number %d\n"
#define BADFMT7		"line %d - format7 statement unexpected\n"
#define BADINBND	"line %d - inbound statement unexpected\n"
#define BADOUTBND	"line %d - outbound statement unexpected\n"
#define BADRANGE	"line %d - number of entries does not match defined \
range\n"
#define NOACCVAL	"line %d - translate statement missing accent value."
#define NOCHARSET	"line %d - translate statement missing character set \
number\n"
#define NOHIVAL		"line %d - translate statement missing high range \
value\n"
#define NOINSEQ		"line %d - translate statement missing input sequence\n"
#define NOLOVAL		"line %d - translate statement missing low range \
value\n"
#define NOPRIME		"line %d - no primary character set defined\n"
#define NORANGE		"line %d - translate statement missing range keyword\n"
#define SYNTAX		"line %d - syntax error\n"


/************************
 *	TYPES		*
 ************************/

/* Index entry to a block of translation table entries.
*/
typedef struct {
	struct csttent *ttents;		/* Pointer to a block a translate
					 * table entries.
					 */
	ushort numents;			/* Number of entries in the block.
					 */
}  ixcsttent;

/* Unsigned character.
 */
typedef unsigned char uchar;


/************************
 *	GLOBALS		*
 ************************/

static short IXcharsets[MAXCHSET];		/* Index to character sets.  */
static ushort IXescseq[MAXESCSEQ];		/* Index to escape sequences. */
static struct csescix *IXescix[MAXINDEX];	/* Index to escape indicies.  */
static struct cscsix *IXcsix[MAXINDEX];		/* Index to character set
						 * indicies.
						 */
static ixcsttent IXttent[2 * MAXINDEX]; 	/* Index of translation table
						 * entry blocks.
						 */
static ushort Charseq[MAXCHARSEQ];		/* Character sequence block. */
static uchar Strings[MAXSTRINGS];		/* String block.  */
static ushort Nxtescseq;			/* Next escape sequence.  */
static ushort Nxtescix;				/* Next escape index.  */
static ushort Nxtcsix;				/* Next character set index.  */
static ushort Nxtixttent;			/* Next translation table 
						 * entry block index.
						 */
static ushort Nxtchrseq;			/* Next character sequence.  */
static ushort Nxtstring;			/* Next string.  */
static ushort Numttent;				/* Total number of translation
						 * table entries.
						 */
static short Numcharsets;			/* Number of character sets.  */
static ushort Nxtchar;				/* Next character position to be
						 * parsed.
						 */
static ushort Curline;				/* Current position in the
						 * source file (for diagnostics)
						 */
static uchar Intern;				/* Internal mode flag.  */
static uchar Silent;				/* Silent mode flag.  */
static uchar Syntaxerr;				/* Syntax error flag.  */
static ushort Errno;				/* Error return code.  */

/*
 * Function:	csinit
 *
 * Description:
 *	Reads a translation source file and builds a translation table.
 *
 * Parameters:
 *	filename
 *	  entry	Name of the translation table source file.
 *	  exit	Unchanged.
 *	silent
 *	  entry TRUE if csinit to operate silently.  FALSE if csinit is
 *		to print syntax error messages.
 *	  exit	Unchanged.
 *	status
 *	  entry Unknown.
 *	  exit	Exit status. 
 *
 * Globals:	IXcharsets, Nxtescix, Nxtcsix, Nxtixttent, Nxtchrseq, Nxtstring,
 *		Numttent, Numcharsets, Nxtchar, Curline, Intern, Silent,
 *		Syntaxerr, Errno.
 *
 * Inputs:	None.
 *
 * Outputs:	Error messages when the silent flag is set to false.
 *
 * Calls:
 *	first_pass	Performs syntax checking and builds all temporary
 *			data structures.
 *	create_table	Second pass of the source file to build translation
 *			table.
 *	build_table	Build final table to return to caller.
 *	free_space	Free temporary space.
 *
 * Returns:
 *	A pointer to the beginning of the translation table or NULL on
 *	an error.
 *
 * Pseudo-code:
 *	Initialize globals
 *	Open source file
 *	Call first_pass to do syntax check and build data structures
 *	Call create_table to fill-in table entries
 *	Call build_table to build the final translation table
 *	Free all temporary structures
 *	Return
 *
 *
 */
struct csttbl *
csinit (filename, silent, status)
register char *filename;
register int silent;
register int *status;
{
	register short int i;
	register FILE *file;
	register struct csttbl *cshdr;

	int format7;

	struct csttbl *build_table();

	*status = CS_NOERR;

	/* Initialize globals.
	 */
	for (i = 0; i < MAXCHSET; ++i)
		IXcharsets[i] = -1;
	Nxtescseq = 0;
	Nxtescix = 0;
	Nxtcsix = 0;
	Nxtixttent = 0;
	Nxtchrseq = 0;
	Nxtstring = 0;
	Numttent = 0;
	Numcharsets = 0;
	Nxtchar = 0;
	Curline = 0;
	Intern = 0;
	Silent = silent;
	Syntaxerr = 0;
	Errno = 0;

	/* Open source file.
	 */
	if (!(file = (FILE *)fopen(filename, "r")))
	{
		*status = CI_BADOPEN;
		return(NULL);
	}

	/* Call first_pass to do most of the work.
	 */
	if (first_pass(file, &format7) == -1 || Syntaxerr)
	{
		free_space();
		if (Syntaxerr)
			*status = CI_SYNTAX;
		else *status = Errno;
		return(NULL);
	}

	/* Call create_table to read the table entries.
	 */
	if (create_table(file) == -1 || Syntaxerr)
	{
		free_space();
		if (Syntaxerr)
			*status = CI_SYNTAX;
		else *status = Errno;
		return(NULL);
	}

	/* Build the final table.
	 */
	if (!(cshdr = build_table(format7)))
	{
		free_space();
		*status = Errno;
		return(NULL);
	}

	/* Free temp space.
	 */
	free_space();

	return(cshdr);
}


/* Local function:	csmalloc
 *
 * Description:
 *	Call malloc to allocate some memory and zero it out.
 *
 */
char *
csmalloc (size)
unsigned size;
{
	char *ptr;

	ptr = (char *)malloc(size);
	if (ptr)
		return((char *)memset(ptr, 0x0000, size));
	else return(NULL);
}
/*
 * Local Function:	first_pass
 *
 * Description:
 *	Check the syntax of the source file and build the required data
 *	structures.
 *
 * Pseudo-code:
 *	Get first token in the source file
 *	If token is not 'inbound' ,'outbound', or 'internal'
 *		Invalid source file, return error
 *	While lines in file (get_line)
 *		Call get_token to read first token
 *		Switch on token
 *			Case 'primary'
 *			Case 'cselect'
 *				Save character set select sequence
 *			Case 'translate':
 *				If 'inbound'
 *					Call get_escix to read escape index
 *				Else
 *					Call get_csix to read character set
 *					  index
 *			Case 'format7':
 *				Set format7 flag
 *			Case 'inbound':
 *				Error
 *			Case 'outbound':
 *				Set bound flag to outbound
 *			Other:
 *				Error, unknown token
 *	When using character sets, ensure a primary definition
 *	Count the number of character sets
 *	Call sort_escix to sort the escape index
 *	Call sort_csix to sort the character set index
 *
 *
 */
first_pass (file, format7)
register FILE *file;			/* Source file.  */
register int *format7;			/* Format7 flag.  */
{
	register short int i;		/* Counter.  */
	register short int token;	/* Current token.  */
	register short int inbound;	/* Inbound flag.  */
	uchar line[MAXLINE];		/* Buffer for lines of input.  */
	uchar charseq[MAXSEQLEN + 1];	/* Buffer for character sequences.  */

	*format7 = 0;

	/* Find the first token in the file.
	 */
	token = get_line(file, line, charseq);
	if (token == TOKENINBND)
		inbound = 1;
	else if (token == TOKENOUTBND || token == TOKENITERN)
		inbound = 0;
	else
	{
		Errno = CI_BADFILE;
		return(-1);
	}

	/* Read source file.
	 */
	while ((token = get_line(file, line, charseq)))
		switch (token)
		{
			case TOKENPRIME:
				/* Save primary character set.
				 */
				if (get_chrset(line, 1) == -1)
					return(-1);
				break;

			case TOKENCSEL:
				/*  Save Character set.  */
				if (get_chrset(line, 0) == -1)
					return(-1);
				break;

			case TOKENTRANS:
				/* Read translate statement.
				 */
				if (inbound)
				{
					if (get_escix(line, file) == -1)
						return(-1);
				}
				else
				{
					if (get_csix(line, file) == -1)
						return(-1);
				}
				break;

			case TOKENFMT7:
				/* Set format7 flag.
				 */
				if (!inbound)
					*format7 = 1;
				else  SYNTAXERR(BADFMT7)

			case TOKENINBND:
				/* Only one inbound and it must come before
				 * outbound.
				 */
				SYNTAXERR(BADINBND)
				break;

			case TOKENOUTBND:
				/* Only one outbound allowed.
				 */
				if (inbound)
					inbound = 0;
				else  SYNTAXERR(BADOUTBND)
				break;

			default:
				/* Unknown token.
				 */
				SYNTAXERR(SYNTAX)
				break;
		}

	/* If character sets have been defined, make sure we have a primary.
	 */
	for (i = 1; i < MAXCHSET; ++i)
		if (IXcharsets[i] != -1)
		{
			if (IXcharsets[0] == -1)
				SYNTAXERR(NOPRIME);
			break;
		}

	/* Calculate the number of character sets.
	 */
	for (Numcharsets = MAXCHSET - 1; Numcharsets >= 0 && IXcharsets[
	  Numcharsets] == -1; --Numcharsets)
		;
	++Numcharsets;

	/* Sort indicies.
	 */
	sort_escix();
	sort_csix();

	return(0);
}



/*
 * Local Function:	create_table
 *
 * Description:
 *	Fill-in the translation table entries
 *
 * Pseudo-code:
 *	Rewind the file file
 *	Find the first statement (inbound, outbound, or internal)
 *	For each translate statement
 *		Call the appropriate routine to save the table entires
 *
 *
 */
create_table (file)
register FILE *file;				/* Source file.  */
{
	register short int token;		/* Current token.  */
	register short int inbound;		/* Bound flag.  */

	uchar line[MAXLINE];			/* Current line of input.  */
	uchar charseq[MAXSEQLEN + 1];		/* Char sequence  */
	uchar buf[MAXSEQLEN + 1];		/* Temp buffer.  */

	/* Get back to the beginning of the file.
	 */
	Curline = 0;
	rewind(file);

	/* Find inbound, outbound, or internal statement.
	 */
	if ((token = get_line(file, line, charseq)) == TOKENINBND)
		inbound = 1;
	else if (token == TOKENOUTBND)
		inbound = 0;
	else 
	{
		inbound = 0;
		Intern = 1;
	}

	/* Read source file and save the table entries.
	 */
	while (1)
	{
		charseq[0] = 0;

		/* Find the next translate statement.
		 */
		while ((token = get_line(file, line, charseq)) != TOKENTRANS
		  && token != EOFILE)
			if (token == TOKENOUTBND)
				inbound = 0;
		if (token == EOFILE)
			return(0);	
		
		/* Get the escape sequence (inbound) or character set
		 * number (outbound).
		 */
		get_token(line, charseq);
		if (inbound)
		{
			/* Get low range value skipping range keyword.
			 */
			get_token(line, buf);
			get_token(line, buf);
			if (save_escix(file, line, charseq, buf[0]) == -1)
				return(-1);
		}
		else
		{
			/* Get the type of translate statement (range or
			 * accent).
			 */
			token = get_token(line,buf);

			/* Get the low range value or accent character.
			 */
			get_token(line, buf);
			if (token == TOKENRANGE)
			{
				if (save_csix(file, line, charseq[0], buf[0], 0)
				  == -1)
					return(-1);
			}
			else
			{
				if (save_csix(file, line, charseq[0], 0, buf[0])
				  == -1)
					return(-1);
			}
		}

		/* Stop this on any syntax error.
		 */
		if (Syntaxerr)
			return(0);
	}
}



/*
 * Local Function:	build_table
 *
 * Description:
 *	Build the final translation table.  See <sys/cstty.h> for a description
 *	of the table.
 *
 * Pseudo-code:
 *	Calculate the size of the final table
 *	Call malloc to allocate the need space
 *	Fill-in the header
 *	Copy the escape sequence index and translation table entries
 *	Copy the character set index and translation table entries
 *	Copy the character sequences
 *	Copy the character set and escape sequence index.
 *	Copy the string block.
 *
 *
 */
struct csttbl *
build_table (format7)
int format7;					/* Format7 flag.  */
{
	register short int i;			/* Counter.  */
	register struct cstthdr *cshdr;		/* Translation table header.  */
	register char *ptrescix;		/* Next escape index.  */
	register char *ptrcsix;			/* Next char set index.  */
	register char *ptrttent;		/* Next table entry.  */

	struct csttbl *cstbl;			/* Translation table.  */
	int size,				/* Size of the table.  */
	    length;				/* Multiple uses.  */
	ushort beginstr;			/* Beginning of string section.
						 */

	/* Calculate the size of the table and allocate space for it.
	 */
	size = sizeof(struct cstthdr) + (2 * (Numcharsets + Nxtescseq + 1)) +
	  (sizeof(struct csescix) * Nxtescix) + (sizeof(struct cscsix) *
	  Nxtcsix) + (sizeof(struct csttent) * Numttent) + (2 * Nxtchrseq) +
	  Nxtstring + sizeof(cstbl->cs_tmax);
	if (!(cstbl = (struct csttbl *)csmalloc((unsigned)size)))
		ERRALLOC(NULL);
	cstbl->cs_tmax = size;
	cshdr = (struct cstthdr *)((char *)cstbl + sizeof(cstbl->cs_tmax));

	/* Fill out the header.
	 */
	cshdr->cs_tlen = size - sizeof(cstbl->cs_tmax);
	cshdr->cs_pesc = sizeof(struct cstthdr) + (2 * (Numcharsets + Nxtescseq
	  + 1));
	cshdr->cs_nesc = Nxtescix;
	cshdr->cs_pchset = cshdr->cs_pesc + (sizeof(struct csescix) * 
	  Nxtescix);
	cshdr->cs_nchset = Nxtcsix;
	cshdr->cs_ptrchar = cshdr->cs_pchset + (sizeof(struct cscsix) *
	  Nxtcsix) + (sizeof(struct csttent) * Numttent);
	if (Numcharsets || Nxtescseq)
	{
		cshdr->cs_nextcs = Numcharsets;
		cshdr->cs_pextcs = sizeof(struct cstthdr);
	}
	else
	{
		cshdr->cs_nextcs = 0;
		cshdr->cs_pextcs = 0;
	}
	cshdr->cs_ttflag = 0;
	if (format7)
		cshdr->cs_ttflag |= CSEXTSO;
	if (Intern)
		cshdr->cs_ttflag |= CSINTERN;

	beginstr = cshdr->cs_ptrchar + (2 * Nxtchrseq);
	ptrttent = (char *)cshdr + cshdr->cs_pchset + (sizeof(struct cscsix) *
	  Nxtcsix);

	/* Copy the escape sequence index and table entries.
	 */
	ptrescix = (char *)cshdr + cshdr->cs_pesc;
	for (i = 0; i < Nxtescix; ++i)
	{
		/* Copy index.
		 */
		memcpy(ptrescix, (char *)IXescix[i], sizeof(struct csescix));

		/* Copy table entries.
		 */
		length = IXttent[IXescix[i]->cs_esctt].numents * sizeof(struct
		  csttent);
		memcpy(ptrttent, (char *)IXttent[IXescix[i]->cs_esctt].ttents,
		  length);

		/* Set table entry pointer.
		 */
		((struct csescix *)ptrescix)->cs_esctt = ptrttent - (char *)
		  cshdr;

		ptrescix += sizeof(struct csescix);
		ptrttent += length;
	}

	/* Copy the character set index and table entries.
	 */
	ptrcsix = (char *)cshdr + cshdr->cs_pchset;
	for (i = 0; i < Nxtcsix; ++i)
	{
		/* Set string pointer (if present).
		 */
		if (IXcsix[i]->cs_cspfx[0])
			IXcsix[i]->cs_plist += beginstr;

		/* Copy index.
		 */
		memcpy(ptrcsix, (char *)IXcsix[i], sizeof(struct cscsix));

		/* Copy table entries.
		 */
		length = IXttent[IXcsix[i]->cs_cstt].numents * sizeof(struct
		  csttent);
		memcpy(ptrttent, (char *)IXttent[IXcsix[i]->cs_cstt].ttents,
		  length);

		/* Set table entry pointer.
		 */
		((struct cscsix *)ptrcsix)->cs_cstt = ptrttent - (char *)cshdr;

		ptrcsix += sizeof(struct cscsix);
		ptrttent += length;
	}

	/* Copy the character sequence block.
	 */
	memcpy((char *)cshdr + cshdr->cs_ptrchar, (char *)Charseq, (2 *
	  Nxtchrseq));

	/* Copy the character set and escape sequence index.
	 */
	if (Numcharsets || Nxtescseq)
	{
		for (i = 0; i < Numcharsets; ++i)
			IXcharsets[i] += beginstr;
		for (i = 0; i < Nxtescseq; ++i)
			IXescseq[i] += beginstr;
		memcpy((char *)cshdr + cshdr->cs_pextcs, (char *) IXcharsets,
		  (2 * Numcharsets));
		*((char *)cshdr + cshdr->cs_pextcs + (2 * Numcharsets)) = '\0';
		*((char *)cshdr + cshdr->cs_pextcs + (2 * Numcharsets) + 1) = 
		  '\0';
		memcpy((char *)cshdr + cshdr->cs_pextcs + (2 * Numcharsets) + 2,
		  (char *)IXescseq, (2 * Nxtescseq));
	}

	/* Copy the string block.
	 */
	if (Nxtstring)
		memcpy((char *)cshdr + beginstr, Strings, Nxtstring);

	return((struct csttbl *)cstbl);
}



/* Local Function:	get_line
 *
 * Description:
 *	Read a line from the source file.
 *
 * Pseudo-code:
 *	Read a line until one with a token is found
 *	If EOF
 *		Return EOFILE
 *	Else return the first token on the line
 *	
 *
 */
get_line (file, line, charseq)
register FILE *file;			/* Source file.  */
register uchar *line;			/* Buffer of line to read.  */
register uchar *charseq;		/* For get_token.  */
{
	register int token;		/* First token in the line.  */

	while (fgets((char *)line, MAXLINE, file))
	{
		++Curline;
		Nxtchar = 0;
		if ((token = get_token(line, charseq)))
			return(token);
	}
	return(EOFILE);
}


/* Local Function:	get_token
 *
 * Description:
 *	Read the first token of a line.
 *
 * Pseudo-code:
 *	Find next character to parse
 *	Determine token type
 *	If character sequence
 *		Call get_charseq to extract it
 *	Return token type
 *
 */
get_token (line, charseq)
register uchar *line;			/* Line of input.  */
register uchar *charseq;		/* To return character sequences.  */
{
	register uchar *c;		/* Current position in parse string.  */
	register short token;		/* Token value to return.  */

	token = CHARSEQ;

	/* Get a pointer to our starting position.
	 */
	c = line + Nxtchar;

	/* Find the first character.
	 */
	while (isspace(*c))
		++c;

	/* Find the token type.
	 */
	switch (*c)
	{
		case 'a':
			if (strncmp(c, "accent", 6) == 0)
			{
				token = TOKENACCENT;
				c += 6;
			}
			else c += get_charseq(c, charseq);
			break;
		case 'c':
			if (strncmp(c, "cselect", 7) == 0)
			{
				token = TOKENCSEL;
				c += 7;
			}
			else c += get_charseq(c, charseq);
			break;
		case 'f':
			if (strncmp(c, "format7", 7) == 0)
			{
				token = TOKENFMT7;
				c += 7;
			}
			else c += get_charseq(c, charseq);
			break;
		case 'i':
			if (strncmp(c, "inbound", 7) == 0)
			{
				token = TOKENINBND;
				c += 7;
			}
			else if (strncmp(c, "internal", 8) == 0)
			{
				token = TOKENITERN;
				c += 8;
			}
			else c += get_charseq(c, charseq);
			break;
		case 'o':
			if (strncmp(c, "outbound", 8) == 0)
			{
				token = TOKENOUTBND;
				c += 8;
			}
			else c += get_charseq(c, charseq);
			break;
		case 'p':
			if (strncmp(c, "primary", 7) == 0)
			{
				token = TOKENPRIME;
				c += 7;
			}
			else c += get_charseq(c, charseq);
			break;
		case 'r':
			if (strncmp(c, "range", 5) == 0)
			{
				token = TOKENRANGE;
				c += 5;
			}
			else c += get_charseq(c, charseq);
			break;
		case 't':
			if (strncmp(c, "translate", 9) == 0)
			{
				token = TOKENTRANS;
				c += 9;
			}
			else c += get_charseq(c, charseq);
			break;
		case '#':
			++c;
		case '\0':
			token = EOL;
			break;
		case '\\':
			if (*(c + 1) == 'a')
			{
				c += 2;
				token = VARACCENT;
			}
			else if (*(c + 1) == 'm')
			{
				c += 2;
				token = VARINSEQ;
			}
			else if (*(c + 1) == 'x')
			{
				c += 2;
				token = VARCHAR;
			}
			else c += get_charseq(c, charseq);
			break;
		default:
			c += get_charseq(c, charseq);
	}

	if ( Debug )
		printf( "get_token: token = %d on %d\n", token,  Curline );

	Nxtchar = c - line;
	return(token);
}


/* Local Function:	get_charseq
 *
 * Description:
 *	Copy a character sequence into a buffer.
 *
 * Pseudo-code:
 *
 */
get_charseq (c, charseq)
register uchar *c;			/* Beginning of char sequence.  */
register uchar *charseq;		/* To return the char sequence.  */
{
	register short int nc, i, j;	/* Counters.  */
	register uchar n;		/* Temp value.  */

	for (nc = 0, i = 0; !isspace(*c) && i < MAXSEQLEN; ++nc, ++c)
		if (*c == '\\')
		{
			++c; ++nc;
			if (isdigit(*c))
			{
				/* Octal value represented as \xxx.
				 */
				for (n = 0, j = 0; isdigit(*c) && j < 3; ++j,
				  ++c, ++nc)
					n = 8 * n + *c - '0';
				charseq[i++] = n;
				--c; --nc;
			}

			/* Special sequences.
			 */
			else if (*c == 'E')
				charseq[i++] = '\033';
			else if (*c == '\\')
				charseq[i++] = '\\';
			else if (*c == '#')
				charseq[i++] = '#';
		}
		else
			charseq[i++] = *c;
	charseq[i] = '\0';
	return(nc);
}



/* Local Function:	get_chrset
 *
 * Description:
 *	Save a character set definition.
 *	Format:		primary <char seq>
 *			cselect <set number> <char seq>
 *
 * Pseudo-code;
 *	Get set number
 *	Get sequence
 *	Make sure we have a proper defintion
 *	Save character set
 *
 */
get_chrset (line, primary)
register uchar *line;				/* Line of input.  */
register int primary;				/* Flag for primary char set. */
{
	register short int i;			/* Counter.  */
	uchar setnum[MAXSEQLEN + 1];		/* Set number.  */
	uchar seq[MAXSEQLEN + 1];		/* Set character sequence.  */

	/* Get character set number.
	 */
	if (primary)
		setnum[0] = 0;
	else
		if (get_token(line, setnum) != CHARSEQ)
		{
			SYNTAXERR(SYNTAX)
			return;
		}

	/* Get character set sequence.
	 */
	if (get_token(line, seq) != CHARSEQ)
	{
		SYNTAXERR(SYNTAX)
		return(0);
	}

	/* Check given character set.
	 */
	if (IXcharsets[setnum[0]] != -1)
	{
		if (!Silent)
			fprintf(stderr, BADCHRSET, Curline, setnum[0]);
		Syntaxerr = 1;
	}
	else if (IXcharsets[setnum[0]] >= MAXCHSET)
		ERROVRFLO(-1)

	/* Save character set.
	 */
	if (Nxtstring + strlen(seq) >= MAXSTRINGS)
		ERROVRFLO(-1)
	IXcharsets[setnum[0]] = Nxtstring;
	for (i = 0; seq[i]; ++i)
		Strings[Nxtstring++] = seq[i];
	Strings[Nxtstring++] = '\0';

	return(0);
}



/* Local Function:	get_escix
 *
 * Description:
 *	Read and build an escape sequence index element.
 *	Form of input:
 *		translate <escape prefix> range <low value> <high value>
 *
 * Pseudo-code:
 *	Call malloc to get space for this structure
 *	Call get_token to get escape prefix
 *	Call get_token to get keyword range
 *	Call get_token to get low value
 *	Call get_token to get high value
 *	Save values
 *	Allocate the block of translation table entries
 *	Return the number of translation table entries
 *
 */
get_escix (line, file)
register uchar *line;				/* Current line of input.  */
register FILE *file;				/* Source file.  */
{
	register short i, token, numents;	/* Necessary counters, etc.  */
	register struct csescix *escix;		/* Escape index structure.  */

	uchar inputseq[MAXSEQLEN + 1];		/* Buffer for input escape
						 * sequence.
						 */
	uchar lowval[MAXSEQLEN + 1];		/* Buffer for low range value.
						 */
	uchar hival[MAXSEQLEN + 1];		/* Buffer for high range value.
						 */
	uchar charseq[MAXSEQLEN + 1];		/* Temp buffer.  */

	numents = 0;
	inputseq[0] = '\0';
	lowval[0] = '\0';
	hival[0] = '\0';

	/* Allocate needed space.
	 */
	if (!Syntaxerr)
	{
		if (Nxtescix >= MAXINDEX)
			ERROVRFLO(-1)
		if (!(IXescix[Nxtescix] = (struct csescix *)csmalloc((unsigned)
		  sizeof(struct csescix))))
			ERRALLOC(-1)
	}

	/* Parse the rest of the line.
	 */
	if ((token = get_token(line, inputseq)) != CHARSEQ && token != VARCHAR)
		SYNTAXERR(NOINSEQ)
	if ((token = get_token(line, lowval)) != TOKENRANGE)
		SYNTAXERR(NORANGE)
	if (token != CHARSEQ)
		if (get_token(line, lowval) != CHARSEQ)
			SYNTAXERR(NOLOVAL)
	if (get_token(line, hival) != CHARSEQ)
		SYNTAXERR(NOHIVAL)

	/* Count the number of translation table entries following.
	 */
	numents = hival[0] - lowval[0] + 1;
	i = 0;
	while ((token = get_line(file, line, charseq)) == CHARSEQ || token == VARACCENT)
		++i;
	if (numents != i)
	{
		if (!Silent)
			fprintf(stderr, BADRANGE, Curline - 1);
		Syntaxerr = 1;
	}

	/* Put the extra line back we just read.
	 */
	fseek(file, (-1 * strlen(line)), 1);
	--Curline;

	if (!Syntaxerr)
	{
		/* Fill-in index.
	 	 */
		escix = IXescix[Nxtescix];
		for (i = 0; inputseq[i] && i < 4; ++i)
			escix->cs_esc[i] = inputseq[i];
		escix->cs_esclo = lowval[0];
		escix->cs_eschi = hival[0];

		/* Save the escape sequence in the outbound index.
		 */
			/* if (inputseq[0]) */
			if (find_esc(inputseq, strlen(inputseq)) == -1)
				if (add_esc(inputseq, strlen(inputseq)) == -1)
					return(-1);

		/* Allocate a block for the translation table entries.
		 */
		if (!Syntaxerr)
		{
			if (!(IXttent[Nxtixttent].ttents = (struct csttent *)
			  csmalloc((unsigned)(numents * sizeof(struct csttent)))
			  ))
				ERRALLOC(-1)
			IXttent[Nxtixttent].numents = numents;
			escix->cs_esctt = Nxtixttent;
			Numttent += numents;
			++Nxtixttent;
		}
		++Nxtescix;
	}

	return(0);
}


/* Local Function:	get_csix
 *
 * Description:
 *	Read and build an character set index element.
 *	Format of input:
 *		translate <character set #> range <low value> <high value>
 *		translate <character set #> accent <accent value>
 *
 * Pseudo-code:
 *	Call malloc to get space for this structure
 *	Call get_token to get character set #
 *	Save set #
 *	Call get_token to get keyword range
 *	Call get_token to get low value
 *	Save value
 *	Call get_token to get high value
 *	Save value
 *	Count the number of translation table entries
 *	Check number of entries against defined range
 *	Call malloc to allocate the translation table entry block
 *
 */
get_csix (line, file)
register uchar *line;				/* Current line of input.  */
register FILE *file;				/* Source file.  */
{
	register short i, token, numents;	/* Necessary counters, etc.  */
	register struct cscsix *csix;		/* Character set index
						 * structure.
						 */

	uchar charset[MAXSEQLEN + 1];		/* Character set number.  */
	uchar lowval[MAXSEQLEN + 1];		/* Low range value or accent
						 * character.
						 */
	uchar hival[MAXSEQLEN + 1];		/* High range value.  */
	uchar charseq[MAXSEQLEN + 1];		/* Temp buffer.  */
	uchar *c1, *c2, t;			/* For sorting cs_plist.  */
	int sorted = 0;				/* For sorting cs_plist.  */

	numents = 0;
	charset[0] = '\0';
	lowval[0] = '\0';
	hival[0] = '\0';

	/* Allocate needed space.
	 */
	if (!Syntaxerr)
	{
		if (Nxtcsix >= MAXINDEX)
			ERROVRFLO(-1)
		if (!(IXcsix[Nxtcsix] = (struct cscsix *)csmalloc((unsigned)
		  sizeof(struct cscsix))))
			ERRALLOC(-1)
	}

	/* Parse the rest of the line.
	 */
	if (get_token(line, charset) != CHARSEQ)
		SYNTAXERR(NOCHARSET)
	token = get_token(line, lowval);
	if (token == TOKENRANGE)
	{
		if (get_token(line, lowval) != CHARSEQ)
			SYNTAXERR(NOLOVAL)
		if (get_token(line, hival) != CHARSEQ)
			SYNTAXERR(NOHIVAL)
	}
	else if (token == TOKENACCENT)
	{
		if (get_token(line, lowval) != CHARSEQ)
			SYNTAXERR(NOACCVAL)
	}
	else SYNTAXERR(SYNTAX)

	/* Fill-in index.
	 */
	if (!Syntaxerr)
	{
		csix = IXcsix[Nxtcsix];
		csix->cs_csnum = charset[0];
		if (hival[0])
		{
			csix->cs_cspfx[0] = '\0';
			csix->cs_plist |= (uchar)lowval[0];
			csix->cs_plist <<= 8;
			csix->cs_plist |= (uchar)hival[0];

			/* Count the number of translation table entries.
			 */
			numents = hival[0] - lowval[0] + 1;
			i = 0;
			while ((token = get_line(file, line, charseq)) ==
			  CHARSEQ || token == VARACCENT || token ==
			  VARINSEQ)
				++i;
			if (numents != i)
			{
				if (!Silent)
					fprintf(stderr, BADRANGE, Curline - 1);
				Syntaxerr = 1;
			}
		}
		else
		{
			csix->cs_cspfx[0] = lowval[0];
			csix->cs_plist = Nxtstring;
			numents = 0;
			while (get_line(file, line, charseq) == CHARSEQ)
			{
				if (charseq[1] || !((charseq[0] >= 040 &&
				  charseq[0] <= 0176) || (charseq[0] >= 0241
				  && charseq[0] <= 0376)))
					SYNTAXERR(SYNTAX)
				if (Nxtstring >= MAXSTRINGS)
					ERROVRFLO(-1)
				Strings[Nxtstring++] = charseq[0];
				++numents;
			}
			Strings[Nxtstring++] = '\0';
			csix->cs_nlist = numents;

			/* Sort cs_plist.
			 */
			if (strlen(&Strings[csix->cs_plist]) > 1)
				while(!sorted)
				{
					sorted = 1;
					c1 = &Strings[csix->cs_plist];
					c2 = &Strings[csix->cs_plist + 1];
					for ( ; *c2; ++c1, ++c2)
						if (*c2 < *c1)
						{
							t = *c1;
							*c1 = *c2;
							*c2 = t;
							sorted = 0;
						}
				}
		}
			
		/* Put back the extra line we read.
		 */
		fseek(file, (-1 * strlen(line)), 1);
		--Curline;

		/* Allocate a block for the translation table entries.
		 */
		if (!Syntaxerr)
		{
			if (!(IXttent[Nxtixttent].ttents = (struct csttent *)
			  csmalloc((unsigned)(numents * sizeof(struct csttent))
			  ))) ERRALLOC(-1)
			IXttent[Nxtixttent].numents = numents;
			csix->cs_cstt = Nxtixttent;
			Numttent += numents;
			++Nxtixttent;
		}
		++Nxtcsix;
	}
	else
	{
		/* Skip over table entries.
		 */
		i = 0;
		while ((token = get_line(file, line, charseq)) == CHARSEQ ||
		  token == VARACCENT)
			++i;
		if (hival[0])
			if (i != (hival[0] - lowval[0] + 1))
			{
				if (!Silent)
					fprintf(stderr, BADRANGE, Curline - 1);
				Syntaxerr = 1;
			}
		fseek(file, (-1 * strlen(line)), 1);
		--Curline;
	}

	return(0);
}



/* Local Function:	sort_escix
 *
 * Description:
 *	Sort the escape sequence index.
 *
 * Pseudo-code:
 *	Simple "Bubble Sort" sorted by escape sequence and low range value.
 *
 *	CAUTION:  This assumes no overlapping ranges such as 
 *		1B 00 00 00  30 32
 *		1B 31 00 00  41 41
 *	The correct handling would either be an error message or split
 *	the range into two like this:
 *		1B 00 00 00  30 31
 *		1B 31 00 00  41 41
 *		1B 00 00 00  32 32
 */
sort_escix()
{
	register short int i, j, n, sorted;
	register uchar *chi, *chj;
	register struct csescix *temp;
	register int	cnt;

	i = 0;
	j = 1;
	sorted = 1;

	while (1)
	{
		chi = IXescix[i]->cs_esc;
		chj = IXescix[j]->cs_esc;
		cnt = sizeof(IXescix[j]->cs_esc) /
			sizeof(*IXescix[j]->cs_esc) + 1;
		do {
			if (*chj == 0 || cnt == 1) {
				cnt = 0;
				chj = & IXescix[j]->cs_esclo;
			}
			if (*chi == 0 || cnt == 1) {
				cnt = 1;
				chi = & IXescix[i]->cs_esclo;
			}
			n = *chj++ - *chi++;
		} while (n == 0 && --cnt > 0);
		/* It is true that the j-th entry < i-th entry if:
		   the current char in entry j < the corresponding one in i,
		   or they are equal but we ran out of j before i.
		   cnt < 0 means the latter case happened. */
		if (n == 0) n = cnt;

		/* Re-order if the second entry is greater than the first
		 */
		if (n < 0)
		{
			temp = IXescix[i];
			IXescix[i] = IXescix[j];
			IXescix[j] = temp;
			sorted = 0;
		}
		if (++j >= Nxtescix)
		{
			if (sorted)
				break;
			else
			{
				i = 0;
				j = 1;
				sorted = 1;
			}
		}
		else ++i;
	}
}


/* Local Function:	sort_csix
 *
 * Description:
 *	Sort the character set index
 *
 * Pseudo-code:
 *	Simple "Bubble Sort" sorted by character set number and low range
 *	  value
 *
 */
sort_csix ()
{
	register short int i, j, sorted;
	register struct cscsix *temp;

	i = 0;
	j = 1;
	sorted = 1;

	while (1)
	{
		/* Sort first by character set number.
		 */
		if (IXcsix[j]->cs_csnum < IXcsix[i]->cs_csnum)
		{
			temp = IXcsix[i];
			IXcsix[i] = IXcsix[j];
			IXcsix[j] = temp;
			sorted = 0;
		}
		else if (IXcsix[j]->cs_csnum == IXcsix[i]->cs_csnum)
		{
			/* Sort secondly by low range value and then accent.
			 */
			if (IXcsix[i]->cs_cspfx[0] && IXcsix[j]->cs_cspfx[0])
			{
				if (IXcsix[j]->cs_cspfx[0] < IXcsix[i]->
				  cs_cspfx[0])
				{
					temp = IXcsix[i];
					IXcsix[i] = IXcsix[j];
					IXcsix[j] = temp;
					sorted = 0;
				}
			}
			else if (IXcsix[i]->cs_cspfx[0])
			{
				temp = IXcsix[j];
				IXcsix[j] = IXcsix[i];
				IXcsix[i] = temp;
				sorted = 0;
			}
			else if (!IXcsix[j]->cs_cspfx[0] && cs_cslo(IXcsix[j])
			  < cs_cslo(IXcsix[i]))
			{
				temp = IXcsix[i];
				IXcsix[i] = IXcsix[j];
				IXcsix[j] = temp;
				sorted = 0;
			}
		}
		if (++j >= Nxtcsix)
		{
			if (sorted)
				break;
			else
			{
				i = 0;
				j = 1;
				sorted = 1;
			}
		}
		else ++i;
	}
}



/* Local Function:	save_escix
 *
 * Description:
 *	Read and save the table entries for inbound sequences.
 *
 * Pseudo-code:
 *
 */
save_escix (file, line, charseq, lowval)
register FILE *file;
register uchar *line;
uchar *charseq;
uchar lowval;
{
	register short int i;			/* Counter.  */
	register short int n;			/* # of escape sequence in table
						 */
	register short int numents;		/* # of table entries.  */
	register short int token;		/* Current token.  */
	register struct csttent *ttent;		/* Block of table entries.  */

	uchar seq[MAXSEQLEN + 1];		/* Temp buffer.  */
	uchar charset;				/* Character set number.  */
	uchar accent;				/* Accent character.  */
	uchar chr;				/* Character in charset.  */

	/* Find the escape sequence in the index table and set up the table
	 * entry block.
	 */
	for (n = 0; n < Nxtescix; ++n)
		if (strcmp(IXescix[n]->cs_esc, charseq) == 0 &&
			IXescix[n]->cs_esclo == lowval)
			break;
	if ( n >= Nxtescix )
	{
		fprintf( stderr, "itt: [save_escix] bad escape index %d\n", n );
		exit( 1 );
	}
	ttent = IXttent[IXescix[n]->cs_esctt].ttents;
	numents = IXttent[IXescix[n]->cs_esctt].numents;

	/* Read in the table entries.
	 */
	for (i = 0; i < numents; ++i, ++ttent)
	{
		charset = accent = chr = seq[0] = '\0';
		token = get_line(file, line, seq);
		if (token == CHARSEQ)
			charset = seq[0];
		else if (token == VARACCENT)
		{
			if (get_token(line, seq) != CHARSEQ)
				SYNTAXERR(SYNTAX)
			else accent = seq[0];
		}
		else SYNTAXERR(SYNTAX)
		if (get_token(line, seq) != CHARSEQ)
			SYNTAXERR(SYNTAX)
		else chr = seq[0];

		if (accent)
		{
			ttent->cs_tttyp = CS_ACC;
			ttent->cs_ttval |= (uchar)(accent - 0300);
			ttent->cs_ttval <<= 8;
			ttent->cs_ttval |= (uchar)chr;
		}
		else if (charset == 0)
		{
			ttent->cs_tttyp = CS_CS0;
			ttent->cs_ttval = chr;
		}
		else if (charset == 040)
		{
			ttent->cs_tttyp = CS_CS40;
			ttent->cs_ttval = chr;
		}
		else
		{
			if (Nxtchrseq >= MAXCHARSEQ)
				ERROVRFLO(-1)
			ttent->cs_tttyp = 1;
			ttent->cs_ttval = Nxtchrseq;
			Charseq[Nxtchrseq] |= (uchar)charset;
			Charseq[Nxtchrseq] <<= 8;
			Charseq[Nxtchrseq++] |= (uchar)chr;
		}
	}
	return(0);
}



/* Local Function:	save_csix
 *
 * Description:
 *	Read and save the table entries for outbound sequences.
 *	Format of entries:
 *		\E<sequence>
 *		<char set #> <char>
 *		<char sequence>
 *
 * Pseudo-code:
 *
 */
save_csix (file, line, charset, lowval, accent)
register FILE *file;			/* Source file.  */
register uchar *line;			/* Current line of input.  */
uchar charset;				/* Character set number.  */
uchar lowval;				/* Low value for range statments.  */
uchar accent;				/* Accent char for accent statemtns.  */
{
	register short int i, j;		/* Counters.  */
	register short int n;			/* # of char set index.  */
	register short int numents;		/* # of table entries.  */
	register short int token;		/* Current token.  */
	register struct csttent *ttent;		/* Block of table entries.  */
	register struct cscsix *csix;		/* Char set index.  */

	uchar seq[MAXSEQLEN + 1];		/* Temp buffer.  */
	uchar seq2[MAXSEQLEN + 1];		/* Temp buffer.  */
	int escix;				/* Escape index number.  */
	short int len;				/* Mutiple uses.  */
	uchar *c;

	/* Find the character set index number.
	 */
	if (accent)
	{
		for (n = 0; n < Nxtcsix; ++n)
			if (IXcsix[n]->cs_csnum == charset && IXcsix[n]->
			  cs_cspfx[0] == accent)
				break;
	}
	else
	{
		for (n = 0; n < Nxtcsix; ++n)
			if (IXcsix[n]->cs_csnum == charset && cs_cslo(IXcsix[n])
			  == lowval)
				break;
	}
	csix = IXcsix[n];
	ttent = IXttent[csix->cs_cstt].ttents;
	numents = IXttent[csix->cs_cstt].numents;

	/* Read in the table entries.
	 */
	for (i = 0; i < numents; ++i, ++ttent)
	{
		seq[0] = '\0';
		token = get_line(file, line, seq);

		/* For accented characters, find the table entry.
		 */
		if (csix->cs_cspfx[0])
		{
			c = &Strings[csix->cs_plist];
			for (j = 0; seq[0] != *c; ++j, ++c)
				;
			ttent = &IXttent[csix->cs_cstt].ttents[j];
			token = get_token(line, seq);
		}

		if ((!Intern && token != CHARSEQ) || (Intern && token != 
		  CHARSEQ && token != VARACCENT && token != VARINSEQ))
			SYNTAXERR(SYNTAX)

		/* Read the rest of the line.
		 */
		if (token != VARINSEQ)
		{
			strcpy(seq2, seq);
			while (get_token(line, seq) == CHARSEQ)
				strcat(seq2, seq);
		}

		/* Save entry.
		 */
		len = strlen(seq2);
		if (Intern)
		{
			/* We handle internal tables special.  Three types
			 * of table entries:
			 *	\n <char set><char> ... (default: char set 0)
			 *	\a <accent char> <char>
			 *	<char set> <char>
			 *	<char>		(assume character set 0)
			 */
			if (token == VARINSEQ)
			{
				/* Internal character sequence.
				 */
				for (j = 0; (token = get_token(line, seq2))
				  && j <= MAXSEQLEN; ++j)
				{
					if (Nxtchrseq >= MAXCHARSEQ)
						ERROVRFLO(-1)
					if ((len = strlen(seq2)) == 1)
					{
						Charseq[Nxtchrseq] = 0;
						Charseq[Nxtchrseq++] |= (uchar)
						  seq2[0];
					}
					else if (len == 2)
					{
						Charseq[Nxtchrseq] |= (uchar)
						  seq2[0];
						Charseq[Nxtchrseq] <<= 8;
						Charseq[Nxtchrseq++] |= (uchar)
						  seq2[1];
					}
					else SYNTAXERR(SYNTAX)
				}
				if (j > MAXSEQLEN)
					ERROVRFLO(-1)
				ttent->cs_tttyp = j;
				ttent->cs_ttval = Nxtchrseq - j;
			}
			else if (token == VARACCENT)
			{
				if (len != 2)
					SYNTAXERR(SYNTAX);
				ttent->cs_tttyp = CS_ACC;
				ttent->cs_ttval |= (uchar)(seq2[0] - 0300);
				ttent->cs_ttval <<= 8;
				ttent->cs_ttval |= (uchar)seq2[1];
			}
			else
			{
				if (len == 1 || (len == 2 && seq2[0] == 0))
				{
					ttent->cs_tttyp = CS_CS0;
					if (len == 1)
						ttent->cs_ttval = seq2[0];
					else ttent->cs_ttval = seq2[1];
				}
				else if (len == 2 && seq2[0] == '\040')
				{
					ttent->cs_tttyp = CS_CS40;
					ttent->cs_ttval = seq2[1];
				}
				else if (len == 2)
				{
					if (Nxtchrseq >= MAXCHARSEQ)
						ERROVRFLO(-1)
					ttent->cs_tttyp = 1;
					ttent->cs_ttval = Nxtchrseq;
					Charseq[Nxtchrseq] |= (uchar)seq2[0];
					Charseq[Nxtchrseq] <<= 8;
					Charseq[Nxtchrseq++] |= (uchar)seq2[1];
				}
				else SYNTAXERR(SYNTAX);
			}
		}
		    /* This should be indicated by a special input syntax,
		        such as:  \s sequence char  */
		else if (seq2[0] == '\033')
		{
			len--;
			if ((escix = find_esc(seq2, len)) == -1)
			    if ((escix = add_esc(seq2, len)) == -1)
				return(-1);
			if (Nxtchrseq >= MAXCHARSEQ)
			    ERROVRFLO(-1)
			ttent->cs_tttyp = 1;
			ttent->cs_ttval = Nxtchrseq;
			Charseq[Nxtchrseq] |= (uchar)(escix + Numcharsets + 1);
			Charseq[Nxtchrseq] <<= 8;
			Charseq[Nxtchrseq++] |= (uchar)seq2[len];
		}
		else if (seq2[0] < Numcharsets || len == 1)
		{
			/* User defined character set.
			 */
			if (len == 1) n = seq[0];
			else n = ((uchar)seq2[0] << 8) | (uchar)seq2[1];

			if ((n >> 8) <= 0xf)
			{
				/* Put char set number in the top 4 bits
				 * of the cs_ttval.
				 */
				ttent->cs_tttyp = CS_CS0;
				ttent->cs_ttval = n;
			}
			else
			{
				/* Save as a 16-bit character.
				 */
				if (Nxtchrseq >= MAXCHARSEQ)
					ERROVRFLO(-1)
				ttent->cs_tttyp = 1;
				ttent->cs_ttval = Nxtchrseq;
				Charseq[Nxtchrseq] = n;
			}
		}
		else
		{
			if (Nxtchrseq + len - 1 > MAXCHARSEQ)
				ERROVRFLO(-1)
			ttent->cs_tttyp = len;
			ttent->cs_ttval = Nxtchrseq;
			for (j = 0; j < len; ++j, ++Nxtchrseq)
			{
				Charseq[Nxtchrseq] |= (uchar)Numcharsets;
				Charseq[Nxtchrseq] <<= 8;
				Charseq[Nxtchrseq] |= (uchar)seq2[j];
			}
		}
	}
	return(0);
}



/* Local Function:	find_esc
 *
 * Description:
 *	Find the index number of the given escape sequence.
 *
 */
find_esc (seq, len)
register uchar *seq;			/* Sequence to search for.  */
register int len;			/* Length of sequence.  */
{
	register short int i;		/* Counter.  */

	for (i = 0; i < Nxtescseq; ++i)
		if (strncmp(&Strings[IXescseq[i]], seq, len) == 0)
			break;

	if (i == Nxtescseq)
		return(-1);
	else return(i);
}



/* Local Function:	add_esc
 *
 * Description:
 *	Add an escape sequence to the index.
 *
 */
add_esc (seq, len)
register uchar *seq;			/* Escape sequence.  */
register int len;			/* Length of sequence.  */
{
	register short int i;		/* Counter.  */

	if (Nxtstring + len >= MAXSTRINGS || Nxtescseq >= MAXESCSEQ)
		ERROVRFLO(-1)

	IXescseq[Nxtescseq] = Nxtstring;
	for (i = 0; i < len; ++i, ++Nxtstring)
		Strings[Nxtstring] = seq[i];
	Strings[Nxtstring++] = '\0';

	return(Nxtescseq++);
}



/* Local Function:	free_space
 *
 * Description:
 *	Free all temporary space.
 *
 * Pseudo-code:
 *	Free escape sequence index and table entries
 *	Free character set index and table entries
 *
 */
free_space ()
{
	register short int i;

	/* Free escape sequence index and table entries.
	 */
	for (i = 0; i < Nxtescix; ++i)
	{
		if(IXttent[IXescix[i]->cs_esctt].ttents)
			free((char *)IXttent[IXescix[i]->cs_esctt].ttents);
		if(IXescix[i])
			free((char *)IXescix[i]);
	}

	/* Free character set index and table entries.
	 */
	for (i = 0; i < Nxtcsix; ++i)
	{
		if(IXttent[IXcsix[i]->cs_cstt].ttents)
			free((char *)IXttent[IXcsix[i]->cs_cstt].ttents);
		if(IXcsix[i])
			free((char *)IXcsix[i]);
	}
}
