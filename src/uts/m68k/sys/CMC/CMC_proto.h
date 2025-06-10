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
 * Protocol switch table.
 *
 * Each protocol has a handle initializing one of these structures,
 * which is used for protocol-protocol and system-protocol communication.
 *
 * The userreq routine interfaces protocols to the system and is
 * described below.
 */

typedef
struct protosw {
	short	pr_type;		/* socket type used for */
	short	pr_family;		/* protocol family */
	short	pr_protocol;		/* protocol number */
	short	pr_flags;		/* see below */
	int	(*pr_cmp)();		/* name checker; 0 => OK */
	int	(*pr_nmck)();		/* more name checking */
	int	(*pr_gname)();		/* get name */
	int	(*pr_bind)();		/* Protocol specific bind	*/
	int	(*pr_connect)();	/* Protocol specific connect	*/
	int	(*pr_close)();		/* Protocol specific close	*/
	int	(*pr_send)();		/* Protocol specific send	*/
	int	(*pr_recv)();		/* Protocol specific recv	*/
	short	pr_channel;		/* h_chan[] offset for protocol */
	short	pr_end;			/* End of available channels	*/
	short	pr_next;		/* Next h_chan[] for allocation */
	short	pr_offset;		/* data offset			*/
	short	pr_maxdata;		/* max user data		*/
} PROTO;

/*
 * Values for pr_flags
 */
#define	PR_ATOMIC	0x01		/* exchange atomic messages only */
#define	PR_ADDR		0x02		/* addresses given with messages */
/* in the current implementation, PR_ADDR needs PR_ATOMIC to work */
#define	PR_CONNREQUIRED	0x04		/* connection required by protocol */
#define	PR_WANTRCVD	0x08		/* want PRU_RCVD calls */
#define	PR_RIGHTS	0x10		/* passes capabilities */

/*
 * Structure per communications domain.
 */
typedef
struct	domain {
	int	dom_family;		/* AF_xxx */
	char	*dom_name;
	struct	protosw *domps, *domps_END;
	struct	domain *dom_next;
} DOMAIN;

