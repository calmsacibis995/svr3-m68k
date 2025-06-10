/*MH***************************** MODULE HEADER *******************************
**
**  MODULE NAME:	bppci.c
**
**  DESCRIPTION:    	Unix processor BPP and CI code.
**
**  NOTES:
**
**  CHANGES:     DESCRIPTION                       DATE     ENGINEER
**  ___________________________________________________________________
**  Original coding.				03/27/86   microsystems
**  Added flag and id setting and checking to   05/22/86	wlv
**   get_sbuf and release_sbuf.
**  Changed spl4() calls to splx using the new	06/16/86	wlv
**   "bpp_disable_level".
**  Added kludge to set cpu_running to FALSE	06/26/86	wlv
**   if pwrite() can't get a BPE.
**  Added disables to pread and pwrite to	07/05/86	jeh
**   protect bpe pool integrity.
**  Added test of bpp_intr_depth in get_sepb()	07/22/86	wlv
**   and get_sbuf() to prevent sleep() calls
**   from interrupt level code.
**  Changed ci_trace() parameters to include	08/28/86	wlv
**   timestamp using "lbolt" and fixed pwrite
**   error exit to re-enable interrupts.
**  Added ci_trace() calls to bpp_wait.		08/29/86	wlv
**  Changed sleep() priorities to PZERO+2.	09/08/86	wlv
**  Added test of bpp_streams in all places 	09/16/86	dap
**   that test bpp_intr_depth.  Calls from 
**   streams modules must never sleep().
**  Ported alloc_mem() to use 5.3 getcpages()	09/17/86	dap
**  Changed "common" include path to "sys/bpp"	11/11/86	wlv
**   and added includes "immu.h" & "region.h"
**  Changed release_sbuf() call to wakeup to	12/30/86	jeh
**   use buf_pool_head (deleted ->next).
*******************************************************************************/

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
#include "sys/bpp/ce.h"
#include "sys/bpp/ci_types.h"
#include "sys/bpp/ci_const.h"
#include "sys/bpp/ci_config.h"
#include "sys/bpp/panic.h"
#ifdef CETRACE
#include "sys/bpp/trace.h"
extern 	ULONG	lbolt;
ULONG bpp_trace_buf[256];

TRACE_T bpp_trace = 
{
   bpp_trace_buf,
   0,
   0xff
};

#endif

#ifdef DEBUG
#define DEBUG1(str,parm) { printf(str,parm); }
#define DEBUG2(str,parm1,parm2) { printf(str,parm1,parm2); }
#else
#define DEBUG1(str,parm) 
#define DEBUG2(str,parm1,parm2) 
#endif

#define msg mc.message
#define cmd mc.command


extern CI_CPUTAB_T ci_initab[];
extern int	   bpp_disable_level;	/* max. disable level for bpp	*/
extern int	   bpp_intr_depth;	/* bpp interrupt depth */

/* SYSTEM GLOBALS */

REG_ENT rt[NUM_REG_ENTRIES];    /* registration table */
CHTBL  	lc[CI_MAX_CPU+1];
CHTBL  *rc[CI_MAX_CPU+1];               /* pointer to remote channel table */
RES  	local_resource[CI_MAX_CPU+2];	/* local shared resource tables */
RES  remote_resource[CI_MAX_CPU+2];	/* remote shared resource tables */
USHORT bpp_status = 0;			/* interface initialization status */
int    bpp_streams = 0;			/* running BPP from STREAMS */

/* MODULE GLOBALS */
/* 
**   	"devname[]" is used when printing out debugging and error information.
**	It is used so that ifdefs are not required with each printf call.
*/
static 	BYTE	devname[] = "bppci_";	/* device name */

/**/ 
/* **************************************************
* 
* Module: bpp_timeout
* Purpose:
*	wake up bpp_wait to check board status when
*	called by the Unix timer interrupt facility.
* 
* Parameters passed:
*	cpu:  cpu number for the wakeup
*
*****************************************************/

bpp_timeout(cpu)
    int  cpu;

{
	wakeup(&ci_initab[cpu]);
}

