/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:init.c	1.7"
/*
 * This file contains code for the crash initialization.
 */

#include "sys/param.h"
#include "a.out.h"
#include "signal.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/var.h"
#include "crash.h"

int	nmlst_tstamp ,		/* timestamps for namelist and cdumpfiles */
	dmp_tstamp ;
extern char *dumpfile;		
extern char *namelist;
extern long vtop();
extern long lseek();
extern char *malloc();
struct syment *sp;		/* pointer to symbol table */
struct user *ubp;		/* pointer to ublock buffer */
long fltcrptr;			/* configuration registers */
long fltarptr;
long conrptr;
extern struct syment *File, *Proc, *Inode, *Mount, *V,
	*Panic, *Curproc;	/* namelist symbol pointers */


/* initialize buffers, symbols, and global variables for crash session */
int
init()
{
	int offset ;
	struct syment	*ts_symb = NULL;
	extern void sigint();
	
	if((mem = open(dumpfile, 0)) < 0)	/* open dump file, if error print */
		fatal("cannot open dump file %s\n",dumpfile);

	rdsymtab();			/* open and read the symbol table */

#ifndef m68k
	/* check timestamps */
	if(nmlst_tstamp == 0 || !(ts_symb = symsrch("crash_sync")))
		fprintf(fp,"WARNING - could not find timestamps, timestamps not checked\n") ;
	else
	{
		if((lseek(mem,vtop((long)ts_symb->n_value,-1),0) == -1)
		  || read(mem, (char *)&dmp_tstamp,sizeof(int)) != sizeof(int))
			fatal("could not process dumpfile with supplied namelist %s\n",namelist) ;

		if(dmp_tstamp != nmlst_tstamp)
		{
			fprintf(fp,"WARNING - namelist and system image not of the same vintage\n") ;
			fprintf(fp,"          crash results unpredictable\n") ;
		}
	}
#endif  /* m68k */

	if(!(V = symsrch("v")))
		fatal("var structure not found in symbol table\n");
	if(!(Inode = symsrch("inode")))
		fatal("inode not found in symbol table\n");
	if(!(Proc = symsrch("proc")))
		fatal("proc not found in symbol table\n");
	if(!(File = symsrch("file")))
		fatal("file not found in symbol table\n");
	if(!(Mount = symsrch("mount")))
		fatal("mount not found in symbol table\n");
	if(!(Panic = symsrch("panicstr")))
		fatal("panicstr not found in symbol table\n");
	if(!(Curproc = symsrch("curproc")))
		fatal("curproc not found in symbol table\n");

	readmem((long)V->n_value,1,-1,(char *)&vbuf,
		sizeof vbuf,"var structure");

	/* Allocate ublock buffer */
	ubp = (user_t*)malloc((unsigned)(sizeof(user_t) +
		sizeof(int)*vbuf.v_nofiles));

	Procslot = getcurproc();
	/* setup break signal handling */
	if(signal(SIGINT,sigint) == SIG_IGN)
		signal(SIGINT,SIG_IGN);
}

