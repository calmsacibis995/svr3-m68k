/*%W%	%Q%	%G%*/
/*IH******************************************************************

INCLUDE NAME:	ci_types.h

DESCRIPTION:	Contains definitions of all data structures used by the
		ci_driver.c module.
		
UPDATE LOG:
     Name	  Date	  Rev	Comments
-------------	--------  ---   --------------------------------------
Dana Holgate	 3/20/86  1.0	Initial Coding
W. Volquardsen	 5/10/86  1.0	Deleted SH_RES structure that was used
				 by ci_init().  (made ci_init() use 
				 common RES definition in ce.h).
Jim Holbrook	 5/15/86  1.0	Moved REG_ENT structure definition to
				 ce.h for future shared use by ce & ci.
W. Volquardsen	 5/29/86  1.0	Moved "msg" & "cmd" defines to bppci.c
				 (the only place they are used.)
**********************************************************************/

typedef unsigned long ADDR;

#ifdef DEBUG
#define DEBUG1(str,parm) { printf(str,parm); }
#define DEBUG2(str,parm1,parm2) { printf(str,parm1,parm2); }
#else
#define DEBUG1(str,parm) 
#define DEBUG2(str,parm1,parm2) 
#endif

typedef struct io_control
{
	LONG	wait;		/* wait of nowait reads */
	LONG	sig_type;	/* signal type for signalling */
} IO_CTL;


typedef struct max_epb
{
	EPB	hdr;	/* header portion */
	BYTE	user_data[MAX_EPB_SIZE - sizeof(EPB)];
} MAX_EPB;