/**/ 
/* **************************************************
* 
* Module: bpp_wait
* Purpose:
*	wait a specific amount of time 
*	for a the bpp status register to send
*	the expected status
* Data used:
* External References:
* Parameters passed:
*	status - expected status return
* Return information:
*	TRUE - the expected status was returned
*	FALSE -  the expected status was never delivered
*
*/
BOOLEAN bpp_wait(status,cpu)
BYTE status;
USHORT cpu;
{
        register int t = BPP_WAIT_TIME * 15;
	register UBYTE stat_reg = 0;

#ifdef BPTRACE
	ci_trace(&bpp_trace, 0x4250505f, 0x57414954);	/* BPP_WAIT */
	ci_trace(&bpp_trace, (ULONG)((cpu << 16) | status), lbolt);
#endif


        while (t--)
	{
                stat_reg = (*ci_initab[cpu].status_read)();
                if ((stat_reg & STAT_BITS) == status)
			goto done;
		timeout(bpp_timeout,cpu,4);  /* set timeout to check status */
		sleep(&ci_initab[cpu],PZERO+2);      /* wait for timeout */
	}
#ifdef CETRACE
    ci_initab[cpu].init_type = (UBYTE)((status << 4 ) | (stat_reg & STAT_BITS));
    ci_trace(&bpp_trace, 0x57544f20, (ULONG)((cpu << 16) | status));/* WTO */
    ci_trace(&bpp_trace, (ULONG)((cpu << 16) | stat_reg), lbolt);
#endif
        return (FALSE);

done:
	DEBUG2("%swait, waiting for: %x\n", devname, status );
	return(TRUE);
        
}
/**/ 
/* **************************************************
* 
* Module: pwrite
* Purpose: attach an EPB to the write pipe and notify the processor
* Data used:
* External References:
* Parameters passed:
*	epb - event parameter block
* Return information:
*	NO_PIPE - the pipe to the processor could not be created
*	NO_BPE - the BPE could not be gotten from the buffer pool
*	NO_ERROR - good completion of routine
*
*/
DEFAULT pwrite(epb)
    register EPB *epb;
{
        BPE *get_bpe();
        register BPE *bpe;       /* outgoing bpe */
	register int pri;	/* save area for interrupt level */
	ULONG cpu;

	DEBUG1("%spwrite enter\n", devname);

#ifdef CETRACE
	ci_trace(&bpp_trace, 0x50575201,epb);			/* PWR */
	ci_trace(&bpp_trace,(epb->type << 16) | epb->status, lbolt);
#endif
	cpu = GET_CPU(epb->dst_id);
	if (cpu > CI_MAX_CPU)
	{
	    panic(CI_PWRITE_BAD_CPU);
	}
	/* Is there a pipe */
        if (lc[cpu].bpp_write == (BPE *)NULL)
                return(NO_PIPE);

	/* get a bpe */
	pri = splx(bpp_disable_level);   /* disable interupts */
        if ((bpe = get_bpe(cpu)) == (BPE *)NULL)
	{
		/* This is a temporary Kludge to provide some
		** means of detecting a dead remote cpu board
		*/
		ci_initab[cpu].cpu_running = FALSE;
		splx(pri);		/* Enable interrupts again */
                return(NO_BPE);
	}

	/* fill the new terminating BPE */
        bpe->next = (BPE *)NULL;
        bpe->msg = (EPB *)NULL;
        bpe->flags &= 0xfffe;   /* mark BPE as terminator */

        lc[cpu].bpp_write->next = bpe;       /* link in the new terminator */
        lc[cpu].bpp_write->msg = epb;        /* fill in current tail bpe */
        lc[cpu].bpp_write->dst_id = epb->dst_id;
        lc[cpu].bpp_write->src_id = epb->src_id;

        lc[cpu].bpp_write->flags |= 0x0001; /* unmark old terminator envelope */
        lc[cpu].bpp_write = bpe;  /* move the pipe tail pointer to terminator */
	splx(pri);		/* enable interrupts again */

#ifdef DEBUG
	dump_epb("pwrite",epb);
#endif
	/* tell the board */
        sendcmd(cpu, BPP_BPE);

	DEBUG1("%spwrite exit\n", devname);
        return(NO_ERROR);
}
/**/ 
/* **************************************************
* 
* Module: pread
* Purpose: scan the read pipe for host bound EPB
* Data used:
* External References:
* Parameters passed: cpu - cpu number
* Return information:
*	pointer to epb - data available
*	NULL - nothing available
*
*/

