
/*IH************************* INCLUDE HEADER **************************
**
**  MODULE NAME:	/u/ce/common/vectors.h  
**
**  DESCRIPTION:    	Definitions of 68010 exception vector numbers.
**
**  NOTES:
**
**  UPDATE LOG:
**  Date	Name		V.R	Comments
**  --------	--------------	---  	------------------------------
**  10/15/86	B. Volquardsen	1.0	Created from common/evt.h
**********************************************************************/


#define MAX_VECTOR	      127	/* Maximum vector number used	*/

#define RESET_VECTOR		0	/* Reset Vector       		*/
#define BUS_ERR_VECTOR  	2	/* Bus Error vector 		*/
#define ADD_ERR_VECTOR		3	/* Address Error vector	    	*/
#define ILLEGAL_VECTOR		4	/* Illegal Instruction vector	*/
#define DIV_ERR_VECTOR		5	/* Divide by Zero vector	*/
#define CHECKPT_VECTOR		6	/* CHK Instruction vector	*/
#define TRAPV_VECTOR		7	/* TRAPV Instruction vector	*/
#define PRIV_VI_VECTOR		8	/* Priviledge Violation vector	*/
#define TRACE_VECTOR		9	/* TRACE vector (single step)	*/
#define L1010_EM_VECTOR		10	/* Line 1010 Emulator vector	*/
#define L1111_EM_VECTOR		11	/* Line 1111 Emulator vector	*/

/*
**  Common System Vector Number Definitions
*/

#define	AUTO_1_VECTOR		25	/* autovector 1 number	    */
#define	AUTO_2_VECTOR		26	/* autovector 2 number	    */
#define	AUTO_3_VECTOR		27	/* autovector 3 number	    */
#define	AUTO_4_VECTOR		28	/* autovector 4 number	    */
#define	AUTO_5_VECTOR		29	/* autovector 5 number	    */
#define	AUTO_6_VECTOR		30	/* autovector 6 number	    */
#define	AUTO_7_VECTOR		31	/* autovector 7 number	    */

#define	TRAP_00_VECTOR		32	/* trap 0 vector number	    */
#define	TRAP_01_VECTOR		33	/* trap 1 vector number	    */
#define	TRAP_02_VECTOR		34	/* trap 2 vector number	    */
#define	TRAP_03_VECTOR		35	/* trap 3 vector number	    */
#define	TRAP_04_VECTOR		36	/* trap 4 vector number	    */
#define	TRAP_05_VECTOR		37	/* trap 5 vector number	    */
#define	TRAP_06_VECTOR		38	/* trap 6 vector number	    */
#define	TRAP_07_VECTOR		39	/* trap 7 vector number	    */
#define	TRAP_08_VECTOR		40	/* trap 8 vector number	    */
#define	TRAP_09_VECTOR		41	/* trap 9 vector number	    */
#define	TRAP_10_VECTOR		42	/* trap 10 vector number    */
#define	TRAP_11_VECTOR		43	/* trap 11 vector number    */
#define	TRAP_12_VECTOR		44	/* trap 12 vector number    */
#define	TRAP_13_VECTOR		45	/* trap 13 vector number    */
#define	TRAP_14_VECTOR		46	/* trap 14 vector number    */
#define	TRAP_15_VECTOR		47	/* trap 15 vector number    */

#define	VCT_VECTOR		64	/* VRTX Config. vector      */

#define	SCC_VECTOR_BASE		80	/* Base of SCC vectors	    */

