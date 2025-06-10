/*IH***************************** INCLUDE HEADER *******************************
**
**  MODULE NAME:	scc.h
**
**  DESCRIPTION:    	Definitions for the SCC (Zilog 8530) serial
**			communications controller chip.
**
**  NOTES:
**
**  CHANGES:     DESCRIPTION                       DATE     ENGINEER
**  ___________________________________________________________________
**  Original coding.				03/27/86	jeh
**
*******************************************************************************/

/* SCC shows us one CRC byte after the received frame is over */
#define GC_CRC_SEEN 1

#define NUM_WRS 16 
#define NUM_RRS 16

/* SCC read register definitions */

#define RR0 0x00
#define RR1 0x01
#define RR2 0x02 
#define RR3 0x03
#define RR10 0x0a
#define RR12 0x0c
#define RR13 0x0d
#define RR15 0x0f

/* SCC write register definitions */

#define WR0 0x00
#define WR1 0x01
#define WR2 0x02
#define WR3 0x03
#define WR4 0x04
#define WR5 0x05
#define WR6 0x06
#define WR7 0x07
#define WR8 0x08
#define WR9 0x09
#define WR10 0x0a
#define WR11 0x0b
#define WR12 0x0c
#define WR13 0x0d
#define WR14 0x0e
#define WR15 0x0f

/* command definitions for WR0 */

#define SC_NOP  	0x00		/* nop */
#define SC_RSEXT	0x10		/* reset external status interrupt */
#define SC_SENDABORT	0x18		/* send SDLC abort */
#define SC_ENRXINT	0x20		/* enable interrupt on next receive */
#define SC_RSTXPENDING	0x28	 	/* reset tx interrupt pending */
#define SC_ERRORESET	0x30		/* error reset command	*/
#define	SC_RSHIGHIUS	0x38		/* reset interrupt under service */
#define SC_RSRXCRC	0x40		/* reset receive crc	*/
#define SC_RSTXCRC	0x80		/* reset transmit CRC	*/
#define SC_RSTXUNDERRUN	0xc0		/* reset transmit underrun/EOM latch */

/* command definitions for WR1 */

#define SC_EXTIE	0x01		/* enable external status interrupts */
#define SC_TXIE		0x02		/* tx interrupt enable */
#define SC_RXIFIRST	0x08		/* rec. interrupts on 1st char. */
#define SC_RXIALL	0x10		/* enable interrupts on rec. char. or
					   special condition */
#define SC_RXISP	0x18		/* enable interrupts on special
					   condition only */
#define SC_DMART	0x20		/* DMA/wait direction, rx */
#define SC_DMARE	0x40		/* DMA function select */
#define SC_DMAENBL	0x80		/* enable wait/DMA function */

/* definitions for WR3 */

#define SC_RXENAB	0x01		/* enable receiver */
#define	SC_SYNINHIBIT	0x02
#define	SC_ADDRESSEARCH	0x04
#define SC_RXCRCENABLE	0x08		/* enable receive CRC */
#define	SC_HUNT		0x10
#define	SC_AUTOENABLES	0x20
#define	SC_RX5BITS	0x00
#define	SC_RX7BITS	0x40
#define	SC_RX6BITS	0x80
#define SC_RX8BITS	0xc0		/* 8-bit character select */

/* definitions for WR4 */


#define	SC_PARENABLE	0x01
#define	SC_EVENPAR	0x02

#define	SC_SYNENABLE	0x00
#define	SC_1STOPBIT	0x04
#define	SC_112STOPBIT	0x08
#define	SC_2STOPBITS	0x0c

#define	SC_8BITSYN	0x00
#define	SC_16BITSYN	0x10
#define	SC_SDLCMODE	0x20		/* sdlc mode operation */
#define	SC_EXTSYN	0x30

#define	SC_X1CLOCK	0x00
#define	SC_X16CLOCK	0x40
#define	SC_X32CLOCK	0x80
#define	SC_X64CLOCK	0xc0

/* definitions for WR5 */

#define SC_TXCRCENABLE	0x01		/* transmit CRC enable */
#define SC_RTS		0x02		/* RTS output control */
#define SC_CRC16	0x04		/* enable CRC-16 polynomial */
#define SC_CRCSDLC	0x00		/* enable SDLC CRC polynomial */
#define SC_TXENABLE	0x08		/* transmit enable */
#define	SC_SENDBREAK	0x10

#define	SC_TX5BITS	0x00
#define	SC_TX7BITS	0x20
#define	SC_TX6BITS	0x40
#define SC_TX8BITS	0x60		/* 8-bit trnsmit characters */
#define SC_DTR		0x80		/* DTR control bit */


/* definitions for WR9 */

#define SC_VIS		0x01		/* vector includes status */
#define SC_NV		0x02		/* no interrupt vector */
#define SC_VECTORS	0x00		/* SCC will present vector */ 
#define SC_DLC		0x04		/* disable lower chain */
#define SC_MIE		0x08		/* channel master interrupt enable */
#define SC_STHIGH	0x10		/* modify bits 4, 5, and 6 of vector */
#define SC_STLOW	0x00		/* modify bits 1, 2, and 3 of vector */
#define SC_BRESET	0x40		/* channel B reset command */
#define SC_ARESET	0x80		/* channel A reset command */

