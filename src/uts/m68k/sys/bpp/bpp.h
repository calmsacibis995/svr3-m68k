#define	BPP_NEXT_SOCKET	1	/* get a socket number */
#define	BPP_CONFIG	2	/* get a copy of the CPU table */
#define	BPP_SIGNAL	3	/* send a signal when data arrives */
#define	BPP_NSIGNAL	4	/* cancel a signal */
#define	BPP_BLOCK	5	/* block on read if no data present */
#define	BPP_NBLOCK	6	/* do not block on read */
#define	BPP_RESET_CPU	7	/* reset specified CPU */


#define	BPP_MVME330 VME330_BOARD
#define BPP_MVME333 VME333_BOARD
#define BPP_MVME372 VME372_BOARD

#define BPP_MAX_CPU	CI_MAX_CPU	/* max number of cpu tab entries */

struct bpp_ios {
	int 	count;
	struct blk_descr {
		char	*data_p;
		int	data_len;
	} data_array;
};

struct cpu_tab {
	int	count;
	struct cpu_descr{
		short	cpu_id;
		short	cpu_type;
	} cpu_descr[BPP_MAX_CPU];
};
