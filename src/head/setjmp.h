/*	@(#)setjmp.h	6.1		*/
#ifndef _JBLEN

#if vax || u3b5
#define _JBLEN	10
#endif

#if pdp11
#define _JBLEN	3
#endif

#if u370
#define _JBLEN	4
#endif

#if u3b
#define _JBLEN	11
#endif

#if m68k
#define _JBLEN	13
#endif

typedef int jmp_buf[_JBLEN];

extern int setjmp();
extern void longjmp();

#endif
