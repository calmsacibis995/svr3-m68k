h11010
s 00046/00000/00000
d D 1.1 86/07/31 12:32:53 fnf 1 0
c Pick up version Matt did for 5.3 m68k library.
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

#ident	"@(#)libc-m32:sys/shmsys.c	1.3"
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
E 1
