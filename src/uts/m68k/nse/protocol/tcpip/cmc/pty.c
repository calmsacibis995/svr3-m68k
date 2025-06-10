/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

/*
 *	Pseudo-teletype driver. This is really two drivers, requiring
 *	two entries in cdevsw.
 *
 *	New implementation for Sys V, includes 4.2BSD PKT mode.
 */

# include <sys/types.h>
# include <sys/param.h>
# include <sys/fs/s5dir.h>
# include <sys/signal.h>
# include <sys/user.h>
# include <sys/CMC/errno.h>
# include <sys/file.h>
# include <sys/termio.h>
# include <sys/conf.h>
# include <sys/tty.h>
# include <sys/sysmacros.h>
# include <sys/CMC/pty.h>
# include <sys/CMC/copyin.h>
# include <sys/cmn_err.h>

struct tty pt_tty[NPTY];
static char ptychars[2*NPTY];

/*
 *	"Slave" side.  Shows up as /dev/ttyp?, and can be used as
 *	a normal tty device.
 */

ptsopen (dev, flag)
{
	register struct tty *tp;

	if (minor (dev) > NPTY ) {
		u.u_error = ENXIO;
		return;
	}

	tp = &pt_tty[(short) minor(dev)];
	if ( (tp->t_state&ISOPEN) == 0 ) {
		ttinit (tp);
		tp->t_cflag &= ~CBAUD;
		tp->t_cflag |= EXTB;
	}
	
	while ( (tp->t_state & CARR_ON) == 0 ) {
		tp->t_state |= WOPEN;
		sleep ((caddr_t)&tp->t_rawq, TTIPRI);
	}

	(*linesw[tp->t_line].l_open)(tp);

	if ( tp->t_tbuf.c_ptr == NULL ) {
		tp->t_tbuf.c_ptr = &ptychars[(short)minor(dev)*2];
		tp->pt_send = tp->pt_flags = 0;
		wakeup (&tp->t_tbuf.c_ptr);
	}

	wakeup ((caddr_t)&tp->t_rawq);
}

ptsclose (dev)
{
	register struct tty *tp;

	tp = &pt_tty[(short) minor(dev)];
	ttywait (tp);
	tp->pt_send = tp->pt_flags = 0;
	tp->t_tbuf.c_ptr = NULL;
	(*linesw[tp->t_line].l_close)(tp);
	wakeup (&tp->t_outq.c_cf);
}

ptsread (dev)			/* normally never called */
{
	register struct tty *tp;

	tp = &pt_tty[(short) minor(dev)];

	if ((tp->t_state&ISOPEN) == 0 ) {
		u.u_error = ENODEV;
		return;
	}

	if ( u.u_fmode&FNDELAY && (tp->t_rawq.c_cc == 0) ) {
		u.u_error = EWOULDBLOCK;
		return;
	}

	(*linesw[tp->t_line].l_read)(tp);
}

ptswrite (dev)			/* normally never called */
{
	register struct tty *tp;

	tp = &pt_tty[(short) minor(dev)];

	if ( (tp->t_state&ISOPEN) == 0) {
		u.u_error = ENODEV;
		return;
	}

	if ( u.u_fmode&FNDELAY && tp->t_state&OASLP ) {
		u.u_error = EWOULDBLOCK;
		return;
	}

	(*linesw[tp->t_line].l_write)(tp);
}

ptsioctl (dev, cmd, arg, mode)
{
	register struct tty *tp;

	tp = &pt_tty[(short) minor(dev)];

	if ( (tp->t_state&ISOPEN) == 0) {
		u.u_error = ENODEV;
		return;
	}

	ttiocom (tp, cmd, arg, mode);

	if (tp->pt_flags&PF_NOSTOP) {
		if (tp->t_iflag&IXON) {		/* ^S/^Q enabled */
			tp->pt_send &= ~TIOCPKT_NOSTOP;
			tp->pt_send |= TIOCPKT_DOSTOP;
			tp->pt_flags &= ~PF_NOSTOP;
			wakeup ((caddr_t)&tp->t_outq.c_cf);
		}
	} else {
		if ( (tp->t_iflag&IXON) == 0 ) {
			tp->pt_send &= ~TIOCPKT_DOSTOP;
			tp->pt_send |= TIOCPKT_NOSTOP;
			tp->pt_flags |= PF_NOSTOP;
			wakeup ((caddr_t)&tp->t_outq.c_cf);
		}
	}
}

