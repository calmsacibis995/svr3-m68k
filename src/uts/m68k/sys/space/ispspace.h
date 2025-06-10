
/* The International Support Package Character Set data structures */


#include "sys/cstty.h"
#include "sys/cs.h"

/* this is defined via sysgen #define CSNTTY 34 */
#define CSTTMAP 11
#define CSTTSEG 189
#define CSTTSIZ 64

char		cs_ttbuf[CSTTSEG*CSTTSIZ];	/* translation table buffers */
struct map	csmap[CSTTMAP + 2];		/* maps of unallocated space
						   for trans table buffers */
struct cstthdr	*cs_ttmap[CSTTMAP] = 0;		/* pointers to allocated
						   translation tables */
struct csttinf	cs_term[CSNTTY];		/* char set info for tty's */
struct csttinf	*cs_index[CSNTTY+1];		/* fast index to cs_term */

struct csinfo	cs_info = {
	NULL,			/* most recently used csttinf address */
	CSNTTY,			/* number of tty structures */
	CSTTSEG,		/* number of trans. table segments */
	CSTTSIZ,		/* size of a trans table segment */
	CSTTMAP,		/* # of translation table maps */
	0 };			/* # of csttinf elements now in use */

extern int ttcsinit();

