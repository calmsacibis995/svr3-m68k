/* @(#)ma_def.h	1.2 (Special for FPACK) */
/*IH*********************** INCLUDE HEADER ****************************
**
**  INCLUDE NAME:	ma_def.h
**
**  DESCRIPTION:	This file contains constant, type and
**			macro definitions involved with the
**			interface to the MAC sublayer of the
**			Data Link Layer subsystem.
**
**  NOTES:
**
**  UPDATE LOG:
**  Date	Name		V/R	Comments
**  --------	-----------	---	-----------------------------
**  1/29/86     R. Robinson     1.0     Original coding.
**
**********************************************************************/

/*
**	The following are constant definitions used through
**	out the MAC sublayer.
*/

	/*
	**	MAC Return/Reply Code Definitions
	*/

#define MA_ERR_MEM 	0x201	/* MAC Resource Allocation Error */
#define MA_ERR_REG 	0x202	/* MAC Registration Error */
#define MA_ERR_INT 	0x203	/* MAC Initialization Error */
#define MA_ERR_TXP 	0x204	/* MAC Transmit Packet Error */
#define MA_ERR_RXP 	0x205	/* MAC Receive  Packet Error */

	/*
	**	MAC Packet Size (IEEE 802.3 )Definitions
	*/

#define MA_TOTMIN  64		/* Minimum Length Of MAC Packet */
#define MA_TOTMAX  1518		/* Maximum Length Of MAC Packet */
#define MA_FCSLEN  4		/* Length Of MAC Frame Check Sequence */

#define MA_DATAMIN (MA_TOTMIN-sizeof(struct ma_header)-MA_FCSLEN)
#define MA_DATAMAX (MA_TOTMAX-sizeof(struct ma_header)-MA_FCSLEN)

/*
**	MAC queue ID for vrtx
*/

#define	MAC_QID		3	/* MAC layer queue ID. (RWL - HO82.01) */

/*
**	The following are structure definitions for a MAC
**	IEEE 802.3 Protocol Data Unit (PDU).
*/

	/*
	**	The following is a type definition for the
	**	MAC packet addressing format.
	*/

typedef UBYTE MA_ADDR[6];


	/*
	**	The following is a type definition for the
	**	MAC packet data length format.
	*/

typedef UBYTE MA_DLEN[2];

	/*
	**	The following is a structure definition for the
	**	header of a MAC Protocol Data Unit.
	*/

struct ma_header
{
	MA_ADDR		rem_addr;	/* Destination Address */
	MA_ADDR		loc_addr;	/* Source Address */
	MA_DLEN		data_len;	/* Data Length */
};

/*
**	The following structure defines the MAC Statistics Table.
*/

struct ma_stat
{
	/*
	**  Transmit related statistics.
	*/

	ULONG	tx_no_err;	/* Number Of MAC Packets Transmitted
				   Without Error */

	ULONG	tx_more;	/* Number Of MAC Packets Transmitted
				   Without Error After More Than One
				   Collision */

	ULONG	tx_one;		/* Number Of MAC Packets Transmitted
				   Without Error After Exactly One
				   Collision */

	ULONG	tx_def;		/* Number Of MAC Packet Transmissions
				   Deferred Because Channel Is Busy */

	ULONG	tx_buff;	/* Number Of MAC Packet Transmissions
				   Aborted Due To Lack Of Ownership
				   Of Next Buffer */

	ULONG	tx_uflo;	/* Number Of MAC Packet Transmissions
				   Aborted Due To Underflow Error */

	ULONG	tx_lcol;	/* Number Of MAC Packet Transmissions
				   Aborted Due To Late Collision */

	ULONG	tx_lcar;	/* Number Of MAC Packet Transmissions
				   Aborted Due To Loss Of Carrier */

	ULONG	tx_rtry;	/* Number Of MAC Packet Transmissions
				   Aborted Due To Excessive
				   Collisions */

	/*
	**  Receive related statistics.
	*/

	ULONG	rx_no_err;	/* Number Of MAC Packets Received
				   Without Error */

	ULONG	rx_fram;	/* Number Of MAC Packets Received
				   With A Framing Error */

	ULONG	rx_oflo;	/* Number Of MAC Packets Received
				   With An Overflow Error */

	ULONG	rx_crc;		/* Number Of MAC Packets Received
				   With An CRC Error */

	ULONG	rx_buff;	/* Number Of MAC Packets Received
				   While Data Chaining But Lost Due
				   To Lack Of Ownership Of Buffer */

