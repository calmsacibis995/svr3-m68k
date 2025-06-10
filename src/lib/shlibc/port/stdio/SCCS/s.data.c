h59830
s 00001/00001/00052
d D 1.2 86/07/31 11:44:41 fnf 2 1
c Add m68k to ifdefs...
e
s 00053/00000/00000
d D 1.1 86/07/31 11:42:35 fnf 1 0
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

#ident	"@(#)libc-port:stdio/data.c	2.9"
/*LINTLIBRARY*/
#include <stdio.h>

#if !u370

/* some slop is allowed at the end of the buffers in case an upset in
 * the synchronization of _cnt and _ptr (caused by an interrupt or other
 * signal) is not immediately detected.
 */
unsigned char _sibuf[BUFSIZ+8], _sobuf[BUFSIZ+8];
/*
 * Ptrs to start of preallocated buffers for stdin, stdout.
 */
#endif

unsigned char _smbuf[_NFILE+1][_SBFSIZ];

FILE _iob[_NFILE] = {
D 2
#if vax || u3b || M32
E 2
I 2
#if vax || u3b || M32 || m68k
E 2
	{ 0, NULL, NULL, _IOREAD, 0},
	{ 0, NULL, NULL, _IOWRT, 1},
	{ 0, _smbuf[2], _smbuf[2], _IOWRT+_IONBF, 2},
#endif
#if pdp11
	{ NULL, 0, NULL, _IOREAD, 0},
	{ NULL, 0, NULL, _IOWRT, 1},
	{ _smbuf[2], 0, _smbuf[2], _IOWRT+_IONBF, 2},
#endif
#if u370
	{ NULL, 0, NULL, _IOREAD, 0},
	{ NULL, 0, NULL, _IOWRT, 1},
	{ NULL, 0, NULL, _IOWRT+_IONBF, 2},
#endif
};
/*
 * Ptr to end of io control blocks
 */
FILE *_lastbuf = { &_iob[_NFILE] };

/*
 * Ptrs to end of read/write buffers for each device
 * There is an extra bufend pointer which corresponds to the dummy
 * file number _NFILE, which is used by sscanf and sprintf.
 */
unsigned char *_bufendtab[_NFILE+1] = { NULL, NULL, _smbuf[2]+_SBFSIZ, };
E 1