EPB *pread(cpu)
    USHORT cpu;
{
        VOID release_bpe();
        register BPE *bpe;
        register EPB *epb;
	register CHTBL *lcptr;	/* pointer to local channel table */
	register int pri;	/* interrupt disable level */

	DEBUG1("%spread enter\n", devname);
	if (cpu > CI_MAX_CPU)
	{
	    panic(CI_PREAD_BAD_CPU);
	}
	lcptr = &lc[cpu];
	pri = splx(bpp_disable_level);		/* disable interrupts */
        if (lcptr->bpp_read->flags & 0x0001)        /* anything in pipe ? */
        {
                bpe = lcptr->bpp_read;              /* get first bpe */
                lcptr->bpp_read = bpe->next;        /* move pipe head */
                epb = bpe->msg;                 /* get msg from envelope */
                release_bpe(bpe,cpu);               /* return the bpe to remote */

#ifdef CETRACE
	ci_trace(&bpp_trace, 0x50524401,epb);			/* PRD */
	ci_trace(&bpp_trace,(epb->type << 16) | epb->status, lbolt);
#endif
		splx(pri);		/* enable interrupts again */
                return(epb);
        }
	splx(pri);		/* enable interrupts again */
	DEBUG1("%spread exit\n", devname);
        return((EPB *)NULL);
}
/**/ 
/* **************************************************
* 
* Module:  get_bpe
* Purpose: get the next available bpe from the BPE pool
* Data used:
* External References:
* Parameters passed: none
* Return information: 
*	bpe pointer - data available
*	NULL - no data available
*
*/
BPE *get_bpe(cpu)
USHORT cpu;
{
        register BPE *bpe;
	register CHTBL *lcptr = &lc[cpu];

        if (lcptr->get_bpe->flags & 0x0001) /* is first available */
        {
                bpe = lcptr->get_bpe;
                lcptr->get_bpe = bpe->next;
		/* clear the bpe */
                bpe->next = (BPE *)NULL;
                bpe->msg = (EPB *)NULL;
                bpe->dst_id = 0;
                bpe->src_id = 0;
                bpe->flags  = 0;
                return(bpe);
        }
        return((BPE *)NULL);    /* empty list */
}
/**/ 
/* **************************************************
* 
* Module: release_bpe
* Purpose: return a BPE to the BPE pool
* Data used:
* External References:
* Parameters passed:
*	pointer to BPE
* Return information: none
*
*/
VOID release_bpe(bpe,cpu)
    register BPE *bpe;
    USHORT cpu;
{
	register CHTBL *lcptr = &lc[cpu];

	DEBUG2("%srelease_bpe, bpe: %x\n", devname,bpe)
        bpe->flags &= 0xfffe;           /* mark as new terminator */
        lcptr->ret_bpe->flags |= 0x0001;    /* unmark old terminator */
        lcptr->ret_bpe->next = bpe;         /* attach to pool */
        lcptr->ret_bpe = bpe;               /* move the tail */
}
/**/ 
/* **************************************************
* 
* Module: get_sepb
* Purpose: get an EPB from the EPB pool
* Data used:
* External References: 
* Parameters passed: none
* Return information:
*	pointer to an EPB
*	NULL - no more EPB's available
*
*/
EPB *get_sepb(cpu)
    USHORT cpu;
{
        register EPB *epb;
	register RES *lrptr;
	register int pri;

/* If cpu is zero, this is a request for a buffer from the local EPB
   pool.  This pool is managed through the local_resource table entry
   with the index of the local cpu number.  We remap the cpu number
   to access this pool.
*/

	if (cpu == LOCAL_CPU)
	{
	    cpu = lc[0].local_cpu;
	}
	lrptr = &local_resource[cpu];
	pri = splx(bpp_disable_level);
        while (lrptr->epb_pool_head->next == (EPB *)TERMINATOR)
	{
		/*
		** if called from interrupt level code, return immediately
		*/
		if (bpp_intr_depth > 0 || bpp_streams)
		{
		    splx(pri);
		    return((EPB *)NULL);
		}
		delay(2);      	/* sleep and try again for epb */
	}

	DEBUG2("%sget_sepb, epb->next: %x\n",devname,lrptr->epb_pool_head->next);
        epb = lrptr->epb_pool_head;
        lrptr->epb_pool_head = epb->next;
	splx(pri);

	/* clear the EPB's header portion */
        epb->next = (EPB *)NULL;
        epb->dst_id = 0;
        epb->src_id = 0;
        epb->epb_size = 0;
        epb->type = 0;
        epb->status = 0;
        epb->buf = (BUF *)NULL;
        epb->user_def = 0;

        return(epb);
}
/**/ 
/* **************************************************
* 
* Module: release_sepb
* Purpose: put an EPB back in the EPB pool
* Data used:
* External References:
* Parameters passed: pointer to an EPB
* Return information: none
*
*/
VOID release_sepb(epb)
    register EPB *epb;
{
	register RES *lrptr = &local_resource[epb->pool.cpu];
	register int pri;

	epb->buf = (BUF *)NULL;	/* make sure epb buf ptr is cleared */
	
	/* Releases to pools belonging to this board must be made by
	   placing the epb on the head of the pool, and releases
	   of epb's belonging to other boards pools must be made
	   by placing the epb on the tail of the pool */

	if (epb->pool.id == UNIX_EPB_POOL_ID)
	{  /* this epb belongs to a local pool */
	    pri = splx(bpp_disable_level);
            epb->next = lrptr->epb_pool_head;
            lrptr->epb_pool_head = epb;
	    splx(pri);
	}
	else
	{  /* this epb belongs to a remote pool */
            epb->next = (EPB *)TERMINATOR;
	    pri = splx(bpp_disable_level);
            lrptr->epb_pool_tail->next = epb;
            lrptr->epb_pool_tail = epb;
	    splx(pri);
	}
}
/**/ 
/* **************************************************
* 
* Module: get_sbuf
* Purpose: get a BUF from the BUF pool
* Data used:
* External References:
* Parameters passed: 
* Return information:
*	pointer to a BUF
*	NULL - no BUF's available
*
*/
BUF *get_sbuf(cpu)
    USHORT cpu;
{
        register BUF *buf;
	register RES *lrptr = &local_resource[CI_MAX_CPU+1];
	register int pri;	/* save area for disable level */

	pri = splx(bpp_disable_level);
        while (lrptr->buf_pool_head->next == (BUF *)NULL)
	{  
		/*
		** if called from interrupt level code, return immediately
		*/
		if (bpp_intr_depth > 0 || bpp_streams)
		{
		    splx(pri);
		    return((BUF *)NULL);
		}
		/* 
		** wait for buffer available 
		*/
		sleep(&lrptr->buf_pool_head,PZERO+2);
	}
        
        buf = lrptr->buf_pool_head;
        lrptr->buf_pool_head = buf->next;
	lrptr->current_bufs--;
	splx(pri);

	if (buf->id != UNIX_BUFFER_ID)
		panic(CI_BAD_BUF_ID_PANIC);

	if (buf->flags == BUFFER_ACTIVE)
		panic(CI_BUF_NOT_FREE_PANIC);

	/* clear buf */
        buf->next = (BUF *)NULL;
	buf->flags = BUFFER_ACTIVE;
        buf->length = 0;
        return(buf);
}
/**/ 
/* **************************************************
* 
* Module: release_sbuf
* Purpose: put a BUF back in the BUF pool
* Data used:
* External References:
* Parameters passed: 
*	 pointer to BUF
* Return information:
*
*/

