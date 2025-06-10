/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/prf.c	10.8"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/mvme131.h"
#include "sys/cmn_err.h"

/* general argument list for cmn_err and panic */

# define ARGS	a0,a1,a2,a3,a4,a5

/*	Save output in a buffer where we can look at it
**	with demon or crash.  If the message begins with
**	a '!', then only put it in the buffer, not out
**	to the console.
*/

extern	char	putbuf[];
extern	int	putbufsz;
int		putbufndx;
short		prt_where;

#define	output(c) \
	if (prt_where & PRW_BUF) { \
		putbuf[putbufndx++ % putbufsz] = c; \
	} \
	if (prt_where & PRW_CONS) \
		putchar(c);

char	*panicstr ;

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much suspended.
 * Printf should not be used for chit-chat.
 */

printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register int	c;
	register uint	*adx;
	register char	*s;
	register int	opri;

	opri = splhi();
	adx = &x1;

loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			splx(opri);
			wakeup(putbuf);
			return;
		}
		output(c);
	}
	c = *fmt++;
	if (c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if (c == 's') {
		s = (char *)*adx;
		while (c = *s++) {
			output(c);
		}
	} else if (c == 'D') {
		printn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}

printn(n, b)
long n;
register b;
{
	register i, nd, c;
	int	flag;
	int	plmax;
	char d[12];

	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		output('-');
	}
	for (i=0;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = nd;
		n = n/b;
		if ((n==0) && (flag==0))
			break;
	}
	if (i==plmax)
		i--;
	for (;i>=0;i--) {
		output("0123456789ABCDEF"[d[i]]);
	}
}

/*
 * Panic is called on unresolvable fatal errors.
 * Change name to force unresolve references.
 * All routines should use cmn_err.
 * NOTE: Nothing is printed here.
 *	 To print a message use cmn_err.
 */

xpanic(fmt, ARGS)
char	*fmt;
int	ARGS ;
{
	extern breakpoint();

	if (!panicstr && fmt) {
		panicstr = fmt;		/* save panic string */
		update();		/* "sync" */
/*		unixcore(); 		/* write unix core image */
	}

	*CNT5 |= CNT5_BDFAIL;		/* generate VME SYSFAIL signal */

#ifdef	DEBUG
	breakpoint();
#endif
	for(;;) {
	}
}

/*
 * prdev prints a warning message.
 * dev is a block special device argument.
 */

prdev(str, dev)
char *str;
dev_t dev;
{
	register maj;

	maj = bmajor(dev);
	if (maj >= bdevcnt) {
		cmn_err(CE_WARN,"%s on bad dev %o(8)\n", str, dev);
		return;
	}
	(*bdevsw[maj].d_print)(dev, str);
}

/*
 * prcom prints a diagnostic from a device driver.
 * prt is device dependent print routine.
 */
prcom(prt, bp, er1, er2)
int (*prt)();
register struct buf *bp;
{
	(*prt)(bp->b_dev, "\nDevice error");
	cmn_err(CE_NOTE,"bn = %D er = %o,%o\n", bp->b_blkno, er1, er2);
}

cmn_err(level, fmt, ARGS)
register int	level ;
char		*fmt;
int		ARGS ;
{
	static	int	panic_level = 0;
	int	pri;

	/*	Set up to print to putbuf, console, or both
	**	as indicated by the first character of the
	**	format.
	*/

	pri = splhi();
	if (*fmt == '!') {
		prt_where = PRW_BUF;
		fmt++;
	} else if (*fmt == '^') {
		prt_where = PRW_CONS;
		fmt++;
	} else {
		prt_where = PRW_BUF | PRW_CONS;
	}

	switch (level) {
		case CE_CONT :
			printf(fmt, ARGS) ;
			break ;

		case CE_NOTE :
			printf("\nNOTICE: ") ;
			printf(fmt, ARGS) ;
			printf("\n") ;
			break ;

		case CE_WARN :
			printf("\nWARNING: ") ;
			printf(fmt, ARGS) ;
			printf("\n") ;
			break ;

		case CE_PANIC :
			prt_where = PRW_CONS | PRW_BUF;
			panic_level++ ;
			printf("\nPANIC (LEVEL %d): ",panic_level) ;
			printf(fmt, ARGS) ;
			printf("\n") ;
			splx(pri);
			xpanic(fmt, ARGS) ;

		default :
			cmn_err(CE_PANIC,
			  "unknown level in cmn_err (level=%d, msg=\"%s\")",
			  level, fmt, ARGS) ;
	}
	splx(pri);
}

/*	The following is an interface routine for the drivers.
*/

dri_printf(ARGS)
{
	cmn_err(CE_CONT, ARGS);
}

/*	Called by the ASSERT macro in debug.h when an
**	assertion fails.
*/

assfail(a, f, l)
register char *a, *f;
{
	cmn_err(CE_PANIC, "assertion failed: %s, file: %s, line: %d", a, f, l);
}


nomemmsg(func, count, contflg, lockflg)
register char	*func;
register int	count;
register int	contflg;
register int	lockflg;
{
	cmn_err(CE_NOTE,
		"%s - Insufficient memory to%s %d%s page%s - %s",
		func, (lockflg ? " lock" : " allocate"),
		count, (contflg ? " contiguous" : ""),
		(count == 1 ? "" : "s"),
		"system call failed");
}
