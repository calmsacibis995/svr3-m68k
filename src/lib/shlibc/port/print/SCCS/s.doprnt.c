h57248
s 00001/00000/00889
d D 1.2 86/07/31 11:40:06 fnf 2 1
c Include nan.h (as picked up from Matt's version).
e
s 00889/00000/00000
d D 1.1 86/07/31 11:38:44 fnf 1 0
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

#ident	"@(#)libc-port:print/doprnt.c	3.10"

/*LINTLIBRARY*/
/*
 *	_doprnt: common code for printf, fprintf, sprintf
 */
#include "shlib.h"
#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include <values.h>
I 2
#include <nan.h>
E 2
#include "print.h"	/* parameters & macros for doprnt */

#define PUT(p, n)     { register unsigned char *newbufptr; \
	              if ((newbufptr = bufptr + n) > bufferend) { \
		              _dowrite(p, n, iop, &bufptr); \
                      } else { \
		              (void) memcpy((char *) bufptr, p, n); \
			      bufptr = newbufptr; \
                      } \
		    } 
#define PAD(s,n)    { register int nn; \
		    for (nn = n; nn > 20; nn -= 20) \
			       _dowrite(s, 20, iop, &bufptr); \
                    PUT(s, nn); \
                   }

#define SNLEN     5    	/* Length of string used when printing a NaN */

/* bit positions for flags used in doprnt */

#define LENGTH 	  1	/* l */
#define FPLUS     2	/* + */
#define FMINUS	  4	/* - */
#define FBLANK	  8	/* blank */
#define FSHARP	 16	/* # */
#define PADZERO  32	/* padding zeroes requested via '0' */
#define DOTSEEN  64	/* dot appeared in format specification */
#define SUFFIX	128	/* a suffix is to appear in the output */
#define RZERO	256	/* there will be trailing zeros in output */
#define LZERO	512	/* there will be leading zeroes in output */

/*
 *	C-Library routines for floating conversion
 */

extern char *fcvt(), *ecvt(), *memchr(), *memcpy();
extern int strlen(), fwrite();


static int
_lowdigit(valptr)
long *valptr;
{	/* This function computes the decimal low-order digit of the number */
	/* pointed to by valptr, and returns this digit after dividing   */
	/* *valptr by ten.  This function is called ONLY to compute the */
	/* low-order digit of a long whose high-order bit is set. */

	int lowbit = *valptr & 1;
	long value = (*valptr >> 1) & ~HIBITL;

	*valptr = value / 5;
	return(value % 5 * 2 + lowbit + '0');
}

/* The function _dowrite carries out buffer pointer bookkeeping surrounding */
/* a call to fwrite.  It is called only when the end of the file output */
/* buffer is approached or in other unusual situations. */
static
_dowrite(p, n, iop, ptrptr)
register char *p;
register int	n;
register FILE	*iop;
register unsigned char **ptrptr;
{
	if (iop->_file != _NFILE) {
		iop->_cnt -= (*ptrptr - iop->_ptr);
		iop->_ptr = *ptrptr;
		_bufsync(iop);
		(void) fwrite(p, 1, n, iop);
		*ptrptr = iop->_ptr;
	} else
		*ptrptr = (unsigned char *) memcpy((char *) *ptrptr, p, n) + n;
}
	static char _blanks[] = "                    ";
	static char _zeroes[] = "00000000000000000000";
	static char uc_digs[] = "0123456789ABCDEF";
	static char lc_digs[] = "0123456789abcdef";
	static char  lc_nan[] = "nan0x";
	static char  uc_nan[] = "NAN0X";
	static char  lc_inf[] = "inf";
	static char  uc_inf[] = "INF";

