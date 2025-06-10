/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		EXECUTE()
*******************************************************************************

DESCRIPTION:
		This function will build and execute an IOPB for any  
		command.  It will do so in a polled or interrupt manner.
                                                                            
REQUIRES:                                                               
		A manner of execution. 
                A command.
                A unit or dev_t, both of which are defined as USHORT.
		The unit information and controller information tables.

SUBROUTINE CALLS:                                                       
		init_iopb() - Initializes a generic IOPB.
		poll() - Handles interaction with controller for actual
		    execution of an IOPB in a polled fashion.
 		make_checksum() - Generates an IOPB checksum.

EFFECTS:                                                                
		Inserts the appropriate data into a raw IOPB.
		Interacts with the controller via execute().
		A controller reset utilizes the CSR rather than sending an IOPB.

ROUTINE RETURN VALUES:                                                  
		SUCCESS upon successfull execution of the command.
		FAILURE upon unsuccessfull execution of the command.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL execute( manner, command, device )
FAST int       manner;
FAST int       command;
FAST USHORT    device;   /* This is either a unit number or a dev_t number. */
{
    UNITINFO  *uinfo;
    CTLRINFO  *cinfo;
    IOPBPTR    iopbptr;
    BUFPTR     bufptr;
    USHORT     unit;

unit = (manner IS POLLED) ? device : DRIVE(device);  /* Set unit accordingly. */

uinfo = &unit_table[unit];            /* Point at the unit info-table.       */
cinfo = &ctlr_table;                  /* Point at the controller info-table. */
iopbptr = riopb[unit];                /* Point at the associated raw IOPB.   */

if (manner IS POLLED)
{
    if(command == XYSETFORMPAR)
	prep_iopb(iopbptr, unit);	  /* dont clear the IOPB bytes */
    else
    	init_iopb(iopbptr, unit);         /* Initialize the IOPB, and */

    iopbptr->byte[LEVEL] = DISABLED;   /* then disable interrupts. */
}

if (manner IS BY_INTERRUPT)
{
    bufptr = &rbuf[unit];               /* Point at the associated raw buf.  */
    while (bufptr->b_flags & B_BUSY)
    {                                    /* If the raw buf is B_BUSY, set the */
        bufptr->b_flags |= B_WANTED;     /* B_WANTED flag and wait patiently. */
        sleep((caddr_t)bufptr, PRIBIO + 1);              
    }
    bufptr->b_flags = B_RAWBUF | B_BUSY; /* Set B_RAWBUF and B_BUSY ONLY!!!   */
    bufptr->b_dev = device;              /* Copy in the dev_t device number.  */
    bufptr->b_bcount = (long)iopbptr;    /* Use b_bcount as an IOPB pointer.  */
    bufptr->b_error = CLEAR;             /* Clear the b_error flag.           */
    iopbptr->buffer = bufptr;            /* Have the IOPB point at this buf.  */
}

switch (command)
{
    /* Set up the IOPB by inserting the command, subfunction, and */
    /* any necessary parameters that haven't already been stuffed */
    /* into their appropriate fields.                             */

    case XYSETCTLRPAR:  iopbptr->byte[COMMAND] |= Write_Par;
                        iopbptr->byte[SUBFUNC] |= W_Controller;
                        iopbptr->byte[CTLRPAR1] = cinfo->param1;
                        iopbptr->byte[CTLRPAR2] = cinfo->param2;
                        iopbptr->byte[CTLRPAR3] = cinfo->param3;
                        iopbptr->byte[THROTTLE] = cinfo->throttle;
                        iopbptr->byte[0x13] = Ctype;
                        break;

    case XYSETDRVPAR:   iopbptr->byte[COMMAND] |= Write_Par;
                        iopbptr->byte[SUBFUNC] |= W_Drive;
                        iopbptr->byte[DRV_PARM] |= uinfo->drive_parms;
                        iopbptr->byte[HEAD] = uinfo->heads - 1;
                        iopbptr->byte[SECTOR] = uinfo->sectors - 1;
                        iopbptr->byte[MAXSECLH] = uinfo->sectors - 1;
                        iopbptr->byte[ACT_SECTORS] = uinfo->sectors;
                        iopbptr->byte[CYLL] = uinfo->cylinders - 1;
                        iopbptr->byte[CYLH] = (uinfo->cylinders - 1) >> 8;
                        break;

    case XYRDCTLRPAR:   iopbptr->byte[COMMAND] = Read_Par;
                        iopbptr->byte[SUBFUNC] = R_Controller;
                        break;

    case XYRDDRVPAR:    iopbptr->byte[COMMAND] = Read_Par;
                        iopbptr->byte[SUBFUNC] = R_Drive;
                        break;

    case XYDRVCONFIG:   iopbptr->byte[COMMAND] = Read_Par;
                        iopbptr->byte[SUBFUNC] = Drv_Conf;
                        break;

    case XYRDDEFECTS:   iopbptr->byte[COMMAND] = Ext_Read;
                        iopbptr->byte[SUBFUNC] = R_Def;
                        break;

    case XYFORMAT:      iopbptr->byte[COMMAND] = Ext_Write;
                        iopbptr->byte[SUBFUNC] = W_Format;
                        break;

    case XYRDTRKHDR:    iopbptr->byte[COMMAND] = Ext_Read;
                        iopbptr->byte[SUBFUNC] = R_Trk_Hdr;
                        break;

    case XYWRTRKHDR:    iopbptr->byte[COMMAND] = Ext_Write;
                        iopbptr->byte[SUBFUNC] = W_Trk_Hdr;
                        break;

    case XYDRVRESET:    iopbptr->byte[COMMAND] = Drive_Reset;
                        break;

    case XYOPTIONAL:    iopbptr->byte[COMMAND] = Opt_Cmd;
                        break;

    case XYREAD:        iopbptr->byte[COMMAND] = Read;
 			break;

    case XYWRITE:       iopbptr->byte[COMMAND] = Write;
 			break;

    case XYRDFORMPAR:	iopbptr->byte[COMMAND] = Read_Par;
			iopbptr->byte[SUBFUNC] = R_Format;
 			break;

    case XYSETFORMPAR:	iopbptr->byte[COMMAND] = Write_Par;
			iopbptr->byte[SUBFUNC] = W_Format;
 			break;

} /* End of switch. */

make_checksum(iopbptr);     /* Generate a checksum to avoid any hangups. */

if (manner IS BY_INTERRUPT) 
{
    m323strategy(bufptr);       /* Execute the IOPB in an interrupt manner. */
    iowait(bufptr);           /* Wait here until the buf is completed.    */

    if (bufptr->b_error & ENXIO)
    {                                        /* strategy() has determined   */
                                             /* this buf to be incorrect. */
        iopbptr->byte[COMMAND] |= ERRS;      /* ERRS for the hook below.    */
    }

    bufptr->b_flags &= ~B_BUSY;   /* Clear B_BUSY for the next ioctl() call. */

    if (bufptr->b_flags & B_WANTED)   /* If the buf is B_WANTED, wake up the */
        wakeup((caddr_t)bufptr);      /* caller who's requesting it above.   */

    if (iopbptr->byte[COMMAND] & ERRS)      /* Check the ERRS bit for errors. */
        return(FAILURE);
    else
        return(SUCCESS);
}

return(poll(iopbptr));      /* Execute the IOPB in a polled manner.   */
                            /* Return the status returned by poll(). */

}

