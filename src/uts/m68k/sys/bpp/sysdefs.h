

/*%W%	%Q%	%G%*/
/*IH******************************************************************

INCLUDE NAME:	/ce/common/sysdefs.h

DESCRIPTION:	This module defines system-wide information
		such as cpu numbers and well-known interface points.
		
UPDATE LOG:
     Name	  Date	  Rel  	Comments
-------------	--------  ---  	-------------------------------
Jim Holbrook	06/06/86  1.0	Creation.
B. Volquardsen	08/07/86  1.0	Added TIM_PID definition.
Jim Holbrook	10/09/86  1.0	Added IP, task and queue ID's for the
				  auto-dialer task.
B. Volquardsen	11/13/86  1.0	Added BSC task priority defines and
				  changed SDLC_PRI to 7.
Jim Holbrook	11/24/86  1.0	Rearranged link task priorities.
**********************************************************************/

/* Well-known interface points for Common Environment */

#define	CI_IP		0	/* Interface control functions IP */
#define	BOOT_IP		1	/* Boot-load function IP */
#define DIAG_IP		2	/* Diagnostic tools IP */

/* VME333 Board Interface Point Definitions
     Only processes on the board need have well-known interface points
     since the unix processes are always first speakers for the protocols
     we are implementing.
*/
#define VBR	0x00000000	/* vector base register setting */ 

/* task priority definitions */
#define CI_PRI		5	/* CI task priority */
#define RJE_PRI		4	/* bisync RJE priority */
#define SDLC1_PRI	7	/* sdlc task priority */
#define SDLC2_PRI	7	/* sdlc task priority */
#define BSC_PRI		8	/* 3270 BSC priority */
#define DIAL_PRI	10	/* Auto-dialer task priority */

#define VME333_NO_1	2	/* cpu number for 1st 333 board */
#define VME333_NO_2	3	/* cpu number for 2nd 333 board */

#define	SDLC1_IP	6	/* local interface point for 1st SDLC task */
#define	SDLC2_IP	7	/* local interface point for 2nd SDLC task */
#define	BSC1_IP		8	/* local interface point for 1st BSC task */
#define	BSC2_IP		9	/* local interface point for 2nd BSC task */
#define DIAL_IP		10	/* Auto-dialer IP */

/* VME330 Board Interface Point Definitions */

#define VME330_NO_1	1	/* cpu number for 1st 330 board */

#define MAC_IP		6	/* interface point for MAC layer */

/*
**  Partition ID mnemonics.
**  ----------------------
**  NOTE:  When adding or deleting a memory partition define, the 
**         total "PARTITIONS" define in ce_vrtx.h must be updated.
*/

#define RT_PID		 0	/* Registration Table Partition ID 	*/
#define	EPB_PID		 1	/* Event Parameter Block Partition ID.  */
#define BUF_PID		 2	/* Data Buffer Block Partition ID.  	*/
#define TIM_PID		 3	/* Timer EPB Block Partition ID. 	*/

/*
**  Queue ID mnemonics.
**  ------------------
**  NOTE:  Any new queue defines must have their queue depth defined
**         in ce_vrtx.h.  Also, the "TOTAL_QUEUES" define in ce_vrtx.h
**  	   must equal the total number of queues to be created by VRTX.
*/
#define TASK_Q_DEPTH	64	/* task queue depths for sc_qcreate() */
#define TIMER_Q_DEPTH   16	/* Timer queue depth */

#define	CI_QID		1	/* Common Interface queue ID.	*/

#define SD1_QID		2	/* sdlc #1 queue id. */
#define SDT1_QID	3	/* sdlc #1 timer queue id. */

#define BSC1_QID	4	/* bisync #1 queue id. */
#define BSC1T_QID	5	/* bisync #1 timer queue id. */

#define SD2_QID		6	/* sdlc #2 queue id. */
#define SDT2_QID	7	/* sdlc #2 timer queue id. */

#define BSC2_QID	8	/* bisync #2 queue id. */
#define BSC2T_QID	9	/* bisync #2 timer queue id. */

#define DIAL_QID	10	/* auto-dialer queue id. */

/*
** User Task ID's
** ---------------
*/

#define	SDLC1_TID	2	/* sdlc #1 task id */
#define	BSC1_TID	3	/* bisync #1 task id */
#define	SDLC2_TID	4	/* sdlc #2 task id */
#define	BSC2_TID	5	/* bisync #2 task id */
#define DIAL_TID	6	/* auto-dialer task id */

/*
**  Task Timeout mnemonics
**  ----------------------
*/
#define	MSPT		50	/* Milliseconds per "VRTX" tick.     */

#define	NO_TIMEOUT	0	/* Used to indicate no timeout value */
				/* on calls to PEND and QPEND.	     */

#define TIMEOUT		(200/MSPT) /* timeout length on calls	*/
