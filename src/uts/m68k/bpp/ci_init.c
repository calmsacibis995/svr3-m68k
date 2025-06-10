/*MH***************************** MODULE HEADER ********************************

  MODULE NAME:	ci_init.c

  DESCRIPTION:    	Contains routines necessary to initialize channels to
			remote cpu's in the Common Environment.

  CONTENTS:      	bpp_init()
			send_adr()
			sendcmd()
			get_memblk()
			build_buf_pool()
			build_epb_pools()
			build_remote_buf_pool()
			str_copy()
			init_cpu()
			dump_sre()
			bpp_intr()

  SYSTEM GLOBALS:     Initializes the local  channel table  - lc[],
				  the remote channel tables - rc[],
				  the local  resource tables
			      and the remote resource tables

  MODULE GLOBALS:

  UPDATE LOG:

     Name  	  Date	  Rev	Comments
--------------  --------  ---	----------------------------------------
B. Volquardsen	05/10/86  1.0	Added buffer request/response support to 
				 ci_intr() and modified to use common
				 shared resource table structure (RES).
J. Holbrook	05/011/86 1.0	Modified initialization of buffer pools
				 to handle separate buffer pools for 
				 each cpu.  Changed lr and rr names to 
				 local_resource and remote_resource
				 respectively.
J. Holbrook	05/22/86	Added buf pool id and flag initialization.
B. Volquardsen	06/06/86	Modified buffer and epb initialization to
				 be table-driven via new ci_initab fields.
J. Holbrook	06/09/86	Changed ci_init and ci_intr to bpp_init
				and bpp_intr to consolidate unix driver.
B. Volquardsen	06/10/86  1.0	Added get_memblk() subroutine to manage
				 allocation of shared memory blocks from
				 acquired system memory area and moved
				 local buffer pool to local_resource of
				 CI_MAX_CPU+1.
J. Holbrook	06/12/86  1.0	Set cpu_running flag in init_cpu.
B. Volquardsen  06/13/86  1.0	Calculate maximum disable level for bpp
				 from "bpp__ile" array and store it in
				 new "bpp_disable_level" variable.  Also
				 added synchronization between init_cpu()
				 and remote pipe_init() on BPP_RUN status.
B. Volquardsen	07/17/86  1.1	Adjust bpp_disable_level to status reg.
				 format (was causing UNIX panic!).
B. Volquardsen	08/29/86  1.1   Added ci_trace() calls to send_adr().
B. Volquardsen  11/11/86  2.0   Changed "common" include path to "sys/bpp",
				 added includes "immu.h" & "region.h", and
				 ifdef'ed out bpp__ile[] ref's for 5.3.
************************************************************************/

#include "sys/errno.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/dir.h"
#ifndef R2
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/cmn_err.h"
#define panic(foo)	cmn_err(CE_PANIC, foo)
#endif
#include "sys/proc.h"
#include "sys/user.h"

#include "sys/bpp/types.h"
#include "sys/bpp/panic.h"
#include "sys/bpp/ce.h"
#include "sys/bpp/ci_const.h"
#include "sys/bpp/ci_types.h"
#include "sys/bpp/ci_config.h"

#ifdef CETRACE
#include "sys/bpp/trace.h"
extern ULONG	lbolt;
extern TRACE_T bpp_trace;
#endif


/* SYSTEM GLOBALS */

extern USHORT	bpp_status;	/* bpp initialization status */
extern CI_CPUTAB_T ci_initab[];

extern REG_ENT 	rt[];    	/* registration table 	*/
extern CHTBL  	lc[];		/* local channel table	*/
extern CHTBL   *rc[];		/* pointer to remote channel table */
extern RES  	local_resource[];	/* local shared resource table 	*/
extern RES  	remote_resource[];	/* remote shared resource table	*/
#ifdef R2
extern int      bpp__ile[];	/* bpp interrupt level array in kernel  */
#endif


/* MODULE GLOBALS */

/* this device name is used when printing out debugging and error information
   it is used so that ifdefs are not required with each printf call */
static BYTE     devname[]="ci_";   /* device name */