/* definitions for WR10 */

#define	SC_LOOPMODE	0x02			/* SDLC loop mode */
#define	SC_AFONUNDERRUN	0x04		/* abort/flag on underrun (1/0) */
#define SC_IDLEMARK	0x08		/* mark line on idle */
#define SC_IDLEFLAG	0x00		/* send flags on idle */
#define	SC_MFIDLE	0x08			/* mark/flag on idle */
#define	SC_GAONPOLL	0x10			/* go ahead on poll */

#define SC_NRZI		0x20		/* select NRZI encoding */
#define SC_NRZ		0x00		/* select NRZ encoding */
#define SC_CRCPRESET1	0x80		/* preset CRC to ones */
#define SC_UNDABRTFLG	0x04		/* send abort on underrun */

#define	SC_FM1		0x40		/* fm1 mode */
#define	SC_FM0		0x60		/* fm0 mode */

/* WR11: Clock mode control */

#define	SC_TOXTAL	0x00			/* crystal output on TRxC pin */
#define	SC_TOCLOCK	0x01			/* output tx clock on TRxC pin */
#define	SC_TOBRGO	0x02			/* output baud rate generator on TRxC pin */
#define	SC_TODPLL	0x03			/* output DPLL on TRxC pin */

#define	SC_TRXC01	0x04			/* use TRxC pin for output */

#define SC_TCTRxC	0x08		/* use TxC pin as Tx clock source */
#define SC_RCRxC	0x00		/* use RxC pin as Rx clock source */
#define	SC_TCRTxC	0x00			/* tx clock on RTxC pin */
#define	SC_TCBRGO	0x10			/* tx baud rate generator on TRxC pin */
#define	SC_TCDPLL	0x18			/* tx DPLL input on TRxC pin */

#define	SC_RCRTxC	0x00			/* rx clock on RTxC pin */
#define	SC_RCTRxC	0x20			/* rx clock on TRxC pin */
#define	SC_RCBRGO	0x40			/* rx baud rate generator on TRxC pin */
#define	SC_RCDPLL	0x60			/* rx clock DPLL input on TRxC pin */

#define	SC_RTXTAL	0x80			/* crystal input on RTxC pin */


/* WR12: Lower byte of baud rate generator time constant */

/* WR13: Upper byte of baud rate generator time constant */

/* WR14: Miscellaneous control bits */

#define	SC_BRGENABLE	0x01		/* baud rate generator enable */
#define	SC_BRGSOURCE	0x02		/* select source as baud rate generator */
#define	SC_DTRRQ	0x04			/* dtr request */
#define	SC_ECHO	0x08				/* echo received characters */
#define	SC_LOCALLOOPBACK	0x10	/* select local loopback */

#define	SC_SEARCH	0x20			/* enter search mode */
#define	SC_RSMISSINGCLOCK	0x40	/* reset missing clock */
#define	SC_PLLDISABLE	0x60		/* PLL disable */
#define	SC_SBRGSOURCE	0x80
#define	SC_SRTxC	0xa0
#define	SC_SFM	0xc0
#define	SC_SNRZI	0xe0			/* another NRZI mode set? */


/* WR15: External/Status interrupt control */

#define SC_ZCIE		0x02		/* zero count interrupt enable */
#define SC_DCDIE	0x08		/* DCD interrupt enable */
#define SC_SHIE		0x10		/* sync/hunt interrupt enable */
#define SC_TXUIE	0x40		/* tx underrun/EOM interrupt enable */
#define SC_CTSIE	0x20		/* CTS interrupt enable */
#define SC_BRKIE	0x80		/* break/abort interrupt enable */

/* definitions for RR0 */

#define	SC_RX_AVAIL	0x01		/* receive character avaliable */
#define SC_INT_PENDING 	0x02		/* interrupt pending flag */
#define SC_TXBUFEMPTY 	0x04		/* transmit buffer empty */
#define SC_DCD		0x08		/* DCD status bit */
#define SC_SYNCHUNT	0x10		/* sync/hunt status */
#define SC_CTS		0x20		/* CTS status bit */
#define SC_TXUNDERRUN	0x40		/* transmit underrun indication */
#define SC_BREAKABORT	0x80		/* frame abort indication */

#define SC_BREAKABORT	0x80

/* definitions for RR1 */

#define	SC_ALLSENT	0x01
#define	SC_RESIDUE_MASK	0x0e		/* mask for residue bits */
#define SC_08_RESIDUE	0x06		/* zero previous, 8 bits last */
#define SC_PARITYERROR	0x10
#define SC_RXOVERRUN	0x20		/* receiver overrun */
#define SC_CRCFRAMERR	0x40		/* CRC error */
#define SC_ENDOFRAME	0x80

