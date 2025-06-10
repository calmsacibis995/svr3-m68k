/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */


/*
 *	This file defines the data structures used to communicate
 *	between an ENP and a Unix host.
 */

/*
 *	Command field values
 */

	/* From host to ENP */

# define SC_RESERVE	1
# define SC_SEND	2
# define SC_RECV	3
# define SC_CLOSE	4
# define SC_SEND_OOB	5
# define SC_PEEK	6
# define SC_STATUS	7
# define SC_CONNECT	8	/* From ENP, means connection complete */
# define SC_BIND	9
# define SC_LISTEN	10
# define SC_RECV_SIG	11
# define SC_SEND_SIG	12
# define SC_GNAMES	13
# define SC_SET_OPT	14
# define SC_GET_OPT	15

#define SC_CTL		0x8000		/* control vs data in b_flags */
#define SC_WAKE		0x80		/* want wakeup on txdone */
#define SC_FLAGS	0x60		/* confluence of two below */
#define SC_PUSH		0x20		/* tcp eol wanted - */
#define SC_URGENT	0x40		/* tcp urgent data */

	/* From ENP to host */

# define SC_HAVE_OOB	129
# define SC_ACCEPT	130	/* Listen has succeeded; reply implies relisten */
# define SC_END_OOB	131	/* host now has all OOB data */

# define SC_REPLY	(1<<15)	/* XXX */

/*
 *		Interface Data Structures
 */

# define HS_INIT	1		/* h_state -> host init'd	*/
# define ENP_INIT	1		/* h_state -> enp init'd	*/

typedef struct HOSTCTL {
	int	h_state;
	int	h_hostbase;
	int	h_enpbase;
} HOSTCTL;


typedef struct SHARED {
	int		enpbase;
	int		hostbase;
	int		hostbfraddr;
	int		numhostbfr;
	int		chanaddr;
	unsigned long	inetaddr;
	char		enetaddr[6];	/* Ethernet addr	*/
	short		status;
} SHARED;


typedef struct	SHARED_RINGS {
	RING	*h_open;
	RING	*h_tonet;
	RING	*h_netcomplete;
	RING	*e_tohost;
	RING	*e_hostcomplete;
	RING	*e_wakeaddr;
	RING	*e_urg;
} SHARED_RINGS;

/*
 * hacks for limiting access to MVME330 memory to short (or byte) accesses
 */
#define enp_assign(lv, rv) {			\
	register short *lval = (short *) &lv;	\
	register long   rval = (long)     rv;	\
						\
	*lval++ = rval >> 16;			\
	*lval   = rval;				\
}
#define enp_rval(x) (				\
	(					\
		((unsigned short *) &(x))[0]	\
	) << 16 | (				\
		((unsigned short *) &(x))[1]	\
	)					\
)
