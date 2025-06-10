/* definitions for MVME 331 driver */

#include "sys/mvme331.h"

#ifdef VME331_5
int m331_ctlcnt = 6;
#define VME331_COUNT 6
#else
#    ifdef VME331_4
	int m331_ctlcnt = 5;
#       define VME331_COUNT 5
#    else
#       ifdef VME331_3
	   int m331_ctlcnt = 4;
#          define VME331_COUNT 4
#       else
#          ifdef VME331_2
	      int m331_ctlcnt = 3;
#             define VME331_COUNT 3
#          else
#             ifdef VME331_1
	         int m331_ctlcnt = 2;
#                define VME331_COUNT 2
#             else
#                ifdef VME331_0
	            int m331_ctlcnt = 1;
#                   define VME331_COUNT 1
#                else
	            int m331_ctlcnt = 0;
#                   define VME331_COUNT 0
#	         endif
#	      endif
#	   endif
#	endif
#    endif
#endif

/* description of the controller */
struct CCcontroller CCcntl[ VME331_COUNT ];

/* command channel buffer */
char cci_buffer[ PKMAX*VME331_COUNT*sizeof( struct CCpacket )
	       + PTMAX*VME331_COUNT*sizeof( struct CCpointer) ];

/* SQA 05/20/86 - 1 */
/* Port buffer */
char PortBuf[VME331_COUNT * PORTBUFLENGTH * MAXPORTS ];

/* halt save arrays */
char m331_haltsave[VME331_COUNT][SIZEOFHALTSAVE];
