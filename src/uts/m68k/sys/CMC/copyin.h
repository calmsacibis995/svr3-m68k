/*
 * This file contains macros which map the standard "copyin" and "copyout"
 * system routines into the "mmuread" and "mmuwrite" routines used on early
 * versions of the VME software.
 *
 * If your system already has copyin and copyout, you should remove these
 * defines.
 */

#ifdef VME27			/* only system known to have this problem */
#define copyin(from,to,lth)	mmuread ((from), (to), (lth))
#define copyout(from,to,lth)	mmuwrite ((to), (from), (lth))
#endif
