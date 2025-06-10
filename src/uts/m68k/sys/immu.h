/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/immu.h	10.5"

/*
 * Page Descriptor Entry Definitions
 */

typedef union spde {
/*         short page descriptor entry
 *
 *  +---------------------+---+-----+--+--+--+---+---+--+--+
 *  |      Page Address   |lck|ndref| g|ci| l|mod|ref|wp|dt|
 *  +---------------------+---+-----+--+--+--+---+---+--+--+
 *             22           1   1     1  1  1  1   1   1  2
 */
	struct {
		uint	pg_pfn  : 22,	/* Physical page frame (hardware) */
			pg_lock	:  1,	/* Lock in core      (software)	*/
			pg_ndref:  1,	/* Need a reference  (software)	*/
			pg_g	:  1,	/* Gate page         (hardware)	*/
			pg_ci	:  1,	/* Cache inhibit     (hardware)	*/
			pg_l	:  1,	/* Cache desc. lock  (hardware)	*/
			pg_mod	:  1,	/* Page modified     (hardware)	*/
			pg_ref	:  1,	/* Page referenced   (hardware)	*/
			pg_wp	:  1,	/* Write protect     (hardware) */
			pg_dt	:  2;	/* Descriptor type   (hardware)	*/
	} pgm;

	struct {
		uint	pg_spde; /* short page descriptor (table) entry */
	} pgi;
} spde_t;

typedef union lpde {
/*                    long page descriptor entry	(* not used *)
 *
 * +---+-----+---+---+--+-+-+--+-+---+---+--+--+     +---------+-+-----+----+--+
 * |l/u|limit|ral|wal|sg|s|g|ci|l|mod|ref|wp|dt|     |Page Addr|-|ndref|lock|--|
 * +---+-----+---+---+--+-+-+--+-+---+---+--+--+     +---------+-+-----+----+--+
 *   1    15   3   3  1  1 1 1  1  1   1  1   2          24     1    1    1   5
 */
	struct {
		uint	pg_lu	:  1,	/* Lower / upper     (hardware)	*/
			pg_limit: 15,	/* Table limit       (hardware)	*/
			pg_ral	:  3,	/* Read access level (hardware)	*/
			pg_wal	:  3,	/* Write access level(hardware)	*/
			pg_sg	:  1,	/* Shared globally   (hardware)	*/
			pg_s	:  1,	/* Supervisor page   (hardware)	*/
			pg_g	:  1,	/* Gate page         (hardware)	*/
			pg_ci	:  1,	/* Cache inhibit     (hardware)	*/
			pg_l	:  1,	/* Cache desc. lock  (hardware)	*/
			pg_mod	:  1,	/* Page modified     (hardware)	*/
			pg_ref	:  1,	/* Page referenced   (hardware)	*/
			pg_wp	:  1,	/* Write protect     (hardware) */
			pg_dt	:  2;	/* Descriptor type   (hardware)	*/

		uint	pg_pfn  : 22,	/* Physical page frame (hardware) */
				:  3,	/* Unused/available  (software)	*/
			pg_ndref:  1,	/* Needs reference   (software)	*/
			pg_lock	:  1,	/* Lock in core      (software)	*/
				:  5;	/* Unused/available  (software)	*/
	} pgm;

	struct {
		uint	pg_hlpde; /* hardware long page descriptor entry */
		uint	pg_slpde; /* software long page descriptor entry */
	} pgi;
} lpde_t;

#define pde_t	spde_t			/* all pde's are short for MMB */

/*
 *	Page Table
 */

#define NPGPT		256		/* Nbr of pages per page table (seg). */
typedef union ptbl {
	int page[NPGPT];
} ptbl_t;

/* Page descriptor (table) entry dependent constants */

#define	NBPP		1024		/* Number of bytes per page */
#define	NBPPT		1024		/* Number of bytes per page table */
#define	BPTSHFT		10 		/* LOG2(NBPPT) if exact */
#define NDPP		2		/* Number of disk blocks per page */
#define DPPSHFT		1		/* Shift for disk blocks per page. */