ADDR    memadr;                	/* physical memory address */
LONG    memsz;                 	/* memory size */
LONG 	block_size;		/* size of total shared memory requested */
SHORT 	cpu_cnt;		/* number of remote cpu's found "alive" */
int	bpp_disable_level = 0;	/* max interrupt disable level required 
				    for given remote cpu configuration	*/


bpp_init()
{
        DEFAULT i;
	int pri;
	USHORT	cpu;
        CE_EPB  *sh_res_epb;
	register CI_CPUTAB_T 	*itp;	/* init. table pointer	*/

	DEBUG1("%sinit\n", devname);

	/* Check for missing device */
	for(i = 1, cpu_cnt = 0; i <= CI_MAX_CPU; i++)
	{
		if ((ci_initab[i].probe()))
		{
			ci_initab[i].cpu_alive = FALSE; /* disallow open */
		}
		else
		{
			DEBUG2("%sinit: cpu: %d is here\n",devname,i);

			ci_initab[i].cpu_alive = TRUE; /* allow open */
			cpu_cnt++;
		}
		
        }

	/* if there are no cpu's then it doesn't make sense to continue */

	if(cpu_cnt == NULL)
		return;


#	ifdef VME10
        pri = spl7();
        vme_bus();
        splx(pri);
#	endif
/*************************************************************************
   Do we need to disable interrupts during initialization for VME1000???
**************************************************************************/

/*   Initialize memory pools

   This cpu provides a local and a remote EPB pool and buffer pool for 
   each configured remote cpu.  Each cpu is given the number of EPB's and
   buffers specified in its ci_initab entries.  The EPB pools are kept 
   balanced because the cpu's release EPB's received from their opposites
   to the opposites' local EPB pool.  Buffers are kept balanced by remote
   board software which requests or returns buffers whenever its supply
   falls below the specified minimum or exceeds the specified maximum.
   Note, however, that the current unidirectional nature of buffer supply
   balancing can result in the host side being unable to get a buffer and
   unable to request buffers from the remote board(s).
*/
	block_size = 0;
	itp = &ci_initab[0];

	/*
	**  Calculate how much shared memory is needed for BPE's, EPB's, BUF's
	**  and channel tables for ALL remote boards combined and the maximum
	**  interrupt disable level required to protect shared resources.  Note
	**  that ci_initab[0] is used for any local host resource requirements.
	*/

	for (i = 0; i <= CI_MAX_CPU; i++, itp++)
	{
	    if ( itp->cpu_alive == TRUE )
	    {
	    	block_size += ((itp->num_local_bpes + itp->num_remote_bpes)*(sizeof(BPE)));
	    	block_size += ((itp->num_local_epbs + itp->num_remote_epbs)*(itp->epb_size));
	    	block_size += ((itp->num_local_bufs + itp->num_remote_bufs)*(itp->buf_size));
            	block_size += sizeof(CHTBL);

#ifdef R2
		if (bpp__ile[i] > bpp_disable_level)
		    bpp_disable_level = bpp__ile[i];
#else
		bpp_disable_level = 5;
#endif

	    }
	}
	/* Adjust bpp disable level to sr format */
	bpp_disable_level = (bpp_disable_level << 8) + 0x2000;	

        if ((alloc_mem(block_size)) == FAILURE)
        {
                DEBUG1("%sinit: Unable to allocate memory\n",devname);

		for(i = 1; i <= CI_MAX_CPU; i++)
		{
			/* disallow open for all cpu's */
			ci_initab[i].cpu_alive = FALSE;
		}
                return;
        }

	/* 
	**  Initialize the local channel cpu number.  This needs to be
	**  initilized before buffer pools are built to set the pool id. 
	*/
	lc[0].local_cpu = CI_MAX_CPU+1;	

	build_buf_pool();

	build_epb_pools(CI_MAX_CPU+1);

}

/* **************************************************
* 
* Module: send_adr
* Purpose:
*	This function sets up communication with the
*	intelligent processor boards by sending the address of the
*	remote channel table to the board.
* Data used:
* External References:
* Parameters passed:
* Return information: none
*
*/

