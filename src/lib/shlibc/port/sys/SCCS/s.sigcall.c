h39563
s 00170/00000/00000
d D 1.1 86/07/31 11:52:52 fnf 1 0
c This routine was carried forward from 5.2 release for m68k, not in 5.3 for 3b2
e
u
U
t
T
I 1
/* @(#)sigcall.c	6.1	 */

/* signal()	- The interface to the system call _signal().
**
** _sigcall()	- Calls the user's system call handler.
**
** The system call as described in the manual is _signal().
**
** signal() maintains its own array (sigtab[]) of signal handlers,
** and passes the address of _sigcode() to the kernel.
** When a signal is sent to this process, the kernel forces the execution
** of _sigcode(), which in turn calls _sigcall().
** _sigcall() looks up the address of the user's handler in sigtab[],
** and calls it.
** 
** 9-14-84 code modified to correct problem occurring with childeath
** signal. The call to _signal was being made before the function array
** was updated with the address of the user's handler. On return from
** the kernel, if a childeath signal was pending and this signal call was
** designed to install a handler address for childeath signals, the result
** would be that the previous function would be performed since the sigtab
** array had not been updated. Thus, the code to insert the new value into
** sigtab now precedes the call to _signal. If an error return occurs,
** the old function value will be restored to sigtab.
**
** 5-7-86 added routines sigset(), sighold(), sigignore(), sigrelse(),
** and sigpause() for R5V3.
*/ 

#include "sys/signal.h"
#include "sys/errno.h"

#define		T_SIGNAL	0
#define		T_SIGSET	1
#define		T_DEFAULT	T_SIGNAL

typedef int (*fn)();

extern int	errno;

static fn sigtab [ NSIG ];
static char setflg [ NSIG ] = T_SIGNAL;
static int whoflg = T_DEFAULT;	

fn sigset( sig, func )
register int sig;
register fn func;
{
	fn signal();

	whoflg = T_SIGSET;
	return(signal(sig,func));
}

fn signal( sig, func )
register int sig;
register fn func;
{
	register fn *p, old, tmp;
	extern int _signal();
	extern int _sigcode();

	if( sig <= 0 || sig > NSIG )
	{	errno = EINVAL;
		return( (fn) -1 );
	}
	tmp = _sigcode;
	if( func == (fn )SIG_HOLD || func == (fn )SIG_DFL || ((int) func)&1 )
		tmp = func;
	p = &sigtab[ sig ] - 1;
	old = *p;
	*p = func;
	if ( whoflg == T_SIGSET ) {
		/* mark as set with sigset(), not signal() */
		setflg[sig-1] = T_SIGSET;
	}
	else {
		/* clear flag so not confused with sigset() call */
		setflg[sig-1] = T_SIGNAL;
	}

	/*
	 * Reset whoflg to T_DEFAULT for next time time through this
	 * code before returning.
	 */
	/*
	 * for sigset() only, set SIGDERER flag in signal number
	 * This will tell the kernel that sigset() is the caller and
	 * not signal().
	 */
	if( ( tmp = (fn) _signal( ((whoflg==T_SIGSET)?(sig|SIGDEFER):sig), tmp ) ) == (fn) -1 )
        {
                *p = old;
		whoflg = T_DEFAULT;
		return( (fn) -1 );
        }
	whoflg = T_DEFAULT;
	return( tmp == _sigcode ? old : tmp );
}

sighold( sig )
register int sig;
{
	extern int _signal();

	if( sig <= 0 || sig > NSIG )
	{	errno = EINVAL;
		return( -1 );
	}
	sig |= SIGHOLD;
	_signal( sig );
	return;
}

sigrelse( sig )
register int sig;
{
	extern int _signal();

	if( sig <= 0 || sig > NSIG )
	{	errno = EINVAL;
		return( -1 );
	}
	sig |= SIGRELSE;
	_signal( sig );
	return;
}

sigignore( sig )
register int sig;
{
	extern int _signal();

	if( sig <= 0 || sig > NSIG )
	{	errno = EINVAL;
		return( -1 );
	}
	sig |= SIGIGNORE;
	_signal( sig );
	return;
}

sigpause( sig )
register int sig;
{
	extern int _signal();

	if( sig <= 0 || sig > NSIG )
	{	errno = EINVAL;
		return( -1 );
	}
	sig |= SIGPAUSE;
	_signal( sig );
	return;
}

void _sigcall( sig, arg )
register int sig;
{
	register fn func;

	if( sig > 0 && sig <= NSIG )
	{	func = sigtab[ sig - 1 ];
		if( func != (fn )SIG_HOLD && func != (fn )SIG_DFL && ( ((int) func)&1 ) == 0 ) {
			(*func)( sig, arg );
			if ( setflg[sig-1] == T_SIGSET )
				sigrelse(sig);
		}
	}
}
E 1
