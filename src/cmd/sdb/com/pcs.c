/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/pcs.c	1.13"

/*
 *	UNIX debugger
 */

#include "head.h"

extern MSG		NOBKPT;
extern MSG		SZBKPT;
extern MSG		EXBKPT;
extern MSG		NOPCS;
extern MSG		BADMOD;

extern L_INT		loopcnt;	/* used here and in runpcs.c */



/* sub process control */

subpcs(modif)
{
	REG INT		check;
	INT		execsig,runmode;
	REG BKPTR	bkptr;
	STRING		comptr;
	extern int errlev;		/* in dis/bits.c */
#if DEBUG
	if (debugflag ==1)
	{
		enter1("subpcs");
	}
	else if (debugflag == 2)
	{
		enter2("subpcs");
		arg("modif");
		printf("0x%x",modif);
		closeparen();
	}
#endif
	execsig=0; loopcnt=cntval;

	switch (modif) {

	    /* delete breakpoint */
	    case 'd': case 'D':
		if (bkptr=scanbkpt(dot)) {
			bkptr->flag = 0;
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("subpcs");
				printf("0x%x",1);
				endofline();
			}
#endif
			return(1);
		}
		else {
			error(NOBKPT);
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("subpcs");
				printf("0x%x",-1);
				endofline();
			}
#endif
			return(-1);
		}

	    /* set breakpoint */
	    case 'b': case 'B':
#if u3b2
		/*	Skip past save instruction.
		*/

		check = get(dot, ISP);
 		if((check & 0xff000000) == (0x10 << 24))
			dot += 2;
#endif
		dis_dot(dot,ISP,'\0');	/* sets errlev if can't disassemble */
		if(errlev > 0)	/* warn if not on instruction boundary */
		    fprintf(FPRT1,
			"Warning: Breakpoint at illegal instruction.\n");
		if (bkptr=scanbkpt(dot)) bkptr->flag = 0;
		for (bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt) {
			if (bkptr->flag == 0) break;
		}
		if (bkptr == 0) {
			if ((bkptr=(BKPTR) sbrk(sizeof *bkptr)) == (BKPTR) -1)
			{
				error(SZBKPT);
			}
			else {
				bkptr->nxtbkpt = bkpthead;
				bkpthead=bkptr;
			}
		}
		bkptr->loc = dot;
		bkptr->initcnt = bkptr->count = cntval;
		bkptr->flag = BKPTSET;
		check=MAXCOM-2; comptr=bkptr->comm;	/* rdc(); */
		if (cmd == 'b' || cmd == 'B' || cmd == 'a')
		{
			do
			{
				*comptr++ = readchar();
			} while (check-- && lastc != '\n');
		}
		else
			*comptr++ = '\n';
		*comptr = '\0';
		if (check)
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("subpcs");
				printf("0x%x",-1);
				endofline();
			} 
#endif
			return(1);
		}
		else {
			error(EXBKPT);
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("subpcs");
				printf("0x%x",-1);
				endofline();
			}
#endif
			return(-1);
		}

	    /* exit */
	    case 'k' :case 'K':
		if (pid) {
			printf("%d: Killed", pid); endpcs(); return(1);
		}
		error(NOPCS);
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("subpcs");
				printf("0x%x",-1);
				endofline();
			}
#endif
		return(-1);

	    /* load program */
	    case 'r': case 'R':
		endpcs();
		setup();
		runmode = CONTIN;
#if u3b || u3b5 || u3b15 || u3b2 || m68k
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("subpcs");
				printf("0x%x",1);
				endofline();
			}
#endif
		return(1);
#else
		/* VAX automatically breaks at start+2; must continue */
		break;
#endif

	    /* single step */
	    case 's': case 'S':
		if (pid) {
			runmode=SINGLE; execsig=getsig(signo);
		}
		else {
			setup();
			loopcnt--;
		}
		break;

	    /* continue with optional signal */
	    case 'c': case 'C': case 0:
		if (pid==0) {
			error(NOPCS);
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("subpcs");
				printf("0x%x",-1);
				endofline();
			}
#endif
			return(-1);
		}
		runmode=CONTIN; execsig=getsig(signo);
		break;

	    default: error(BADMOD);
	}

	runpcs(runmode,execsig);
	delbp();
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("subpcs");
				printf("0x%x",1);
				endofline();
			}
#endif
	return(1);
}
