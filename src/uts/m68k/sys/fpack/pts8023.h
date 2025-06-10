/*
 * File name:	pts8023.h
 *
 * Description:
 *
 *	Packet type switch interface headers
 *
 * Contents:
 *
 *	Contains defines and structure definitions for packet type switch 
 *	interface.
 *
 * System:
 *
 *	VME S8000 Local Area Network
 *
 * Copyright 1986 (C) by Motorola, Inc.
 *
 * Revision history:
 *
 *	03/17/86	Created (RWL)
 *	04/22/86	Converted from S6300 to VME S8000 (RWL)
 *
 * Version:
 *
 *	@(#)pts8023.h	1.2
 */
  
#ifndef	_FPACK
#define	_FPACK

#define FP_GET_ADDR	1	/* fpack_ioc - Get address cmd */

#define FP_BAD_TYPE	144	/* Packet type is not valid ethernet type */
#define FP_DUP		145	/* Attempted to add packet type already in table */
#define FP_NO_ROOM	146	/* Packet type switch table full or heap full */
#define FP_TOOBIG	147	/* Packet too large */
#define FP_NOT_UP	148	/* Ethernet is not up */

#endif /* _FPACK */

typedef char	EN_ADDR[6];

typedef	struct fp_enhdr {
	EN_ADDR	fp_dst;
	EN_ADDR	fp_src;
	short 	fp_type;
} FP_ENHDR;

/* Packet-type switch table format */

#define FP_MAX 10		/* number of table entries in switch */

typedef struct fpacksw {
	short	fp_type;	/* type field from E-net header */
	int 	(*fp_tx)();	/* pointer to end-of-transmit routine */
	int 	(*fp_rx)();	/* pointer to end-of-recieve routine */
	int	fp_count;	/* count of packets of this type */
	int	fp_bpp_socket;	/* RALPH!!!! */
	int	fp_bpp_id;	/* RALPH!!!! */
	char	*fp_packet;	/* RALPH!!!! */
	int	fp_flags;	/* the inevitable catch-all */
#define	FP_L_FREE		0x01	/* Return ownership to LANCE after rx_end */
#define	FP_M_FREE		0x02	/* Return m structure after fp_tx() */
} fpacksw;
