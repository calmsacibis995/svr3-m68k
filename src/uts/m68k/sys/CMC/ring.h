/*
 * data structure for enp and host to asynchrouously communicate
 */

typedef struct RING
{
	short	r_rdidx;
	short	r_wrtidx;
	short	r_size;
	short	r_pad;			/* to make VAXen happy */
	int	r_slot[ 1 ];
}RING;

typedef struct RING4
{
	short	r_rdidx;
	short	r_wrtidx;
	short	r_size;
	short	r_pad;			/* to make VAXen happy */
	int	r_slot[ 4 ];
} RING4;

typedef struct RING8
{
	short	r_rdidx;
	short	r_wrtidx;
	short	r_size;
	short	r_pad;			/* to make VAXen happy */
	int	r_slot[ 8 ];
} RING8;

typedef struct RING16
{
	short	r_rdidx;
	short	r_wrtidx;
	short	r_size;
	short	r_pad;			/* to make VAXen happy */
	int	r_slot[ 16 ];
} RING16;

typedef struct RING32
{
	short	r_rdidx;
	short	r_wrtidx;
	short	r_size;
	short	r_pad;			/* to make VAXen happy */
	int	r_slot[ 32 ];
} RING32;