#define PNUMSHFT	10		/* Shift for page number from addr. */
#define PNUMMASK	0xFF		/* Mask for page number in segment. */
#define POFFMASK        0x3FF		/* Mask for offset into page. */
#define PGFNMASK	0x3FFFFF	/* Mask page frame nbr after shift. */

#define	NPTPP		1		/* Nbr of page tables per page.	*/
#define	NPTPPSHFT	0		/* Shift for NPTPP. */

#define	PG_VALID	1		/* bit indep. valid flag */

#define PG_INV		0		/* Invalid descriptor */
#define PG_VPD		1		/* Valid page descriptor */

/* byte addr to virtual page */

#define pnum(X)   ((uint)(X) >> PNUMSHFT) 

/* page offset */

#define poff(X)   ((uint)(X) & POFFMASK)

/* byte address to page within segment */

#define psnum(X)   (((uint)(X) >> PNUMSHFT) & PNUMMASK)

/*	Disk blocks (sectors) and pages.
*/

#define	ptod(PP)	((PP) << DPPSHFT)
#define	dtop(DD)	(((DD) + NDPP - 1) >> DPPSHFT)
#define dtopt(DD)	((DD) >> DPPSHFT)

/*	Disk blocks (sectors) and bytes.
*/

#define	dtob(DD)	((DD) << SCTRSHFT)
#define	btod(BB)	(((BB) + NBPSCTR - 1) >> SCTRSHFT)
#define	btodt(BB)	((BB) >> SCTRSHFT)

/*	Page tables (256 entries == 1024 bytes) to pages.
*/

#define	pttopgs(X)	((X + NPTPP - 1) >> NPTPPSHFT)
#define	pttob(X)	((X) << BPTSHFT)
#define	btopt(X)	(((X) + NBPPT - 1) >> BPTSHFT)

/* Size in clicks of I/O area at top of 32-bit address space */
#define IO_32_SZ 2*1024	/* here for now...should be in param.h */

/* Size in clicks of the VMEbug RAM area at physical/virtual 0 */
#define BUGRAMSZ 64

#define NULLADDR	0

/* #define TEXTRO 	/* Flag to mark kernel text read only */


/*	The following macros are used to set, clear, and test
 *	the value of the bits in a page descriptor (table) entry 
 */

#define BITSET		1
#define BITCLR		0
#define	PG_REF		0x00000008
#define	PG_NDREF	0x00000100

#define pg_setvalid(P)	((P)->pgm.pg_dt = PG_VPD)	/* Set valid bit.    */
#define pg_clrvalid(P)	((P)->pgm.pg_dt = PG_INV)	/* Clear valid bit.  */
#define pg_tstvalid(P)	((P)->pgm.pg_dt == PG_VPD)	/* Test valid bit.   */

#define pg_setlock(P)	((P)->pgm.pg_lock = BITSET)	/* Set lock bit.     */
#define pg_clrlock(P)	((P)->pgm.pg_lock = BITCLR)	/* Clear lock bit.   */
#define pg_tstlock(P)	((P)->pgm.pg_lock)		/* Test lock bit.    */

#define pg_setmod(P)	((P)->pgm.pg_mod = BITSET)	/* Set modify bit.   */
#define pg_clrmod(P)	((P)->pgm.pg_mod = BITCLR)	/* Clear modify bit. */
#define pg_tstmod(P)	((P)->pgm.pg_mod)		/* Test modify bit.  */

#define pg_setcw(P)	((P)->pgm.pg_wp = BITSET)	/* Set write prot (cw)*/
#define pg_clrcw(P)	((P)->pgm.pg_wp = BITCLR)	/* Clr write prot (cw)*/
#define pg_tstcw(P)	((P)->pgm.pg_wp)		/* Test write prot(cw)*/

