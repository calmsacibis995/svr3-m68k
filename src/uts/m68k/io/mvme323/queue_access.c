/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		DEQ_BUF()
*******************************************************************************

DESCRIPTION:
		This function dequeues the next buf structure from 
		the BUF queue.
                                                                            
REQUIRES:                                                               
		Nothing.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Re-arranges pointers in BUF queue head node.

ROUTINE RETURN VALUES:                                                  
		Pointer to dequeued buf structure.
		EMPTY if the queue was empty.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL BUFPTR deq_buf()
{
    FAST BUFPTR     bufptr;

    if ((bufptr = Queue.buffs->av_forw) IS_NOT EMPTY ) 
    {
        /* If the head node "forward" pointer is not EMPTY, then there is a */
        /* buf structure available.  Reset the head node "forward" pointer  */
        /* to equal the "forward" pointer of the buf currently on top of   */
        /* the BUF queue, thereby removing the buf from the linked list.   */   
    
        Queue.buffs->av_forw = bufptr->av_forw;
        
        /* If the dequeued buf was the only one on the queue, then its */
        /* forward pointer would be EMPTY. If so, set the head node's  */
        /* "back" pointer to EMPTY, thereby flushing the queue.        */
    
        if (Queue.buffs->av_forw IS EMPTY) 
            Queue.buffs->av_back = EMPTY;   
    }
    
    /* Return either the address of the dequeued buf, */
    /* or if the queue was empty, return EMPTY.       */
     
    return(bufptr);
}

/******************************************************************************
TITLE:		DEQ_IOPB()
*******************************************************************************

DESCRIPTION:
		This function removes the top IOPB from a queue.
                                                                            
REQUIRES:                                                               
		A pointer to the queue.

SUBROUTINE CALLS:                                                       
		put_addr() - Puts a data or "next" IOPB address into an IOPB.
		get_addr() - Gets a data or "next" IOPB address into an IOPB.

EFFECTS:                                                                
		Adjusts pointers of the queue head and the affected IOPBs
		on the queue.

ROUTINE RETURN VALUES:                                                  
		A pointer to the dequeued IOPB.
		EMPTY if the queue is empty.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL IOPBPTR  deq_iopb (queueptr)
IOPBPTR                  queueptr;
{
    ULONG      address, get_addr();
    IOPBPTR    iopbptr;

    /* If the "next" IOPB pointer of the head node is not EMPTY, then there */
    /* is an IOPB on the chain.  Turn OFF the Chain Enable bit of the IOPB  */
    /* on top before taking it.  Then get the "next" IOPB address from the */
    /* IOPB to be dequeued, and put it into the "next" IOPB pointer of the */
    /* head node, thereby making it the new "top of the queue" IOPB.       */
    
    if ((iopbptr = (IOPBPTR)get_addr(queueptr, NEXT_IOPB)) IS_NOT EMPTY)
    {
        iopbptr->byte[COMMAND] &= ~CHEN;     
        address = get_addr(iopbptr, NEXT_IOPB);
        put_addr(queueptr, address, NEXT_IOPB);
    
        /* If the queue was emptied, set the "back" pointer of */
        /* the head node to EMPTY, thereby flushing the queue. */
    
        if (address IS EMPTY)     
            put_addr(queueptr, (ULONG)NULL, LAST_IOPB);
    }
    
    /*  Return the value returned by get_addr():  */ 
    /*    1) The address of the dequeued IOPB, or  */
    /*    2) EMPTY if the queue was empty.         */
    
    return(iopbptr);
}

/******************************************************************************
TITLE:		ENQ_BUF()
*******************************************************************************

DESCRIPTION:
		This function enqueues a buf structure from the operating 
                system onto the driver's BUF queue.  
                                                                            
REQUIRES:                                                               
		A buf pointer.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Re-arranges pointers in BUF queue head node and any affected
		buf structure already on the queue.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL enq_buf (bufptr)
FAST BUFPTR    bufptr;
{

    bufptr->av_forw = EMPTY;  /* Set the buffer's "forward" pointer to EMPTY. */

    if (Queue.buffs->av_forw IS EMPTY)  
    {
        /* If the head node's "forward" pointer is EMPTY, then */
        /* the BUF queue is empty.  Set the "forward" pointer  */
        /* of the head node to point to the buf struture sent. */
    
        Queue.buffs->av_forw = bufptr;   
    }
    else
    {
        /* Otherwise, set the "forward" pointer of the last buf */
        /* in the queue to point to the new buf structure.      */
    
        Queue.buffs->av_back->av_forw = bufptr; 
    }
    
    /* Set the "back" pointer of the head node */
    /* to point to the new buf structure.      */
    
    Queue.buffs->av_back = bufptr;
}

