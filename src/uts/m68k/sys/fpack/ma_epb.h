/* @(#)ma_epb.h	1.2 (Special for FPACK) */
/*IH*********************** INCLUDE HEADER ****************************
**
**  INCLUDE NAME:	ma_epb.h
**
**  DESCRIPTION:	This file will hold all declarations for the MAC
**			event parameter blocks (MA_EPB).
**
**  NOTES:		
**
**  UPDATE LOG:
**  Date	Name		V/R	Comments
**  --------	-----------   	---	-----------------------------
**  02/18/86	R. Robinson	1.0	Added MAC epb MA_EPB and related
**					parameters
**
**********************************************************************/


/*
**	C O N S T A N T S
*/

#define MACSZ		 6      /* # bytes in MAC */


/*
**  This is the EPB for all events being posted to the MAC sublayer.
*/

typedef	struct	ma_epb
{
	EPB		hdr;		     /* EPB header structure */
	UBYTE		rem_addr[MACSZ];     /* destination address  */
	UBYTE		loc_addr[MACSZ];     /* source address       */
	USHORT		status;		     /* Tx/Rx status         */
	DEFAULT		svc_class;	     /* service class 	     */
} MA_EPB;

/*
**  MAC sublayer types have a base value of 0x200
*/

#define	MA_BASE			0x200	/* MA_EPB: base number	     */
#define MA_DATA_REQUEST		0x201	/* MA_EPB: MA_DATA.request   */
#define MA_DATA_INDICATION	0x202	/* MA_EPB: MA_DATA.indication */
#define MA_DATA_CONFIRM		0x203	/* MA_EPB: MA_DATA.confirm   */
#define	MA_XNS_NET		0x27F	/* MA_EPB: XNS Request	*/