send_adr(cpu, address)
USHORT	cpu;
ULONG address;
{
        register        i, w;
        BYTE            errflg;
	UBYTE stat_reg;
	UBYTE status;

	status = BPP_ADDR_1;

	DEBUG1("send_adr: address: %x\n", address);
        for(i = 0;(i < sizeof(ULONG));i++, status++)
        {
#ifdef BPTRACE
	ci_trace(&bpp_trace, 0x53414452, address);	/* SADR */
	ci_trace(&bpp_trace, (ULONG)((cpu << 16) | status), lbolt);
#endif
                                                /* write ith byte     */
                (*ci_initab[cpu].data_write)((address >> (i * 8)) & 0x00ff);

                w = 0;                          /* reset delay ctr */
                while(1)
                {
                                                /* check status       */
                        stat_reg = (*ci_initab[cpu].status_read)();
                        if((stat_reg & STAT_BITS) == status)
                                break;
                        if(w++ >= BPP_WAIT_TIME)
			{
#ifdef CETRACE
	ci_initab[cpu].init_type = (UBYTE)((status << 4 ) | (stat_reg & STAT_BITS));
	ci_trace(&bpp_trace, 0x53414452, (ULONG)((cpu << 16) | status));/* SADR */
	ci_trace(&bpp_trace, (ULONG)((cpu << 16) | stat_reg), lbolt);
#endif
                                return(errflg = FAILURE);
			}
                }
        }

        return(errflg = SUCCESS);
}



/* **************************************************
* 
* Module: sendcmd
* Purpose: send a command to an intelligent processor
* Data used:
* External References:
* Parameters passed:
*	cpu - cpu to be signaled
* Return information: none
*
*
*/


sendcmd(cpu,command)
USHORT cpu;
ULONG command;
{

	(*ci_initab[cpu].data_write)(command);
}

/* **************************************************
* 
* Module: 	get_memblk()
*
* Purpose: 	This function allocates a block of shared memory
*		of the size specified and updates the memadr pointer
*		by that amount.
*
* Data used:	global memadr pointer
*
* Input Parms:	size = size (in bytes) of requested memory chunk
*
* Returns:	starting address of requested memory chunk, if avail.
*		or zero, if not available.
*
*/

get_memblk(size)
    LONG   size;
{
	register ADDR   address;
	register LONG	temp_size;

	if ((temp_size = memsz - size) < 0 )
		return(NULL);

	memsz = temp_size;
	address = memadr;
	memadr += (((size + 1)/ 2) * 2);/* force to even byte boundary */
        return(address);
}

/**********************************************************************/
/* Build EPB pools.

   This function builds one pair of local and remote EPB pools for the
   cpu passed as an argument.
*/
/**********************************************************************/

build_epb_pools(cpu)
    register USHORT cpu;
{
    	register ULONG   pool_size;
        register MAX_EPB *max_epb;
	register RES	*rrptr;		/* pointer to resource table */
	register RES	*lrptr;		/* pointer to resource table */
	register CI_CPUTAB_T *cp;	/* pointer to config. table  */
	register int 	i;
	POOL_T		pool;



	pool.cpu = cpu;
	pool.id = UNIX_EPB_POOL_ID;

	/*
	**  Calculate EPB pool size and request shared memory for it.
	*/
	if (cpu == CI_MAX_CPU + 1)
	    cp = &ci_initab[0];
	else
	    cp = &ci_initab[cpu];	/* point to this cpu's initab entry */
	pool_size = (cp->num_local_epbs + cp->num_remote_epbs)*(cp->epb_size);

        if ((max_epb = (MAX_EPB *)get_memblk(pool_size)) == (MAX_EPB *)NULL)
		panic("build_epb_pools:  insufficient shared memory");

        /* Initialize EPB Pools */
	/*  
	**  Build Local EPB pool for this channel 
	*/
	if (cp->num_local_epbs)
	{
	    rrptr = &remote_resource[cpu];/* point to start of remote resource
	    				     table. */
	    lrptr = &local_resource[cpu];/* point to start of local resource
	    				     table */
            lrptr->epb_pool_head = (EPB *)max_epb;      
            lrptr->first_epb     = (EPB *)max_epb; /* save addr. of first alloc. */

            for (i = cp->num_local_epbs; i > 1; i--)
            {
	    	max_epb->hdr.pool = pool;	/* initialize pool id */
                max_epb->hdr.next = (EPB *)(max_epb + 1);
                max_epb++;
            }       
            max_epb->hdr.next = (EPB *)TERMINATOR;
	    max_epb->hdr.pool = pool;	/* initialize pool id */
            rrptr->epb_pool_tail = (EPB *)max_epb;      /* local's pool tail */
            lrptr->last_epb     = (EPB *)max_epb; /* save addr. of last alloc. */
            max_epb++;
	}

        /* 
	**  Build Remote EPB pool for this channel 
	*/
	if (cp->num_remote_epbs)
	{
	    pool.id = BOARD_EPB_POOL_ID;
            rrptr->epb_pool_head = (EPB *)max_epb;     /* remote's pool head */
            rrptr->first_epb     = (EPB *)max_epb; /* save addr. of first alloc. */
            for (i = cp->num_remote_epbs; i > 1; i--)
            {
	    	max_epb->hdr.pool = pool;	/* initialize pool id */
                max_epb->hdr.next = (EPB *)(max_epb + 1);
                max_epb++;
            }       
            max_epb->hdr.next = (EPB *)TERMINATOR;
	    max_epb->hdr.pool = pool;	/* initialize pool id */
            lrptr->epb_pool_tail = (EPB *)max_epb;      /* remote's pool tail */
            rrptr->last_epb     = (EPB *)max_epb; /* save addr. of last alloc. */
	}
}