/******************************************************************************
TITLE:		ENQ_IOPB()
*******************************************************************************

DESCRIPTION:
		This function enqueues an IOPB onto a queue.
                                                                            
REQUIRES:                                                               
		A flag indicating which queue to use.
		A pointer to the IOPB.

SUBROUTINE CALLS:                                                       
		put_addr() - Puts a data or "next" IOPB address into an IOPB.
		get_addr() - Gets a data or "next" IOPB address into an IOPB.

EFFECTS:                                                                
		Adjusts pointers of the queue head and the affected IOPBs
		on the queue.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL enq_iopb (queueptr, iopbptr)
FAST IOPBPTR    queueptr, iopbptr;
{
    IOPBPTR    lastiopb;
    ULONG      get_addr();

    /* If the "next" IOPB pointer of the head node is not EMPTY, then */
    /* the queue is not empty, and the IOPB must be added to the end  */
    /* of the queue.  If it is NULL, then the IOPB is just added to   */
    /* the top of the queue.                                          */
    
    if (get_addr(queueptr, NEXT_IOPB) IS_NOT EMPTY)      
    {
        /* Get the address of the last IOPB on the queue. */
    
        lastiopb = (IOPBPTR)(get_addr(queueptr, LAST_IOPB));
    
        /* Set the Chain Enable bit in the last IOPB on the queue.   */
        /* Then put the address of the new IOPB into the "next" IOPB */
        /* field of the last IOPB on the queue.  The new IOPB then   */
        /* becomes the last IOPB on the chain.                       */    
    
        lastiopb->byte[COMMAND] |= CHEN;
	lastiopb->byte[IOPB_MOD] = STD_SUPER_DATA;
        put_addr(lastiopb, (ULONG)iopbptr, NEXT_IOPB);
    }
    else                                                
    {
        /* The queue is empty, so just add the */ 
        /* new IOPB to the top of the queue.   */
    
        put_addr(queueptr, (ULONG)iopbptr, NEXT_IOPB);
    }
    
    /* Finally, set the "next" IOPB field of the new IOPB to EMPTY. */
    /* and set the "back" pointer of the head node to point to the */
    /* IOPB that was just added to the queue.                      */
    
    put_addr(iopbptr, (ULONG)NULL, NEXT_IOPB);           
    put_addr(queueptr, (ULONG)iopbptr, LAST_IOPB);
    
}

/*****************************************************************************
TITLE:		FLUSH_QUEUE()
*******************************************************************************

DESCRIPTION:
		This function stuffs NULLs into the data and next IOPB 
		address fields of the appropriate IOPB queue head node. 
                                                                            
REQUIRES:                                                               
		A pointer to the IOPB queue.

SUBROUTINE CALLS:                                                       
		put_addr() - Puts a data or "next" IOPB address into an IOPB.

EFFECTS:                                                                
		Effectively flushes a queue.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL flush_queue (queueptr)
IOPBPTR            queueptr;
{
    put_addr(queueptr, (ULONG)NULL, NEXT_IOPB);
    put_addr(queueptr, (ULONG)NULL, LAST_IOPB);
}

/******************************************************************************
TITLE:		INIT_QUEUES()
*******************************************************************************

DESCRIPTION:
		This function initializes the queue head nodes of the FREE
		and READY IOPB queues, and the BUF queue.  Then it allocates
		ICNT IOPBs and queues them onto the FREE IOPB queue, and UCNT 
		IOPBs and puts them into the raw IOPB array.
                                                                            
REQUIRES:                                                               
		Nothing.

SUBROUTINE CALLS:                                                       
		enq_iopb() - Enqueues an IOPB onto a specified queue.
		flush_queue() - Flushes a queue.
		rmalloc() - Dynamically allocates memory. (system)

EFFECTS:                                                                
		Sets up queue structures and allocates IOPB data space.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL init_queues()
{
    FAST USHORT     index;

    Boot_Flag = NO;     /* Clear the boot flag on initial probe() call. */

    Queue.ready = &ready_head;     /* Initialize the driver's master */
    Queue.free = &free_head;       /* queue pointer structure.       */
    Queue.buffs = &buf_head;
    
    flush_queue(Queue.free);       /* Flush both of the IOPB queues. */
    flush_queue(Queue.ready);

    Queue.buffs->av_forw = EMPTY;   /* Flush the BUF queue. */
    Queue.buffs->av_back = EMPTY;

    /* Allocate ICNT IOPBs and enqueue them onto the FREE IOPB queue. */
    
    for (index = 0; index < ICNT; ++index)    
    {
        iopb[index] = &io_vec[ index ];
        enq_iopb(Queue.free, iopb[index]);
    }
    
    /* Allocate UCNT IOPBs and put them into the raw IOPB array. */
    
    for (index = 0; index < UCNT; ++index)  
        riopb[index] = &io_vec[ ICNT + 1 ];
    
    /* Initialize the buf structure array. */

    for (index = 0; index < UCNT; ++index)  
        rbuf[index].b_flags = CLEAR;

}

