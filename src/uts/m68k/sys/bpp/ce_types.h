/*%W%	%Q%	%G%*/
/*IH******************************************************************

INCLUDE NAME:	ce_types.h

DESCRIPTION:	
		
UPDATE LOG:
     Name	  Date	  Rev  	Comments
-------------	--------  ---   -------------------------------------
Dana Holgate	03/15/86  1.0	Initial Release
B. Volquardsen	04/08/86  1.0	Added "include common/ce.h" & deleted 
				 lines moved to /u/ce/common/ce.h.
B. Volquardsen	04/21/86  1.0	Moved BPE, CHTBL & RES structures to 
				 "common/ce.h" and added macros 
				 COPY_TO_VME and COPY_FROM_VME.
B. Volquardsen	05/14/86  1.0	Deleted COPY_*_VME macros & externs
				 and moved "msg" & "cmd" defines to
				 ce_bpp.c where they are used.
B. Volquardsen	06/23/86  1.0	Moved USER_INIT define from ce_const.h
**********************************************************************/


typedef struct list		/* processing list structure */
{
	EPB	*head;		/* list head */
	EPB	*tail;		/* list tail */
} LIST;

/* private global RAM memory variables */

typedef struct prv_ram
{
	BYTE 	*prv_ram_end;	/* address of first byte after private ram */
				/* this includes the prv_ram structure and */
				/* the registration table		   */
	BYTE	*wrk_ram_start;	/* starting address VRTX & CI work space ram */
	BYTE	*wrk_ram_end;	/* ending address VRTX & CI work space ram */
				/* includes VRTX workspace and EPB and BUF */
				/* pools created by CI for Bootloader use */
	USHORT user_init;	/* if 0xAAAA then doing user's VRTX init */
	CHTBL ch;		/* channel information */
	RES res;		/* shared resource information */
	LIST list[NUM_LISTS];	/* processing lists */
	LONG timeout;	/* sc_qpend timeout parameter */
	EPB *bpo_epb;	/* permanent storage for an unprocessed output BPE */
	BOOLEAN unprocessed_bpo; /* flag, indicating unprocessed bpo_epb */
	EPB *spx_epb;	/* permanent storage for unprocessed shared mem EPB */
	BOOLEAN unprocessed_spx;/* flag indicating unprocessed spx_epb */
	EPB *psx_epb;	/* permanent storage for unprocessed private mem EPB */
	BOOLEAN unprocessed_psx;/* flag indicating unprocessed psx_epb */
	EPB *lb_epb;	/* permanent storage for unprocessed shared mem EPB */
	BOOLEAN unprocessed_lb;/* flag indicating unprocessed lb_epb */
	EPB *rb_epb;	/* permanent storage for unprocessed shared mem EPB */
	BOOLEAN unprocessed_rb;/* flag indicating unprocessed rb_epb */
	ULONG max_ips;	/* maximum entries in registration table */
	SHORT *rt;	/* registration table, pointer to array of entries */
} RAM;

#define USER_INIT	0xAAAA		/* user VRTX init flag value */

/* ROM variables structure definition */

typedef struct rom_hooks
{
	BYTE version[10];		/* version number */
	USHORT cpu_number;		/* assigned cpu number */
	BYTE *vrtx;			/* address of VRTX code */
	RAM *ram;			/* address of private ram */
	ULONG reg_tbl_size;		/* registration table size */
	DEFAULT (*buf_xfer)();		/* buffer transfer function vector */
	DEFAULT (*symbol)();		/* symbol lookup function vector */
	VOID (*ci_task)();		/* common interface task address */
	ULONG partition1_block_size;	/* EPB size */
	ULONG partition2_block_size; 	/* BUF size */
	VOID (*auto_boot)();		/* user auto boot address */
	struct vct *vct;		/* address of vrtx config table */
} ROM;
