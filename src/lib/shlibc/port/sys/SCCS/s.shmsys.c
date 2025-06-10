h12775
s 00048/00000/00000
d D 1.1 86/07/31 11:47:17 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
e
u
U
t
T
I 1
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:sys/shmsys.c	1.7"
#ifndef	pdp11
#include	"sys/types.h"
#include	"sys/ipc.h"
#include	"sys/shm.h"

#define	SHMSYS	52

#define	SHMAT	0
#define	SHMCTL	1
#define	SHMDT	2
#define	SHMGET	3

shmat(shmid, shmaddr, shmflg)
int shmid;
char *shmaddr;
int shmflg;
{
	return(syscall(SHMSYS, SHMAT, shmid, shmaddr, shmflg));
}

shmctl(shmid, cmd, buf)
int shmid, cmd;
struct shmid_ds *buf;
{
	return(syscall(SHMSYS, SHMCTL, shmid, cmd, buf));
}

shmdt(shmaddr)
char *shmaddr;
{
	return(syscall(SHMSYS, SHMDT, shmaddr));
}

shmget(key, size, shmflg)
key_t key;
int size, shmflg;
{
	return(syscall(SHMSYS, SHMGET, key, size, shmflg));
}
#endif
E 1
