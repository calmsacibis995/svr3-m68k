/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		STRATEGY()
*******************************************************************************

DESCRIPTION:
		This function accepts buf structures from the operating system,
		and takes the steps necessary to effect the required data 
		transfer.  After checking for the sanity of the buf structure 
		sent, it raises its priority to that of the interrupt handler.
		It then queues the buf structure on the BUF queue, and calls
		buf_to_iopb().  If the function returns TRUE, meaning that 
		there is an available IOPB structure(s) to execute, then go()
		is called to send the IOPB to the controller.

REQUIRES:                                                               
		A pointer to a buf structure.

SUBROUTINE CALLS:                                                       
		iodone() - Returns a buf to the operating system. (system)      
		enq_buf() - Enqueues a buf structure onto the BUF queue.
		buf_to_iopb() - Attempts to make IOPBs out of bufs.    
		go() - Sets controller in motion with READY IOPB Queue.

EFFECTS:                                                                
		If buf structure is incorrect, calls iodone() here.
		Queues the buf structure onto the BUF queue.
		Temporarily locks out interrupts from this device.
		Attempts to get an IOPB to associate with the buf structure.
		If an IOPB is ready, sends it to the controller.
		
ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/
/* Physical I/O breakup routine */
m323breakup(bp)
register struct buf *bp;
{
	dma_breakup(m323strategy, bp, BSIZE);
}


m323strategy(bufptr)
FAST BUFPTR      bufptr;
{
    FAST UNITINFO     *uinfo;
    FAST PARTINFO     *pinfo;
    FAST USHORT        unit, part;
    FAST ULONG         last;
    USHORT    old_priority;
    register struct iotime *ip;

    unit = DRIVE(bufptr->b_dev);        /* Set up local variables. */
    part = PARTITION(bufptr->b_dev);
    uinfo = &unit_table[unit];
    pinfo = (PARTINFO *)&(part_table[(unit*PCNT) + part]);


    if (unit >= UCNT)           /* If the unit number is too big */
    {
        bufptr->b_flags |= B_ERROR;     
        bufptr->b_error = ENXIO;   /*  "non-existant" device */
	bufptr->b_resid = bufptr->b_bcount;
        iodone(bufptr);
        return;
    }
    
    if (pinfo->p_status == ABSENT) 
    {
        dri_printf("M323: Unit %d Not Operational\n",unit);
        bufptr->b_flags |= B_ERROR;    /*  unit isn't operational */
        bufptr->b_error = ENXIO;   /*  "non-existant" device */
	bufptr->b_resid = bufptr->b_bcount;
        iodone(bufptr);
        return;
    }
    
    /* If this is a raw buf structure, the b_bcount field is used */
    /* as a pointer to a user-created IOPB.  Therefore, the next  */
    /* test is irrelevant to raw bufs.                            */

    if (! RAWBUF(bufptr))
    {
	/* no partial disk block i/o */
	if(bufptr->b_bcount&(BSIZE-1))
    	{
		bufptr->b_flags |= B_ERROR;
        	bufptr->b_error = ENXIO;   /*  "non-existant" device */
		bufptr->b_resid = bufptr->b_bcount;
       		iodone(bufptr);
	        return;
    	}

	last = bufptr->b_blkno;
    
	if(last >= pinfo->length) 
	{
		if((last == pinfo->length) && (bufptr->b_flags&B_READ))
		{
			bufptr->b_resid = bufptr->b_bcount;
		}
		else
		{
			/*  request over-runs partition, */
      			bufptr->b_flags |= B_ERROR;  
		}
        	iodone(bufptr);
	        return;
	}
    }

    /* log I/O statistics for this drive */
    ip = &m323stat[POS(bufptr->b_dev)];
    if( ( bufptr->b_flags & NON_RWIO) )
	++(ip->io_other);
    else {
	++(ip->io_cnt);	
	ip->io_bcnt += bufptr->b_bcount/BSIZE;
    }

   /* Convert the Ctlr's hardware priority level to a CPU priority level, and */
   /* temporarily assign it to this routine.  This will effectively lock out  */
   /* interrupts while data and registers that are common to the interrupt    */
   /* service routine are accessed.  The old priority level will be stored    */
   /* in "old_priority".                                                      */

    old_priority = SPL( INTR_PRIORITY );
        
    enq_buf(bufptr);    /* Enqueue the buf structure onto the BUF queue. */


    /* If there are any IOPBs on the FREE IOPB queue, dequeue them, and   */
    /* make new IOPBs for as many buf structures as are on the BUF queue. */
    /* Queue them onto the READY IOPB queue, and dispatch them to the     */
    /* controller by calling go().                                        */

    if (buf_to_iopb())     
        go();               

    splx(old_priority);    /* Restore the former CPU priority level. */
}

