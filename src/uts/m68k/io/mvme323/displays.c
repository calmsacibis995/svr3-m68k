/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		DUMP_IOPB()
*******************************************************************************

DESCRIPTION:
		This function displays a range of IOPB bytes.  It simply
		leads into disp_iopb().

REQUIRES:                                                               
		A pointer to the IOPB to be displayed.
		The first and last bytes in the IOPB to be displayed.

SUBROUTINE CALLS:                                                       
		dri_printf() - Prints to the console. (system)

EFFECTS:                                                                

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL dump_iopb (iopbptr, first, last)
IOPBPTR          iopbptr;
FAST USHORT      first, last;
{
     FAST USHORT offset;

if (last >= ISIZE)
    dri_printf("WARNING - Attempting To Display Non-informational IOPB Bytes\n");

dri_printf("\nIOPB @ 0x%x Bytes 0x%x thru 0x%x\n", iopbptr, first, last );

for (offset = first; offset <= last; ++offset)
    dri_printf("BYTE[0x%x]>>> 0x%x\n", offset, iopbptr->byte[offset] );
}