/******************************************************************************
TITLE:		POLL()
*******************************************************************************

DESCRIPTION:
		This function executes a discrete IOPB in a "polled" fashion.
		First it waits for the RBS and AIOP bits to clear.  It then 
		stuffs the address of the IOPB into the controllers address 
		registers, and sets AIO.  When RIO is set by the controller,
		the ERRS bit is checked for execution errors.  If OK, the
		function returns SUCCESS, otherwise FAILURE. 
                                                                            
REQUIRES:                                                               
		A pointer to the IOPB to be executed.

SUBROUTINE CALLS:                                                       
		stuff_addr() - Stuffs an address into the controller's
		    address registers.
		test_bit() - Tests a CSR register bit. (via TEST_(x) macro)
		set_bit() - Sets a CSR register bit. (via SET_(x) macro)
		clear_bit() - Clears a CSR register bit. (via CLEAR_(x) macro)

EFFECTS:                                                                
		This routine performs all of the machinations that normally
		would be done by a call to strategy(), except that the 
		interrupt handler is not invoked.

ROUTINE RETURN VALUES:                                                  
		SUCCESS if IOPB is executed normally, or
		FAILURE if RBS is set, RIO is not set in a reasonable amount
 		of time, or ERRS is set in the returned IOPB.

*******************************************************************************
EDIT HISTORY:
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL poll (iopbptr)
IOPBPTR     iopbptr;
{
    USHORT   timeout, flip_flop = 0;
    ULONG    cnt;

    if (TEST_(RBS) IS_NOT CLEAR)
    {
        /* If RBS is not clear, bail out. */
    
        dri_printf("M323:  RBS NOT CLEAR\n");
        return(FAILURE);
    }

    while (TEST_(AIOP) IS_NOT CLEAR )  /* Wait for AIOP to clear. */ 
    {
	if(TEST_(FERR) IS_NOT CLEAR)  
		cmn_err(CE_PANIC, "M323: controller reported fatal error\n");
    }
    
    stuff_addr(iopbptr);             /* Set up the address registers. */
    
    SET_(AIO);                        /* Start the controller by setting AIO. */ 
    for ( cnt = 0; TEST_(RIO) IS CLEAR; ++cnt)
        {
        if ( cnt >= LOOP_LIMIT )
            {
            dri_printf("M323:  RIO NEVER SET\n");
            CLEAR_(RBS);
            return(FAILURE);
            }
        else
            if ( (cnt % 500) IS 499 )
                dri_printf("M323: WAITING ON RIO...CSR=> 0x%x  \r", read_reg(CSR) );
        }
    
    CLEAR_(RIO);     /* Clear the RIO and RBS bits. */
    CLEAR_(RBS);                                 
    
    if (iopbptr->byte[COMMAND] & ERRS)   /* Check the ERRS bit for success. */
        return(FAILURE);
    else
        return(SUCCESS);
}