int
_doprnt(format, args, iop)
register char	*format;
va_list	args;
register FILE	*iop;
{
                
	/* bufptr is used inside of doprnt instead of iop->_ptr; */
	/* bufferend is a copy of _bufend(iop), if it exists.  For */
	/* dummy file descriptors (iop->_file = _NFILE), bufferend */
	/* may be meaningless.				*/

	unsigned char *bufptr, *bufferend;

	/* This variable counts output characters. */
	int	count = 0;

	/* Starting and ending points for value to be printed */
	register char	*bp;
	char *p;

	/* Field width and precision */
	int	width, prec;

	/* Format code */
	register int	fcode;

	/* Number of padding zeroes required on the left and right */
	int	lzero, rzero;

	/* Flags - bit positions defined by LENGTH, FPLUS, FMINUS, FBLANK, */
	/* and FSHARP are set if corresponding character is in format */
	/* Bit position defined by PADZERO means extra space in the field */
	/* should be padded with leading zeroes rather than with blanks */
	register int	flagword;

	/* Values are developed in this buffer */
	char	buf[max(MAXDIGS, 1+max(MAXFCVT+MAXEXP, MAXECVT))];

	/* Pointer to sign, "0x", "0X", or empty */
	char	*prefix;

	/* Exponent or empty */
	char	*suffix;

	/* Buffer to create exponent */
	char	expbuf[MAXESIZ + 1];

	/* Length of prefix and of suffix */
	int	prefixlength, suffixlength;

	/* Combined length of leading zeroes, trailing zeroes, and suffix */
	int 	otherlength;

	/* The value being converted, if integer */
	long	val;

	/* The value being converted, if real */
	double	dval;

	/* Output values from fcvt and ecvt */
	int	decpt, sign;

	/* Pointer to a translate table for digits of whatever radix */
	char	*tab;

	/* Work variables */
	int	k, lradix, mradix;

	/* Variables used to flag an infinities and nans, resp. */
	/* Nan_flg is used with two purposes: to flag a NaN and */
	/* as the length of the string ``NAN0X'' (``nan0x'') */
	 int	 Inf = 0, NaN_flg = 0 ;

	/* Pointer to string "NAN0X" or "nan0x" */
	 char	 *SNAN ;

        /* Flag for negative infinity */
	 int neg_inf = 0;

	/* initialize buffer pointer and buffer end pointer */
	{ register int fno = iop->_file;
	bufptr = iop->_ptr;
	bufferend = (fno == _NFILE) ? 
			(unsigned char *)((long) bufptr | (-1L & ~HIBITL))
				 : _bufendtab[fno];
	}

	/*
	 *	The main loop -- this loop goes through one iteration
	 *	for each string of ordinary characters or format specification.
	 */
	for ( ; ; ) {
		register int n;

		if ((fcode = *format) != '\0' && fcode != '%') {
			bp = format;
			do {
				format++;
			} while ((fcode = *format) != '\0' && fcode != '%');
		
			count += (n = format - bp); /* n = no. of non-% chars */
			PUT(bp, n);
		}
		if (fcode == '\0') {  /* end of format; return */
			register int nn = bufptr - iop->_ptr;
			iop->_cnt -= nn;
			iop->_ptr = bufptr;
			if (bufptr + iop->_cnt > bufferend && /* in case of */
					iop->_file != _NFILE) /* interrupt */
				_bufsync(iop);	/* during last several lines */
			if (iop->_flag & (_IONBF | _IOLBF) &&
				    (iop->_flag & _IONBF ||
				     memchr((char *)(bufptr-count), '\n', count) != NULL))
				(void) _xflsbuf(iop);
			return(ferror(iop) ? EOF : count);
		}

		/*
		 *	% has been found.
		 *	The following switch is used to parse the format
		 *	specification and to perform the operation specified
		 *	by the format letter.  The program repeatedly goes
		 *	back to this switch until the format letter is
		 *	encountered.
		 */
		width = prefixlength = otherlength = flagword = suffixlength = 0;
		format++;

	charswitch:

		switch (fcode = *format++) {

		case '+':
			flagword |= FPLUS;
			goto charswitch;
		case '-':
			flagword |= FMINUS;
			goto charswitch;
		case ' ':
			flagword |= FBLANK;
			goto charswitch;
		case '#':
			flagword |= FSHARP;
			goto charswitch;

		/* Scan the field width and precision */
		case '.':
			flagword |= DOTSEEN;
			prec = 0;
			goto charswitch;

		case '*':
			if (!(flagword & DOTSEEN)) {
				width = va_arg(args, int);
				if (width < 0) {
					width = -width;
					flagword ^= FMINUS;
				}
			} else {
				prec = va_arg(args, int);
				if (prec < 0)
					prec = 0;
			}
			goto charswitch;

		case '0':	/* obsolescent spec:  leading zero in width */
				/* means pad with leading zeros */
			if (!(flagword & (DOTSEEN | FMINUS)))
				flagword |= PADZERO;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		      { register num = fcode - '0';
			while (isdigit(fcode = *format)) {
				num = num * 10 + fcode - '0';
				format++;
			}
			if (flagword & DOTSEEN)
				prec = num;
			else
				width = num;
			goto charswitch;
		      }

		/* Scan the length modifier */
		case 'l':
			flagword |= LENGTH;
			/* No break */
		case 'h':
			goto charswitch; 

		/*
		 *	The character addressed by format must be
		 *	the format letter -- there is nothing
		 *	left for it to be.
		 *
		 *	The status of the +, -, #, and blank
		 *	flags are reflected in the variable
		 *	"flagword".  "width" and "prec" contain
		 *	numbers corresponding to the digit
		 *	strings before and after the decimal
		 *	point, respectively. If there was no
		 *	decimal point, then flagword & DOTSEEN
		 *	is false and the value of prec is meaningless.
		 *
		 *	The following switch cases set things up
		 *	for printing.  What ultimately gets
		 *	printed will be padding blanks, a
		 *	prefix, left padding zeroes, a value,
		 *	right padding zeroes, a suffix, and
		 *	more padding blanks.  Padding blanks
		 *	will not appear simultaneously on both
		 *	the left and the right.  Each case in
		 *	this switch will compute the value, and
		 *	leave in several variables the informa-
		 *	tion necessary to construct what is to
		 *	be printed.  
		 *
		 *	The prefix is a sign, a blank, "0x",
		 *	"0X", or null, and is addressed by
		 *	"prefix".
		 *
		 *	The suffix is either null or an
		 *	exponent, and is addressed by "suffix".
		 *	If there is a suffix, the flagword bit
		 *	SUFFIX will be set.
		 *
		 *	The value to be printed starts at "bp"
		 *	and continues up to and not including
		 *	"p".
		 *
		 *	"lzero" and "rzero" will contain the
		 *	number of padding zeroes required on
		 *	the left and right, respectively.
		 *	The flagword bits LZERO and RZERO tell
		 *	whether padding zeros are required.
		 *
		 *	The number of padding blanks, and
		 *	whether they go on the left or the
		 *	right, will be computed on exit from
		 *	the switch.
		 */



		
		/*
		 *	decimal fixed point representations
		 *
		 *	HIBITL is 100...000
		 *	binary, and is equal to	the maximum
		 *	negative number.
		 *	We assume a 2's complement machine
		 */

		case 'i':
		case 'd':
			/* Fetch the argument to be printed */
			if (flagword & LENGTH)
				val = va_arg(args, long);
			else
				val = va_arg(args, int);

			/* Set buffer pointer to last digit */
	                p = bp = buf + MAXDIGS;

			/* If signed conversion, make sign */
			if (val < 0) {
				prefix = "-";
				prefixlength = 1;
				/*
				 * Negate, checking in
				 * advance for possible
				 * overflow.
				 */
				if (val != HIBITL)
					val = -val;
				else     /* number is -HIBITL; convert last */
					 /* digit now and get positive number */
					*--bp = _lowdigit(&val);
			} else if (flagword & FPLUS) {
				prefix = "+";
				prefixlength = 1;
			} else if (flagword & FBLANK) {
				prefix = " ";
				prefixlength = 1;
			}

		decimal:
			{ register long qval = val;
				if (qval <= 9) {
					if (qval != 0 || !(flagword & DOTSEEN))
						*--bp = qval + '0';
				} else {
					do {
						n = qval;
						qval /= 10;
						*--bp = n - qval * 10 + '0';
					} while (qval > 9);
					*--bp = qval + '0';
				}
			}

			/* Calculate minimum padding zero requirement */
			if (flagword & DOTSEEN) {
				register leadzeroes = prec - (p - bp);
				if (leadzeroes > 0) {
					otherlength = lzero = leadzeroes;
					flagword |= LZERO;
				}
			}

			break;

		case 'u':
			/* Fetch the argument to be printed */
			if (flagword & LENGTH)
				val = va_arg(args, long);
			else
				val = va_arg(args, unsigned);

			p = bp = buf + MAXDIGS;

			if (val & HIBITL)
				*--bp = _lowdigit(&val);

			goto decimal;

		/*
		 *	non-decimal fixed point representations
		 *	for radix equal to a power of two
		 *
		 *	"mradix" is one less than the radix for the conversion.
		 *	"lradix" is one less than the base 2 log
		 *	of the radix for the conversion. Conversion is unsigned.
		 *	HIBITL is 100...000
		 *	binary, and is equal to	the maximum
		 *	negative number.
		 *	We assume a 2's complement machine
		 */

		case 'o':
			mradix = 7;
			lradix = 2;
			goto fixed;

		case 'X':
		case 'x':
			mradix = 15;
			lradix = 3;

		fixed:
			/* Fetch the argument to be printed */
			if (flagword & LENGTH)
				val = va_arg(args, long);
			else
				val = va_arg(args, unsigned);

			/* Set translate table for digits */
			  tab = (fcode == 'X') ? uc_digs : lc_digs;

			/* Entry point when printing a double which is a NaN */
		put_pc:
			/* Develop the digits of the value */
			p = bp = buf + MAXDIGS;
			{ register long qval = val;
				if (qval == 0) {
					if (!(flagword & DOTSEEN)) {
						otherlength = lzero = 1;
						flagword |= LZERO;
					}
				} else
					do {
						*--bp = tab[qval & mradix];
						qval = ((qval >> 1) & ~HIBITL)
								 >> lradix;
					} while (qval != 0);
			}

			/* Calculate minimum padding zero requirement */
			if (flagword & DOTSEEN) {
				register leadzeroes = prec - (p - bp);
				if (leadzeroes > 0) {
					otherlength = lzero = leadzeroes;
					flagword |= LZERO;
				}
			}

			/* Handle the # flag */
			if (flagword & FSHARP && val != 0)
				switch (fcode) {
				case 'o':
					if (!(flagword & LZERO)) {
						otherlength = lzero = 1;
						flagword |= LZERO;
					}
					break;
				case 'x':
					prefix = "0x";
					prefixlength = 2;
					break;
				case 'X':
					prefix = "0X";
					prefixlength = 2;
					break;
				}

			break;

		case 'E':
		case 'e':
			/*
			 * E-format.  The general strategy
			 * here is fairly easy: we take
			 * what ecvt gives us and re-format it.
			 */

			/* Establish default precision */
			if (!(flagword & DOTSEEN))
				prec = 6;

			/* Fetch the value */
	                dval = va_arg(args, double);

                        /* Check for NaNs and Infinities */
			if (IsNANorINF(dval))  {
			   if (IsINF(dval)) {
			      if (IsNegNAN(dval)) 
				neg_inf = 1;
			      Inf = 1;
			      bp = (fcode == 'E')? uc_inf: lc_inf;
			      p = bp + 3;
			      break;
                           }
			   else {
				if (IsNegNAN(dval)) {
					prefix = "-";
					prefixlength = 1;
				}
			        val  = GETNaNPC(dval); 
				NaN_flg = SNLEN;
				mradix = 15;
				lradix = 3;
				if (fcode == 'E') {
					SNAN = uc_nan;
					tab =  uc_digs;
				}
				else {
					SNAN =  lc_nan;
					tab =  lc_digs;
				}
				goto put_pc;
                           }
			}
			/* Develop the mantissa */
			bp = ecvt(dval, min(prec + 1, MAXECVT), &decpt, &sign);

			/* Determine the prefix */
		e_merge:
			if (sign) {
				prefix = "-";
				prefixlength = 1;
			} else if (flagword & FPLUS) {
				prefix = "+";
				prefixlength = 1;
			} else if (flagword & FBLANK) {
				prefix = " ";
				prefixlength = 1;
			}

			/* Place the first digit in the buffer*/
			p = &buf[0];
			*p++ = (*bp != '\0') ? *bp++ : '0';

			/* Put in a decimal point if needed */
			if (prec != 0 || (flagword & FSHARP))
				*p++ = '.';

			/* Create the rest of the mantissa */
			{ register rz = prec;
				for ( ; rz > 0 && *bp != '\0'; --rz)
					*p++ = *bp++;
				if (rz > 0) {
					otherlength = rzero = rz;
					flagword |= RZERO;
				}
			}

			bp = &buf[0];

			/* Create the exponent */
			*(suffix = &expbuf[MAXESIZ]) = '\0';
			if (dval != 0) {
				register int nn = decpt - 1;
				if (nn < 0)
				    nn = -nn;
				for ( ; nn > 9; nn /= 10)
					*--suffix = todigit(nn % 10);
				*--suffix = todigit(nn);
			}

			/* Prepend leading zeroes to the exponent */
			while (suffix > &expbuf[MAXESIZ - 2])
				*--suffix = '0';

			/* Put in the exponent sign */
			*--suffix = (decpt > 0 || dval == 0) ? '+' : '-';

			/* Put in the e */
			*--suffix = isupper(fcode) ? 'E'  : 'e';

			/* compute size of suffix */
			otherlength += (suffixlength = &expbuf[MAXESIZ]
								 - suffix);
			flagword |= SUFFIX;

			break;

		case 'f':
			/*
			 * F-format floating point.  This is a
			 * good deal less simple than E-format.
			 * The overall strategy will be to call
			 * fcvt, reformat its result into buf,
			 * and calculate how many trailing
			 * zeroes will be required.  There will
			 * never be any leading zeroes needed.
			 */

			/* Establish default precision */
			if (!(flagword & DOTSEEN))
				prec = 6;

			/* Fetch the value */
			dval = va_arg(args, double);

                        /* Check for NaNs and Infinities  */
			if (IsNANorINF(dval)) {
			   if (IsINF(dval)) {
			      if (IsNegNAN(dval))
				neg_inf = 1;
                              Inf = 1;
			      bp = lc_inf;
			      p = bp + 3;
			      break;
                           }
                           else {
				if (IsNegNAN(dval)) {
					prefix = "-";
					prefixlength = 1;
				}
			        val  = GETNaNPC(dval);
				NaN_flg = SNLEN;
				mradix = 15;
				lradix = 3;
				tab =  lc_digs;
				SNAN = lc_nan;
				goto put_pc;
                           }
                        } 
			/* Do the conversion */
			bp = fcvt(dval, min(prec, MAXFCVT), &decpt, &sign);

			/* Determine the prefix */
		f_merge:
			if (sign && decpt > -prec && *bp != '0') {
				prefix = "-";
				prefixlength = 1;
			} else if (flagword & FPLUS) {
				prefix = "+";
				prefixlength = 1;
			} else if (flagword & FBLANK) {
				prefix = " ";
				prefixlength = 1;
			}

			/* Initialize buffer pointer */
			p = &buf[0];

			{ register int nn = decpt;

				/* Emit the digits before the decimal point */
				k = 0;
				do {
					*p++ = (nn <= 0 || *bp == '\0' 
						|| k >= MAXFSIG) ?
				    		'0' : (k++, *bp++);
				} while (--nn > 0);

				/* Decide whether we need a decimal point */
				if ((flagword & FSHARP) || prec > 0)
					*p++ = '.';

				/* Digits (if any) after the decimal point */
				nn = min(prec, MAXFCVT);
				if (prec > nn) {
					flagword |= RZERO;
					otherlength = rzero = prec - nn;
				}
				while (--nn >= 0)
					*p++ = (++decpt <= 0 || *bp == '\0' ||
				   	    k >= MAXFSIG) ? '0' : (k++, *bp++);
			}

			bp = &buf[0];

			break;

		case 'G':
		case 'g':
			/*
			 * g-format.  We play around a bit
			 * and then jump into e or f, as needed.
			 */
		
			/* Establish default precision */
			if (!(flagword & DOTSEEN))
				prec = 6;
			else if (prec == 0)
				prec = 1;

			/* Fetch the value */
			dval = va_arg(args, double);

			/* Check for NaN and Infinities  */
			if (IsNANorINF(dval)) {
			   if (IsINF(dval)) {
			      if (IsNegNAN(dval)) 
				neg_inf = 1;
			   bp = (fcode == 'G') ? uc_inf : lc_inf;
			   p = bp + 3;
                           Inf = 1;
			   break;
                           }
                           else {
				if (IsNegNAN(dval)) {
					prefix = "-";
					prefixlength = 1;
				}
			        val  = GETNaNPC(dval);
				NaN_flg = SNLEN;
				mradix = 15;
				lradix = 3;
				if ( fcode == 'G') {
					SNAN = uc_nan;
					tab = uc_digs;
				}
				else {
					SNAN = lc_nan;
					tab =  lc_digs;
				}
				goto put_pc;
                           }
                        }

			/* Do the conversion */
			bp = ecvt(dval, min(prec, MAXECVT), &decpt, &sign);
			if (dval == 0)
				decpt = 1;

			{ register int kk = prec;
				if (!(flagword & FSHARP)) {
					n = strlen(bp);
					if (n < kk)
						kk = n;
					while (kk >= 1 && bp[kk-1] == '0')
						--kk;
				}
				
				if (decpt < -3 || decpt > prec) {
					prec = kk - 1;
					goto e_merge;
				}
				prec = kk - decpt;
				goto f_merge;
			}

		case '%':
			buf[0] = fcode;
			goto c_merge;

		case 'c':
			buf[0] = va_arg(args, int);
		c_merge:
			p = (bp = &buf[0]) + 1;
			break;

		case 's':
			bp = va_arg(args, char *);
			if (!(flagword & DOTSEEN))
				p = bp + strlen(bp);
			else { /* a strnlen function would  be useful here! */
				register char *qp = bp;
				while (*qp++ != '\0' && --prec >= 0)
					;
				p = qp - 1;
			}
			break;

		default: /* this is technically an error; what we do is to */
			/* back up the format pointer to the offending char */
			/* and continue with the format scan */
			format--;
			continue;

		}
       
		if (Inf) {
		   if (neg_inf) {
			prefix = "-";
			prefixlength = 1;
			neg_inf = 0;
                   }
		   else if (flagword & FPLUS) {
			prefix = "+";
			prefixlength = 1;
			}
		   else if (flagword & FBLANK) {
			prefix = " ";
			prefixlength = 1;
		   }
		   Inf = 0;
		}
		 
		/* Calculate number of padding blanks */
		k = (n = p - bp) + prefixlength + otherlength + NaN_flg;
		if (width <= k)
			count += k;
		else {
			count += width;

			/* Set up for padding zeroes if requested */
			/* Otherwise emit padding blanks unless output is */
			/* to be left-justified.  */

			if (flagword & PADZERO) {
				if (!(flagword & LZERO)) {
					flagword |= LZERO;
					lzero = width - k;
				}
				else
					lzero += width - k;
				k = width; /* cancel padding blanks */
			} else
				/* Blanks on left if required */
				if (!(flagword & FMINUS))
					PAD(_blanks, width - k);
		}

		/* Prefix, if any */
		if (prefixlength != 0)
			PUT(prefix, prefixlength);

		/* If value is NaN, put string NaN*/
		if (NaN_flg) {
			PUT(SNAN,SNLEN);
			NaN_flg = 0;
                }

		/* Zeroes on the left */
		if (flagword & LZERO)
			PAD(_zeroes, lzero);
		
		/* The value itself */
		if (n > 0)
			PUT(bp, n);

		if (flagword & (RZERO | SUFFIX | FMINUS)) {
			/* Zeroes on the right */
			if (flagword & RZERO)
				PAD(_zeroes, rzero);

			/* The suffix */
			if (flagword & SUFFIX)
				PUT(suffix, suffixlength);

			/* Blanks on the right if required */
			if (flagword & FMINUS && width > k)
				PAD(_blanks, width - k);
		}
	}
}
E 1
