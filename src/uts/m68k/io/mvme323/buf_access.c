/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		BUF_TO_IOPB()
*******************************************************************************

DESCRIPTION:
		This function will take buf structures and make IOPB's out of
		them until it runs out of either one of them.  It does this by
		dequeueing a buf structure from the BUF queue and an IOPB from
		the FREE IOPB queue.  It makes an IOPB out of the buf struc-
		ture and queues it onto the READY IOPB queue.  It calls 
		itself before returning in case it can make more IOPBs.  It 
		returns TRUE if it made an IOPB, and FALSE if there were no 
		buf structures or free IOPBs to use.
 
REQUIRES:                                                               
		A buf structure and a FREE IOPB to return successfully.

SUBROUTINE CALLS:                                                       
		make_iopb() - Makes an IOPB out of a buf structure.
		deq_iopb() - Dequeues an IOPB from the FREE IOPB queue.
		deq_buf() - Dequeues a buf from the BUF queue.
		enq_iopb() - Enqueues an IOPB onto a queue.
		buf_to_iopb() - Recursively calls itself.        

EFFECTS:                                                                
		This function acts as an interface between strategy() and
		make_iopb().  It dequeues buf structures and free IOPBs, and
		queues IOPBs onto the READY IOPB queue.

ROUTINE RETURN VALUES:                                                  
		TRUE if at least one IOPB was made and queued onto the 
		READY IOPB queue.
		FALSE if there was either no buf structure or no free IOPB.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL buf_to_iopb()
{
    FAST IOPBPTR iopbptr;
    IOPBPTR      deq_iopb();
    FAST BUFPTR  bufptr;
    BUFPTR       deq_buf();
    register struct iobuf *dp;

    /* If either the BUF queue OR the FREE IOPB */
    /* queue are empty, then return FALSE.      */
    
    if (Queue.buffs->av_forw IS EMPTY) 
        return(FALSE);

    /* Periodically, allow controller to complete all IOPBs in its queue
       before sending it more IOPBs to perform. */
    if(m323cflush)     
	return(FALSE);

    if ((iopbptr = deq_iopb(Queue.free)) IS EMPTY )
        return(FALSE);
    
    bufptr = deq_buf();               /* Dequeue a buf structure. */
    make_iopb(iopbptr, bufptr);       /* Make an IOPB out of it.  */
    iopbptr->flags |= IOPBACTIVE;
    m323qcnt += 1;		      /* count the number of requests enqueued*/
    enq_iopb(Queue.ready, iopbptr);  /* Enqueue it on the READY IOPB queue. */
    
    dp = &m323utab[POS(iopbptr->buffer->b_dev)];
    dp->io_start = lbolt;	/* record the start time */
    buf_to_iopb();       /* Call this routine recursively until */
                         /* no more buf-IOPB pairs can be made. */
    return(TRUE);
}

/******************************************************************************
TITLE:		UPDATE_BUF()
*******************************************************************************

DESCRIPTION:
		This function is responsible for updating the buf structure(s)
		that is/are associated with the IOPB or IOPB chain that is sent
	 	to it.  After doing so, the bufs are returned to the system via
		a call to iodone().

REQUIRES:                                                               
		An IOPB pointer.

SUBROUTINE CALLS:                                                       
		get_addr() - Get the "next" IOPB address from an IOPB.
		update_iopb() - Recursively calls this routine.
		iodone() - Returns a buf structure back to the operating
		    system. (system)

EFFECTS:                                                                
		Updates the b_flags, b_error, b_resid fields of the buf 
		structure that is associated with a completed IOPB.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL update_buf(iopbptr)
register IOPBPTR    iopbptr;
{
    register BUFPTR     bufptr;
    register struct iobuf *dp;
    IOPBPTR    nxt_iopbptr;
    ULONG      get_addr();
    BOOL       retry = NO;
    UINT       unit, errortype;

bufptr = iopbptr->buffer;  
dp = &m323utab[POS(bufptr->b_dev)];
unit = DRIVE(bufptr->b_dev);     
errortype = SOFTERR;

if ( (iopbptr->byte[COMMAND] & ERRS) &&
     ( ( RAWBUF(bufptr) || Debug ||
         INTERVENTION(iopbptr->byte[COMPCODE]) ||
         MISC_ERROR(iopbptr->byte[COMPCODE])  ||
         RESET_RETRY(iopbptr->byte[COMPCODE]) ||
         YOUR_LOST(iopbptr->byte[COMPCODE])   ||
        (RETRY_HARD(iopbptr->byte[COMPCODE]) && bufptr->b_error >= Retries)) ) )
    {

    dri_printf("\nM323: command 0x%x failed on unit %d slice %d  Completion Code 0x%x", 
		iopbptr->byte[COMMAND], unit, PARTITION(bufptr->b_dev), 
		iopbptr->byte[COMPCODE]);

    if ( RETRY_HARD(iopbptr->byte[COMPCODE]) )
        dri_printf("...Retry No. %d\n", bufptr->b_error + 1 );
    else
        dri_printf("\n");
    }

if ( RAWBUF(bufptr) )  /* If raw, copy the IOPB to its associated raw IOPB. */
    copy(iopbptr, bufptr->b_bcount, sizeof(IOPB));