VOID release_sbuf(remote_cpu, buf)
    USHORT   remote_cpu;
    register BUF *buf;
{
	register RES *lrptr = &local_resource[CI_MAX_CPU+1];
	register int pri;	/* save area for disable level */

	DEBUG2("%srelease_sbuf, buf: %x\n", devname,buf)

	if (buf->flags != BUFFER_ACTIVE)
		panic(CI_BUF_NOT_ACTIVE_PANIC);

	buf->id = UNIX_BUFFER_ID;
	buf->flags = 0;
        buf->next = (BUF *)NULL;
	pri = splx(bpp_disable_level);	/* disable interrupts */
        lrptr->buf_pool_tail->next = buf;
        lrptr->buf_pool_tail = buf;
	lrptr->current_bufs++;
	splx(pri);			/* enable interrupts again */
	wakeup(&lrptr->buf_pool_head); /* wakeup anyone waiting for buf */
}
/**/ 
/* **************************************************
* 
* Module: uwrite
* Purpose: attach an EPB to the user read queue
* Data used:
* External References:
* Parameters passed: 
*	ip - interface point in registration table
*	epb - pointer to an EPB
* Return information:
*
*/
VOID uwrite(ip,epb)
    USHORT  ip;
    register EPB     *epb;
{
	register REG_ENT *rgptr = &rt[ip];
	register int pri;

#ifdef CETRACE
	ci_trace(&bpp_trace, 0x55575201,epb);			/* UWR */
	ci_trace(&bpp_trace,(epb->type << 16) | epb->status, lbolt);
#endif
	pri = splx(bpp_disable_level);
        if (rgptr->rq_tail == (EPB *)NULL)      /* empty list ? */
        {
                epb->next = (EPB *)TERMINATOR;
                rgptr->rq_tail = rgptr->rq_head = epb;
        }
	else
	{ /* place on end of list */
        	epb->next = (EPB *)TERMINATOR;  /* mark as terminator */
        	rgptr->rq_tail->next = epb;     /* link into list */
        	rgptr->rq_tail = epb;           /* move tail */
	}
	splx(pri);
}
/**/ 
/* **************************************************
* 
* Module: uread
* Purpose: read an EPB from the user read queue	
* Data used:
* External References:
* Parameters passed:
*	ip - interface point
* Return information:
*	NULL - no EPB's available
*	pointer to an EPB - data available
*
*/