#define pg_setref(P)	((P)->pgm.pg_ref = BITSET) 	/* Set ref bit.	  */
#define pg_clrref(P)	((P)->pgm.pg_ref = BITCLR)	/* Clear ref bit. */
#define pg_tstref(P)	((P)->pgm.pg_ref)		/* Test ref bit.  */

#define pg_setndref(P)	((P)->pgm.pg_ndref = BITSET) 	/* Set ndref bit.   */
#define pg_clrndref(P)	((P)->pgm.pg_ndref = BITCLR)	/* Clear ndref bit. */
#define pg_tstndref(P)	((P)->pgm.pg_ndref)		/* Test ndref bit.  */

#define pg_setci(P)	((P)->pgm.pg_ci = BITSET)	/* Set cache inhibit. */
#define pg_clrci(P)	((P)->pgm.pg_ci = BITCLR)	/* Clr cache inhibit. */
#define pg_tstci(P)	((P)->pgm.pg_ci)		/* Test cache inhibit.*/

#define pg_setpfn(P,PFN) ((P)->pgm.pg_pfn = PFN)	/* Set Page frame no. */
#define	pg_getpfn(P)	((P)->pgm.pg_pfn)		/* Get Page frame no. */

#define	pg_setpde(P,PDE) ((P)->pgi.pg_spde = PDE)	/* Set entire pde */
#define	pg_getpde(P)	((P)->pgi.pg_spde)		/* Get entire pde */

#define	pg_getaddr(P)	(ctob(pg_getpfn(P)))		/* Get page address. */


/*
 * Segment Descriptor Entry Definitions
 */

typedef union ssde {
/*         short segment descriptor entry	(* not used *)
 *
 *      +---------------------+---+--+--+
 *      |    Table  Address   |ref|wp|dt|
 *      +---------------------+---+--+--+
 *                 22+6           1   1  2
 */
	struct {
		uint	pt_tpfn : 22,	/* Physical table pfn (hardware) */
				:  6,	/* Zeros for table address (hardware) */
			pt_ref	:  1,	/* Page referenced   (hardware)	*/
			pt_wp	:  1,	/* Write protect     (hardware) */
			pt_dt	:  2;	/* Descriptor type   (hardware)	*/
	} ptm;

	struct {
		uint	pt_ssde; /* short page descriptor (table) entry */
	} pti;
} ssde_t;


typedef union lsde {
/*                    long segment descriptor entry
 *
 * +---+-----+---+---+--+-+-+-+-+-+---+--+--+ +-------------+-+-+-+-+
 * |l/u|limit|ral|wal|sg|s|-|-|-|-|ref|wp|dt| |Table Address|-|-|-|-|
 * +---+-----+---+---+--+-+-+-+-+-+---+--+--+ +-------------+-+-+-+-+
 *   1    15   3   3  1  1 1 1 1 1  1   1  2         22+6      1 1 1 1
 */
	struct {
		uint	pt_lu	:  1,	/* Lower / upper     (hardware)	*/
			pt_limit: 15,	/* Table limit       (hardware)	*/
			pt_ral	:  3,	/* Read access level (hardware)	*/
			pt_wal	:  3,	/* Write access level(hardware)	*/
			pt_sg	:  1,	/* Shared globally   (hardware)	*/
			pt_s	:  1,	/* Supervisor page   (hardware)	*/
			pt_zero	:  4,	/* Unused zero bits  (hardware) */
			pt_ref	:  1,	/* Page referenced   (hardware)	*/
			pt_wp	:  1,	/* Write protect     (hardware) */
			pt_dt	:  2;	/* Descriptor type   (hardware)	*/

		uint	pt_tpfn : 22,	/* Physical table pfn (hardware) */
				:  6,	/* Zeros for table address (hardware) */
			        :  4;	/* Unused/available  (software) */
	} ptm;

	struct {
		uint	pt_hlsde; /* hardware long segment descriptor entry */
		uint	pt_slsde; /* software long segment descriptor entry */
	} pti;
} lsde_t;

#define sde_t	lsde_t			/* all sde's are long for MMB */

