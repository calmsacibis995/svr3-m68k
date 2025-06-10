/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		CLEAR_BIT()
*******************************************************************************

DESCRIPTION:
		This function clears a requested bit in the CSR if it is
		currently set, thereby avoiding violation of the register 
		protocol.
                                                                            
REQUIRES:                                                               
		A bit offset in the CSR

SUBROUTINE CALLS:                                                       
		read_reg() - Reads a controller register.
		write_reg() - Writes to a controller register.

EFFECTS:                                                                
		The reading of the CSR will set the RBS bit.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL clear_bit (bit)
FAST UCHAR       bit;
{
    /* Only clear a bit if it is currently set. */

    if ((read_reg(CSR) & bit) IS_NOT CLEAR) 
        write_reg(CSR, bit);
}

/******************************************************************************
TITLE:		READ_ADDR()
*******************************************************************************

DESCRIPTION:
		This function reads the controller's address registers and
		returns a pointer to a completed IOPB.

REQUIRES:                                                               
		Nothing.

SUBROUTINE CALLS:                                                       
		read_reg() - Reads a controller register.

EFFECTS:                                                                

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL IOPBPTR read_addr()
{
    FAST ULONG big_addr = 0;        /* 32 Bit Address */

    /* Assemble the 32 bit address from the controller registers. */

    big_addr |= (ULONG)read_reg(ADDR_4) << 24;
    big_addr |= (ULONG)read_reg(ADDR_3) << 16;
    big_addr |= (ULONG)read_reg(ADDR_2) << 8;
    big_addr |= (ULONG)read_reg(ADDR_1);

    return((IOPBPTR)big_addr);
}

/******************************************************************************
TITLE:		READ_REG()
*******************************************************************************

DESCRIPTION:
		This function simply reads the controller register requested.
                                                                            
REQUIRES:                                                               
		Register offset.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Reading CSR will set RBS bit.

ROUTINE RETURN VALUES:                                                  
		Byte value read.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL read_reg(offset)
FAST UCHAR      offset;
{
    switch (offset)
    {
        case ADDR_1:   return ((UCHAR)ctlr_table.reg_address->addr1);

        case ADDR_2:   return ((UCHAR)ctlr_table.reg_address->addr2);

        case ADDR_3:   return ((UCHAR)ctlr_table.reg_address->addr3);

        case ADDR_4:   return ((UCHAR)ctlr_table.reg_address->addr4);

        case MODIFIER: return ((UCHAR)ctlr_table.reg_address->modifier);

        case CSR:      return ((UCHAR)ctlr_table.reg_address->csr);

        case FATAL:    return ((UCHAR)ctlr_table.reg_address->fatal);
    }
}

/******************************************************************************
TITLE:		SET_BIT() 
*******************************************************************************

DESCRIPTION:
		This function sets a requested bit in the CSR if it is NOT
		currently set, thereby avoiding violation of the register 
		protocol.
                                                                            
REQUIRES:                                                               
		A bit offset in the CSR

SUBROUTINE CALLS:                                                       
		read_reg() - Reads a controller register.
		write_reg() - Writes to a controller register.

EFFECTS:                                                                
		The reading of the CSR will set the RBS bit.

ROUTINE RETURN VALUES:                                                  
		None.
                                                                            
*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL set_bit (bit)
FAST UCHAR     bit;
{
    /* Set the bit only if it is currently clear. */

    if ((read_reg(CSR) & bit) IS CLEAR)
        write_reg(CSR, bit);
}

/******************************************************************************
TITLE:		STUFF_ADDR()
*******************************************************************************

DESCRIPTION:
		This function loads the controller's address registers.
		It receives either the READY_QUEUE flag (NULL) from go(),
		or the address of a discrete IOPB from execute().
		First, however, it has to strip the DVMA factor from the 
		"next" IOPB address in the READY IOPB queue head node, 
		as well as in any IOPB in the chain that points to a 
		"next" IOPB.  By doing so, the controller can directly DMA 
		the IOPB(s).
                                                                            
REQUIRES:                                                               
		A pointer to an IOPB.

SUBROUTINE CALLS:                                                       
                put_addr() - Writes an address into an IOPB.
		get_addr() - Returns an address from an IOPB.
		write_reg() - Writes to a controller register.

EFFECTS:                                                                
		Allows controller to directly DMA IOPBs.
		Puts controller in a state where it needs only the AIO bit
		to be set in order to take off.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL stuff_addr (iopbptr)
IOPBPTR           iopbptr;
{
    IOPBPTR       current, next;
    FAST ULONG    big_addr;        /* 32 bit address */
    ULONG         get_addr();
    CTLRINFO     *cinfo = &ctlr_table;


    if (iopbptr POINTS_TO READY_QUEUE) 
    {
        for (current = Queue.ready;
             (next = (IOPBPTR)get_addr(current, NEXT_IOPB)) IS_NOT NULL;)
        {
            put_addr(current, (ULONG)next, NEXT_IOPB);

            current = next;
	    if (cinfo->param1 & ICS) 
	        make_checksum(current);
        }

        big_addr = get_addr(Queue.ready, NEXT_IOPB);
    }
    else    /* Either it's a polled command or a RAW IOPB. */
    {
        big_addr = (ULONG)iopbptr;   
    }
    write_reg(ADDR_4, (UCHAR)(big_addr >> 24));
    write_reg(ADDR_3, (UCHAR)(big_addr >> 16));
    write_reg(ADDR_2, (UCHAR)(big_addr >> 8));
    write_reg(ADDR_1, (UCHAR)(big_addr));
    write_reg(MODIFIER, STD_SUPER_DATA);
}

/******************************************************************************
TITLE:		TEST_BIT()
*******************************************************************************

DESCRIPTION:
		This function tests a requested bit in the CSR register, and
		returns SET or CLEAR to the caller.
                                                                            
REQUIRES:                                                               
		A bit number in the CSR.

SUBROUTINE CALLS:                                                       
		read_reg() - Reads a controller register.

EFFECTS:                                                                
		By reading the CSR, RBS will become set.

ROUTINE RETURN VALUES:                                                  
		SET if bit is set, or
		CLEAR if bit is clear.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL test_bit (bit)
FAST UCHAR      bit;
{
    if (read_reg(CSR) & bit)
        return(SET);
    else
        return(CLEAR);
}

/******************************************************************************
TITLE:		WRITE_REG()
*******************************************************************************

DESCRIPTION:
		This function simply writes the byte value sent to the
		the controller register requested.
                                                                            
REQUIRES:                                                               
		Register offset and value to be written.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		See the Controller Manual for effects of writing to the 
                CSR register.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/
 
LOCAL write_reg (offset, value)
FAST UCHAR       offset, value;
{
    switch (offset)
    {
        case ADDR_1:   ctlr_table.reg_address->addr1 = value;
                       break;

        case ADDR_2:   ctlr_table.reg_address->addr2 = value;
                       break;

        case ADDR_3:   ctlr_table.reg_address->addr3 = value;
                       break;

        case ADDR_4:   ctlr_table.reg_address->addr4 = value;
                       break;

        case MODIFIER: ctlr_table.reg_address->modifier = value;
                       break;

        case CSR:      ctlr_table.reg_address->csr = value;
                       break;
    }
}

