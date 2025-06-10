/*	%W%	*/

/*
 *	This file defines the external data structures used
 *	by the MVME350 driver.
 */

#ifndef	VME350_1
#define	VME350_1	0
#endif
#ifndef	VME350_2
#define	VME350_2	0
#endif
#ifndef	VME350_3
#define	VME350_3	0
#endif

#define VME350CTL  (VME350_0 + VME350_1 + VME350_2 + VME350_3)

struct iotime m350stat[VME350CTL];
struct iobuf m350tab[VME350CTL] = {
	tabinit(0,&m350stat[0].ios),
#if VME350CTL>1
	tabinit(0,&m350stat[1].ios),
#if VME350CTL>2
	tabinit(0,&m350stat[2].ios),
#if VME350CTL>3
	tabinit(0,&m350stat[3].ios),
#endif
#endif
#endif
};

#include "sys/mvme350.h"

int m350maxbsize = M350MAXBSIZE;	/* maximum size of dma buffers */

struct	m350pkt m350pkt[VME350CTL];		/* one per controller */
struct	m350env m350env[VME350CTL*3];		/* three per controller */
struct	m350chan m350chan[VME350CTL];		/* one per controller */
struct	iostat m350iostp[VME350CTL];

#undef	M350IOCTL
#undef	M350REWIND
#undef	M350ERASE
#undef	M350RETENSION
#undef	M350WRTFM
#undef	M350RDFM
#undef	M350SETDMA
#undef	M350GETDMA