ptyproc (tp, cmd)
register struct tty *tp;
{
	if ( tp->t_state & ISOPEN )
	switch (cmd) {
	case T_OUTPUT:
	start:
		if (tp->t_state&TTSTOP)
			break;
		if (tp->pt_flags&PF_STOPPED) {
			tp->pt_flags &= ~PF_STOPPED;
			tp->pt_send = TIOCPKT_START;
		}
		if (tp->t_outq.c_cc) {
			wakeup ((caddr_t)&tp->t_outq.c_cf);
			tp->t_state |= BUSY;
		}
		break;

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		tp->pt_flags |= PF_STOPPED;
		tp->pt_send |= TIOCPKT_STOP;
		break;

	case T_WFLUSH:
		tp->pt_send |= TIOCPKT_FLUSHWRITE;
	case T_RESUME:
		tp->t_state &= ~TTSTOP;
		goto start;

	case T_RFLUSH:
		tp->pt_send |= TIOCPKT_FLUSHREAD;
		break;
	}
}



/*
 *	"Controller" side.  This is considerably more complicated
 *	than the slave side, mostly because it has to spend time
 *	defeating the line discipline operation.
 *
 *	Shows up as /dev/ptyp?
 *
 *	NOTE: the cdevsw entry for the controller MUST NOT have
 *	a tty struct array associated with it.
 *
 */

ptcopen (dev, flag)
{
	register struct tty *tp;

	if (minor(dev) > NPTY) {
		u.u_error = ENXIO;
		return;
	}

	tp = &pt_tty[(short) minor(dev)];
	if ( tp->t_state & CARR_ON ) {		/* already opened */
		u.u_error = EIO;
		return;
	}

	tp->t_proc = ptyproc;
	if (tp->t_state&WOPEN)
		wakeup ((caddr_t)&tp->t_rawq);
	tp->t_state |= CARR_ON;
}

ptcclose (dev)
{
	register struct tty *tp;

	tp = &pt_tty[(short) minor(dev)];

	if (tp->t_state & ISOPEN)
		signal (tp->t_pgrp, SIGHUP);
	ttyflush (tp, FREAD|FWRITE);
	tp->t_state &= ~(CARR_ON | ISOPEN);
	tp->t_state = 0;
}

ptcwrite (dev)
{
	register struct tty *tp;
	register char *cp;
	register struct cblock *cb;
	register int n;

	tp = &pt_tty[(short) minor(dev)];
	if ((tp->t_state&ISOPEN) == 0) {
		u.u_error = EIO;
		return;
	}

	while (tp->t_rbuf.c_ptr == NULL) {
		cb = getcf();
		if ( cb == NULL ) {
			cfreelist.c_flag = 1;
			sleep ((caddr_t)&cfreelist, TTOPRI);
		}
		else {
			tp->t_rbuf.c_ptr = cb->c_data;
			tp->t_rbuf.c_size = cfreelist.c_size;
			tp->t_rbuf.c_count = tp->t_rbuf.c_size;
		}
	}

	while (u.u_count) {
		n = min(tp->t_rbuf.c_count, u.u_count);
		cp = tp->t_rbuf.c_ptr + (tp->t_rbuf.c_size -
						tp->t_rbuf.c_count);
		copyin (u.u_base, cp, n);
		tp->t_rbuf.c_count -= n;
		u.u_count -= n;
		u.u_base += n;

		(*linesw[tp->t_line].l_input) (tp, 0);
	}
}

