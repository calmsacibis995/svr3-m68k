/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		INIT()
*******************************************************************************

DESCRIPTION:
                                                                            
           Check if a controller exists at the configured address

REQUIRES:                                                               

SUBROUTINE CALLS: attach() - Set up Driver for any Disk Drives that should be 
                             out there.
                  clear_bit() - Set a Bit in a Register to 0.
                  init_queues() - Get the BUF/IOPB Queues Ready for Use.

RETURN VALUE: None

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323init()
{
    CTLRINFO       *cinfo = &ctlr_table;
    UNITINFO       *uinfo;
    USHORT          status;
    USHORT          unit, controller, max_ctlrs;
    UCHAR	    ctlralive;
    register struct iobuf *dp;

max_ctlrs = m323_cnt / UCNT;

if ( Boot_Flag )
    init_queues(YES);                           /* Initialize IOPB/BUF Queue */

for ( controller = 0; controller < max_ctlrs; ++controller )
    {
    ctlralive = FALSE;
    cinfo->reg_address = (REGISTERS *)m323_addr[controller];   

          /* SEE IF THE CONTROLLER IS OUT THERE */

#ifdef DEBUG
    dri_printf("MVME323 Disk Controller at 0x%x",&cinfo->reg_address->fill1);
#endif
	/* Probe by reading the status register. This will cause the 
	   controller to set RBS (register busy semaphore) in the status
	   register for all subsequent reads until the RBS bit is cleared
	   by setting the "Clear RBS" bit in the command register (the 
	   code is the "clear_bit(RBS)" statement performed soon after the
	   bprobe.
	*/
    if ( (status = bprobe( &cinfo->reg_address->csr, -1 )) NOT= 0 )
        {
#ifdef DEBUG
        dri_printf("...NOT FOUND\n");
#endif
        cinfo->c_status = ABSENT;
        }
    else
        {
#ifdef DEBUG
        dri_printf("...FOUND\n");
#endif
        cinfo->c_status = PRESENT;

	 /* clear the "register busy semaphore" bit which was set by the 
	    controller after bprobe read the status register. */
        clear_bit( RBS );

        for ( unit = 0; unit < UCNT; ++unit )
            {
            uinfo = &unit_table[(controller * M323DRV) + unit];

            uinfo->int_level = m323_ilev[controller];
            uinfo->vector = m323_ivec[controller] / 4;
            uinfo->reg_address = cinfo->reg_address;

	    dp = &m323utab[(controller * M323DRV) + unit];
	    dp->b_flags = B_CONF;    /* read config off disk at firstopen. */

	   /* set up for error logging */
	    dp->io_addr = (paddr_t)&m323eregs[controller];
	    dp->io_nreg = M323EREG;
	    dp->io_stp = (struct iostat *)&m323stat[(controller * M323DRV) + unit];
	   
            if(attach( unit ))        /* Try to Attach this Controller's Units */
		ctlralive = TRUE;	
            }
        }
	if(ctlralive == TRUE)
		setformparm(controller);
    }
}

