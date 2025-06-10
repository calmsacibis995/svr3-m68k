/*
 * built in stream module definitions - used by TLI/rfs
 */


extern struct streamtab sckinfo, spinfo;
extern struct streamtab timinfo, trwinfo,clninfo, loginfo;
struct fmodsw fmodsw[] = {
/* name         streamtab       */
"timod", 	&timinfo,	/* user TLI module */
"tirdwr", 	&trwinfo,	/* uucp TLI module */
"CLONE",	&clninfo,	/* pseudo module   */
"LOG",		&loginfo,	/* streams logger  */
};
int fmodcnt = (sizeof(fmodsw)/sizeof(fmodsw[0]));

/* Allocation of structures used by timod and tirdwr */
struct tim_tim {
	long 	 tim_flags;
	queue_t	*tim_rdq;
	mblk_t  *tim_iocsave;
};

struct trw_trw {
	long 	 trw_flags;
	queue_t	*trw_rdq;
	mblk_t  *trw_mp;
};

#define NUMTIMOD	10
struct tim_tim tim_tim[NUMTIMOD];
struct trw_trw trw_trw[NUMTIMOD];
int tim_cnt = NUMTIMOD;
int trw_cnt = NUMTIMOD;

/* storage allocation for strlog - stream logger */
#define	NLOG			3
int 	log_cnt		=	NLOG;
int	log_bsz		=	20;
struct log log_log[NLOG];

/* storage allocation for streams pipe driver */
#define SPCNT	20
struct sp {
	queue_t *sp_rdq;		/* this stream's read queue */
	queue_t *sp_ordq;		/* other stream's read queue */
} sp_sp[SPCNT];
int	spcnt	= SPCNT;


