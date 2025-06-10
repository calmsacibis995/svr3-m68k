/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		IOCTL()
*******************************************************************************

DESCRIPTION:
		This function is the Special Interface Function for this
		device, performing routines such as formatting, en/disabling
		switches, setting parameters, displaying internal information,
		resetting the controller and drives, and reading or writing
		headers or data.s.
                                                                            
REQUIRES:                                                               
		A device number.
		An encoded command between the caller and this routine.
		An ioctl table containing any additional necessary data or
		pointers.

SUBROUTINE CALLS:                                                       
                rmalloc() - Allocates system memory. (system)
		get_table() - Gets requested data and copies it into the
		    user's ioctl table.
                read_parms() - Reads parameters into user table.
		set_parms() - Sets parameters sent by the user. 
 		format() - Formats one track.
 		header_data() - Reads/writes track header/data.
 		execute() - Executes an IOPB.
		dri_printf() - Prints to console. (system)

EFFECTS:                                                                
		Too numerous to mention here.

ROUTINE RETURN VALUES:                                                  

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323ioctl(dev, cmd, arg, mode)
int     dev;
int     cmd;
char   *arg;
int     mode;
{
    IOCTL_TABLE   *ioctl_table, local_table;
    short         status;
    int           unit, header_data();

    COPY_IN( arg, local_table )

    ioctl_table = &local_table;

    switch (cmd)
    {
        case XYWHATAMI:
        case XYREVNO:
        case XYRDCTLRPAR:
        case XYRDDRVPAR:
 	case XYRDFORMPAR:
 	case XYDRVCONFIG:
 	case XYOPTIONAL:

            if ( (status = read_parms(cmd, ioctl_table, dev) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYCTLRINFO:
        case XYUNITINFO:
        case XYPARTINFO:

            if ( (status = get_table(cmd, ioctl_table, dev) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYSETCTLRPAR:
        case XYSETDRVPAR: 
        case XYSETPARTS: 
        case XYSETFORMPAR:

            if ( (status = set_parms(cmd, ioctl_table, dev) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYFORMAT:       

            if ( (status = format( ioctl_table, dev ) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYREAD:
        case XYWRITE:        
        case XYRDTRKHDR:        
        case XYWRTRKHDR:        
        case XYRDDEFECTS:

            if ( (status = header_data( cmd, ioctl_table, dev) ) NOT= IOCTL_OK )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYDRVRESET:

            unit = DRIVE(dev);
            init_iopb(riopb[unit], unit);

            if ( execute(BY_INTERRUPT, cmd, dev) IS_A FAILURE )
                u.u_error = EIO;
            else
                u.u_error = IOCTL_OK;
            break;

        case XYRESETCTLR: 
            
            write_reg(CSR, RESET); 
            u.u_error = IOCTL_OK;
            break;

        default: 

            u.u_error = EINVAL;

    } /* End of switch. */

COPY_OUT( arg, ioctl_table );

if ( u.u_error IS_NOT IOCTL_OK )
    return( IOCTL_FAILED );
else
    return( IOCTL_OK );
}

