h21746
s 00050/00000/00000
d D 1.1 86/07/31 12:32:44 fnf 1 0
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

#ident	"@(#)libc-m32:sys/msgsys.c	1.3"
#include	"sys/types.h"
#include	"sys/ipc.h"
#include	"sys/msg.h"

#define	MSGSYS	49

#define	MSGGET	0
#define	MSGCTL	1
#define	MSGRCV	2
#define	MSGSND	3

msgget(key, msgflg)
key_t key;
int msgflg;
{
	return(syscall(MSGSYS, MSGGET, key, msgflg));
}

msgctl(msqid, cmd, buf)
int msqid, cmd;
struct msqid_ds *buf;
{
	return(syscall(MSGSYS, MSGCTL, msqid, cmd, buf));
}

msgrcv(msqid, msgp, msgsz, msgtyp, msgflg)
int msqid;
struct msgbuf *msgp;
int msgsz;
long msgtyp;
int msgflg;
{
	return(syscall(MSGSYS, MSGRCV, msqid, msgp, msgsz, msgtyp, msgflg));
}

msgsnd(msqid, msgp, msgsz, msgflg)
int msqid;
struct msgbuf *msgp;
int msgsz, msgflg;
{
	return(syscall(MSGSYS, MSGSND, msqid, msgp, msgsz, msgflg));
}
E 1