else
    {  
    if ( (iopbptr->byte[COMMAND] & ERRS) &&    /* NON-SOFT ERROR OCCURED?  */
          NOT(SOFT_ERROR( iopbptr->byte[COMPCODE] )) ) 
        {
        if ( RETRY_HARD( iopbptr->byte[COMPCODE] ) && /* Is Retryable?      */
             bufptr->b_error < Retries )              /* Retries Available? */
            {
            ++bufptr->b_error; /* INCREMENT RETRY COUNT */
	    m323elog(iopbptr, dp, errortype);
            retry = YES;
            }
        else         
            {
	    errortype = HARDERR;
            bufptr->b_resid = bufptr->b_bcount;
            bufptr->b_flags |= B_ERROR;
            bufptr->b_error = EIO;
	    m323elog(iopbptr, dp, errortype);
            }
        }
    else             /* NO ERROR */
        {
        bufptr->b_resid = 0;        /* The operation was a success,  */
        bufptr->b_error = CLEAR;    /* so set the flags accordingly. */
        }
    }

/* The following code has been removed because this driver was not designed
   to use the interrupt at end of chain feature in the controller. To 
   effectively use the feature, the driver must maintain a seperate ready queue 
   for each drive so that only commands for the same drive will be chained. 
   The interrupt will occur when all commands in the chain have completed. */
#ifdef 0
if (iopbptr->byte[COMMAND] & CHEN)
    {  
    /* recursively call update_buf for all IOPBs in the chain. */
    nxt_iopbptr = (IOPBPTR)get_addr(iopbptr, NEXT_IOPB);
    update_buf(nxt_iopbptr); 
    }
#endif

if ( retry )
    m323strategy( bufptr );
else
    iodone(bufptr);      /* Signal the world that the buf is done. */
}



/*
 *	m323elog: performs error logging
 */

m323elog(iopbptr, dp, errortype)
register IOPBPTR iopbptr;
register struct iobuf *dp;
register USHORT errortype;
{	
	register struct m323eregs *erp;
	register BUFPTR bufptr;
	register PARTINFO  *pinfo;
	register UINT part, unit;

	bufptr = iopbptr->buffer;
	unit = DRIVE(bufptr->b_dev);     /* Set up local variables and pointers. */
	part = PARTITION(bufptr->b_dev);     
        pinfo = (PARTINFO *)&(part_table[(unit*PCNT) + part]);


	erp = (struct m323eregs *)dp->io_addr;

	erp->er_blkoff = bufptr->b_blkno + pinfo->first; /* block offset */
	erp->er_cmd = iopbptr->byte[COMMAND];   /* iopb command byte */
	erp->er_cmplcode = iopbptr->byte[COMPCODE]; /* iopb completion code */
	erp->er_st2 = iopbptr->byte[DISKSTAT]; /* drive status */
	erp->cyl = (iopbptr->byte[CYLH] << 8) + iopbptr->byte[CYLL];
	erp->head = iopbptr->byte[HEAD];

	/* get major number of block device for error logging */
	dp->b_dev = makedev( M323, minor(dp->b_dev));
	dp->b_actf = bufptr;

	fmtberr( dp, 0 ); 		/* post to error log */

	/* finalize the error report for this error */  
	logberr(dp, errortype);
}