EPB *uread(ip)
    register USHORT  ip;
{
        register EPB *epb;       /* the returned epb */
	register REG_ENT *rgptr = &rt[ip];
	register int pri;

        if (rgptr->rq_head == (EPB *)NULL)      /* empty list ? */
                return((EPB *)NULL);

	pri = splx(bpp_disable_level);
        epb = rgptr->rq_head;           /* get first from list */

        if (epb->next == (EPB *)TERMINATOR)  /* last in list ? */
                rgptr->rq_head = rgptr->rq_tail = (EPB *)NULL;
        else
                rgptr->rq_head = epb->next;     /* move head to next */

	splx(pri);
        
#ifdef CETRACE
	ci_trace(&bpp_trace, 0x55524401,epb);			/* URD */
	ci_trace(&bpp_trace,(epb->type << 16) | epb->status, lbolt);
#endif
        return(epb);
}
/**/ 
/* **************************************************
* 
* Module: un_uread
* Purpose: put an EPB back on the read queue
* Data used:
* External References:
* Parameters passed:
*	ip - interface point
*	pointer  to an EPB
* Return information: none
*
*/

VOID un_uread(ip,epb)
    register USHORT ip;
    register EPB *epb;
{
	register REG_ENT *rgptr = &rt[ip];

	DEBUG2("%s_un_read, ip: %x\n", devname, ip);
	DEBUG2("%s_un_read, epb: %x\n", devname, epb);
        if (rgptr->rq_head == (EPB *)NULL)      /* empty list ? */
        {
                epb->next = (EPB *)TERMINATOR;
                rgptr->rq_tail = rgptr->rq_head = epb;
                return;
        }

        epb->next = rgptr->rq_head;
        rgptr->rq_head = epb;

}
/**/ 
/* **************************************************
* 
* Module: find_empty
* Purpose: find the first available slot in the registration table
* Data used:
* External References:
* Parameters passed: ip - interface point
* Return information:
*	TRUE - an empty slot was found and filled
*	FALSE - an empty slot was not found
*
*/
BOOLEAN find_empty(ip)
    USHORT *ip;
{
        register USHORT i;
	register REG_ENT *rgptr = &rt[1];


        for (i=1; i < NUM_REG_ENTRIES; i++)
	{
                if (rgptr->process_id == EMPTY)
                {
                        *ip = i;
                        return(TRUE);
                }
		rgptr++;
        
	}
        return(FALSE);
}
/**/ 
/* **************************************************
* 
* Module: dst_ok
* Purpose: check to see if the process attached to an interface is stiil ok
* Data used:
* External References:
* Parameters passed:
*	ip - interface point
* Return information:
*	TRUE - process is ok,
*	FALSE - process is either dead or never existed
*
*/
BOOLEAN dst_ok(ip)
    register USHORT  ip;
{
	DEBUG1("%sdst_ok\n", devname);
        if (ip > NUM_REG_ENTRIES)
	{
		DEBUG1("%sdst_ok, ip > NUM_REG_ENTRIES\n", devname);
                return(FALSE);
	}

        if (rt[ip].process_id == EMPTY)
	{
		DEBUG1("%sdst_ok, process_id = EMPTY\n", devname);
                return(FALSE);
	}
	return(TRUE);
}
/**/ 
/* **************************************************
* 
* Module: registration
* Purpose: used for register and de-registering an interface point
* Data used:
* External References:
* Parameters passed:
*	epb - pointer to an EPB
*	pid - process id
* Return information:
*
*/
LONG registration(epb,pid)
    register CE_EPB *epb;
    SHORT pid;
{
        VOID uwrite();
        VOID ret_or_rel();

	register REG_ENT *rgptr;
        USHORT ip;
	ULONG cpu;

	cpu = GET_CPU(epb->hdr.dst_id);

        if (epb->hdr.buf != (BUF *)NULL)
                return(ENOTBLK);

        ip = GET_IP(epb->hdr.src_id);
	rgptr = &rt[ip];

        switch(epb->hdr.type)
        {
        case CE_DEREGISTRATION_REQUEST:
                if (ip >= NUM_REG_ENTRIES)
                        return(ERANGE);

                if (rgptr->process_id == pid)
                {
                        rgptr->process_id = EMPTY;
                        rgptr->proc = (struct proc *)NULL;
                        release_sepb(epb);
			/* release any epbs in read queue */
                        ret_or_rel(ip);
                }
                else
                        return(ECHILD);
                break;
        case CE_REGISTRATION_REQUEST:
		/* any IP will do */
                if (ip == 0)
                {
                        if (!find_empty(&ip))
                        {
                                return(ENOSPC);
                        }
                        else
                        {
				rgptr = &rt[ip];
                                rgptr->process_id = pid;
                                rgptr->proc = u.u_procp;
                                epb->hdr.src_id = PUT_ID(lc[cpu].local_cpu,CI_IP);
                                epb->hdr.dst_id = PUT_ID(lc[cpu].local_cpu,ip);
                                epb->hdr.epb_size = sizeof(CE_EPB);
                                epb->hdr.status = NO_ERROR;
                                epb->hdr.type = CE_REGISTRATION_RESPONSE;
                                uwrite(ip,epb);
                        }
                }
                else            /* I want this one */
                {
                        if (ip > NUM_REG_ENTRIES)
                                return(ERANGE);
                        if (rgptr->process_id != EMPTY)
                                return(EMLINK);
                        rgptr->process_id = pid;
                        rgptr->proc = u.u_procp;
                        epb->hdr.src_id = PUT_ID(lc[cpu].local_cpu,CI_IP);
                        epb->hdr.epb_size = sizeof(CE_EPB);
                        epb->hdr.dst_id = PUT_ID(lc[cpu].local_cpu,ip);
                        epb->hdr.status = NO_ERROR;
                        epb->hdr.type = CE_REGISTRATION_RESPONSE;
                        uwrite(ip,epb);
                }
                break;
        }
        return(NO_ERROR);
}
/* ********
**  where possible, epb's waiting for a user to read should
**  be returned to the sender
************
*/
/**/ 
/* **************************************************
* 
* Module: ret_or_rel
* Purpose: return or release the information destined for an interface point
* Data used:
* External References:
* Parameters passed:
*	ip - interface point
* Return information: none
*
*/
VOID ret_or_rel(ip)
USHORT ip;
{
        VOID ret_to_sender();
        EPB *uread();

        register EPB *epb;
	ULONG cpu;

        while ((epb = uread(ip)) != (EPB *)NULL)
        {
		/*
			was the epb already returned from it's 
			original destination 
		*/
		DEBUG2("%sret_or_release, epb: %x\n", devname, epb);
		DEBUG2("%sret_or_release, ci_status: %x\n", devname, epb->status);
                if (epb->status >= CI_STATUS)
                {
			cpu = epb->pool.cpu;
                        if (epb->buf != (BUF*)NULL)
                                release_sbuf(cpu, epb->buf);

                        release_sepb(epb);
                }
                else
                        ret_to_sender(epb,UNDELIVERABLE);
        }
}
/**/ 
/* **************************************************
* 
* Module: ret_to_sender
* Purpose:
* Data used:
* External References:
* Parameters passed:
* Return information:
*
*/
VOID ret_to_sender(epb,status)
    register EPB *epb;
    USHORT status;
{
        POOL_T pool;
        register EPB *r_epb;
	ULONG cpu;

        swap_id(epb);
        epb->status = status;
        cpu = GET_CPU(epb->dst_id);


        if (cpu == lc[cpu].local_cpu)     /* return to local */
        {
                if (dst_ok(GET_IP(epb->dst_id)))
                        uwrite(GET_IP(epb->dst_id), epb);
                else
                {
                        if (epb->buf != (BUF *)NULL)
                                release_sbuf(cpu, epb->buf);
                        release_sepb(epb);
                }
        }
        else    /* return to remote */
        {
                if ((r_epb = get_sepb(cpu)) == (EPB *)NULL)
                {
                        add_retry(epb);
                        return;
                }
                
                pool = r_epb->pool;
                copy (epb,r_epb,epb->epb_size);
                r_epb->pool = pool;

                if (pwrite(r_epb) != NO_ERROR)
                {
                        release_sepb(r_epb);
                        add_retry(epb);
                        return;
                }
                release_sepb(epb);
        }
}
/**/ 
/* **************************************************
* 
* Module: swap_id
* Purpose: swap the source and destination id's
* Data used:
* External References:
* Parameters passed:
*	epb - pointer to an EPB
* Return information: none
*
*/
swap_id(epb)
    register EPB *epb;
{
        register ULONG tmp_id;

        tmp_id = epb->src_id;
        epb->src_id = epb->dst_id;
        epb->dst_id = tmp_id;
}
/**/ 
/* **************************************************
* 
* Module:
* Purpose:
* Data used:
* External References:
* Parameters passed:
* Return information:
*
*/
add_retry(epb)
    register EPB *epb;
{
	ULONG cpu;

	cpu = GET_CPU(epb->src_id);

        printf("%s: Releasing undelivered message - please fix me!!\n",devname);
        if (epb->buf != (BUF*)NULL)
                release_sbuf(cpu,epb->buf);
	release_sepb(epb);
}
/**/ 
/* **************************************************
* 
* Module:
* Purpose:
* Data used:
* External References:
* Parameters passed:
* Return information:
*
*/
copy(s,t,n)
register UBYTE *s;
register UBYTE *t;
register DEFAULT n;
{
        while (n--)
                *t++ = *s++;
}
/**/ 
/* **************************************************
* 
* Module:
* Purpose:
* Data used:
* External References:
* Parameters passed:
* Return information:
*
*/
#ifdef DEBUG
dump_epb(note,epb)
BYTE *note;
EPB *epb;
{
        printf("%s:epb = %x\n next %x\t dst_id %x\t src_id %x\n type %d\t status %d\nsize %d\tu_def %x\nbuf %x\n",
            note,epb,epb->next,epb->dst_id, epb->src_id, epb->type, epb->status, epb->epb_size,epb->user_def,epb->buf);
}
/**/ 
/* **************************************************
* 
* Module:
* Purpose:
* Data used:
* External References:
* Parameters passed:
* Return information:
*
*/
dump_bpe(note,bpe)
    BYTE *note;
    register BPE *bpe;
{
        printf("%s:bpe = 0x%x\nnext = 0x%x\tmsg/cmd = 0x%x\ndst_id = 0x%x\tsrc_id = 0x%x\nflags = 0x%x\n",
                note,bpe, bpe->next, bpe->msg, bpe->dst_id, bpe->src_id, bpe->flags);
}
#endif

