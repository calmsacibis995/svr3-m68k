

/*IH************************* INCLUDE HEADER **************************
**
**  MODULE NAME:	/u/ce/common/evt.h  
**
**  DESCRIPTION:    	Definitions of 68010 Exception Vector Table 
**			addresses for all boards..
**
**  NOTES:
**
**  UPDATE LOG:
**  Date	Name		V.R	Comments
**  --------	--------------	---  	------------------------------
**  04/08/86	B. Volquardsen	1.0	Created and enhanced from part
**					 of /u/ce/vme333/ce_vrtx.h.
**********************************************************************/




#define RESET_SSP   	(VBR + 0x00)	/* Reset Stack Pointer      	*/
#define RESET_PC	(VBR + 0x04)	/* Reset Program Counter    	*/
#define BUS_ERROR	(VBR + 0x08)	/* Bus Error vector 	    	*/
#define ADDRESS_ERROR	(VBR + 0x0C)	/* Address Error vector	    	*/
#define ILLEGAL_INST	(VBR + 0x10)	/* Illegal Instruction vector	*/
#define ZERO_DIVIDE	(VBR + 0x14)	/* Divide by Zero vector	*/
#define CHECKPT_INST	(VBR + 0x18)	/* CHK Instruction vector	*/
#define TRAPV_INST	(VBR + 0x1C)	/* TRAPV Instruction vector	*/
#define PRIV_VIOLATION	(VBR + 0x20)	/* Priviledge Violation vector	*/
#define TRACE_INST	(VBR + 0x24)	/* TRACE vector (single step)	*/
#define L1111_EMUL	(VBR + 0x2C)	/* TRACE breakpoint vector	*/

/*
**  Common System Vector Table Definitions
*/

#define	AUTO_VECTOR_1	(VBR + 0x64)	/* autovector 1 address	    */
#define	AUTO_VECTOR_2	(VBR + 0x68)	/* autovector 2 address	    */
#define	AUTO_VECTOR_3	(VBR + 0x6C)	/* autovector 3 address	    */
#define	AUTO_VECTOR_4	(VBR + 0x70)	/* autovector 4 address	    */
#define	AUTO_VECTOR_5	(VBR + 0x74)	/* autovector 5 address	    */
#define	AUTO_VECTOR_6	(VBR + 0x78)	/* autovector 6 address	    */
#define	AUTO_VECTOR_7	(VBR + 0x7C)	/* autovector 7 address	    */

#define	TRAP_VECTOR_0	(VBR + 0x80)	/* trap 0 address	    */
#define	TRAP_VECTOR_1	(VBR + 0x84)	/* trap 1 address	    */
#define	TRAP_VECTOR_2	(VBR + 0x88)	/* trap 2 address	    */
#define	TRAP_VECTOR_3	(VBR + 0x8C)	/* trap 3 address	    */
#define	TRAP_VECTOR_4	(VBR + 0x90)	/* trap 4 address	    */
#define	TRAP_VECTOR_5	(VBR + 0x94)	/* trap 5 address	    */
#define	TRAP_VECTOR_6	(VBR + 0x98)	/* trap 6 address	    */
#define	TRAP_VECTOR_7	(VBR + 0x9C)	/* trap 7 address	    */
#define	TRAP_VECTOR_8	(VBR + 0xA0)	/* trap 8 address	    */
#define	TRAP_VECTOR_9	(VBR + 0xA4)	/* trap 9 address	    */
#define	TRAP_VECTOR_10	(VBR + 0xA8)	/* trap 10 address	    */
#define	TRAP_VECTOR_11	(VBR + 0xAC)	/* trap 11 address	    */
#define	TRAP_VECTOR_12	(VBR + 0xB0)	/* trap 12 address	    */
#define	TRAP_VECTOR_13	(VBR + 0xB4)	/* trap 13 address	    */
#define	TRAP_VECTOR_14	(VBR + 0xB8)	/* trap 14 address	    */
#define	TRAP_VECTOR_15	(VBR + 0xBC)	/* trap 15 address	    */

#define VRTX_TRAP_VEC	TRAP_0_VECTOR	/* VRTX   trap vector	    */
#define	VRTX_VCT_VECTOR	(VBR + 0x100)	/* VCT pointer location     */

#define TRACER_TRAP_VEC TRAP_1_VECTOR	/* TRACER trap vector	    */
#define TRACER_BKPT_VEC	L1111_EMUL   	/* Tracer Breakpoint vector */
#define TRACER_STEP_VEC	TRACE_INST	/* Tracer Single-Step vector*/