#define PT_INV		0		/* Invalid pointer */
#define PT_VSD		2		/* Valid pointer to short descr. */
#define PT_VLD		3		/* Valid pointer to long descr. */

/*	The following macros are used to set, clear, and test
 *	the value of the bits in a segment descriptor (table) entry 
 */

#define pt_setvalid(P,V) ((P)->ptm.pt_dt = (V))		/* Set valid bit. */
#define pt_clrvalid(P)	 ((P)->ptm.pt_dt = PT_INV)	/* Clear valid bit. */
#define pt_tstvalid(P,V) ((P)->ptm.pt_dt == (V))	/* Test valid bit. */

#define pt_setref(P)	((P)->ptm.pt_ref = BITSET) 	/* Set ref bit.	*/
#define pt_clrref(P)	((P)->ptm.pt_ref = BITCLR)	/* Clear ref bit. */
#define pt_tstref(P)	((P)->ptm.pt_ref)		/* Test ref bit. */

#define pt_setprot(P)	((P)->ptm.pt_wp = BITSET)	/* Set wrt prot bit. */
#define pt_clrprot(P)	((P)->ptm.pt_wp = BITCLR)	/* Clear wrt prot bit */
#define pt_tstprot(P)	((P)->ptm.pt_wp)		/* Test wrt prot bit. */

#define pt_setlim(P,L)	((P)->ptm.pt_limit = L)		/* Set index limit. */

/*	The following macro sets up several pointer descriptor 
 *	sets up fields with default values.  Individual fields 
 *	may be changed after setup but most are fixed for the port.
 *
 *	Setup modifies only the upper 28 bits of the long descriptor.
 *	Software bits, valid and write protect and address fields are
 *	not touched.
 *
 *	Default setups for the MMB:
 *		L/U = L = 0		(* always *)
 *		seven bits = 0		(* always *)
 *		limit = 255		(*  255 is for page table *)
 *		ral = 7			(* always *)
 *		wal = 7			(* always *)
 *		next six bits = 0	(* always *)
 */
#define SDEFGP0  0x00fffc00	/* setup value for MMB pointer table entries */
#define pt_setup(P) ((P)->pti.pt_hlsde = SDEFGP0)

#define pt_setpfn(P,PFN) ((P)->ptm.pt_tpfn = PFN) /* Set Table Page frame no. */
#define	pt_getpfn(P)	((P)->ptm.pt_tpfn)	/* Get Table Page frame no. */
#define	pt_getaddr(P)	((caddr_t)ctob(pt_getpfn(P)))	/* Get table address. */


/*	Segment descriptor (table) dependent constants.	*/

#define NBPS		0x40000 /* Number of bytes per segment */
#define SNUMSHFT	18	/* Shift for segment number from address. */
#define SNUMMASK	0x7F	/* Mask for segment number after shift. */
#define SOFFMASK	0x3FFFF	/* Mask for offset into segment. */
#define PPSSHFT		8	/* Shift for pages per segment. */

#define snum(X)   (((uint)(X) >> SNUMSHFT) & SNUMMASK)
#define soff(X)   ((uint)(X) & SOFFMASK)

#define	SEGSIZE		btoc(NBPP)	/* pointer table size in clicks */
#define	SEG_RW		1		/* user permissions read/write */
#define	SEG_RO		2		/* user permissions read-only */


/*
 * Domain Descriptor Entry Definitions
 *
 * The domain descriptors are the same format as the segment descriptors
 * in the current implementation.
 */
#define dde_t sde_t

/*	Domain descriptor (table) dependent constants.	*/

#define DNUMSHFT	25		/* Shift for domain number from addr. */
#define DNUMMASK	0x7F		/* Mask for domain number after shft. */
#define DOFFMASK	0x1FFFFFF	/* Mask for offset into domain. */

#define dnum(X)   (((uint)(X) >> DNUMSHFT) & DNUMMASK)
#define doff(X)   ((uint)(X) & DOFFMASK)

/*
 * Memory Management Unit Definitions
 */

