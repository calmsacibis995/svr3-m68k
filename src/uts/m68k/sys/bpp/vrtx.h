
/*IH*********************** INCLUDE HEADER ****************************
**
**  INCLUDE NAME:	/u/ce/common/vrtx.h
**
**  DESCRIPTION:	This file contains all common function code 
**			declarations, error return mnemonics, and VRTX
**			structure definitions for the VRTX C language
**			interface.
**
**  NOTES:		See /u/ce/vme3xx/ce_vrtx.h for board-specific
**			VRTX configuration defines.
**
**  UPDATE LOG:
**  Date	Name		V.R	Comments
**  --------	--------------	---  	------------------------------
**  04/08/86	B. Volquardsen	1.0	Created from ce_vrtx.h to sep-
**					 arate common VRTX definitions
**					 from CE board unique defines.
**  04/09/86	B. Volquardsen	1.0	Added timer_req return codes.
**********************************************************************/

/*
**  Define error mnemonics.
*/

/*	Mnemon	Value	   Meaning		Affected commands    */
/*	------	-----	   --------------------	-------------------- */

#define	RET_OK	0	/* Sucessful return.	[Valid commands.]    */

#define	ER_TID	1	/* Task ID error.	TCREATE, TDELETE,    */

			/*			TSUSPEND, TRESUME,   */
			/*			TPRIORITY, TINQUIRY  */

#define	ER_TCB	2	/* No TCBs available.	TCREATE		     */

#define	ER_MEM	3	/* No memory available.	GBLOCK, PCREATE,     */
			/*			PEXTEND, QCREATE     */

#define	ER_NMB	4	/* Not a memory block.	RBLOCK		     */

#define	ER_MIU	5	/* Mailbox in use.	POST		     */

#define	ER_ZMW	6	/* Zero message.	POST, QPOST	     */

#define	ER_BUF	7	/* Buffer full.		RXCHR		     */

#define	ER_WTC	8	/* WAITC in progress.	WAITC		     */

#define	ER_ISC	9	/* Invalid system call.	[Invalid commands.]  */

#define	ER_TMO	10	/* Time-out.		PEND, QPEND	     */

#define	ER_NMP	11	/* No message present.	ACCEPT, QACCEPT	     */

#define	ER_QID	12	/* Queue Id error.	QPOST, QPEND,	     */
			/*			QACCEPT, QCREATE     */

#define	ER_QFL	13	/* Queue full.		QPOST		     */

#define	ER_PID	14	/* Partition ID error.	GBLOCK, RBLOCK,	     */
			/*			PCREATE, PEXTEND     */

#define	ER_INI	15	/* Workspace too small.	VRTX_INIT	     */

#define	ER_ICP	17	/* Invalid VCT param.	VRTX_INIT	     */

/*
**	Timer function request return codes
*/
#define NO_TIMERS_AVAIL		-1	/* start_timer(): None avail	*/
#define INVALID_TIMER_ID	-2	/* stop_timer():  Bad Timer ID 	*/
#define TIMER_NOT_ACTIVE	-3	/* stop_timer():  timer expired */

/*
**  VRTX task execution mode mnemonics
*/
#define USER_MODE	0	/* User mode for tasks	 	     */
#define SUPER_MODE	1	/* Supervisor mode for tasks	     */

/*
**  VRTX configuration table (vct) structure. 
**	See page 4-2 of the VRTX User's Manual.
*/
#define	RESERVED	0	/* value to set VCT reserved fields */
#define	UNUSED		0	/* value to set VCT unused fields   */

struct	vct
{
	BYTE	*sys_ram;	/* pointer to vrtx system ram	    */
	LONG	 sys_ram_size;	/* vrtx system ram size		    */
	SHORT	 sys_stk_size;	/* vrtx system stack size	    */
	SHORT	 isr_stk_size;	/* ISR stack space		    */
	LONG	 reserved_1;	/* Must be zero			    */
	LONG	 reserved_2;	/* Must be zero			    */
	SHORT	 disable_level;	/*				    */
	SHORT	 user_stk_size;	/* user stack size		    */
	LONG	 reserved_3;	/* Must be zero			    */
	SHORT	 user_task_cnt;	/* total number of tasks	    */
	SHORT	 reserved_4;	/* Must be zero			    */
	VOID	(*txrdy_addr)();/* TXRDY driver address		    */
	VOID	(*sys_tcreate)();/* pointer to tcreate hook function */
	VOID	(*sys_tdelete)();/* pointer to tdelete hook function */
	VOID	(*sys_tswch)();	/* pointer to tswitch hook function */
	BYTE	*CVT_addr;	/* Component Vector Table address   */
};

/*
** VRTX TCB structure.  (See page C-4 of the VRTX User's Manual.)
*/

typedef	struct	tcb
{
struct	tcb	*tbnext;
struct	tcb	*tblink;
	BYTE	*tbext;
	BYTE	 tbpri;
	BYTE	 tbid;
	SHORT	 tbstat;
	DEFAULT	 tbd0;
	DEFAULT	 tbd1;
	DEFAULT	 tbd2;
	DEFAULT	 tbd3;
	DEFAULT	 tbd4;
	DEFAULT	 tbd5;
	DEFAULT	 tba0;
	DEFAULT	 tba1;
	DEFAULT	 tba2;
	DEFAULT	 tba3;
	BYTE	*tbsp;
	BYTE	*tbusp;
	BYTE	*tbtstack;
	SHORT	 tbflags;
	DEFAULT	 tbdelay;
	SHORT	 tbqid;
	DEFAULT	 reserved;
} TCB;

/*
**	Declarations for user arrays which receive information from VRTX calls.
*/

typedef	struct t_info_rec
{	DEFAULT	task_id;
	DEFAULT	task_priority;
	DEFAULT	task_status;
}T_INFO_REC;


typedef	struct q_info_rec
{	DEFAULT	msg_addr;
	DEFAULT	count;
}Q_INFO_REC;
