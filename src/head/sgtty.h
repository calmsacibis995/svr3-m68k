/*	@(#)sgtty.h	6.1	 */
/*	3.0 SID #	1.1	*/
/*
 * Structure for stty and gtty system calls.
 */

struct sgttyb {
	char	sg_ispeed;		/* input speed */
	char	sg_ospeed;		/* output speed */
	char	sg_erase;		/* erase character */
	char	sg_kill;		/* kill character */
	int	sg_flags;		/* mode flags */
};

/*
 * Modes
 */
#define	HUPCL	01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define ANYP	0300
#define	NLDELAY	001400
#define	TBDELAY	002000
#define	CRDELAY	030000
#define	VTDELAY	040000
#define BSDELAY 0100000
#define ALLDELAY 0177400

/*
 * Delay algorithms
 */
#define	CR0	0
#define	CR1	010000
#define	CR2	020000
#define	CR3	030000
#define	NL0	0
#define	NL1	000400
#define	NL2	001000
#define	NL3	001400
#define	TAB0	0
#define	TAB1	002000
#define	NOAL	004000
#define	FF0	0
#define	FF1	040000
#define	BS0	0
#define	BS1	0100000

/*
 * Speeds
 */
#define B0	0
#define B50	1
#define B75	2
#define B110	3
#define B134	4
#define B150	5
#define B200	6
#define B300	7
#define B600	8
#define B1200	9
#define	B1800	10
#define B2400	11
#define B4800	12
#define B9600	13
#define EXTA	14
#define EXTB	15
#define B19200	14
#define B38400	15

/*
 *	ioctl arguments
 */
#define FIOCLEX		(('f'<<8)|1)
#define FIONCLEX	(('f'<<8)|2)
#define	TIOCHPCL	(('t'<<8)|2)
#define	TIOCGETP	(('t'<<8)|8)
#define	TIOCSETP	(('t'<<8)|9)
#define	TIOCEXCL	(('t'<<8)|13)
#define	TIOCNXCL	(('t'<<8)|14)
