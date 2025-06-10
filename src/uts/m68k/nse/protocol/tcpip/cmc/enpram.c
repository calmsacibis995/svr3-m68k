#include <sys/param.h>
#include <sys/CMC/types.h>
#include <sys/fs/s5dir.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/CMC/errno.h>
#include <sys/user.h>
#include <sys/CMC/enpram.h>
#include <sys/CMC/longnames.h>
#include <sys/CMC/netbfr.h>
#include <sys/CMC/ring.h>
#include <sys/CMC/CMC_proto.h>
#include <sys/CMC/CMC_socket.h>
#include <sys/CMC/in.h>
#include <sys/CMC/enp.h>
#include <sys/CMC/user_socket.h>
#include <sys/CMC/enp_addr.h>
#include <sys/CMC/copyin.h>

#define ENPVADDR	(ENP_VADDR + 0x1000)
#define GOCMD		0x8080
extern HOSTCTL host;
extern enp_in_system;


enpropen()
{}

enprclose()
{}

enprread()
{
	/* test for existance of the board */
	if (!(enp_in_system)) {
		u.u_error = ENXIO;
		return(1);
	}

	if (u.u_count > 0) {
		if (short_copyout(ENPVADDR + u.u_offset
				  , u.u_base, u.u_count) != 0)
			u.u_error = EFAULT;

		u.u_base += u.u_count;
		u.u_count = 0;
	}
}

enprwrite()
{
	/* test for existance of the board */
	if (!(enp_in_system)) {
		u.u_error = ENXIO;
		return(1);
	}

	if (u.u_count > 0) {
		if (short_copyin(u.u_base
				 , ENPVADDR + u.u_offset, u.u_count) != 0)
			u.u_error = EFAULT;

		u.u_base += u.u_count;
		u.u_count = 0;
	}
}

enprioctl(dev, cmd, arg, mode)
{
	register SOCKET *s;

	switch (cmd) {
	case ENPIOC_JUMP:		/* Go command */
		/* test for existance of the board */
		if (!(enp_in_system)) {
			u.u_error = ENXIO;
			return(1);
		}

		enpgo(ENP_VADDR, arg);
		break;

	case ENPIOC_RESET:		/* reset command */
		if (host.h_state & HS_INIT) {
			for ( s = sock_table; s < &sock_table[NSOCKETS]; s++) {
				if (s->so_state & (SS_RESV | SS_OPEN)) {
					u.u_error = EEXIST;
					return;
				}
							}
		}
	case ENPIOC_FRESET:		/* force reset command */
		if(!(resetenp(ENP_VADDR)))
			host.h_state &= ~HS_INIT;
		break;

	case ENPIOC_GADDR:		/* get ethernet address from enp */
		/* test for existance of the board */
		if (!(enp_in_system)) {
			u.u_error = ENXIO;
			return(1);
		}

		if(kern_to_user(ENP_VADDR+0x1214, arg, 10))
			u.u_error = EFAULT;
			return(-1);

		break;
	}

}
