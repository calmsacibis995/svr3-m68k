/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		ATTACH()
*******************************************************************************

DESCRIPTION:
		This function attempts to bring a Unit on line by issuing five
		IOPBs:
		    1) Drive Reset
		    2) Set Drive Parameters
		    3) Read Drive Parameters
		    4) Set Controller Parameters
		    5) Read Controller Parameters
		    6) Controller Reset
		The driver's internal unit and controller information tables
		will be updated to reflect the results.  The operating system 
		will be informed of the unit being on line or not.
                                                                            
REQUIRES:                                                               
		The unit and controller information tables.

SUBROUTINE CALLS:                                                       
		execute() - Executes an IOPB in a polled fashion.
		dri_printf() - Prints message on console device. (system)

EFFECTS:                                                                
		Resets the drive.
		Sets drive and controller parameters.
		Initializes unit and controller information tables.

ROUTINE RETURN VALUES:                                                  
		TRUE - Attach was successfull.
		FALSE - Attach failed.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL attach (unit)
USHORT        unit;
{
    FAST CTLRINFO    *cinfo;
    FAST UNITINFO    *uinfo;
    FAST PARTINFO    *pinfo;
    FAST IOPBPTR     iopbptr;
    FAST USHORT      part, fail_flag = 0;

cinfo = &ctlr_table;        /* Point at the information tables. */
uinfo = &unit_table[unit];
pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + FIRST_PARTITION]);  
    
iopbptr = riopb[unit];    /* Get the raw IOPB associated with this unit. */
    
/* Send a Drive Reset IOPB, a Set Controller Parameters IOPB, Set Drive */
/* Parameters IOPB, and a Read Drive Paramaters IOPB. If they ALL       */
/* return successfully, then store the values returned in the unit      */
/* information table. If ANY of them fail, then mark the unit offline   */
/* and return FALSE.                                                    */
    
if ( execute( POLLED, XYDRVRESET, unit ) IS_NOT SUCCESSFULL )
    fail_flag = XYDRVRESET;
else if ( execute( POLLED, XYSETDRVPAR, unit ) IS_NOT SUCCESSFULL )
    fail_flag = XYSETDRVPAR;
else if ( execute( POLLED, XYRDDRVPAR, unit ) IS_NOT SUCCESSFULL )
    fail_flag = XYRDDRVPAR;
else
    {
    uinfo->heads = iopbptr->byte[HEAD] + 1;
    uinfo->cylinders = ((iopbptr->byte[CYLH] << 8) |
    iopbptr->byte[CYLL]) + 1;
    uinfo->sectors = iopbptr->byte[SECTOR] + 1;

    if ( iopbptr->byte[ACT_SECTORS] > uinfo->sectors )
        uinfo->trk_spares = iopbptr->byte[ACT_SECTORS] - uinfo->sectors;
    else
        uinfo->trk_spares = 0;

    uinfo->drive_parms = iopbptr->byte[DRV_PARM] & INT_LEVEL_MASK;

    if ( unit == 0 ) /* If this is the 1st Unit, execute Controller Cmds */
        {
        if ( execute( POLLED, XYSETCTLRPAR, unit ) IS_NOT SUCCESSFULL )
            fail_flag = XYSETCTLRPAR;
        else if ( execute( POLLED, XYRDCTLRPAR, unit ) IS_NOT SUCCESSFULL )
            fail_flag = XYRDCTLRPAR;
        else
            {
            cinfo->param1 = iopbptr->byte[CTLRPAR1];
            cinfo->param2 = iopbptr->byte[CTLRPAR2];
            cinfo->param3 = iopbptr->byte[CTLRPAR3];
            cinfo->throttle = iopbptr->byte[THROTTLE];
            }

        }
    }
    
write_reg( CSR, RESET );     /* Controller Reset */

if ( fail_flag IS_NOT 0 )    /* An Error Occured */
    {
#ifdef DEBUG
    dri_printf("mvme323 drive %d: Failed ", unit);
    switch( fail_flag )
        {
        case XYDRVRESET:     dri_printf("Drive Reset\n");                break;
        case XYSETDRVPAR:    dri_printf("Set Drive Parameters\n");       break;
        case XYSETCTLRPAR:   dri_printf("Set Controller Parameters\n");  break;
        case XYRDDRVPAR:     dri_printf("Read Drive Parameters\n");      break;
        case XYRDCTLRPAR:    dri_printf("Read Controller Parameters\n"); break;
        default:             dri_printf("For Some Unknown Reason\n");    break;
        }
#endif

    for ( part = 0; part < PCNT; ++part, pinfo++ )  
        pinfo->p_status = ABSENT;

    return(FALSE);
    }
else
    {

#ifdef DEBUG
    /* Send the ON LINE display to the console device. */
    dri_printf("mvme323 drive %d:  cyl %d  hd %d  sec %d  \n", 
    unit, uinfo->cylinders, uinfo->heads, uinfo->sectors);
#endif

    /* Mark all of the unit's partitions PRESENT. */

    for (part = 0; part < PCNT; ++part, pinfo++)  
        pinfo->p_status = PRESENT;
    
    return(TRUE);
    }
}


LOCAL setformparm(ctlr)
USHORT  ctlr;	/* controller */
{
	register FORMATINFO *finfo;
	register IOPBPTR iopbptr;
	USHORT unit;

	unit = 0;	
	iopbptr = riopb[(ctlr * M323DRV) + unit];    /* Get the raw IOPB */
	finfo = &format_parm;
        iopbptr->byte[INTERLEAVE] |= finfo->interleave;
        iopbptr->byte[FLD_1] = finfo->field_1;
        iopbptr->byte[FLD_2] = finfo->field_2;
        iopbptr->byte[FLD_3] = finfo->field_3;
        iopbptr->byte[FLD_4] = finfo->field_4;
        iopbptr->byte[FLD_5H] = finfo->field_5H;
        iopbptr->byte[FLD_5L] = finfo->field_5L;
        iopbptr->byte[FLD_5AH] = finfo->field_5AH;
        iopbptr->byte[FLD_5AL] = finfo->field_5AL;
        iopbptr->byte[FLD_6] = finfo->field_6;
        iopbptr->byte[FLD_7] = finfo->field_7;
        iopbptr->byte[FLD_12] = finfo->field_12;

        if ( execute( POLLED, XYSETFORMPAR, unit ) IS_NOT SUCCESSFULL )
		dri_printf("M323: Failed to set format parameters.\n", unit);
}



