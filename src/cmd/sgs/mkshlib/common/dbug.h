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
