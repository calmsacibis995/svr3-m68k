/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		CLOSE()
*******************************************************************************

DESCRIPTION:
		This function is called when a device is unmounted, or
		if a special file for this device that was opened explicitly
		is closed.

REQUIRES:                                                               
		A device number.

SUBROUTINE CALLS:                                                       
		None.


ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323close (devnum, flag, otyp)
dev_t    devnum;
int      flag;
int	 otyp;
{
    FAST USHORT      ctl, unit, part;
    FAST UNITINFO   *uinfo;
    FAST PARTINFO   *pinfo;
    FAST int 	    *otbl;	/* address of open table */	     			
    ctl =  CTL(devnum);
    unit = DRIVE(devnum);        /* Set up local variables and pointers. */
    part = PARTITION(devnum);
    uinfo = &unit_table[unit];
    pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + part]);
    otbl = &m323otbl[((ctl * UCNT) + unit) * OTYPCNT];


   /* mark device closed */
   if(otyp == OTYP_LYR)
	--otbl[OTYP_LYR];
   else if (otyp < OTYPCNT)
	otbl[otyp] &= ~( 1 << part);
    
}