ptcread (dev)
{
	register struct tty *tp;
	register struct cblock *cb;
	register int n;
	register int c;

	tp = &pt_tty[(short) minor(dev)];

again:
	if ((tp->t_state&ISOPEN) == 0) {
		u.u_error = EIO;
		goto out;
	}

	if ( (tp->t_state&TTSTOP) || (tp->t_outq.c_cc == 0) ) {
		if ( u.u_fmode&FNDELAY) {
			u.u_error = EWOULDBLOCK;
			return;
		}
		sleep ((caddr_t)&tp->t_outq.c_cf, TTIPRI);
		goto again;
	}

	if (tp->pt_flags&PF_PKT) {
		if (tp->pt_send) {
			passc (tp->pt_send);
			tp->pt_send = 0;
			return;
		}
		else	passc (0);
	}

	while ( tp->t_outq.c_cc && u.u_count ) {
		cb = tp->t_outq.c_cf;
		n = cb->c_last - cb->c_first;
		if ( u.u_count >= n ) {
			copyout (&cb->c_data[cb->c_first], u.u_base, n);
			u.u_base += n;
			u.u_count -= n;
			putcf (getcb (&tp->t_outq));
		} else {
			if ( (c = getc (&tp->t_outq)) < 0 )
				break;
			passc (c);
		}
	}

out:
	tp->t_state &= ~BUSY;

	if (tp->t_state&OASLP) {
		if (tp->t_outq.c_cc <= ttlowat[tp->t_cflag&CBAUD]) {
			tp->t_state &= ~OASLP;
			wakeup ((caddr_t) &tp->t_outq);
		}
	}
	if (tp->t_state&TTIOW) {
		if (tp->t_outq.c_cc == 0) {
			tp->t_state &= ~TTIOW;
			wakeup ((caddr_t)&tp->t_oflag);
		}
	}
}

ptcioctl (dev, cmd, arg, mode)
{
	register struct tty *tp;
	register v;

	tp = &pt_tty[(short) minor(dev)];

	while ( tp->t_tbuf.c_ptr == NULL )
		sleep ( &tp->t_tbuf.c_ptr, TTOPRI );

	switch (cmd) {
	case TIOCPKT:
		v = fuword( arg );
		if ( v == -1 ) {
			u.u_error = EFAULT;
			return;
		}
		if ( v )
			tp->pt_flags |= PF_PKT;
		else	tp->pt_flags &= ~PF_PKT;
		return;

	case TIOCREMOTE:
		v = fuword( arg );
		if ( v == -1 ) {
			u.u_error = EFAULT;
			return;
		}
		if ( v )
			tp->pt_flags |= PF_REMOTE;
		else	tp->pt_flags &= ~PF_REMOTE;
		ttyflush (tp, FREAD|FWRITE);
		return;

/*
	case TIOCSETP:
		while (getc (&tp->t_outq) >= 0)
			;
		break;
*/
	}

	ttiocom (tp, cmd, arg, mode);

	if (tp->pt_flags&PF_NOSTOP) {
		if (tp->t_iflag&IXON) {		/* ^S/^Q enabled */
			tp->pt_send &= ~TIOCPKT_NOSTOP;
			tp->pt_send |= TIOCPKT_DOSTOP;
			tp->pt_flags &= ~PF_NOSTOP;
			wakeup ((caddr_t)&tp->t_outq.c_cf);
		}
	} else {
		if ( (tp->t_iflag&IXON) == 0 ) {
			tp->pt_send &= ~TIOCPKT_DOSTOP;
			tp->pt_send |= TIOCPKT_NOSTOP;
			tp->pt_flags |= PF_NOSTOP;
			wakeup ((caddr_t)&tp->t_outq.c_cf);
		}
	}
}

ptcselect()
{
	cmn_err (CE_PANIC,"ptcselect",NULL);
}
