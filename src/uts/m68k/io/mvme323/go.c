/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		GO()
*******************************************************************************

DESCRIPTION:
		This function, called by strategy() and interrupt(), stuffs the
		address of the top IOPB in the READY IOPB queue into the 
		controller's address registers and then sets AIO in order
		to set the controller in motion.  It must wait for RBS and
		AIOP to clear before setting AIO.
                                                                            
REQUIRES:                                                               
		At least one IOPB to be on the READY IOPB queue.

SUBROUTINE CALLS:                                                       
		stuff_addr() - Puts an IOPB address into the controller's
		    registers for execution.
		test_bit() - Tests a CSR register bit. (via TEST_() macro)
		set_bit() - Sets a CSR register bit. (via SET_() macro) 
		flush_queue() - Flushes a queue.

EFFECTS:                                                                
		Sets the controller in motion.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL go()
{

    while (TEST_(RBS) IS_NOT CLEAR)   /* Wait for RBS and AIOP to clear */ 
    {
	if(TEST_(FERR) IS_NOT CLEAR)  
		cmn_err(CE_PANIC, "M323: controller reported fatal error\n");
    }
      
                                           /* before accessing the AIO bit.  */
    while (TEST_(AIOP) IS_NOT CLEAR)
    {
	if(TEST_(FERR) IS_NOT CLEAR)  
		cmn_err(CE_PANIC, "M323: controller reported fatal error\n");
    }
                              /* Load the controller's address registers with */
    stuff_addr(READY_QUEUE); /* the first IOPB in the READY IOPB queue.      */
    SET_(AIO);             /* Activate the controller by setting the AIO bit. */

    flush_queue(Queue.ready);                /* Flush the READY IOPB queue. */

    CLEAR_(RBS);                  /* We're done manipulating the controller's */
                                  /* registers, so clear the RBS bit.         */
}

