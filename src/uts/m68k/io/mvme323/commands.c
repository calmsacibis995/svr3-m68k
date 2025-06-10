/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		COPY()
*******************************************************************************

DESCRIPTION:
		This support routine simply copies data from one structure 
    		to another.

REQUIRES:                                                               
		Two character pointers and a byte count.

SUBROUTINE CALLS:                                                       
		None.

EFFECTS:                                                                
		Destination structure gets a copy of the full structure.

ROUTINE RETURN VALUES:                                                  
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL copy(source, destination, byte_count)
char      *source;
char      *destination;
FAST ULONG byte_count;
{
     FAST ULONG index;

for (index = 0; index < byte_count; index++)
    destination[index] = source[index];
}

/******************************************************************************
TITLE:		FORMAT()
*******************************************************************************

DESCRIPTION:
		This function is responsible for formatting one track at a 
		time.  It copies a user format table into the local
		format table.  Then execute() is called to fill in
		the IOPB with the appropriate data, most importantly the
		cylinder and head values sent by the user.  If the operation
 		fails, the IOPB completion code and the offending sector
		is copied back into the user format table and returned.

REQUIRES:                                                               
		The user format table, including the cylinder and head values.
		The device number.

SUBROUTINE CALLS:                                                       
                init_iopb() - Initializes an IOPB.
		copyin() - Copies data from user data space in to kernel 
		    data space. (system)
		copyout() - Copies data from kernel data space out to user 
		    data space. (system)
		execute() - Executes an IOPB. 

EFFECTS:                                                                
		Formats one track.

ROUTINE RETURN VALUES:                                                  
		IOCTL_OK (0) if successfull, or
		IOCTL_FAILED (-1) if unsuccessfull.
       		If the format was unsuccessfull, the IOPB completion code and
		the failing sector is copied into the user format table.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL format(ioctl_table, devnum)
IOCTL_TABLE *ioctl_table;
int          devnum;
{
    char            status, *user_data = (char *)ioctl_table->ptr_1;
    BYTE            flag = NO;
    USHORT          unit;
    IOPBPTR         iopbptr;
    FORMAT_TABLE    format_table;

    unit = DRIVE(devnum);          /* Extract the unit number and then */
    iopbptr = riopb[unit];         /* get the associated raw IOPB.     */
    init_iopb(iopbptr, unit);      /* Initialize the associated raw IOPB. */

    status = copyin( user_data,                 /* Copy in the format table. */
                     (char *)&format_table,
                     sizeof(FORMAT_TABLE) );

    if ( status == -1 )
        return(IOCTL_FAILED);

    /* Fill in the applicable IOPB bytes. */

    iopbptr->byte[COUNTL] = ONE_TRACK;
    iopbptr->byte[CYLH] = (UCHAR)(format_table.cylinder >> 8);
    iopbptr->byte[CYLL] = (UCHAR)format_table.cylinder;
    iopbptr->byte[HEAD] = (UCHAR)format_table.head;

    /* Perform the actual FORMAT command. */

    if (execute(BY_INTERRUPT, XYFORMAT, devnum) IS_A FAILURE)
        {
        flag = YES;
        format_table.f_status = iopbptr->byte[COMPCODE];
        format_table.sector = iopbptr->byte[SECTOR];
        }

    status = copyout((char *)&format_table,
                              user_data,
                              sizeof(FORMAT_TABLE));

    if ( (status == -1) || (flag == YES) )
        return( IOCTL_FAILED );
    else
        return( IOCTL_OK );
}