/**********************************************************************/
/*  Build the buffer pool.

    All buffers are initially place in the local cpu buffer pool.

   05/20/86	B. Volquardsen	1.0	Added code to initialize buffer
					 id and flags.
*/
/**********************************************************************/

build_buf_pool()
{
	register LONG	 pool_size;
        register BUF    *buf;
	register RES	*lrptr;		/* pointer to resource table */
	register int 	 i;
	register int 	 buf_count;
	register int	 tmp_count;
	register CI_CPUTAB_T  *itp;	/* initialization table ptr. */

	/* Initialize BUF pool 
	
	   We put all of the buffers into the this cpu's pool initially
	   and allocate some to send to each remote cpu later during
	   initialization for the cpu's.

	   The local BUF pool is kept in the 0'th entry of the local
	   resource table since there is currently only one local BUF
	   pool and the 0'th entry is otherwise unused.
	*/
	itp = &ci_initab[0];
	buf_count = 0;
	pool_size = 0;

	for ( i = 0; i <= CI_MAX_CPU; i++, itp++ )
	{
	    if ( itp->cpu_alive == TRUE )
	    {
	    	tmp_count = (itp->num_remote_bufs + itp->num_local_bufs);
	    	buf_count += tmp_count;
		pool_size += (tmp_count * itp->buf_size);
	    }		 
	}
	if ((buf = (BUF *)get_memblk(pool_size)) == (BUF *)NULL )
		panic("build_buf_pool:  insufficient shared memory");

	lrptr = &local_resource[CI_MAX_CPU+1];	/* start of local resource */
        lrptr->buf_pool_head = buf;
	lrptr->first_buf = buf;

        for (i = 1; i < buf_count; i++)
        {
                buf->next = buf + 1;
		buf->flags = 0;
		buf->id = UNIX_BUFFER_ID;
                buf ++;
        }
        buf->next = (BUF *)NULL;
	buf->flags = 0;
	buf->id = UNIX_BUFFER_ID;
        lrptr->buf_pool_tail = buf;
	lrptr->last_buf = buf;
	lrptr->minimum_bufs = ci_initab[0].min_remote_bufs;
	lrptr->maximum_bufs = ci_initab[0].max_remote_bufs;
	lrptr->current_bufs = buf_count;
	lrptr->median = (lrptr->minimum_bufs + lrptr->maximum_bufs)/2;

}

/**********************************************************************/
/* Build remote buffer pool.

   This function acquires a fixed number of BUFs from the local buffer
   pool and places head and tail pointers to a linked list of these
   buffers into the remote resource structure for the cpu specified
   in the function argument.

   05/20/86	B. Volquardsen	1.0	Added code to initialize buffer
					 id and flags.
*/
/**********************************************************************/

