/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		GET_ADDR() 
*******************************************************************************

DESCRIPTION:
		This function builds a 32 bit address from either an IOPB's
		next IOPB bytes or the data address bytes.

REQUIRES:                                                               
		An IOPB pointer.
		Either a NEXT_IOPB, DATA_ADDR or LAST_IOPB flag.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		None.

ROUTINE RETURN VALUES:                                                  
		A 32 bit address derived from the IOPB.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL ULONG  get_addr (iopbptr, where)
IOPBPTR                iopbptr;
UCHAR                   where;
{
    ULONG  address;

    switch (where)
    {
        case NEXT_IOPB:   address = iopbptr->byte[NIOPB1] << 24;
                          address |= iopbptr->byte[NIOPB2] << 16;
                          address |= iopbptr->byte[NIOPB3] << 8;
                          address |= iopbptr->byte[NIOPB4];
		          break;

	case LAST_IOPB:
        case DATA_ADDR:   address = iopbptr->byte[DATA1] << 24;
                          address |= iopbptr->byte[DATA2] << 16;
                          address |= iopbptr->byte[DATA3] << 8;
                          address |= iopbptr->byte[DATA4];
			  break;
    }
    return(address);
}

/******************************************************************************
TITLE:		INIT_IOPB()
*******************************************************************************

DESCRIPTION:
		This function initializes a general purpose IOPB.
                                                                            
REQUIRES:                                                               
		Pointer to the IOPB.
		The unit number.
		The unit information table.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Clears out remnants of old IOPBs, and inserts the unit number,
		interrupt level and interrupt vector values.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL init_iopb (iopbptr, unit)
IOPBPTR          iopbptr;
USHORT           unit;
{
    UNITINFO   *uinfo;
    FAST USHORT offset;

    uinfo = &unit_table[unit];

    /* First clear all of the IOPB bytes. */

    for ( offset = 0; offset < ISIZE; ++offset)  
    	iopbptr->byte[offset] = CLEAR;

    /* Then clear the address pointers. */
    iopbptr->buffer = NULL;

    /* Insert the unit number, interrupt level, and vector values. */

    iopbptr->byte[UNIT] |= (UCHAR)unit;
    iopbptr->byte[LEVEL] = (UCHAR)uinfo->int_level;
    iopbptr->byte[VECTOR] = (UCHAR)uinfo->vector;
}

/* Same as init_iopb except the IOPB bytes are not cleared. */
LOCAL prep_iopb (iopbptr, unit)
IOPBPTR          iopbptr;
USHORT           unit;
{
    UNITINFO   *uinfo;
    FAST USHORT offset;

    uinfo = &unit_table[unit];

    /* Then clear the address pointers. */
    iopbptr->buffer = NULL;

    /* Insert the unit number, interrupt level, and vector values. */

    iopbptr->byte[UNIT] |= (UCHAR)unit;
    iopbptr->byte[LEVEL] = (UCHAR)uinfo->int_level;
    iopbptr->byte[VECTOR] = (UCHAR)uinfo->vector;
}

