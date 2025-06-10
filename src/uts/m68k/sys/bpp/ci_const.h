/*IH******************************************************************

INCLUDE NAME:	ci_const.h

DESCRIPTION:	
		
UPDATE LOG:
     Name	  Date	  Rev	Comments
-------------	--------  ---   --------------------------------------
Dana Holgate	04/01/86  1.0	Initial Coding
B. Volquardsen	06/10/86  1.0	Added definitions for board-specific
				 configuration table entries.
B. Volquardsen	06/23/86  1.0	Deleted function return codes that are
				 now in "common/ce.h".
**********************************************************************/

#define CPU_NO		2	/* cpu number */
#define VERSION		"I01.00.00"	/* version stamp */

#define INTRLEVEL	5	/* interrupt level */
#define INTRVECTOR	0x120	/* interrupt vector */

#define BUF_HDR_SIZE	10	/* size of header fields in BUF */
#define NUM_REG_ENTRIES	50	/* number of registration table entries */
#define CH_TBL_SZ	sizeof(CHTBL)	/* sizeof channel table */

#define N_330_LCL_BPES	30	/* number of local BPEs for 330 board */
#define N_330_REM_BPES	30	/* number of remote BPEs for 330 board */
#define N_333_LCL_BPES	30	/* number of local BPEs for 333 board */
#define N_333_REM_BPES	30	/* number of remote BPEs for 333 board */

#define N_330_LCL_EPBS	30	/* number of local EPBs for 330 board */
#define N_330_REM_EPBS	30	/* number of remote EPBs for 330 board */
#define N_333_LCL_EPBS	30	/* number of local EPBs for 333 board */
#define N_333_REM_EPBS	30	/* number of remote EPBs for 333 board */
#define N_LOCAL_EPBS	8	/* number of local UNIX EPB's */

#define N_330_LCL_BUFS	15	/* number of local BUFs for 330 board */
#define N_330_REM_BUFS	15	/* number of remote BUFs for 330 board */
#define N_333_LCL_BUFS	9	/* number of local BUFs for 333 board */
#define N_333_REM_BUFS	5	/* number of remote BUFs for 333 board */

#define MIN_330_BUFS	8	/* minimum bufs limit for 330 board */
#define MAX_330_BUFS	22	/* maximum bufs limit for 330 board */

#define MIN_333_BUFS	2	/* minimum bufs limit for 333 board */
#define MAX_333_BUFS	8 	/* maximum bufs limit for 333 board */

#define	MIN_131_BUFS	5	/* minimum number of host BUFs */
#define	MAX_131_BUFS	25	/* maximum number of host BUFs */

#ifdef VME10
#define OFF_BOARD_MEM	0x60000	/* starting addr of off board memory */
#endif

#ifdef VME120
#define OFF_BOARD_MEM	0x80000 /* starting addr of off board memory */
#endif

#ifdef VME131
#define OFF_BOARD_MEM	0x80000 /* starting addr of off board memory */
#endif

#define BPP_WAIT_TIME	0x20 /* seconds to wait for response from board */

#define SIGON		0	/* turn signalling on */
#define SIGOFF		1	/* turn signalling off */
#define WAIT		2	/* do wait reads */
#define NOWAIT		3	/* do nowait reads */
#define RESET		4	/* reset the board */