typedef struct _VAR {	/*  virtual address     */
	uint v_ssl	: 14,	/*  segment number      */
	     v_psl	:  8,	/*  page number         */
	     v_byte	: 10;	/*  offset within page  */
} VAR;

/*  masks to extract portions of a VAR  */

#define MSK_IDXSEG  0x3ffff  /*  lower 18 bits == PSL || BYTE  */


/* 
 * MMB control interface definitions
 *
 * Two 32 bit registers provide the MMB programmer interface.
 * They appear in the CPU address space at address 0x10000.
 *
 * The root pointer is set to the base of the first translation table
 * used in a table search.
 * The translation control register is defined below.
 *
 *		+---------------------+
 *	0x10000	|     root pointer    |	 R/W
 *		+---------------------+
 *	0x10004	| translation control |  R/W
 *		+---------------------+
 */

typedef union tcr {

/*         translation control register
 *
 *  +-+-----+-----+-----+----+----+-----+-----+-----+-----+
 *  |e| zero| sre | fcl | ps | is | tia | tib | tic | tid |
 *  +-+-----+-----+-----+----+----+-----+-----+-----+-----+
 *   1   5     1     1    4     4    4     4     4     4
 */
	struct {
		uint	tc_e	:  1,	/* Enable address translation */
				:  5,	/* Unused, taken to be zero */
			tc_sre	:  1,	/* Supervisor root pointer enable */
			tc_fcl	:  1,	/* Function code lookup enable */
			tc_ps	:  4,	/* Page size = (2**tc_ps) */
			tc_is	:  4,	/* Initial shift */
			tc_tia	:  4,	/* Table index A */
			tc_tib	:  4,	/* Table index B */
			tc_tic	:  4,	/* Table index C */
			tc_tid	:  4;	/* Table index D */
	} tcm;

	struct {
		uint	tc_tcr;	/* translation control register */
	} tci;
} tcr_t;


/*  MMB-specific routines  */

extern	mmbstore();		/* Routine to set MMB register */

/*  MMB-specific constants  */

#define MMB_RP		0	/* Root pointer */
#define MMB_TCR		4	/* Translation Control Register */

/*	The following constants are the values the MMB expects/allows
 *	for the associated tcr field assuming a 32 bit logical address.
 */

#define MMB_SRE		0	/* Supervisor root pointer not enabled */
#define MMB_FCL		1	/* Function code lookup required */
#define MMB_PS		0xa	/* Page size is 1K */
#define MMB_IS32	0	/* Logical address must be 32 bits */
#define MMB_TIA		7	/* Table index A */
#define MMB_TIB		7	/* Table index B */
#define MMB_TIC		8	/* Table index C */
#define MMB_TID		0	/* Table index D */

#define MMB_ENBL	0x80000000	/* enable MMB */

/* The fixed values for 32 bit operation are summed up in the macro below. */
#define MMB_FIXED	0x1a07780	/* Fixed values for 32 bit addresses */

#define mmb_enable()	mmbstore(MMB_TCR,(MMB_ENBL|MMB_FIXED))
#define mmb_settcr(T)	mmbstore(MMB_TCR,T)
#define mmb_setrp(RP)	mmbstore(MMB_RP,RP)

#define mmu_enable()	mmb_enable()
#define mmu_setup()	mmb_setrp(fcltbl)


/*	The following variables describe most memory managed by
**	the kernel.  Only certain I/O areas are not included.
*/

extern pde_t	*kptbl;		/* Kernel page table.  Used to	*/
				/* map a contiguous block of	*/
				/* kernel virtual space based	*/
				/* at kernel virtual 0.		*/
extern int	kpbase;		/* The address of the start of	*/
				/* the first physical page of	*/
				/* memory above the kernel.	*/
				/* Physical memory from here to	*/
				/* the end of physical memory	*/
				/* is represented in the pfdat.	*/
extern int	maxmem;		/* Maximum available free	*/
				/* memory.			*/
