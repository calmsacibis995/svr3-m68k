/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:cmd.h	1.4"
/* crash/cmd.h */
/*
 *		Copyright 1984 AT&T
 */

struct	tsw	t[] = {
	"adv",		ADV,	"advertise table",
	"buf",		BUFHDR,	"buffer headers",
	"buffer",	BUFFER,	"buffer data",
	"b",		BUFFER,	NULL,
	"bufhdr",	BUFHDR,	NULL,
	"hdr",		BUFHDR,	NULL,
	"callout",	TOUT,	"callout table",
	"calls",	TOUT,	NULL,
	"call",		TOUT,	NULL,
	"timeout",	TOUT,	NULL,
	"time",		TOUT,	NULL,
	"tout",		TOUT,	NULL,
	"c",		TOUT,	NULL,
	"dblock",	DBLK,	"allocated stream data block headers",
	"ds",		DS,	"data address namelist search",
	"file",		FILES,	"file table",
	"files",	FILES,	NULL,
	"f",		FILES,	NULL,
	"findslot",	FNDSLT,	"find table and slot number for given address",
	"freelist",	FLIST,	"list free message or data block headers",
	"gdp",		PRGDP,	"gdp structures",
	"inode",	INODE,	"inode table",
	"inode -",	INODE,	"inode table with block addresses",
	"ino",		INODE,	NULL,
	"i",		INODE,	NULL,
	"kfp",		KFP,	"frame pointer for start of stack trace",
	"fp",		KFP,	NULL,
	"r9",		KFP,	NULL,
	"lck",		LCK,	"record lock tables",
	"l",		LCK,	NULL,
	"lnkr",		LNKR,	"link record table",
	"map",		MAP,	"map structures",
	"mblock",	MESS,	"allocated stream message block headers",
	"mount",	MOUNT,	"mount table",
	"mnt",		MOUNT,	NULL,
	"m",		MOUNT,	NULL,
	"nm",		NM,	"name search",
	"od",		OD,	"dump symbol values",
	"dump",		OD,	NULL,
	"rd",		OD,	NULL,
	"pdt",		PDT,	"page descriptor tables",
	"pfdat",	PFDAT,	"pfdat structure",
	"pf",		PFDAT,	NULL,
	"proc",		PROC,	"process table",
	"proc -",	PROC,	"process table long listing",
	"proc -r",	PROC,	"runnable processes only",
	"ps",		PROC,	NULL,
	"p",		PROC,	NULL,
	"proto",	PROTO,	"protocol structures",
	"qrun",		QRUN,	"list of servicable stream queues",
	"queue",	C_QUEUE,	"allocated stream queues",
	"quit",		QUIT,	"exit",
	"q",		QUIT,	NULL,
	"rcvd",		RCVD,	"receive descriptor",
	"region",	REGION,	"region table",
	"regs",		REGS,	"mmu registers",
	"sdt",		SDT,	"segment descriptor table",
	"sndd",		SNDD,	"send descriptor",
	"sram",		SRAM,	"srams",
	"srmount",	SRMNT,	"server mount table",
	"kernel",	STACK,	NULL,
	"k",		STACK,	NULL,
	"stack",	STACK,	"stack dump",
	"stk",		STACK,	NULL,
	"s",		STACK,	NULL,
	"stat",		STAT,	"dump statistics",
	"stream",	STREAM,	"allocated stream table slots",
	"strstat",	STRSTAT,	"streams statistics",
	"trace",	TRACE,	"stack trace",
	"trace -r",	TRACE,	"trace using saved frame pointer in kfp",
	"t",		TRACE,	NULL,
	"ts",		TS,	"text address namelist search",
	"tty",		TTY,	"tty structures",
	"tty -",	TTY,	"tty structures with full stty parameters",
#ifndef m68k
	"tty type", 	TTY,	"use tty structure type(i.e. ad for adli or con for console)",
#else
	"tty type",	TTY,	"use type tty structure(i.e. acia,m400,n720,mccm,m5,mfp,m564)",
	"acia",		TTY,	NULL,	/* Exormacs only */
	"m400",		TTY,	NULL,	/* VME10 only */
	"n720",		TTY,	NULL,	/* VM03 */
	"mccm",		TTY,	NULL,	/* VM03, Exormacs */
	"m5",		TTY,	NULL,	/* VME120 */
	"mfp",		TTY,	NULL,	/* VME120 */
	"m564",		TTY,	NULL,	/* VM04 */
#endif
	"term",		TTY,	NULL,
#ifndef m68k
	"ad",		TTY,	NULL,
	"con",		TTY,	NULL,
#endif
	"user",		UAREA,	"user area",
	"u_area",	UAREA,	NULL,
	"uarea",	UAREA,	NULL,
	"u",		UAREA,	NULL,
	"ublock",	UAREA,  NULL,
	"var",		VAR,	"system variables",
	"vtop",		VTOP,	"virtual to physical address",
	"tunables",	VAR,	NULL,
	"tunable",	VAR,	NULL,
	"tune",		VAR,	NULL,
	"v",		VAR,	NULL,
	"!",		Q,	"escape to shell",
	"?",		Q,	"print this list of available commands",
	NULL,		NULL,	NULL
};

struct	prmode	prm[] = {
	"decimal",	DECIMAL,
	"dec",		DECIMAL,
	"e",		DECIMAL,
	"octal",	OCTAL,
	"oct",		OCTAL,
	"o",		OCTAL,
	"directory",	DIRECT,
	"direct",	DIRECT,
	"dir",		DIRECT,
	"d",		DIRECT,
	"write",	WRITE,
	"w",		WRITE,
	"character",	CHAR,
	"char",		CHAR,
	"c",		CHAR,
	"inode",	INODE,
	"ino",		INODE,
	"i",		INODE,
	"byte",		BYTE,
	"b",		BYTE,
	"hex",		HEX,
	"h",		HEX,
	"x",		HEX,
	"hexadec",	HEX,
	"hexadecimal",	HEX,
	"string",	STRING,
	"s",		STRING,
	NULL,		NULL
} ;