/******************************************************************************
TITLE:		GET_TABLE()
*******************************************************************************

DESCRIPTION:
		This function is responsible for copying the contents of the
		driver's controller, unit, or partition information table
		out to a data area sent by the user.
                                                                            
REQUIRES:                                                               
		The driver's controller, unit or partition information table.
		A pointer to the user's area to store a copy of the table.
		A device number.

SUBROUTINE CALLS:                                                       
		copyout() - Copies from kernel space out to user space.

EFFECTS:                                                                
		Fills user's data area with a copy of the the appropriate 
		information table.

ROUTINE RETURN VALUES:                                                  
		The return value of copyout(): 
		    0 if successfull, or
		   -1 if unsuccessfull

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL get_table(cmd, ioctl_table, devnum)
int             cmd;
IOCTL_TABLE    *ioctl_table;
int             devnum;
{
    char              status, *user_data = (char *)ioctl_table->ptr_1;
    USHORT            unit;
    UNITINFO         *uinfo;

    unit = DRIVE(devnum);

    switch (cmd)   
        {                    
        case XYCTLRINFO:
                 
            status = copyout((char *)&ctlr_table, user_data, sizeof(CTLRINFO));
            
            if ( status == -1 )
                return( IOCTL_FAILED );

            break;

        case XYUNITINFO:

            status = copyout((char *)&unit_table[unit],
                                      user_data, 
                                      sizeof(UNITINFO));
            
            if ( status == -1 )
                return( IOCTL_FAILED );

            break;

        case XYPARTINFO:
            
            status = copyout((char *)&(part_table[(unit * PCNT) + FIRST_PARTITION]),
                                       user_data,
                                       PCNT * sizeof(PARTINFO));

            if ( status == -1 )
                return( IOCTL_FAILED );

            break;
    }

return( IOCTL_OK );
}

/******************************************************************************
TITLE:		HEADER_DATA()	
*******************************************************************************

DESCRIPTION:
      		This function is responsible for reading/writing track headers
		or reading/writing track data.  A work buffer is allocated for
                the controller to read/write the header/data information. The 
                modified work buffer address, the cylinder, and the head values 
		are stuffed into the IOPB.  If the command is a read/write track
		data command, the sector number and sector count are also put
 		into the IOPB.  If the command is write track header/data 
 		command, the header/data information is copied from the user 
		data area into the work buffer, and the command executed.  If 
		the command is a read track header/data command, the
		command is executed, and the work buffer data is copied into
		the user data area.

REQUIRES:                                                               
		A command, the ioctl table data, and the device number.

SUBROUTINE CALLS:                                                       
                init_iopb() - Initializes an IOPB.
                put_addr() - Puts an address into an IOPB.
                execute() - Executes an IOPB.
		copyin() - Copies data from user data space in to kernel 
		    data space. (system)
		copyout() - Copies data from kernel data space out to user 
		    data space. (system)
                rmalloc() - Allocates system memory. (system)
                rmfree() - De-allocates system memory. (system)

EFFECTS:                                                                
		The write track data commands over-write whatever
		was previously on the disk.

ROUTINE RETURN VALUES:                                                  
 		IOCTL_OK if successfull, or
		IOCTL_FAILED if the command or copyin/copyout fail.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL header_data(cmd, ioctl_table, devnum)
int               cmd;
IOCTL_TABLE      *ioctl_table;
int               devnum;
{
    USHORT        unit;
    IOPBPTR       iopbptr;
    UNITINFO     *uinfo;
    ULONG         address;

   unit = DRIVE(devnum);         /* Extract the unit number.      */
   iopbptr = riopb[unit];        /* Get the associated raw IOPB.  */
   uinfo = &unit_table[unit];    /* Point at the unit info-table. */

   init_iopb(iopbptr, unit);     /* Initialize the raw IOPB. */

    /* Allocate an appropriately-sized buffer. */

    address = (ULONG)work_buffer;     
    put_addr(iopbptr, address, DATA_ADDR);
    iopbptr->byte[DATA_MOD] = STD_SUPER_DATA;

    /* Stuff the data common to all commands into the IOPB. */

    iopbptr->byte[CYLH] = (UCHAR)(ioctl_table->var_1 >> 8);
    iopbptr->byte[CYLL] = (UCHAR)ioctl_table->var_1;
    iopbptr->byte[HEAD] = (UCHAR)ioctl_table->var_2;

    switch (cmd)
        {
        case XYREAD:     
                         iopbptr->byte[SECTOR] = (UCHAR)ioctl_table->var_3;
                         iopbptr->byte[COUNTL] = 1;

                         if ((execute(BY_INTERRUPT, XYREAD, devnum)
                             IS_A FAILURE) ||
                             (copyout(work_buffer,
                                     (char *)ioctl_table->ptr_1,
                                     uinfo->sect_size)
                             CAUSES_A BUS_ERROR))
		         {
			     ioctl_table->status = iopbptr->byte[COMPCODE];
                             return(IOCTL_FAILED);
			 }

                         break;

        case XYWRITE:   
                         iopbptr->byte[SECTOR] = (UCHAR)ioctl_table->var_3;
                         iopbptr->byte[COUNTL] = 1;

                         if ((copyin((char *)ioctl_table->ptr_1,
                                     work_buffer, 
                                     uinfo->sect_size)
                             CAUSES_A BUS_ERROR) ||
                             (execute(BY_INTERRUPT, XYWRITE, devnum)
                             IS_A FAILURE))
		         {
			     ioctl_table->status = iopbptr->byte[COMPCODE];
                             return(IOCTL_FAILED);
		         }

                         break;

        case XYRDDEFECTS: 
        case XYRDTRKHDR: if ((execute(BY_INTERRUPT, cmd, devnum)
                             IS_A FAILURE) ||
                             (copyout((caddr_t)work_buffer,
                                     (caddr_t)ioctl_table->ptr_1,
                                     cmd == XYRDTRKHDR ?
                                     (uinfo->sectors + uinfo->trk_spares) * 4 :
                                      DEFECT_MAP_SIZE )
                             CAUSES_A BUS_ERROR))
                         {
                             return(IOCTL_FAILED);
                         }

                         break;

        case XYWRTRKHDR: if ((copyin((caddr_t)ioctl_table->ptr_1,
                                    (caddr_t)work_buffer,
                                    (uinfo->sectors + uinfo->trk_spares) * 4)
                             CAUSES_A BUS_ERROR) ||
                             (execute(BY_INTERRUPT, XYWRTRKHDR, devnum)
                             IS_A FAILURE))
                         {
                             return(IOCTL_FAILED);
                         }

                         break;

    }/* switch */

