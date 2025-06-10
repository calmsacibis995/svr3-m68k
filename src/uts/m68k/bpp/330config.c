

/*MH***************************** MODULE HEADER *******************************
**
**  MODULE NAME:	330config.c
**
**  DESCRIPTION:    	This module contains the configuration table
**			for the Unix ci configuration.  It defines cpu
**			numbers and addresses to be used for specific
**			boards and tells which boards are expected to be in
**			the system.
**
**  NOTES:		This version is for debugging with a single 330 board.
**
**  CHANGES:     DESCRIPTION                      DATE        ENGINEER
**  ___________________________________________________________________
**  Original coding.				04/16/86	jeh
**  Modified to use pointers to functions for	05/02/86	jeh
**   board control functions.
**  Added initialization of new fields in the	06/09/86	wlv
**   ci_initab structures.
**  Changed "common" include path to "sys/bpp"	11/11/86	wlv
**
*******************************************************************************/

#include  "sys/bpp/types.h"
#include  "sys/bpp/ci_config.h"
#include  "sys/bpp/ci_const.h"
#include  "sys/bpp/ce.h"

ULONG	bprobe(), sprobe(),
	v30_1_ctrl_write(), v30_1_data_write(), v30_1_status_read(),
	v30_1_intr_write(), v30_1_rst_write(),
	v30_1_probe();

ULONG	v33_1_ctrl_write(), v33_1_data_write(), v33_1_status_read(),
	v33_1_intr_write(), v33_1_rst_write(),
	v33_1_probe();

ULONG	v33_2_ctrl_write(), v33_2_data_write(), v33_2_status_read(),
	v33_2_intr_write(), v33_2_rst_write(),
	v33_2_probe();



/* BPP configuration table */

USHORT	ci_cfg_cpu = CI_MAX_CPU;	/* size of cpu configuration table */

CI_CPUTAB_T	ci_initab[CI_MAX_CPU+1] =
{
	/*
	**  entry 0 in the config table is a special entry which gives only
	**  the information required to initialize the local cpu structures.
	*/
    {
	CI_MAX_CPU+1,			/* cpu number */
	VME131_BOARD,			/* board type */
	0,				/* initialization function */
	0,				/* board probe routine */
	0,				/* control write function */
	0,				/* data register write function */
	0,				/* status register read function */
	0,				/* board interrupt function */
	0,				/* interrupt mask */
	0,				/* board reset function */
 	0,				/* value to write for reset function */
	0,				/* host interrupt level */
	0,				/* interrupt vector location */
	0,				/* initialization type -- not used */
	TRUE,				/* life flag - dead or alive */
	TRUE,				/* cpu running flag */
	0,           			/* number of local BPEs to create */
	0,				/* number of remote BPEs to create */
	MAX_EPB_SIZE,			/* size of this channel's EPBs */
	N_LOCAL_EPBS,			/* number of local EPBs to create */
	0,				/* number of remote EPBs to create */
	MAX_BUF_SIZE,			/* size of this channel's buffers  */
	0,				/* number of local BUFs to create */
	0,				/* number of remote BUFs to create */
	MIN_131_BUFS,			/* minimum number of host BUFs */
	MAX_131_BUFS			/* maximum number of host BUFs */
    },
    	/* 
    	**  VME330 Board number 1 
    	*/
    {
	1,				/* cpu number */
	VME330_BOARD,			/* board type */
	0,				/* initialization function */
	v30_1_probe,			/* board probe routine */
	v30_1_ctrl_write,		/* control write function */
	v30_1_data_write,		/* data register write function */
	v30_1_status_read,		/* status register read function */
	v30_1_intr_write,		/* board interrupt function */
	0,				/* interrupt mask */
	v30_1_rst_write,		/* board reset function */
 	0xff,				/* value to write for reset function */
	4,				/* host interrupt level */
	0xC0,				/* interrupt vector location */
	1,				/* initialization type -- not used */
	FALSE,				/* life flag - dead or alive */
	FALSE,				/* cpu running flag */
	N_330_LCL_BPES,        		/* number of local BPEs to create */
	N_330_REM_BPES,			/* number of remote BPEs to create */
	MAX_EPB_SIZE,			/* size of this channel's EPBs */
	N_330_LCL_EPBS,			/* number of local EPBs to create */
	N_330_REM_EPBS,			/* number of remote EPBs to create */
	MAX_BUF_SIZE,			/* size of this channel's buffers  */
	N_330_LCL_BUFS,			/* number of local BUFs to create */
	N_330_REM_BUFS,			/* number of remote BUFs to create */
	MIN_330_BUFS,			/* minimum number of remote BUFs */
	MAX_330_BUFS			/* maximum number of remote BUFs */
    }
};
