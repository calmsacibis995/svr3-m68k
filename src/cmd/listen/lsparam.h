/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)listen:lsparam.h	1.3"

/*
 * lsparam.h:	listener paramters.  Includes default pathnames.
 */

/* DEBUGMODE causes debug statements to be compiled in. */

/* #define DEBUGMODE	1 */

#ifdef	DEBUGMODE
#define	DEBUG(ARGS)	debug ARGS
#else
#define	DEBUG(ARGS)
#endif

/*
 * CHARADDR is a debug aid only!!!!
 * with DEBUGMODE, if CHARADDR is defined, logical addresses which
 * are represented by printable characters, will be displayed in the
 * debug/log files
 */

#ifdef	DEBUGMODE
#define CHARADDR
#endif

/* listener parameters							*/

#define MAXNAMESZ	15		/* must coexist with ms-net (5c) */
#define SNNMBUFSZ	16		/* starlan network only		*/
#define NAMEBUFSZ	64
#define QLEN		1		/* t_bind requestes qlen	*/
#define MINMSGSZ	(SMBIDSZ+1)	/* smallest acceptable msg size	*/
#define RCVBUFSZ	BSIZE		/* receive buffer size		*/
#define DBFLINESZ	BSIZE		/* max line size in data base 	*/
#define ALARMTIME	45		/* seconds to wait for t_rcv	*/
#define PATHSIZE	64		/* max size of pathnames	*/
#define NENVIRON	10		/* number of server env variables */

/*
 * LOGMAX is default no of entries maintained
 * LOGMIN is minimum value that can be specified on cmd line
 * Logmax == 0, = Logfile not maintained by the listener
 */

#define LOGMAX	100		/* default value for Logmax	*/
#define LOGMIN	50		/* minimum acceptable value	*/

/*
 * if SMB server is defined, code is included to parse MS-NET messages
 * if undef'ed, the parsing routine logs an approp. error and returns an err.
 */

#define	SMBSERVER	1		/* undef to remove SMBSERVICE support*/

/*
 * if listener (or child) dies, dump core for daignostic purposes
 */

/* #define COREDUMP */

/*
 * listener (effectively) does a setuid to LSUIDNAME after getting the uid
 * from the passwd file via getpwnam.
 */

#define LSUIDNAME	"listen"
#define LSGRPNAME	"adm"

/* the following filenames are used in homedir:	*/

#define	LOGNAME	"log"			/* listener's logfile		*/
#define	OLOGNAME "o.log"		/* listener's saved logfile	*/
#define DBGNAME	"debug"			/* debug output file		*/
#define VDBGNAME "vdebug"		/* debug output file with -v	*/
#define PIDNAME	"pid"			/* listener's process id's	*/
#define DBFNAME	"dbf"			/* listener data base file	*/
#define LCKNAME	"lock"			/* lock file			*/

/*
 * default intermediary server process:
 */

#define INTNAME	"/usr/net/servers/sl"

/*
 * defaults which are normally overriden by cmd line/passwd file, etc
 */

#define NETSPEC	"starlan"

#ifndef	S4
#define	locking	lockf
#endif