build_remote_buf_pool(cpu)
    register USHORT cpu;
{
	register RES	*rrptr;		/* pointer to resource table */
	register BUF	*buf, *next;	/* pointer to shared buffer */
	register int 	i, buf_count;


	rrptr = &remote_resource[cpu];	/* point to start of remote resource */

	if (! (buf = (BUF *) get_sbuf(LOCAL_CPU)))
	   panic(CI_BUF_INIT_PANIC);
	rrptr->buf_pool_head = buf;
	rrptr->first_buf = buf;
	buf->id = BOARD_BUFFER_ID;
	buf->flags = 0;
	buf_count  = ci_initab[cpu].num_remote_bufs; 

	for (i = 1; i < buf_count; i++ )
	{
	   if (! (next = (BUF *) get_sbuf(LOCAL_CPU)))
	       panic(CI_BUF_INIT_PANIC);
	   buf->next = next;
	   buf = next;
	   buf->id = BOARD_BUFFER_ID;
	   buf->flags = 0;
	}

	buf->next = (BUF *) NULL;
	rrptr->buf_pool_tail = buf;
	rrptr->last_buf = buf;
	rrptr->current_bufs = buf_count;

	return(0);
}

char *
str_cpy(s1, s2)
register char *s1, *s2;
{
	register char *os1;

	os1 = s1;
	while(*s1++ = *s2++)
		;
	return((char *)os1);
}

