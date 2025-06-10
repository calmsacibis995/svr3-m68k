
/*%W%	%Q%	%G%*/
/*IH******************************************************************

INCLUDE NAME:	/u/ce/common/ce.h

DESCRIPTION:	This module defines data structures and
		other information common to all CE modules.
		
UPDATE LOG:
     Name	  Date	  Rel  	Comments
-------------	--------  ---  	-------------------------------
B. Volquardsen	04/08/86  1.0	Created by extracting common items from
				 ce_types.h
B. Volquardsen	04/21/86  1.0	Added definitions of BPP command and 
				 status values. Moved BPE, CHTBL and RES 
				 structures from ce_types.h to this file.
B. Volquardsen	05/04/86  1.0	Changed EPB "sys_reserved" field
				 to "pool_t" structure.
B. Volquardsen  05/10/86  1.0	Modified RES structure definition to
				 work for both shared memory managers
				 and requesters and added TERMINATOR
				 definition.
J. Holbrook	05/10/86  1.0	Moved REG_ENT structure definition from
				 ci_types.h to ce.h for future common use.
B. Volquardsen  05/20/86  1.0	Added id and flag fields to BUF structure.
S. Rao          06/04/86  1.0   Changed MAX_BUF_SIZE from 1200 to 1600
				 for LAN support.
B. Volquardsen  06/05/86  1.0   Changed MAX_EPB_SIZE from 400 to 600
				 for BSC support.
B. Volquardsen	06/09/86  1.0	Added "first" and "last" allocated fields
				 to CHTBL for BPE's and to resource table
				 for EPB's and BUF's.
B. Volquardsen  06/13/86  1.0	Added BPP_GO define to fix host-board 
				 synchronization problem.
B. Volquardsen  06/23/86  1.0	Moved Pipe Write return codes from the
				"ce_const.h" & "ci_const.h" headers to
				 this header file.
B. Volquardsen	08/15/86  2.0	Added panic fields to CHTBL structure.
**********************************************************************/

#define MAX_BUF_SIZE 1600
#define MAX_EPB_SIZE 600

	/*
	**	BPP Command Definitions
	**
	**	NOTE:  Values are limited to 8 significant bits (0-255)
	*/

#define	BPP_RESET	1		/* Board Reset Command		*/
#define	BPP_OPEN	2		/* Initialize Pipe Command	*/
#define BPP_BPE		4		/* Read Buffer Pipe Envelope	*/
#define BPP_GO		7		/* Complete Init & Set BPP_RUN	*/

	/*
	**	BPP Status Definitions
	**
	**	NOTE:  Values are limited to 3 significant bits (0-7)
	*/

#define STAT_BITS	0x07

#define	BPP_WAIT	0		/* Board not ready yet - wait	*/
#define	BPP_READY	1		/* Board ready to accept cmds.	*/
#define	BPP_PIPE	2		/* Initializing Pipe		*/
#define BPP_ADDR_1	3		/* Acknowledge 1st Address byte	*/
#define BPP_ADDR_2	4		/* Acknowledge 2nd Address byte	*/
#define BPP_ADDR_3	5		/* Acknowledge 3rd Address byte	*/
#define BPP_ADDR_4	6		/* Acknowledge 4th Address byte	*/
#define	BPP_RUN		7		/* Pipe is open and running	*/

typedef struct buf	/* Data Buffer structure */
{
	struct buf	*next;	/* forward link to next buf */
	char 		id;	/* Buffer Identifier	*/
	UBYTE		flags;	/* Buffer control flags	*/
	ULONG		length;	/* length of the data in the buf */
	UBYTE		data[MAX_BUF_SIZE - 10]; /* actual data */
} BUF;

#define	BUFFER_ACTIVE	0x08	/* bit indicating active buffer */
#define UNIX_BUFFER_ID 	0x55	/* Unix Buffer ID value = 'U'	*/
#define BOARD_BUFFER_ID	0x42	/* Board Buffer ID value = 'B'	*/

typedef struct  pool_t	/* pool id substructure for EPB */
{
	USHORT	cpu;	/* number of cpu owning pool */
	USHORT	id;	/* pool id in local cpu */
} POOL_T;

/* Pool ID's to distinguish the EPB pools for a single channel */

#define	UNIX_EPB_POOL_ID	0xf1
#define BOARD_EPB_POOL_ID	0xf2


typedef struct epb		/* EPB structure */
{
	struct epb	*next;	/* forward link to next EPB in linked list */
	struct pool_t	pool;	/* cpu and pool id from which EPB originally
				   acquired */
	ULONG		dst_id; /* high word is cpu number */
	ULONG		src_id; /* low word is ip number */
	USHORT		epb_size; /* size of total EPB (hdr + user data) */
	USHORT		type;	/* EPB type */
	USHORT		status;	/* EPB status */
	BUF		*buf;	/* pointer to data buffer */
	ULONG		user_def; /* user definable field */
} EPB;

#define TERMINATOR		-1L	/* EPB pool terminator */

typedef struct ce_epb
{
	EPB	hdr;
	/* registration */
	USHORT	qid;		/* VRTX queue id of task registering */
	/* message channel creation */
	EPB *epb_pool_head;	/* shared epb pool head */
	EPB *epb_pool_tail;	/* shared epb pool tail */
	USHORT minimum_bufs;	/* mimimum number of buffers to keep */
	USHORT maximum_bufs;	/* maximum number of buffers to keep */
	/* buffer transfer */
	ULONG buf_count;	/* number of buffers being transfered */
	/* boot loader */
	ULONG	boot_addr;	/* starting address of data or for execution */
	ULONG	boot_length;	/* length of data only in all BUFs */
	BOOLEAN	eot;		/* end of transmission - last of series */
} CE_EPB;

