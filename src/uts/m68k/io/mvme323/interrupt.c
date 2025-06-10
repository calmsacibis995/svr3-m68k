/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		INTR()
*******************************************************************************

DESCRIPTION:
		This is the interrupt service routine for this controller.  It
		first makes sure it is OK to look at the controller's registers
		by waiting for RBS to clear.  Then it verifies that the 
		interrupt is meant for this unit by ensuring that RIO was set 
		by the controller.  If it is a legitimate interrupt, it reads 
		the IOPB address from the controller's address registers, and 
		then clears RIO and RBS.  Then the Main Bus DVMA memory map 
		resources are released.  The buf structure associated with the
		IOPB is updated by update_iopb(), and the IOPB is returned to 
		the FREE IOPB queue.  In the same manner as in strategy(), 
		any available buf structures are linked with any available 
		IOPBs and sent to the controller for immediate execution.
                                                                            
REQUIRES:                                                               
		The unit number that has interrupted.

SUBROUTINE CALLS:                                                       
		valid_iopb() - Checks validity of an IOPB address.
		dri_printf() - Prints a string to the console. (system)
		test_bit() - Tests a CSR register bit. (via TEST_() macro)
		clear_bit() - Clears a CSR register bit. (via CLEAR_() macro)
		read_addr() - Reads IOPB address from controller registers.
		update_buf() - Updates buf structure before returning it
		    to the operating system.
		enq_iopb() - Enqueues an IOPB onto specified queue.
		buf_to_iopb() - Attempts to make IOPBs out of bufs.
		go() - Sets the controller in motion with READY IOPB queue.

EFFECTS:                                                                
		This routine grabs control of the controllers registers for 
		the time necessary to read the address registers.
		The buf structure is updated for system use.
		Any waiting buf structures are linked with any available
		IOPBs, and sent to the controller for execution.

ROUTINE RETURN VALUES:                                                  
		SERVICED (1) if interrupt was serviced.
		NOT_SERVICED (0) if interrupt was not for this unit.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323intr(unit)
int    unit;
{
    IOPBPTR       done_iopb, read_addr();
    BOOL          valid_iopb();

    /* First wait for the register semaphore (RBS) is clear. */
    /* Then confirm that the interrupt was meant for this    */
    /* unit by checking whether the controller has set RIO.  */
    while (TEST_(RBS) IS_NOT CLEAR )
    {
	if(TEST_(FERR) IS_NOT CLEAR)  
		cmn_err(CE_PANIC, "M323: controller reported fatal error\n");
    }

    if (TEST_(RIO) IS SET)
    {
        /* If this interrupt is ours, then get the address of the  */
        /* completed IOPB from the controller's address registers. */

        done_iopb = read_addr();
        CLEAR_(RIO);   /* Clear the RIO and RBS bits in the CSR. */
        CLEAR_(RBS);

        if (valid_iopb(done_iopb))
        {
	    /* Update the buf structure before returning it. */
 	    /* Then queue the IOPB back onto the FREE QUEUE. */

	    /* If interrupt at end of chain is to be supported then code
	       needs to be added here to return all chained IOPBs to the
	       free queue. */
            update_buf(done_iopb);     
	    done_iopb->flags &= ~IOPBACTIVE;
	    /* If the controller does not have any IOPBs queued, then
	       zero the count of IOPBs sent to the controller, since it was
	       last flushed forceably. */ 
	    if(--m323qcnt <= 0)	/* decrement count of uncompleted IOPBs */
	    {
		    m323qcnt = 0;
		    m323rcnt = 0;
		    m323cflush = FALSE;
	    }
	    else
	    {
		    /* Count the number of IOPBs completed since the last
		       time the controllers queue was empty. Determine
		       when it is necessary to withhold IOPBs from the 
		       controller to allow the controller to complete all
		       the IOPBs in its queue. */
	    	    if(++m323rcnt >= FLUSHLIM)
		    {
			m323cflush = TRUE; /*allow controller to empty its 
					     queue */
		    }
	    }
            enq_iopb(Queue.free, done_iopb); 

        }
	else
            cmn_err(CE_PANIC,"MVME323 returned bogus IOPB address=> 0x%x\n",
                    done_iopb);

        /* If any more buf structures are available, attempt to make */
        /* IOPB's out of them, if possible.  If it IS possible, then */
        /* ship 'em out to the controller here, by calling go().     */

        if (buf_to_iopb())  
            go();               

        return(SERVICED);  
    }
    else                    
    {
        /* Even though the interrupt was NOT for this device, */
        /* we still have to clear RBS before returning.       */ 

        CLEAR_(RBS);
        return(NOT_SERVICED);     
    }
}

