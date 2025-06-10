/*
 * done processing structure -- managed by bfrdone
 */

typedef struct bdone {
	int		(*b_proc)();
	char		 *b_param;
} BDONE;

#define NDONESTK	4		/* four levels of done processing */

/*
 * The bfr itself
 */

typedef struct bfr {
	struct bfr 	*b_link;		/* the next one */
	unsigned short	 b_flags;		/* Status/flags	*/
	short		 b_len;			/* size of bfr */
	char		*b_addr;		/* bfr place */
	short		 b_msglen;		/* data in bfr */
	short		 b_resv;		/* reserved */
	BDONE		 b_done[NDONESTK];	/* done processing routines */
	short		 b_donesp;		/* index into b_done */
	short		 b_level;		/* curr proto level */
	short		 b_device;		/* current device for bfr */
} BFR;

/*
 *	b_flags -	bits 0-5 used for TCP "t_ctl" bits
 *			bit 6 used for TCP "TCPURGENT" bit
 *			bits 12-15 used for BFR control bits
 */

# define B_NO_REPLACE	0x4000		/* See iftcpinput		*/

/*	b_donesp - default		*/

#define BDEMPTY			-1		/* index zero pops to this */

/*
 * b_level values
 * see protodef.c
 */

#define LINK_LEVEL		0
#define NETWORK_LEVEL		1
#define TRANSPORT_LEVEL		2
#define INTERFACE_LEVEL		3

#define INIT_OPS		256		/* initialize operations */
#define TERM_OPS		257		/* terminate operations */

/*
 * network packets
 */

#define PKTDATASIZE	1514
#define PKTSUMSIZE	4

typedef struct pkt {
	BFR	p_bfr;
	char	p_data[PKTDATASIZE];
	char 	p_checksum[PKTSUMSIZE];
} PKT;
