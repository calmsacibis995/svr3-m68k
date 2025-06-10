/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		READ()
*******************************************************************************

DESCRIPTION:
		This character interface routine is called to transfer data
		directly from the device to user memory.  A buffer is
		sent to the system call physio(), which in turn locks in 
		the user data and calls strategy().

REQUIRES:                                                               
		A valid device number.
                A buf structure.

SUBROUTINE CALLS:                                                       
		physio() - Sets up the buf structure sent to it to look 
		    like a system buf structure, locks in the user data area,
		    and then calls strategy(). (system)

EFFECTS:                                                                
		Temporarily utilizes the global read_buf buf structure. 

ROUTINE RETURN VALUES:                                                  
		ENXIO - If device is non-existent.
		Return value from physio() is returned by this routine.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323read (dev)
FAST int     dev;
{
	int	m323breakup();		
	FAST PARTINFO   *pinfo;

	pinfo = (PARTINFO *)&(part_table[(DRIVE(dev) * PCNT) + PARTITION(dev)]);

	if(physck(pinfo->length,B_READ))
		physio(m323breakup, 0, dev, B_READ);
}

