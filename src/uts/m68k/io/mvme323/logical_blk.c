/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		CONVERT_BLKNO()
*******************************************************************************

DESCRIPTION:
		This function takes a logical block number from a buf structure,
		converts it into appropriate cylinder, head, and sector values,
		and stuffs those values into an IOPB.
                                                                            
REQUIRES:                                                               
		A logical block number.
		A unit number so it can access the unit information table.
		A pointer to the IOPB.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Writes conversion values into the IOPB passed to the function.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL convert_blkno (block, unit, iopbptr)
daddr_t              block;
USHORT               unit;
FAST IOPBPTR         iopbptr;
{
    UINT            cylinder, head, sector;
    FAST UNITINFO  *uinfo = &unit_table[unit];

    /* Compute the cylinder, head and sector values. */

    cylinder = block / (uinfo->sectors * uinfo->heads);
    head = (block - (cylinder * uinfo->sectors * uinfo->heads))/uinfo->sectors;
    sector = block - ((cylinder * uinfo->sectors * uinfo->heads) +
                      (head * uinfo->sectors));

    /* Then stuff the values into the IOPB. */

    iopbptr->byte[CYLL] = (UCHAR)(cylinder);
    iopbptr->byte[CYLH] = (UCHAR)(cylinder >> 8);
    iopbptr->byte[HEAD] = (UCHAR)head;
    iopbptr->byte[SECTOR] = (UCHAR)sector;
}

