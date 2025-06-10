
/*IH***************************** INCLUDE HEADER *******************************
**
**  MODULE NAME:	env.vme333.h
**
**  DESCRIPTION:    	Definitions for addresses and registers of the VME333
**			communications board.
**
**  NOTES:
**
*  CHANGES:     DESCRIPTION                       DATE     ENGINEER
*  ___________________________________________________________________
*  Original coding.				03/13/86	jeh
*  Changed sc_post to post message other than   09/25/86        jeh
*  zero, which is invalid as a message.
*  Modified for shared timers and port manage-  10/25/86	jeh
*  ment, and also for the fact that I can't
*  stand the use of macros any more.
*******************************************************************************/

/* NUMBER OF PORTS */
#define	NUM_PORTS	6

/* SCC port address definitions */

#define	SCC0_Actrl	0xff8c05	/* port 1 A-side control register */
#define SCC0_Adata	0xff8c07	/* port 1 A-side data register    */
#define	SCC0_Bctrl	0xff8c01	/* port 1 B-side control register */
#define SCC0_Bdata	0xff8c03	/* port 1 B-side data register    */
#define	SCC1_Actrl	0xff8c15	/* port 2 A-side control register */
#define SCC1_Adata	0xff8c17	/* port 2 A-side data register    */
#define	SCC1_Bctrl	0xff8c11	/* port 2 B-side control register */
#define SCC1_Bdata	0xff8c13	/* port 2 B-side data register    */
#define	SCC2_Actrl	0xff8c25	/* port 3 A-side control register */
#define SCC2_Adata	0xff8c27	/* port 3 A-side data register    */
#define	SCC2_Bctrl	0xff8c21	/* port 3 B-side control register */
#define SCC2_Bdata	0xff8c23	/* port 3 B-side data register    */


/* SCC register address structure definition */

typedef struct
{
   ubt08	*cmd;		/* pointer to control register */
   ubt08	*data;		/* pointer to data register	*/
} scc_reg_t;

/* EVT numbers used for the VME333 SCC channels */

#define	SCC1_CHA_VECTOR		0x58
#define SCC1_CHB_VECTOR		0x50
#define SCC2_CHA_VECTOR		0x68
#define SCC2_CHB_VECTOR		0x60
#define SCC3_CHA_VECTOR		0x78
#define SCC3_CHB_VECTOR		0x70

/* VME333 Status and Control Registers */

#define TIMER_REG	(ubt08 *)0xff8b01	/* timer count register
					           time = (255-load_val)*12.8 micro-sec */
#define WATCH_DOG	(ubt08 *)0xf00000	/* watchdog timer -- must be accessed
					           every 100ms if board is jumpered
					   timeout.	*/
#define ADDR_EXT	(ubt16 *)0xff8f01	/* address extension register
					         -- address lines A22 to A31	*/
#define LOCAL_STS	(ubt16 *)0xff8e00	/* local status register	*/
#define	DSR0_MASK	0x01		/* DSR channel 0	*/
#define	DSR1_MASK	0x02		/* DSR channel 1	*/
#define	DSR2_MASK	0x04		/* DSR channel 2	*/
#define	DSR3_MASK	0x08		/* DSR channel 3	*/
#define	DSR4_MASK	0x10		/* DSR channel 4	*/
#define	DSR5_MASK	0x20		/* DSR channel 5	*/
#define JMPK4		0x100		/* K4 jumper - 0 = set	*/
#define	VMEIACK		0x200		/* VME bus IRQ - 0 = pending */

#define	LOCAL_CTRL	(ubt08 *)0xff8e01	/* local control register */
#define	DTR1		0x40		/* DTR port 1	*/
#define	DTR3		0x80		/* DTR port 3	*/
#define	DP3		0x20		/* DMA control - 1 = ch3 full duplex */
#define	DP1		0x10		/* DMA control - 1 = ch1 full duplex */
#define	WWP		0x08		/* parity control - 1 = good parity */
#define STS_MASK	0x07		/* VME status bits mask	*/

#define ADDR_MOD	(ubt08 *)0xff8d01	/* VMEbus address modifier register */
#define	VME_CMD		(ubt16 *)0xff8a01	/* VME command byte register */
#define	VME_INT		(ubt08 *)0xff8903	/* VME interrupt register */
#define VME_ISTS	(ubt08 *)0xff8900	/* VME interrupt status word	*/

/* DMA channel address definitions */

#define	DMAC0		0xff8800	/* DMA channel 0 base	*/
#define DMAC1		0xff8840	/* DMA channel 1 base	*/
#define DMAC2		0xff8880	/* DMA channel 2 base	*/
#define DMAC3		0xff88c0	/* DMA channel 3 base	*/

