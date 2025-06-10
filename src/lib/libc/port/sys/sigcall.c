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
**
** 12-10-86 rewrote sigset() and signal() to not modify the user function
** table when SIGHOLD is passed to sigset().  Removed use of the
** static variable whoflg (recursion problems).  Now return EINVAL when
** signal() iscalled with SIGHOLD as an argument.
*/ 

#include "sys/signal.h"
#include "sys/errno.h"

#define		T_SIGNAL	0
#define		T_SIGSET	1

typedef void (*fn)();

extern int	errno;

static fn sigtab [NSIG];
static char setflg [NSIG] = T_SIGNAL;

sighold(sig)
register int sig;
{
	extern int _signal();

	if (sig <= 0 || sig > NSIG) {
		errno = EINVAL;
		return(-1);
	}
	return(_signal(sig | SIGHOLD));
}

sigrelse(sig)
register int sig;
{
	extern int _signal();

	if (sig <= 0 || sig > NSIG) {
		errno = EINVAL;
		return(-1);
	}
	return(_signal(sig | SIGRELSE));
}

sigignore(sig)
register int sig;
{
	extern int _signal();

	if (sig <= 0 || sig > NSIG) {
		errno = EINVAL;
		return(-1);
	}
	return(_signal(sig | SIGIGNORE));
}

sigpause(sig)
register int sig;
{
	extern int _signal();

	if (sig <= 0 || sig > NSIG) {
		errno = EINVAL;
		return(-1);
	}
	return(_signal(sig | SIGPAUSE));
}

fn sigset(sig, func)
register int sig;
register fn func;
{
	register fn *p, oldfunc;
	extern int _signal();
	extern void _sigcode();

	if (sig <= 0 || sig > NSIG) {
		errno = EINVAL;
		return((fn) -1);
	}
	p = &sigtab[sig-1];
	oldfunc = *p;
	if (func != (fn)SIG_HOLD) {
		*p = func;
		setflg[sig-1] = T_SIGSET;
		if (func != (fn)SIG_DFL && ((int)func & 1) == 0)
			func = _sigcode;
	}
	if ((func = (fn) _signal(sig|SIGDEFER, func)) == (fn) -1) {
                *p = oldfunc;
		return((fn) -1);
        }
	return(func == _sigcode ? oldfunc : func);
}

fn signal(sig, func)
register int sig;
register fn func;
{
	register fn *p, oldfunc;
	extern int _signal();
	extern void _sigcode();

	if (sig <= 0 || sig > NSIG || func == (fn)SIG_HOLD) {
		errno = EINVAL;
		return((fn) -1);
	}
	p = &sigtab[sig-1];
	oldfunc = *p;
	*p = func;
	setflg[sig-1] = T_SIGNAL;
	if (func != (fn)SIG_DFL && ((int)func & 1) == 0)
		func = _sigcode;
	if ((func = (fn) _signal(sig, func)) == (fn) -1) {
                *p = oldfunc;
		return((fn) -1);
        }
	return((func == _sigcode) ? oldfunc : func);
}

void _sigcall(sig, arg)
register int sig;
{
	register fn func;

	if (sig > 0 && sig <= NSIG) {
		func = sigtab[ sig - 1 ];
		if (func != (fn)SIG_HOLD && func != (fn)SIG_DFL && (((int) func)&1) == 0) {
			(*func)(sig, arg);
			if (setflg[sig-1] == T_SIGSET)
				sigrelse(sig);
		}
	}
}
