/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/*  @(#)lp0.h	8.2        */

/*  generic lp structure */

struct lp {
	struct clist	l_outq;		/* output queue */
	char	flag, ind;		/* flag, indentation */
	int	ccc, mcc, mlc;		/* current character count, maximum   */
					/* character count, maximum line cnt  */
	int	line, col;		/* line, column */
	int	(* lp_routine)();	/* pntr to device specific function */
	int 	lev;			/* interrupt priority level */ 
	int	dev;			/* minor device number */
};

#define	L_OPEN	0x01	/* printer open */
#define	L_NOCR	0x02	/* no cr needed */
#define	L_ASLP	0x04	/* asleep */
#define	L_BUSY	0x08	/* interrupt in progress */
#define	L_TOUT	0x10	/* timeout in progress */
#define	L_WACK	0x20	/* waiting for printer acknowledge */
#define L_FAULT 0x40	/* printer has faulted */

#define	LPHIWAT	100
#define	LPLOWAT	40
#define	LPPRI	(PZERO+8)
