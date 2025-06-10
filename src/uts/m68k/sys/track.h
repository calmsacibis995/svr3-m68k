/*
 *	comm trace structure
 */

#define CTCNT	400			/* 400 trace entries */

struct track_hd {			/* track header structure */
	int	start;			/* offset to current position */
	int	end;			/* buffer size count */
};
struct track {				/* actual trace entry structure */
	int	trc_id;			/* trace id */
	int	trc_f1;
	int	trc_f2;
	int	trc_f3;
	int	trc_f4;
	int	trc_f5;
	int	trc_f6;
	int	trc_f7;
};