/******************************************************************************

/******************************************************************************
TITLE:		MAKE_CHECKSUM()
*******************************************************************************

DESCRIPTION:
		This function generates a checksum of an IOPB and stuffs
		the value into the checksum bytes of the IOPB. 
REQUIRES:                                                               
		An IOPB pointer.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Updates the checksum bytes of the IOPB.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL make_checksum(iopbptr)
IOPBPTR             iopbptr;
{
    FAST ULONG  checksum;
    FAST USHORT index;

    for(index = 0, checksum = 0; index < CHECKH; index++)
	checksum += iopbptr->byte[index];

    iopbptr->byte[CHECKH] = (UCHAR)(checksum >> 8);
    iopbptr->byte[CHECKL] = (UCHAR)checksum;
}

/******************************************************************************
TITLE:		MAKE_IOPB()
*******************************************************************************

DESCRIPTION:
		This function builds an executable IOPB from a buf structure
		and the appropriate unit information table entries.

REQUIRES:                                                               
		Pointers to an IOPB and a buf structure.

SUBROUTINE CALLS:                                                       
		init_iopb() - Initializes a generic IOPB.
		copy() - Copies data from one location to another.
		convert_blkno() - Converts a logical block number from the 
		    operating system into a cylinder, head and sector numbers.

EFFECTS:                                                                
		The IOPB returned from this function is ready to be queued onto
		the ready IOPB queue.  The data address is converted to a form
		useable by the controller.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL make_iopb (iopbptr, bufptr)
IOPBPTR          iopbptr;
BUFPTR           bufptr;
{
    UNITINFO      *uinfo;
    PARTINFO      *pinfo;
    ULONG          data_addr;
    USHORT         part, unit, cylinder, head, sector, count;

    unit = DRIVE(bufptr->b_dev);     /* Set up local variables and pointers. */
    part = PARTITION(bufptr->b_dev);     
    uinfo = &unit_table[unit];
    pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + part]);

    if RAWBUF(bufptr)  /* If it's a raw buf, the IOPB has already been made. */
    {
        copy(bufptr->b_bcount, iopbptr, sizeof(IOPB));
        return;
    }


    /* We have to create the IOPB from scratch.  After init_iopb()  */
    /* clears all of the IOPB bytes, it stuffs the unit number, the */
    /* interrupt level, and interrupt vector.                       */

    init_iopb(iopbptr, unit);

    if (bufptr->b_flags & B_READ)      
        iopbptr->byte[COMMAND] = Read;
    else                                   /* Insert the command. */
        iopbptr->byte[COMMAND] = Write;

    /* Utilizing the arguments sent to it, convert_blkno() will  */
    /* compute and insert the head, sector, and cylinder numbers */
    /* into the HEAD, SECTOR, CYLL and CYLH fields of the IOPB.  */

    if(bufptr->b_flags & B_ABS)   /* do not add offset to b_blkno */
    	convert_blkno(bufptr->b_blkno, unit, iopbptr);
    else
    	convert_blkno(bufptr->b_blkno + pinfo->first, unit, iopbptr);

    /* Convert the byte count into a sector count. */

    iopbptr->byte[COUNTL] = (UCHAR)(bufptr->b_bcount/BSIZE);
    iopbptr->byte[COUNTH] = (UCHAR)(bufptr->b_bcount/BSIZE >> 8);

    data_addr = paddr(bufptr);
    
    /* Fill in the data address/modifier bytes. */

    put_addr(iopbptr, data_addr, DATA_ADDR);  
    iopbptr->byte[DATA_MOD] = STD_SUPER_DATA;

    iopbptr->buffer = bufptr;  /* The IOPB has to keep track of which  */
    			       /* buf structure it is related to here. */
}

/******************************************************************************
TITLE:		PUT_ADDR()
*******************************************************************************

DESCRIPTION:
		This function puts a 32 bit address into an IOPB's next IOPB
		address field or its data address field.
                                                                            
REQUIRES:                                                               
		An IOPB pointer.
		The 32 bit address to be stuffed.             
		Either a NEXT_IOPB, DATA_ADDR or LAST_IOPB flag.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		The appropriate field in the IOPB is updated.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL put_addr (iopbptr, address, where)
FAST IOPBPTR    iopbptr;
FAST ULONG      address;
FAST UCHAR      where;
{
    switch (where)
    {
        case NEXT_IOPB:  iopbptr->byte[NIOPB1] = (UCHAR)(address >> 24);
                         iopbptr->byte[NIOPB2] = (UCHAR)(address >> 16);
                         iopbptr->byte[NIOPB3] = (UCHAR)(address >> 8);
                         iopbptr->byte[NIOPB4] = (UCHAR)address;
                         break;

	case LAST_IOPB:
        case DATA_ADDR:  iopbptr->byte[DATA1] = (UCHAR)(address >> 24);
                         iopbptr->byte[DATA2] = (UCHAR)(address >> 16);
                         iopbptr->byte[DATA3] = (UCHAR)(address >> 8);
                         iopbptr->byte[DATA4] = (UCHAR)address;
                         break;
    }
}

/******************************************************************************
TITLE:		VALID_IOPB()
*******************************************************************************

DESCRIPTION:
		This function tries to match the IOPB pointer sent with both 
		the IOPB address array and the raw IOPB address array.  If it is
		found in either of those two arrays, the IOPB is valid and 
                the function returns TRUE.  Otherwise it is invalid, and FALSE
	 	is returned.
 
REQUIRES:                                                               
		A pointer to the IOPB, the IOPB address array, and the raw IOPB
		address array.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		None.

ROUTINE RETURN VALUES:                                                  
		RAW_IOPB if the IOPB is a valid raw IOPB.
		NORMAL_IOPB if the IOPB is a valid normal IOPB.
		FALSE if the IOPB is invalid.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL BOOL valid_iopb (iopbptr)
FAST IOPBPTR           iopbptr;
{
    FAST int index;

    for (index = 0; index < ICNT; ++index)
        if (iopbptr == iopb[index])
            return(TRUE);

    for (index = 0; index < UCNT; ++index)
        if (iopbptr == riopb[index])
            return(TRUE);

    return(FALSE);
}

