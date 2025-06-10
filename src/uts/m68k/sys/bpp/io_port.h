/* io_port.h */
/*
**  Definitions of structures and parameters used by io_port_mgt.c 
**
*/

/*	
**	Return Codes from io_port_mgt functions
*/
#define PORT_OPENED 	0	/* open: port opened OK		*/
#define	PORT_IN_USE	1	/* open: port already in use	*/
#define PORT_NUMBER_BAD 2	/* bad port number argument	*/
#define PORT_CLOSED	0	/* close: port closed OK	*/
#define PORT_NOT_OPEN	3	/* close: port was never opened	*/

/*
**	Port Configuration Definitions
*/
#define	NUM_SCC_PORTS	6	/* Number of SCC ports configured */
#define DEBUGGER_PORT	1	/* MVME333bug console port	*/
#define ASYNC_BUF_SIZE  106	/* size of async I/O buffers	*/

typedef struct io_tbl		/* I/O Function Table	*/
{
	PTF	txfun;		/* transmit interrupt function 	*/
	ULONG	txarg;		/* argument passed to txfun() 	*/
	PTF	rxfun;		/* receive  interrupt function 	*/
	ULONG	rxarg;		/* argument passed to rxfun()	*/
	PTF	spfun;		/* special condition function	*/
	ULONG	sparg;		/* argument passed to spfun()	*/
	PTF	exfun;		/* external status function	*/
	ULONG	exarg;		/* argument passed to exfun()	*/
} IO_TBL;


typedef struct	async_buf	/* Asynchronous I/O Buffer	*/
{
	char	*buf;		/* start of buffer		*/
	USHORT	buf_size;	/* end of buffer		*/
	USHORT	put;		/* where to put next character	*/
	USHORT	get; 		/* where to get next character	*/
	TCB	*task_pended;	/* ptr to task pended on I/O	*/
	USHORT	status;		/* driver status flags		*/
#define AB_TX_ON	0x01	/* Transmit has started	*/

	char	buffer[ASYNC_BUF_SIZE];	/* Async I/O buffer	*/

} ASYNC_BUF;


typedef struct	io_port		/* Logical Port Table Entry 	*/
{
	UBYTE	type;		/* type of port 		*/
#define		T_ASYNC	0x41	/*  - Async   protocol.	*/
#define 	T_BSC	0x42	/*  - Bisync  protocol.	*/
#define		T_RJE	0x52	/*  - BSC RJE protocol.	*/
#define		T_SDLC	0x53	/*  - SDLC    protocol.	*/

	UBYTE	status;		/* status of this port		*/
#define		IN_USE	0x01	/*  - port is in use	*/

	IO_TBL	io;		/* io function table		*/

	ASYNC_BUF *rx_buffer;	/* ptr to async receive  buffer	*/
	ASYNC_BUF *tx_buffer;	/* ptr to async transmit buffer	*/

} IO_PORT;
