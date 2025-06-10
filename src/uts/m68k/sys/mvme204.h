/*	@(#)	mvme204.h	1.0	*/
/*
 * See the MVME204-1/-2 DUAL PORTED DYNAMIC MEMORY VMEmodule USER'S
 * MANUAL, First Edition (MVME204/D2, September, 1985), pages 3-3
 * through 3-5 for more information on the following CSR values.
 *
 */
#define M204_PE		0x80	/* Parity Error			*/
#define M204_RP		0x40	/* VMX32bus Private		*/
#define M204_PR1	0x20	/* Private RAM			*/
#define M204_PR0	0x10	/* Private RAM			*/
#define M204_PBEN	0x08	/* Private Bus Error Enable	*/
#define M204_MCACHE	0x04	/* Memory Cacheable		*/
#define M204_WWP	0x02	/* Write Wrong Parity		*/
#define M204_EPER	0x01	/* Enable Parity Error Report	*/

/* minimum number of repeated uncorrected errors
 * before display of message on system console device
 */
#define M204_MINCNT 20