	/*
	**  Hardware related statistics.
	*/

	ULONG	hard_babl;	/* Number Of Times The Transmitter
				   Timed Out */

	ULONG	hard_cerr;	/* Number Of Times The Transmission
				   Transceiver Test Collision Failed */

	ULONG	hard_miss;	/* Number Of MAC Packets Received
				   But Lost Due To Lack Of Buffers */

	ULONG	hard_merr;	/* Number Of Memory Addressing
				   Errors */

	/*
	**  Processing error related statistics.
	*/

	ULONG	err_mem;	/* Number Of MAC Packets Received
				   But Lost Due To Lack Of EPBs */

	ULONG	err_abrt;	/* Number of MAC Packet Transmissions
				   Aborted Due To Error In Request
				   Processing */
};

/*
**	The following are macro definitions for
**	manipulating MAC addresses.
*/

	/*
	**	The following are a macro definitions
	**	for accessing word values defined as
	**	byte arrays.
	*/

#define PUT_16(val,ptr) {				\
			   ptr[0] = (UBYTE)(val >> 8);	\
			   ptr[1] = (UBYTE)val;		\
			}

#define GET_16(ptr)	((ptr[0] << 8) | ptr[1])

/*
**	The following are constant/structure definitions used, by
**	XNS Network Manager, through out the MAC sublayer.
*/

	/*
	**	The following is the XNS Network Manager parameter
	**	structure definition.
	*/

struct ma_net
{
	/*
	**	'flg_eid' is the update flag for the MAC Ethernet
	**	SAP ID value. When set (TRUE) the MAC will set its
	**	Ethernet SAP ID to the value in the 'net_eid'
	**	parameter. When reset (FALSE) the MAC will set the
	**	'net_eid' parameter to its current Ethernet SAP ID
	**	value.
	*/

	BOOLEAN		flg_eid;
	ULONG		net_eid;

	/*
	**	'flg_stat' is the update flag for the MAC statistics
	**	values. When set (TRUE) the MAC will set its
	**	statistics to the values in the 'net_stat' parameter.
	**	When reset (FALSE) the MAC will set the 'net_stat'
	**	parameter to its current statistics values.
	*/

	BOOLEAN		flg_stat;
	struct		ma_stat		net_stat;

	/*
	**	'flg_mode' is the update flag for the mode register
	**	value in the LANCE Initialization Block. When set
	**	(TRUE) the MAC will set the mode register in the
	**	LANCE Initialization Block to the value in the
	**	'net_mode' parameter. The MAC will also reset the
	**	LANCE chip. When reset (FALSE) the MAC will set the
	**	'net_mode' parameter to the current value of the mode
	**	register in the LANCE Initialization Block.
	*/

	BOOLEAN		flg_mode;
	USHORT		net_mode;

	/*
	**	'flg_padr' is the update flag for the Node's
	**	Physical Addres value in the LANCE Initialization
	**	Block. When set (TRUE) the MAC will set the
	**	physical address in the LANCE Initialization Block
	**	to the value in the 'net_padr' parameter. The MAC
	**	will also reset the LANCE chip. When reset (FALSE)
	**	the MAC will set the 'net_padr' parameter to the
	**	current value of the physical address in the LANCE
	**	Initialization Block.
	*/

	BOOLEAN		flg_padr;
	MA_ADDR		net_padr;

	/*
	**	'flg_ladf' is the update flag for the Logical
	**	Address Filter value in the LANCE Initialization
	**	Block. When set (TRUE) the MAC will set the 
	**	Logical Addres Filter in the LANCE Initialization
	**	Block to the value in the 'net_ladf' parameter.
	**	The MAC will also reset the LANCE chip. When reset
	**	(FALSE) the MAC will set the 'net_ladf' parameter
	**	to the current value of the Logical Address Filter
	**	in the LANCE Initialization Block.
	*/

	BOOLEAN		flg_ladf;
	USHORT		net_ladf[4];

	/*
	**	'flg_ladr' is the update flag for an addition of a
	**	Logical Addres to the Logical Address Table maintained
	**	by the MAC. When set (TRUE) the MAC will add the
	**	Logical Address in 'net_ladr' parameter to its
	**	Logical Address Table. This will result in a new
	**	Logical Address Filter being generated. The MAC will
	**	also reset the LANCE chip. When reset (FALSE) nothing
	**	will occur.
	*/

	BOOLEAN		flg_ladr;
	MA_ADDR		net_ladr;
};