return(IOCTL_OK); 
}

/******************************************************************************
TITLE:		READ_PARMS()
*******************************************************************************

DESCRIPTION:
		This function is resposible for reading controller or
		or drive parameters into a table supplied by the user.  
		An appropriate IOPB is sent to the controller to get
		some of the information, and the rest is supplied from
		the driver's internal information tables.  The driver's 
                internal tables will NOT be updated with any data read
		from the controller.
                                                                            
REQUIRES:                                                               
		A pointer to the user's data area.
		A device number.
                The controller information tables.

SUBROUTINE CALLS:                                                       
                init_iopb() - Initializes an IOPB.
                copy() - Copies one data structure to another.
		copyout() - Copies from kernel space to user space. (system)
		execute() - Executes an IOPB.

EFFECTS:                                                                
		In the case of controller or drive parameters, an IOPB 
		reflecting the user's data is sent to the controller.

ROUTINE RETURN VALUES:                                                  
		IOCTL_OK (0) if successfull, or
		IOCTL_FAILED (-1) if unsuccessfull, or
		Return status from copyout() (also 0 or -1).

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL read_parms (cmd, ioctl_table, devnum)
int               cmd;
IOCTL_TABLE      *ioctl_table;
int               devnum;
{
    char          status, *user_data = (char *)ioctl_table->ptr_1;
    USHORT        unit, i, j;
    IOPBPTR       iopbptr;
    CTLRINFO     *cinfo, temp_cinfo;
    UNITINFO     *uinfo, temp_uinfo;
    UCHAR        format_parms[FORMAT_PARMS];

    unit = DRIVE(devnum);          /* Extract the unit number.           */
    iopbptr = riopb[unit];         /* Point at the appropriate raw IOPB. */
    uinfo = &unit_table[unit];     /* Point at the driver's unit and     */
    cinfo = &ctlr_table;           /* controller information tables.     */

    init_iopb(iopbptr, unit);      /* Initialize the associated raw IOPB. */

    /* Send an IOPB requesting either controller or drive parameters.   */
    /* If successfull, copy the data from the IOPB to the user's table. */

    switch (cmd)
        {
        case XYRDCTLRPAR:

            if (execute(BY_INTERRUPT, XYRDCTLRPAR, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            copy((char *)cinfo, (char *)&temp_cinfo, sizeof(CTLRINFO));

            temp_cinfo.param1 = iopbptr->byte[CTLRPAR1];
            temp_cinfo.param2 = iopbptr->byte[CTLRPAR2];
            temp_cinfo.param3 = iopbptr->byte[CTLRPAR3];
            temp_cinfo.throttle = iopbptr->byte[THROTTLE];
              
            status = copyout((char *)&temp_cinfo, user_data, sizeof(CTLRINFO));

            if ( status == -1 )
                return( IOCTL_FAILED );

            break;

        case XYRDDRVPAR:
        case XYDRVCONFIG:

            if (execute(BY_INTERRUPT, cmd, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            copy((char *)uinfo, (char *)&temp_uinfo, sizeof(UNITINFO));        

            temp_uinfo.heads = iopbptr->byte[HEAD] + 1;
            temp_uinfo.cylinders = ((iopbptr->byte[CYLH] << 8) |
	                             iopbptr->byte[CYLL]) + 1;
            temp_uinfo.sectors = iopbptr->byte[SECTOR] + 1;

            if ( cmd == XYDRVCONFIG )
	        temp_uinfo.drive_parms = uinfo->drive_parms;
            else
                temp_uinfo.drive_parms = iopbptr->byte[DRV_PARM] & 
                                                                 INT_LEVEL_MASK;

            status = copyout((char *)&temp_uinfo, user_data, sizeof(UNITINFO));

            if ( status == -1 )
                return( IOCTL_FAILED );

            break;

        case XYOPTIONAL:

            iopbptr->byte[SUBFUNC] = (char)ioctl_table->var_1;
            iopbptr->byte[0x12] = (char)ioctl_table->var_2;
            iopbptr->byte[0x13] = (char)ioctl_table->var_3;

            if (execute(BY_INTERRUPT, XYOPTIONAL, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            ioctl_table->long_var = (long)(iopbptr->byte[0x10] << 8) | 
                                          iopbptr->byte[0x11];

            break;

        case XYWHATAMI:

            if (execute(BY_INTERRUPT, XYRDDRVPAR, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            /* Hi Nibble gets Partition Cnt, Lo Nibble gets Partition Number */
	    ioctl_table->status = (PCNT << 4) | PARTITION(devnum);

            ioctl_table->var_1 = Ctlr_Type;
            ioctl_table->var_2 = DISK;
            ioctl_table->var_3 = iopbptr->byte[ACT_SECTORS];
            ioctl_table->long_var = BSIZE;
	    break;

        case XYREVNO:

            if (execute(BY_INTERRUPT, XYRDCTLRPAR, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            ioctl_table->var_1 = Major_Rev;
            ioctl_table->var_2 = Minor_Rev;

	    ioctl_table->var_3 = iopbptr->byte[SUBREV] |
		       ((UINT)iopbptr->byte[REVISION] << 8);

            ioctl_table->long_var = (iopbptr->byte[PART_HI] << 8) |
                        iopbptr->byte[PART_LO];
            break;

	case XYRDFORMPAR:

            if (execute(BY_INTERRUPT, XYRDFORMPAR, devnum) IS_A FAILURE)
		return(IOCTL_FAILED);

            if ((format_parms[F_PAR_STAT] = iopbptr->byte[COMPCODE]) 
                 IS F_PAR_OK)    
 	        {
                format_parms[0] = iopbptr->byte[INTERLEAVE] & INT_LEVEL_MASK;
                format_parms[1] = iopbptr->byte[FLD_1];
                format_parms[2] = iopbptr->byte[FLD_2];
                format_parms[3] = iopbptr->byte[FLD_3];
                format_parms[4] = iopbptr->byte[FLD_4];
                format_parms[5] = iopbptr->byte[FLD_5H];
                format_parms[6] = iopbptr->byte[FLD_5L];
                format_parms[7] = iopbptr->byte[FLD_5AH];
                format_parms[8] = iopbptr->byte[FLD_5AL];
                format_parms[9] = iopbptr->byte[FLD_6];
                format_parms[10] = iopbptr->byte[FLD_7];
                format_parms[11] = iopbptr->byte[FLD_12];
                }

            status = copyout((char *)&format_parms[0],
                                     user_data,
                                     FORMAT_PARMS);

            if ( status == -1 )
                return( IOCTL_FAILED );

    } /* End of switch. */

return( IOCTL_OK );
}

/******************************************************************************
TITLE:		SET_PARMS()
*******************************************************************************

DESCRIPTION:
		This function is resposible for setting a controller's 
		parameters with values supplied by the user.  It will
		will send an IOPB to the controller if necessary, and update
		either the driver's controller, unit, or unit partition
		table, depending upon the command sent.
                                                                            
REQUIRES:                                                               
		The ioctl() comand.
		A pointer to the user's data area.
		The device number.            

SUBROUTINE CALLS:                                                       
                init_iopb() - Initializes an IOPB.
                copy() - Copies one data structure to another.
		copyin() - Copies from user space to kernel space. (system)
		execute() - Executes an IOPB.

EFFECTS:                                                                
		In the case of controller or drive parameters, an IOPB 
		reflecting the user's data is sent to the controller.
		Partition parameters are just stuffed into the table.

ROUTINE RETURN VALUES:                                                  
		IOCTL_OK (0) if successfull, or
		IOCTL_FAILED (-1) if unsuccessfull, or
		Return status from copyout() (also 0 or -1).

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

LOCAL set_parms (cmd, ioctl_table, devnum)
int              cmd;
IOCTL_TABLE     *ioctl_table;
int              devnum;
{
    char       status, *user_data = (char *)ioctl_table->ptr_1;
    USHORT     unit, i, j;
    CTLRINFO  *cinfo, ctlr_backup;
    UNITINFO  *uinfo, unit_backup;
    PARTINFO  *pinfo, part_backup[PCNT];
    IOPBPTR    iopbptr;
    UCHAR      format_parms[FORMAT_PARMS];

    unit = DRIVE(devnum);       /* Set up the local variables and pointers. */
    uinfo = &unit_table[unit];  
    cinfo = &ctlr_table;                       
    pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + FIRST_PARTITION]);

    iopbptr = riopb[unit];
    init_iopb(iopbptr, unit);   /* Initialize the associated raw IOPB. */
                    
                              /* When setting controller or unit parameters, */
    switch (cmd)              /* stash the current table first.  If either   */
        {                     /* copyin() or  execute() fail, restore it and */
                              /* and return IOCTL_FAILED to ioctl().         */
        case XYSETCTLRPAR:

            copy((char *)cinfo, (char *)&ctlr_backup, sizeof(CTLRINFO));

            status = copyin( user_data, (char *)cinfo, sizeof(CTLRINFO) );

            if ( status == -1 )        /* copyin Failed */
                return( IOCTL_FAILED );

            if ( execute(BY_INTERRUPT, XYSETCTLRPAR, devnum) IS_A FAILURE )
                {
                copy((char *)&ctlr_backup, (char *)cinfo, sizeof(CTLRINFO));
                return(IOCTL_FAILED);
                }

            break;

        case XYSETDRVPAR:    

            copy((char *)uinfo, (char *)&unit_backup, sizeof(UNITINFO));

            status = copyin( user_data, (char *)uinfo, sizeof(UNITINFO) );

            if ( status == -1 )        /* copyin Failed */
                return( IOCTL_FAILED );

            if ( execute( BY_INTERRUPT, XYSETDRVPAR, devnum ) IS_A FAILURE)
                {
                copy((char *)&unit_backup, (char *)uinfo, sizeof(UNITINFO));
                return(IOCTL_FAILED);
                }

            break;

        case XYSETPARTS:

            copy((char *)pinfo, (char *)part_backup,
                  PCNT * sizeof(PARTINFO));

            status = copyin(user_data, (char *)pinfo, PCNT * sizeof(PARTINFO));

            if ( status == -1 )        /* copyin Failed */
                {
                copy((char *)part_backup, (char *)pinfo,
                       PCNT * sizeof(PARTINFO));

                return( IOCTL_FAILED );
                }

            break;

	case XYSETFORMPAR:

           status = copyin(user_data,(char *)format_parms,FORMAT_PARMS);

            if ( status == -1 )        /* copyin Failed */
                return( IOCTL_FAILED );

            iopbptr->byte[INTERLEAVE] |= format_parms[0] & INT_LEVEL_MASK;
            iopbptr->byte[FLD_1] = format_parms[1];
            iopbptr->byte[FLD_2] = format_parms[2];
            iopbptr->byte[FLD_3] = format_parms[3];
            iopbptr->byte[FLD_4] = format_parms[4];
            iopbptr->byte[FLD_5H] = format_parms[5];
            iopbptr->byte[FLD_5L] = format_parms[6];
            iopbptr->byte[FLD_5AH] = format_parms[7];
            iopbptr->byte[FLD_5AL] = format_parms[8];
            iopbptr->byte[FLD_6] = format_parms[9];
            iopbptr->byte[FLD_7] = format_parms[10];
            iopbptr->byte[FLD_12] = format_parms[11];

            execute(BY_INTERRUPT, XYSETFORMPAR, devnum);

            format_parms[F_PAR_STAT] = iopbptr->byte[COMPCODE];

            status = copyout((char *)format_parms,
                                     user_data,
                                     FORMAT_PARMS);

            if ( status == -1 )        /* copyout Failed */
                return( IOCTL_FAILED );

    } /* End of switch. */

return(IOCTL_OK);
}


