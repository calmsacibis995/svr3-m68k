/*
 *	Copyright (c) 1984, Communication Machinery Corporation
 */

/*
 *	This file contains material and information which is
 *	proprietary to Communication Machinery Corporation, and
 *	which may not be divulged without the express written
 *	permission of Communication Machinery Corporation.
 */

# define NPTY 32

# define pt_flags t_tbuf.c_ptr[0]
# define pt_send t_tbuf.c_ptr[1]

/*
 *	Values for pt_flags field
 */

# define PF_PKT		0x1
# define PF_REMOTE	0x2
# define PF_NOSTOP	0x4
# define PF_STOPPED	0x8

/*
 *	Values or'd into PKT mode control byte
 */

# define TIOCPKT_FLUSHREAD	0x1
# define TIOCPKT_FLUSHWRITE	0X2
# define TIOCPKT_STOP		0x4
# define TIOCPKT_START		0x8
# define TIOCPKT_NOSTOP		0x10
# define TIOCPKT_DOSTOP		0x20

# define TIOCPKT (('p'<<8) | 128)
# define TIOCREMOTE (('p'<<8) | 129)