extern int	freemem;	/* Current free memory.		*/
extern int	availrmem;	/* Available resident (not	*/
				/* swapable) memory in pages.	*/
extern int	availsmem;	/* Available swapable memory in	*/
				/* pages.			*/


/*
**	Implementation specific constants and variables.
*/

#define SYSSEGS 0x01000000	/* Kernel virtual address of syssegs area */
#define ALLFFFS 0xffffffff	/* Last 32 bit address */

#define FCLTSZ	8		/* size of function code lookup table */
#define UDTLIM	0		/* User domain table index limit */
#define SDTLIM	127		/* System domain table index limit */
#define USTLIM	127		/* User segment table index limit */
#define SSTLIM	127		/* System segment table index limit */
#define PGTLIM	255		/* Page table limit (default in pt_setup(P) ) */

/*	The following variables are used in tree walks of the
**	MMU translation tree.
*/

extern	sde_t *udtbl;		/* user domain table 		*/
extern	sde_t *sdtbl;		/* system domain table 		*/


/*	Conversion macros
*/

/*	Get page number from system virtual address.  */

#define	svtop(X)	((uint)(X) >> PNUMSHFT) 

/*	Get system virtual address from page number.  */

#define	ptosv(X)	((uint)(X) << PNUMSHFT)


/*	These macros are used to map between kernel virtual
**	and physical address.  Note that all physical memory
**	within the same contiguous block as the kernel
**	is mapped into kernel virtual address space
**	at the actual physical address.
*/

#define kvtophys(kva) (((caddr_t)pg_getaddr(kvtopde(kva)))+poff(kva))
#define phystokv(paddr) (paddr)


/*	Between kernel virtual address and physical page frame number.
*/

#define pfntokv(pfn)   ((pfn) << PNUMSHFT)


/*	Between kernel virtual addresses and the kernel
**	domain table entry.
*/

#define	kvtodde(kva)  ((sde_t *)&(sdtbl[dnum(kva)]))


/*	Between kernel virtual addresses and the kernel
**	segment table entry.
*/

#define	kvtosde(kva) ((sde_t *)&((sde_t *)pt_getaddr(kvtodde(kva)))[snum(kva)])


/*	Between kernel virtual addresses and the kernel
**	page table entry.
*/

#define	kvtopde(kva) ((pde_t *)&((pde_t *)pt_getaddr(kvtosde(kva)))[psnum(kva)])


/*	Between kernel virtual addresses and the kernel syssegs page table. 
**	Only valid for addresses mapped by kptbl.
*/

#define	kvtokptbl(kva)	(&kptbl[pnum(((uint)(kva) - SYSSEGS))])


/*	Between kernel virtual address and its physical page frame.
**	Only valid for addresses mapped by kptbl.
*/

#define	kvtopfn(kva)	(pg_getpfn((pde_t *)kvtokptbl(kva)))


/*	Between current user virtual addresses and the current user
**	domain table entry.
*/

#define	uvtodde(uva)  (udtbl)	/* There is only one */


/*	Between current user virtual addresses and the current user
**	segment table entry.
*/

#define	uvtosde(uva) ((sde_t *)&((sde_t *)pt_getaddr(uvtodde(uva)))[snum(uva)])


/*	Between current user virtual addresses and the current user page
**	table.
*/

#define	uvtopde(uva) ((pde_t *)&((pde_t *)pt_getaddr(uvtosde(uva)))[psnum(uva)])


/*	The following routines are involved with the pfdat
**	table described in pfdat.h
*/

#define	pdetopfdat(pde)	(&pfdat[pg_getpfn(pde)])
#define	pfntopfdat(pfn)	(&pfdat[pfn])
#define	pfdattopfn(pfd)	(pfd - pfdat)


/* flags used in ptmemall() call
*/

#define PHYSCONTIG 02
#define NOSLEEP    01

/* CPU function codes
*/
#define USERDATA	1
#define USERPROG	2
#define SUPVDATA	5
#define SUPVPROG	6
#define CPUSPACE	7
