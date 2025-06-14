/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sh:fault.c	1.8"
/*
 * UNIX shell
 */

#include	"defs.h"

extern done();

char	*trapcom[MAXTRAP];
BOOL	trapflg[MAXTRAP] =
{
	0,
	0,	/* hangup */
	0,	/* interrupt */
	0,	/* quit */
	0,	/* illegal instr */
	0,	/* trace trap */
	0,	/* IOT */
	0,	/* EMT */
	0,	/* float pt. exp */
	0,	/* kill */
	0, 	/* bus error */
	0,	/* memory faults */
	0,	/* bad sys call */
	0,	/* bad pipe call */
	0,	/* alarm */
	0, 	/* software termination */
	0,	/* unassigned */
	0,	/* unassigned */
	0,	/* death of child */
	0,	/* power fail */
};

(*(sigval[]))() = 
{
	0,
	done,
	fault,
	fault,
	done,
	done,
	done,
	done,
	done,
	0,
	done,
	done,
	done,
	done,
	fault,
	fault,
	done,
	done,
	done,
	done
};

/* ========	fault handling routines	   ======== */


fault(sig)
register int	sig;
{
	register int	flag;

	signal(sig, fault);
	if (sig == SIGSEGV)
	{
		if (setbrk(brkincr) == -1)
			error(nospace);
	}
	else if (sig == SIGALRM)
	{
		if (flags & waiting)
			done();
	}
	else
	{
		flag = (trapcom[sig] ? TRAPSET : SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
		if (sig == SIGINT)
			wasintr++;
	}
}

stdsigs()
{
	setsig(SIGHUP);
	setsig(SIGINT);
	ignsig(SIGQUIT);
	setsig(SIGILL);
	setsig(SIGTRAP);
	setsig(SIGIOT);
	setsig(SIGEMT);
	setsig(SIGFPE);
	setsig(SIGBUS);
	signal(SIGSEGV, fault);
	setsig(SIGSYS);
	setsig(SIGPIPE);
	setsig(SIGALRM);
	setsig(SIGTERM);
	setsig(SIGUSR1);
	setsig(SIGUSR2);
}

ignsig(n)
{
	register int	s, i;

	if ((i = n) == SIGSEGV)
	{
		clrsig(i);
		failed(badtrap, "cannot trap 11");
	}
	else if ((s = (signal(i, SIG_IGN) == SIG_IGN)) == 0)
	{
		trapflg[i] |= SIGMOD;
	}
	return(s);
}

getsig(n)
{
	register int	i;

	if (trapflg[i = n] & SIGMOD || ignsig(i) == 0)
		signal(i, fault);
}


catchsig(n)		/* GG hack */
register int	n;
{
	trapflg[n] |= SIGMOD;
	signal( n, fault );
}


setsig(n)
{
	register int	i;

	if (ignsig(i = n) == 0)
		signal(i, sigval[i]);
}

oldsigs()
{
	register int	i;
	register char	*t;

	i = MAXTRAP;
	while (i--)
	{
		t = trapcom[i];
		if (t == 0 || *t)
			clrsig(i);
		trapflg[i] = 0;
	}
	trapnote = 0;
}

clrsig(i)
int	i;
{
	free(trapcom[i]);
	trapcom[i] = 0;
	if (trapflg[i] & SIGMOD)
	{
		trapflg[i] &= ~SIGMOD;
		signal(i, sigval[i]);
	}
}

/*
 * check for traps
 */
chktrap()
{
	register int	i = MAXTRAP;
	register char	*t;

	trapnote &= ~TRAPSET;
	while (--i)
	{
		if (trapflg[i] & TRAPSET)
		{
			trapflg[i] &= ~TRAPSET;
			if (t = trapcom[i])
			{
				int	savxit = exitval;

				execexp(t, 0);
				exitval = savxit;
				exitset();
			}
		}
	}
}
