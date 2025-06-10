/******************************************************************************
FILE = %W%	%G%	Inc.
*******************************************************************************
TITLE:		OPEN()
*******************************************************************************

DESCRIPTION:
		This function is called when the device is mounted, or when
		a device special file for this device is opened explicitly.
		If the device is there and not busy, the device is marked
		OPEN.
                                                                            
REQUIRES:                                                               
		A device number from the operating system.
		The appropriate unit information table.

SUBROUTINE CALLS:                                                       
		None.

*******************************************************************************
EDIT HISTORY:
  1.0     08/10/86 TAW   Port From 751 4.2BSD Driver.
*******************************************************************************
COMMENTS:
*******************************************************************************/

m323open (devnum, flag, otyp)
dev_t   devnum;
int     flag;
int	otyp;
{
    USHORT      ctl, unit, part;
    IOPBPTR     iopbptr;
    UNITINFO   *uinfo;
    PARTINFO   *pinfo;
    BUFPTR      bufptr;
    int		*otbl;		/* address of open table */
    register struct iobuf *dp;

    ctl = CTL(devnum);
    unit = DRIVE(devnum);     /* Set up the local variables and pointers. */
    part = PARTITION(devnum);
    uinfo = &unit_table[unit];
    pinfo = (PARTINFO *)&(part_table[(unit * PCNT) + part]);
    otbl = &m323otbl[((ctl * UCNT) + unit) * OTYPCNT];

    /* If the unit number is too big, or if the device isn't there, then  */
    /* return ENXIO.  */
    if ((unit >= UCNT) || (pinfo->p_status IS ABSENT))
    {
        u.u_error = ENXIO;
	return;
    }

   /* check for open in progress */
	dp = &m323utab[POS(devnum)];

	while( dp->b_flags&B_COPEN ) {	/* open in progress? */
		dp->b_flags |= B_WANTED;
		sleep(dp,PRIBIO);
	}

#ifdef AUTOCONFIG

   /* Replace some default drive parameters with config info from disk
      if it is valid and it hasnt been done previously. 
   */
	if((autoconf) && (dp->b_flags & B_CONF))  {
		dp->b_flags |= B_COPEN;
		if( m323cf(devnum) ) {	 /* replace drive parameters */
			u.u_error = EIO;     /* cant read block 0 */
			dp->b_flags &= ~B_COPEN; /* abort open */ 
		}
		else
			dp->b_flags &= ~(B_CONF|B_COPEN); /* config & OPEN done */
		if(dp->b_flags & B_WANTED)
			wakeup(dp);
	}
#endif

   /* keep track of open type also */
   if(otyp == OTYP_LYR)
	++otbl[OTYP_LYR];
   else if( otyp < OTYPCNT)
	otbl[otyp] |= ( 1 << part);

   u.u_error = 0;
}


#ifdef AUTOCONFIG
/* Read configuration information from the disk. Update critical drive and
   format parameters based on the information. */
m323cf(dev)
dev_t   dev;
{
	register UNITINFO  *uinfo;
	register struct buf *bp;
	struct volumeid *vp;
	struct dkconfig *cfp;
	USHORT unit;
	IOPBPTR	iopbptr;

	unit = DRIVE(dev);

	/* get a buffer	for the	config data */
	bp = geteblk ();
	bp->b_flags  |=  B_ABS | B_READ;
	bp->b_error = 0;
	bp->b_proc = u.u_procp;
	bp->b_dev = dev;
	bp->b_blkno = 0;	/* reading absolute block zero */

	/* setup ptrs assuming vid is sector 0,	config area is sector 1	*/
	vp = (struct volumeid *)(bp->b_un.b_addr);
	cfp = (struct dkconfig *)((int)bp->b_un.b_addr + 256);

	/* send	to strategy */
	m323strategy (bp);
	iowait (bp);

	bp->b_flags  &= ~B_ABS;
	/* error check,	signature string */
	if (bp->b_flags & B_ERROR ) {
		brelse(bp);
		return(-1);
	}
	if (sigcheck(vp->vid_mac)) {

		/* Replace selected default drive parameters with info from 
	  	   configuration area on disk.
		*/
		uinfo = &unit_table[POS(dev)];
		uinfo->heads = cfp->ios_hds;
		uinfo->cylinders = cfp->ios_trk;
		uinfo->sectors = cfp->ios_spt;
		uinfo->sect_size = cfp->ios_psm;
		brelse(bp);

		/* ADD CODE HERE AT SOME FUTURE DATE TO ASSURE EXLCUSIVE USE OF
		   THE RAW IOPB FOR THE SPECIFIED DRIVE. */
		iopbptr = riopb[unit];		/* get raw IOPB */
		init_iopb(iopbptr, unit);	/* initialize the raw IOPB */
       		if (execute( BY_INTERRUPT, XYSETDRVPAR, dev ) IS_A FAILURE)
		{
			return(-1);
		}
	}
	brelse (bp);
	return(0);	/* no error */
}

LOCAL int sigcheck(str)  /* validate signature string in VID */
register char	*str;
{
	static char *valid[] = { "MOTOROLA", "EXORMACS", NULL };
	register char	**x, *y, *z;

	for (x = valid; *x != NULL; ++x) {
		for (y = str, z	 = *x; *z != '\0'; ++z, ++y)
			if (*y != *z)
				break;
		if (*z == '\0')
			return 1;
	}
	return 0;
}
#endif