VOID init_cpu(cpu)
    register USHORT	cpu;		/* cpu index to initialize */
{
        DEFAULT i;
        register BPE     *bpe; 
	register CE_EPB  *sh_res_epb;	/* pointer to epb to send resources */
	register CHTBL	 *rcptr;	/* pointer to remote channel table */
	register CHTBL	 *lcptr;	/* pointer to local channel pointer */
		 LONG	  pool_size;    /* size of bpe pools */
	int status;

	extern char *str_cpy();

	build_epb_pools(cpu);

	build_remote_buf_pool(cpu);

	sh_res_epb = (CE_EPB *)get_sepb(cpu);	/* get EPB from this channel's
						   pool */

	lcptr = &lc[cpu];		/* address of local channel table */
        rcptr = (CHTBL *)get_memblk(sizeof(CHTBL));/* remote channel table */
	rc[cpu] = rcptr;

	/*
	** Calculate size and then acquire shared memory for bpe pools
	*/
	pool_size = (ci_initab[cpu].num_remote_bpes + ci_initab[cpu].num_local_bpes)*(sizeof(BPE));


	/* initialize BPE pools */
        /* BPE pool 1 */

        if ((bpe = (BPE *)get_memblk(pool_size)) == (BPE *)NULL)
		panic("init_cpu:  insufficient shared memory for bpes");

	lcptr->first_bpe = bpe;	  /* save for diagnostic purposes */

	/* outgoing pipe terminator */
        lcptr->bpp_write = rcptr->bpp_read = bpe;

	/* mark as terminator */
        lcptr->bpp_write->flags = 0x0000;

        bpe++;

        lcptr->get_bpe = bpe;           /* head of local's pool */
        for (i = 2; i < ci_initab[cpu].num_local_bpes; i++)
        {
                bpe->next = bpe + 1;
                bpe->flags = 0x0001;    /* pool member */
                bpe++;
        }
        bpe->flags = 0x0000;    	/* terminator */
	lcptr->last_bpe = bpe;		/* diagnostic info	*/

        rcptr->ret_bpe = bpe;      	/* tail of local's pool */

        bpe++;

        /* BPE pool 2 */

	/* incoming pipe terminator */
        lcptr->bpp_read = rcptr->bpp_write = bpe;
	rcptr->first_bpe = bpe;

	/* mark as terminator */
        lcptr->bpp_read->flags = 0x0000;

        bpe++;

        rcptr->get_bpe = bpe;           /* head of remote's pool */
        for (i = 2; i < ci_initab[cpu].num_remote_bpes; i++)
        {
                bpe->next = bpe + 1;
                bpe->flags = 0x0001;    /* pool member */
                bpe++;
        }
        bpe->flags = 0x0000;    	/* terminator */
        lcptr->ret_bpe = bpe;       	/* tail of remote's pool */
	rcptr->last_bpe = bpe;		/* diagnostic info	*/


	/* finish initialization of channel tables */

        str_cpy(rcptr->version,VERSION);

/*  Set the cpu numbers in the channel tables.  The cpu number for the
    remote cpu is taken from the configuration table, but is actually
    treated as an index to local channel and resource data structures
    during message handling.

    The local cpu number is a temporary kludge to get a unique number
    fro the local cpu.  This assignment will change when the protocol
    is enhanced to provide more dynamic configuration.
*/

	lcptr->remote_cpu = rcptr->local_cpu = ci_initab[cpu].cpu_number;
	lcptr->local_cpu = rcptr->remote_cpu = CI_MAX_CPU + 1;


        rcptr->intr_level = ci_initab[cpu].icc_intr_level;
        rcptr->intr_vector = ci_initab[cpu].icc_intr_vector;

#ifdef DEBUG
	printf("rc[]->version: %s\n", rcptr->version);	
	printf("rc[]->local_cpu: %x\n", rcptr->local_cpu);	
	printf("rc[]->remote_cpu: %x\n", rcptr->remote_cpu);	
	printf("rc[]->intr_level: %x\n", rcptr->intr_level);	
	printf("rc[]->intr_vector: %x\n", rcptr->intr_vector);	
	printf("rc[]->get_bpe: %x\n", rcptr->get_bpe);	
	printf("rc[]->ret_bpe: %x\n", rcptr->ret_bpe);
	printf("rc[]->bpp_read: %x\n", rcptr->bpp_read);	
	printf("rc[]->bpp_write: %x\n", rcptr->bpp_write);	
#endif

/* init registration table */

        for (i = 0; i < NUM_REG_ENTRIES; i++)
        {
                rt[i].process_id = EMPTY;
                rt[i].rq_head = (EPB *)NULL;
                rt[i].rq_tail = (EPB *)NULL;
        }

/* create message channel with board */

        if (!bpp_wait(BPP_READY,cpu))    /* wait for bppsr to = BPP_READY */
        {
                DEBUG2("%sinit: cpu: %d, create pipe, board not ready\n", devname,cpu);
                return;
        }

	sendcmd(cpu,BPP_PIPE);

        if (!bpp_wait(BPP_PIPE,cpu))        /* wait for bppsr = BPP_PIPE */
        {
	     DEBUG2("%sinit: cpu: %d create pipe, no response from board\n", devname,cpu);
                return;
        }

        /* put rc into bppdr */
	send_adr(cpu,rcptr);

	/* send command to let bpp on board start running */
	sendcmd(cpu,BPP_GO);

        if (!bpp_wait(BPP_RUN,cpu))         /* wait for bppsr = BPP_RUN */
        {
                DEBUG2("%sinit: cpu: %d create pipe, board can't create\n", devname,cpu);
                return;
        }

        DEBUG2("Remote version: %s\tCpu number: %d\n", rcptr->version,rcptr->local_cpu);

        DEBUG2("Local  version: %s\tCpu number: %d\n", lcptr->version,lcptr->local_cpu);


/* init and send shared resource EPB to board */

        sh_res_epb->hdr.next == (EPB *)NULL;
        sh_res_epb->hdr.dst_id = PUT_ID(rcptr->local_cpu,0);
        sh_res_epb->hdr.src_id = PUT_ID(lcptr->local_cpu,0);
        sh_res_epb->hdr.epb_size = sizeof(CE_EPB);
        sh_res_epb->hdr.type = CE_GETBUF_RESPONSE;
        sh_res_epb->hdr.status = NO_ERROR;
        sh_res_epb->hdr.buf = remote_resource[cpu].buf_pool_head;
        sh_res_epb->hdr.user_def = 0;

        sh_res_epb->epb_pool_head = remote_resource[cpu].epb_pool_head;
        sh_res_epb->epb_pool_tail = remote_resource[cpu].epb_pool_tail;
        sh_res_epb->minimum_bufs = ci_initab[cpu].min_remote_bufs;
        sh_res_epb->maximum_bufs = ci_initab[cpu].max_remote_bufs;
        sh_res_epb->buf_count    = ci_initab[cpu].num_remote_bufs;

        sh_res_epb->qid = 0;
        sh_res_epb->boot_addr = 0;
        sh_res_epb->boot_length = 0;
        sh_res_epb->eot = 0;
#ifdef DEBUG
	dump_sre(sh_res_epb);
#endif
        if ((status = pwrite(sh_res_epb)) != NO_ERROR)
        {
                DEBUG2("%sinit: status: %x", devname,status);
                DEBUG2("%sinit: cpu: %d create pipe, can't send resources", devname,cpu);
                return;
        }

        DEBUG1("%sinit: initialization complete\n",devname);

	ci_initab[cpu].cpu_running = TRUE;
        return;
}
dump_sre(sre)
CE_EPB *sre;
{
#	ifdef DEBUG

        printf("sre->hdr.next:%x \n", sre->hdr.next );
        printf("sre->hdr.dst_id:%x \n", sre->hdr.dst_id);
        printf("sre->hdr.src_id:%x \n", sre->hdr.src_id);
        printf("sre->hdr.epb_size:%x \n", sre->hdr.epb_size);
        printf("sre->hdr.type :%x \n", sre->hdr.type  );
        printf("sre->hdr.status :%x \n", sre->hdr.status );
        printf("sre->hdr.buf :%x \n", sre->hdr.buf );
        printf("sre->hdr.user_def :%x \n", sre->hdr.user_def );

        printf("sre->epb_pool_head:%x \n", sre->epb_pool_head);
        printf("sre->epb_pool_tail :%x \n", sre->epb_pool_tail );
        printf("sre->minimum_bufs :%x \n", sre->minimum_bufs );
        printf("sre->maximum_bufs :%x \n", sre->maximum_bufs );
        printf("sre->buf_count :%x \n", sre->buf_count );

        printf("sre->qid :%x \n:", sre->qid );
        printf("sre->boot_addr:%x \n", sre->boot_addr);
        printf("sre->boot_length:%x \n", sre->boot_length);
        printf("sre->eot:%x \n", sre->eot);

#	endif
}
/* **************************************************
* 
* Module: bpp_intr
* Purpose: fields cpu data pipe available interrupt
* Data used:
* External References:
* Parameters passed: cpu id based on cpu that interupted
* Return information: none
*
*/
bpp_intr(cpu)
    USHORT cpu;
{
        extern VOID 	ret_to_sender();
	extern VOID 	get_buf_request();
	extern VOID 	get_buf_response();
	extern VOID 	ret_buf_request();
	extern VOID 	ret_buf_response();
        extern VOID 	uwrite();
        extern EPB  	*pread();
        extern BOOLEAN 	dst_ok();

        register EPB *epb;
        USHORT ip;

        DEBUG2("%sintr: interrupt on cpu %d\n",devname,cpu);

        while ((epb = pread(cpu)) != (EPB *)NULL)
        {
                ip = GET_IP(epb->dst_id);
                if (dst_ok(ip))
		{
                        uwrite(ip,epb);
			bpp_signal(ip,epb);
		}
                else if (ip == CI_IP)
		{
			switch(epb->type)
			{
			   case CE_GETBUF_REQUEST:
				/*
				** Another board wants us to send him some 
				**  buffers from our shared memory.
				*/
				get_buf_request(epb);
				break;

			   case CE_GETBUF_RESPONSE:
				/*
				** We asked a board to send us some buffers
				**  from shared memory and he has responded.
				*/
				get_buf_response(epb);
				break;

			   case CE_RETBUF_REQUEST:
			   	/*
			   	** Another board wants to return some buffers
				**  in shared memory that we manage.
			   	*/
			   	ret_buf_request(epb);
			   	break;

			   case CE_RETBUF_RESPONSE:
				/*
				** We tried to return some shared memory
				**  buffers to another board and the board 
				**  has sent us a response.
				*/
				ret_buf_response(epb);
				break;
			   default:
				/*
				** No other CI requests are currently
				**  implemented.  We simply return these
				**  requests with an error status.
				*/
				ret_to_sender(epb, NOT_IMPLEMENTED);
				break;

			}
		}
		else if (epb->status != NO_ERROR)
			/*
			** This must be a negative response to an IP that
			**  is no longer registered - just discard it
			*/
			release_sepb(epb);

		else
			/* return to sender as undeliverable */

                        ret_to_sender(epb,DST_NOT_REGISTERED);
        }
	/*
		this interrupt handler must return 0, so that
		it will go through the correct exit point
		in the interrupt return handler
	*/
	return(0);
}