/*
**  Common Environment EPB types 
*/
	/* boot loader */
#define CE_LOAD_REQUEST			 1
#define CE_LOAD_DATA			 2
#define CE_LOAD_RESPONSE		 3
#define CE_EXECUTE_REQUEST		 4
#define CE_PROTECT_REQUEST		 5
#define CE_PROTECT_RESPONSE		 6
#define CE_UNPROTECT_REQUEST		 7
#define CE_UNPROTECT_RESPONSE		 8

	/* buffer transfer */
#define CE_GETBUF_REQUEST		 9
#define CE_GETBUF_RESPONSE		10
#define CE_RETBUF_REQUEST		11
#define CE_RETBUF_RESPONSE		12

	/* registration */
#define CE_REGISTRATION_REQUEST		13
#define CE_REGISTRATION_RESPONSE	14
#define CE_DEREGISTRATION_REQUEST	15

	/* message channel */
#define CE_PIPE_INIT_REQUEST		16
#define CE_BPE_INDICATION		17

/* 
**  EPB status return codes
*/

#ifndef NO_ERROR
#define NO_ERROR		0
#endif
#define	CI_STATUS		0xfff0
#define DST_NOT_REGISTERED	0xfff0
#define UNDELIVERABLE		0xfff1
#define IDENTIFIER_IN_USE	0xfff2
#define REG_TABLE_FULL		0xfff3
#define INVALID_IP		0xfff4
#define IP_IN_USE		0xfff5
#define NO_PIPE_TO_DST		0xfff6
#define BUF_XFER_ERROR		0xfff7
#define NOT_IMPLEMENTED		0xffff

/* 
**  Pipe Write return codes
*/

#define NO_PIPE			-1	/* Buffered Pipe does not exist */
#define NO_BPE			-3	/* No Buffered Pipe Envelopes avail */


typedef struct bp_env
{
	struct bp_env *next;		/* forward link */
	union 
	{
		ULONG command;		/* command (bit 0 set) */
		EPB *message;		/* message (bit 0 clear) */
	} mc;
	ULONG  dst_id;			/* destination, high = cpu, low = ip */
	ULONG  src_id;			/* source, high = cpu, low = ip */
	USHORT flags;			/* bit 0: set = in use */
					/*	  clear = empty */
} BPE;

/* the following two structures should occur once for each channel/remote cpu */
/* but for now we're only allowing a single channel/remote cpu */

typedef struct ch_table		/* channel information table */
{
	BYTE version[10];	/* version number */
	USHORT remote_cpu;	/* remote CPU number */
	USHORT local_cpu;	/* local CPU number */
	UBYTE intr_level;	/* interrupt level */
	UBYTE intr_vector;	/* interrupt vector */
	BPE *get_bpe;		/* BPE pool head pointer */
	BPE *ret_bpe;		/* BPE pool tail pointer */
	BPE *bpp_read;		/* Buffered Pipe head pointer */
	BPE *bpp_write;		/* Buffered Pipe tail pointer */
	BPE *first_bpe;		/* 1st BPE in this channel's pool */
	BPE *last_bpe;		/* Last BPE in this channel's pool */
	char panic_tid[4];	/* Name of task that panicked */
	ULONG panic_code;	/* Code indicating type of panic */
	char panic_msg[16];	/* Descriptive message */

} CHTBL;

typedef struct res_table	/* shared resource table */
{
	EPB *epb_pool_head;	/* shared epb pool head */
	EPB *epb_pool_tail;	/* shared epb pool tail */
	EPB *first_epb;		/* first EPB allocated to this channel  */
	EPB *last_epb;		/* last EPB allocated to this channel   */
	BUF *buf_pool_head;	/* buffer pool head */
	BUF *buf_pool_tail;	/* buffer pool tail */
	BUF *first_buf;		/* first BUF allocated to this channel  */
	BUF *last_buf;		/* last BUF allocated to this channel   */
	USHORT minimum_bufs;	/* mimimum number of buffers to keep 	*/
	USHORT maximum_bufs;	/* maximum number of buffers to keep 	*/
	USHORT current_bufs;	/* current number of bufs in pool 	*/
	USHORT median;		/* median number of bufs to keep 	*/
	BOOLEAN buf_request_pending; /* GETBUF request in progress 	*/
} RES;


/* registration table entry */

typedef struct reg_ent
{
	SHORT	process_id;	/* process id of user process */
	struct  proc *proc;	/* pointer to process slot */
	EPB	*rq_head;	/* read queue head */
	EPB	*rq_tail;	/* read queue tail */
} REG_ENT;

#define	EMPTY	-1L		/* empty flag for registration */

#define LOCAL_CPU	0	/* in dst_id it means epb is destined for same 
				   CPU as CI is running on - allows user to 
				   be ignorant of the actual local CPU number */
#define CI_IP		0	/* CI task's assigned IP */

/* source and destination macros */
#define PUT_ID(cpu,ip)	((cpu << 16) + ip)
#define GET_CPU(id)	(id >> 16)
#define GET_IP(id)	(id & 0xFFFF)

/* the following define list indices into the list array */
#define PSXL	0	/* private to shared memory xfer list */
#define BPOL	1	/* buffered pipe output list */
#define SPXL	2	/* shared to private memory xfer list */
#define LBL	3	/* Local boot list */
#define RBL	4	/* Remote boot list */

#define NUM_LISTS 5	/* total number of lists */
