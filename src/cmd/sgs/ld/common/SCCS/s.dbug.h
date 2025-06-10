h10364
s 00000/00000/00027
d D 1.2 86/08/18 08:45:38 fnf 2 1
c Checkpoint changes at first working shared library version
e
s 00027/00000/00000
d D 1.1 86/07/30 14:26:01 fnf 1 0
c date and time created 86/07/30 14:26:01 by fnf
e
u
U
t
T
I 1
/*
 *	The default is to use the macro based debugging package.
 *	If this is not available, then just define NO_DBUG and all
 *	the macros will magically disappear.   Fred Fish, 29-Jul-86
 */

#ifndef NO_DBUG
#  ifndef EOF
#    include <stdio.h>	/* dbug.h needs stdio.h */
#  endif
#  include <local/dbug.h>
#else
#  define DBUG_ENTER(a1)
#  define DBUG_RETURN(a1) return(a1)
#  define DBUG_VOID_RETURN return
#  define DBUG_EXECUTE(keyword,a1)
#  define DBUG_2(keyword,format)
#  define DBUG_3(keyword,format,a1)
#  define DBUG_4(keyword,format,a1,a2)
#  define DBUG_5(keyword,format,a1,a2,a3)
#  define DBUG_PUSH(a1)
#  define DBUG_POP()
#  define DBUG_PROCESS(a1)
#  define DBUG_FILE (stderr)
#  define DBUG_SETJMP setjmp
#  define DBUG_LONGJMP longjmp
#endif
E 1
