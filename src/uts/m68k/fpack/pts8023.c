/*
** File name:	pts8023.c
** 
** Description:
**
**	LANCE Driver level code to handle foreign (non-XNS) packets.
**
** Contents:
**
**	fpack_up - foreign packet user registration
**	fpack_tx - transmit a foreign packet
**	fpack_dn - foreign packet user deregistration
** 
** Copyright (c) 1986 by Motorola Inc.
**
** Revision History
**
** 	03/14/86 - Created (RWL)
*/

static char sccs_str[] = "@(#)pts8023.c	1.5";

#include <sys/errno.h>
#include <sys/param.h>
#include <sys/types.h>

#include <sys/bpp/types.h>
#include <sys/bpp/ce.h>
#include <sys/bpp/ci_config.h>
#include <sys/bpp/ci_const.h>
#include <sys/bpp/ci_types.h>
#include <sys/bpp/bpp.h>

#include <sys/fpack/ma_def.h>
#include <sys/fpack/ma_epb.h>
#include <sys/fpack/pts8023.h>

#define MAX_8023_PACKET		1518
#define REG			register

int	fpack_rx();
void	fp_ena_cp();

#ifdef DEBUG
int	rx_packet_socket;
int	rx_packet_id;
#else
static	int	rx_packet_socket;
static	int	rx_packet_id;
#endif
static	int	zero_sub();
static	EN_ADDR	en_host_addr;
static	int	mac_sap;
static	int	first;
static	int	en_is_up;
static	MA_EPB	ma_epb;
static	BUF	buf;

static	struct	bppio {
	int 	count;
	char	*epb_p;
	int	epb_len;
	char	*buf_p;
	int	buf_len;
} bpp_out, bpp_in;

/* packet type switch */

fpacksw	fpsw[FP_MAX];

/****************************************************************************/
/*  
/*  fpack_up
/*  
/*  Register a packet type switch user.  Make an entry into the packet-type
/*  switch table.
/*  
/****************************************************************************/

int
fpack_up( pack_typ, tx_end, rx_end, node_addr )
REG	ushort	pack_typ;		/* packet type found in E-net header */
	int	(*tx_end)();		/* pointer to end-of-transmit routine */
	int	(*rx_end)();		/* pointer to end-of-receive routine */
	char	*node_addr;
{
REG	int	i;
REG	fpacksw	*fpp, *fppsav;

#ifdef DEBUG
printf("fpack_up: typ = 0x%x, tx_end = 0x%x, rx_end = 0x%x, node_addr = 0x%x\n", pack_typ, tx_end, rx_end, node_addr);
#endif

	/* See if the Ethernet controller is up */

	if( !first || !en_is_up )
		fpack_init();

	if( !en_is_up )
		return( FP_NOT_UP );

	/* Ensure no conflict with 802.3 packets.  IEEE 802.3 packets have
	 * a length field where Ethernet has a type field.
	 */

	if( pack_typ <= MAX_8023_PACKET )
		return( FP_BAD_TYPE );

	/* Look for type already in table and find a free slot */

	fpp	= fpsw;
	fppsav	= NULL;
	for( i = 0; i < FP_MAX; i++ )
	{
		if( fpp->fp_type == pack_typ )
			return( FP_DUP );
		if( ( fpp->fp_type == 0 ) && ( fppsav == NULL ) )
			fppsav	= fpp;
		fpp++;
	}

	if( !fppsav )
		return( FP_NO_ROOM );

	fppsav->fp_tx		= tx_end;
	fppsav->fp_rx		= rx_end;
	fppsav->fp_count	= 0;

	/* Sign on with the BPP */

	if( ( fppsav->fp_bpp_socket = bpp_knxt() ) < 0 )
		return( FP_NO_ROOM );
	if( ( bpp_kopen( fppsav->fp_bpp_socket, fpack_rx ) ) < 0 )
		return( FP_NO_ROOM );

	/* Register with BPP */

	if( ( fppsav->fp_bpp_id = reg_dereg( fppsav->fp_bpp_socket, CE_REGISTRATION_REQUEST ) ) < 0 )
		return( FP_NO_ROOM );

	/* Do this last or there may be a race problem */

	fppsav->fp_type	= pack_typ;

	/* Return our Ethernet addr to the caller */

	fp_ena_cp( en_host_addr, node_addr );
	return( 0 );
}

/****************************************************************************/
/*  
/*  fpack_tx
/*  
/*  Verify that we are accepting packets of this type.
/*  Send the packet the MAC layer via BPP/CE.
/*  
/****************************************************************************/

fpack_tx( packet, size )
	char 		*packet;	/* pointer to packet */
	int 		size;		/* size of packet */
{
REG	int 		i;
REG	fpacksw 	*fpp;
REG	struct fp_enhdr	*fp_enh_p;
REG	EPB		*ep;		/* ptr to MA_EPB.hdr */
REG	struct bppio	*bp;		/* ptr to bpp_out */

	fp_enh_p = ( struct fp_enhdr * ) packet;

#ifdef DEBUG
printf("fpack_tx: packet = 0x%x, type = 0x%x, size = %d\n", packet, fp_enh_p->fp_type, size);
#endif

	if( size > MAX_8023_PACKET )
		return( FP_TOOBIG );

	/* Verify that we are accepting packets of this type */

	fpp = fpsw;
	for( i = 0; i < FP_MAX; i++ )
	{
		if( fpp->fp_type == fp_enh_p->fp_type )
			break;
		fpp++;
	}

	if( i == FP_MAX )
		return( FP_BAD_TYPE );
	
	/* If the controller's not up, reject request */

	if( !en_is_up )
		return( FP_NOT_UP );
	/* else it's a type we handle - try to send it */

	/* Set up epb for data indication */

	ep		= &ma_epb.hdr;
	ep->src_id	= fpp->fp_bpp_id;
	ep->dst_id	= mac_sap;
	ep->type	= MA_DATA_REQUEST;
	ep->epb_size	= sizeof( MA_EPB );
	ep->buf		= NULL;
	ep->user_def	= fpp->fp_type;
	ep->status	= 0;

	fp_ena_cp( fp_enh_p->fp_dst, ma_epb.rem_addr );
	fp_ena_cp( fp_enh_p->fp_src, ma_epb.loc_addr );
	ma_epb.status	= 0;
	ma_epb.svc_class = 0;

	/* Set up i/o structure */

	bp		= &bpp_out;
	bp->count	= 2;
	bp->epb_p	= ( char * ) &ma_epb;
	bp->epb_len	= sizeof( MA_EPB );
	bp->buf_p	= packet + sizeof( struct fp_enhdr );
	bp->buf_len	= size - sizeof( struct fp_enhdr );

	/* Send it */

	if( bpp_kwrite( fpp->fp_bpp_socket, bp ) < 0 )
		return( FP_NO_ROOM );
	
	fpp->fp_packet	= packet;		/* save for tx_end */

	/* Done - normal return to caller */

	return( 0 );
}

/****************************************************************************/
/*  
/*  fpack_dn
/*  
/*  De-register a foreign packet user.  Remove entry from the packet-type
/*  switch table.
/*  
/****************************************************************************/

int
fpack_dn( type )
	short	type;		/* packet type to be de-registered */
{
REG	int i;
REG	fpacksw *fpp;

#ifdef DEBUG
printf("fpack_dn: pack_typ = 0x%x\n", type);
#endif

	/* Find packet type in switch table */

	fpp	= fpsw;
	for( i = 0; i < FP_MAX; i++ )
	{
		if( fpp->fp_type == type )
			break;
		fpp++;
	}

	/* If packet type was not found, return error */

	if( i == FP_MAX )
		return( FP_BAD_TYPE );
	
	/* Remove packet type from table.  Blot out type field first
	 * to avoid race problem.
	 */

	fpp->fp_type	= 0;
	fpp->fp_tx	= zero_sub;
	fpp->fp_rx	= zero_sub;
	reg_dereg( fpp->fp_bpp_socket, CE_DEREGISTRATION_REQUEST );
	fpp->fp_bpp_id	= 0;
	fpp->fp_bpp_socket = 0;

	/* Done.  Normal return */

	return( 0 );
}

/****************************************************************************/
/*  
/*  fpack_ioc
/*  
/*  Process information/control requests.
/*  
/****************************************************************************/

int
fpack_ioc( pack_typ, cmd_typ, arg )
	int 	pack_typ;
	int 	cmd_typ;
	char	*arg;
{
REG	int	err;

#ifdef DEBUG
printf("fpack_ioc: pack_typ = 0x%x, cmd_typ = 0x%x, arg = 0x%x\n", pack_typ, cmd_typ, arg);
#endif

	/* See if the Ethernet controller is up */

	if( !en_is_up )
		return( FP_NOT_UP );

	err = 0;

	switch( cmd_typ )
	{
	case FP_GET_ADDR: /* Return our Ethernet addr to the caller */
		fp_ena_cp( en_host_addr, arg );
		break;

	default:
		err = EINVAL;
		break;
	}

	return( err );
}

/****************************************************************************/
/*  
/*  fpack_init
/*  
/*  Do first time things
/*  
/****************************************************************************/

int
fpack_init()
{
REG	int			i;
REG	int			savepri;
REG	int			err;
REG	struct bppio		*bp;		/* ptr to bpp_out */
	struct cpu_tab		cpu_tab;
REG	struct cpu_tab		*cp;		/* ptr to cpu_tab */
REG	struct cpu_descr	*cpu_descr_p;
REG	struct ma_net		*ma_net_p;

#ifdef DEBUG
printf("fpack_init: \n");
#endif

	/* We only need to register with BPP the first time */

	if( !first )
	{
		/* get a socket to talk to BPP */
		if( ( rx_packet_socket = bpp_knxt() ) < 0 )
			return( rx_packet_socket );
		if( err = bpp_kopen( rx_packet_socket, fpack_rx ) < 0 )
			return( err );

		/* register with BPP */
		if( ( err = reg_dereg( rx_packet_socket, CE_REGISTRATION_REQUEST ) ) < 0 )
			return( err );
		ma_epb.hdr.src_id	= rx_packet_id	= err;
		first	= TRUE;
	} /* if first time */

	/* get CPU ID for MVME330 - assume only one on system */

	cp		= &cpu_tab;
	cp->count	= BPP_MAX_CPU;
	if( ( err = bpp_kcnfg( rx_packet_socket, cp ) ) < 0 )
	{
		reg_dereg( rx_packet_socket, CE_DEREGISTRATION_REQUEST );
		return( err );
	}
	cpu_descr_p	= cp->cpu_descr;
	for( i = cp->count; i > 0; i-- )
	{
		if( cpu_descr_p->cpu_type == BPP_MVME330 )
			break;
	}

	if( i == 0 )
		return( -EIDRM );
	
	mac_sap	= ma_epb.hdr.dst_id	= ( cpu_descr_p->cpu_id << 16 ) | MAC_QID;

	/* get ethernet ID from MVME330 */

	ma_epb.hdr.type	= MA_XNS_NET;
	ma_net_p		= ( struct ma_net * ) buf.data;
	ma_net_p->flg_eid	= TRUE;	/* tell MAC where to send packets */
	ma_net_p->net_eid	= rx_packet_id;	
	ma_net_p->flg_stat	= FALSE;
	ma_net_p->flg_mode	= FALSE;
	ma_net_p->flg_padr	= FALSE;	/* ask for physical addr */
	ma_net_p->flg_ladr	= FALSE;
	ma_net_p->flg_ladf	= FALSE;

	bp		= &bpp_out;
	bp->count	= 2;
	bp->epb_p	= ( char * ) &ma_epb;
	bp->epb_len	= sizeof( MA_EPB );
	bp->buf_p	= ( char * ) ma_net_p;
	bp->buf_len	= sizeof( struct ma_net );

	if( ( err = bpp_kwrite( rx_packet_socket, bp ) ) < 0 )
	{
		reg_dereg( rx_packet_socket, CE_DEREGISTRATION_REQUEST );
		return( err );
	}

	/* wait for reply from MAC */

	en_is_up	= 0;
	savepri		= spl4();
	while( !en_is_up )
		sleep( &en_is_up, PZERO+1 );
	splx( savepri );

	if( en_is_up == 1 )
		return( 0 );

	en_is_up	= 0;
	return( ENOSPC );
}

/****************************************************************************/
/*  
/*  fpack_rx
/*  
/*  Handle an incoming BPP packet.  Packets can be:
/*	1. response to network request (at init time only)
/*	2. data indications - incoming ethernet packets
/*	3. data response - acknowlegement that a packet was transmitted
/*  
/****************************************************************************/

int
fpack_rx( socno, ep )
	int		socno;
REG	EPB		*ep;
{
	struct bppio	blk_in;
REG	struct bppio	*bp;			/* ptr to blk_in */

	bp	= &blk_in;
	while( TRUE )
	{
		bp->count	= 2;
		if( bpp_kpoint( socno, bp ) <= 0 )
			break;

		ep	= ( EPB * ) bp->epb_p;
		switch( ep->type )
		{
		case MA_DATA_INDICATION: /* Data coming in */
			fp_data_ind( ep );
			break;

		case MA_DATA_CONFIRM: /* Block has been transmitted */
			fp_data_confirm( ep );
			break;

		case MA_XNS_NET: /* Startup time network stuff */
			fp_xns_net( ep );
			break;
	
		default: /* Why are we here? */
#ifdef DEBUG
printf("fp_rxint: unexpected epb type = %#x\n", ep->type);
#endif
			break;
		}
		bpp_krel( socno, bp );
	}

	return( 0 );
}

fp_data_ind( epb_p )
REG	MA_EPB		*epb_p;
{
REG	int		i;
REG	struct fpacksw	*fpp;
REG	BUF		*buf_p;

#ifdef DEBUG
printf("fp_data_ind: packet type = 0x%x\n", epb_p->hdr.user_def);
#endif

	/* find packet type in table */

	fpp	= fpsw;
	for( i = 0; i < FP_MAX; i++ )
	{
		if( fpp->fp_type == ( short ) epb_p->hdr.user_def )
			break;
		fpp++;
	}

	/* If packet type was not found, return error */

	if( i == FP_MAX )
		return( FP_BAD_TYPE );
	
	/* call user's receive routine */

	buf_p	= epb_p->hdr.buf;
	(*fpp->fp_rx)( ( struct fp_enhdr * ) buf_p->data, buf_p->length);

	return( 0 );
}

fp_data_confirm( epb_p )
REG	EPB		*epb_p;
{
REG	int		i;
REG	struct fpacksw	*fpp;

	/* locate switch table entry for this user */
	
	fpp	= fpsw;
	for( i = 0; i < FP_MAX; i++ )
	{
		if( fpp->fp_bpp_id == epb_p->dst_id )
			break;
		fpp++;
	}

	/* If packet type was not found, return error */

	if( i == FP_MAX )
		return( FP_BAD_TYPE );

	/* if user defined a transmit interrupt routine, call user */

	if( fpp->fp_tx != NULL )
		(*fpp->fp_tx)( fpp->fp_packet );

	return( 0 );
}

fp_xns_net(epb_p)
REG	EPB		*epb_p;
{
REG	struct ma_net	*ma_net_p;
	
	/* save the ethernet physical address */

	if( epb_p->status == 0 )
	{
		ma_net_p	= ( struct ma_net * ) epb_p->buf->data;
		fp_ena_cp( ma_net_p->net_padr, en_host_addr );
		en_is_up	= 1;
	}
	else 
		en_is_up	= 2;

	wakeup( &en_is_up );
	return( 0 );
}

/****************************************************************************/
/*  
/*  reg_dereg
/*  
/*  register or deregister with BPP
/*  
/****************************************************************************/

int
reg_dereg(socno, type)
REG	int		socno;
REG	int		type;
{
REG	int		err;
REG	int		reg_id;
REG	EPB		*ep;
REG	struct bppio	*bp;

#ifdef DEBUG
printf("fpack reg_dereg: socno = 0x%x, type = 0x%x\n", socno, type);
#endif

	ep		= &ma_epb.hdr;
	ep->next	= NULL;
	ep->pool.id	= 0;
	ep->pool.cpu	= 0;
	ep->dst_id	= 0;
	ep->src_id	= 0;
	ep->epb_size	= sizeof( MA_EPB );
	ep->type	= type;
	ep->status	= 0;
	ep->buf		= NULL;
	ep->user_def	= 0;

	bp		= &bpp_out;
	bp->count	= 1;
	bp->epb_p	= ( char * ) &ma_epb;
	bp->epb_len	= sizeof( MA_EPB );

	if( ( err = bpp_kwrite( socno, bp ) ) < 0 )
		return( err );

	if( type == CE_REGISTRATION_REQUEST )
	{
		/* save our ID */
		bp		= &bpp_in;
		bp->count	= 1;
		if( ( err = bpp_kpoint( socno, bp ) ) < 0 )
			return( err );
		if( bp->count != 1 )
			return( -EIDRM );
		ep	= ( EPB * ) bp->epb_p;
		reg_id	= ep->dst_id;		/* save registration id */
		bpp_krel( socno, bp );
		return( reg_id );
	}
	return( 0 );
}

/****************************************************************************/
/*  
/*  Miscellaneous little support routines 
/*
/****************************************************************************/

/* stub to return zero - used as a null routine */

static int
zero_sub()
{
	return( 0 );
}

/* copy an ethernet address from one place to another */

void
fp_ena_cp(cin_p, cout_p)
REG	char		*cin_p;
REG	char		*cout_p;
{
REG	int		i;

	for( i = 1; i <= 6; i++ )
		*cout_p++ = *cin_p++;

	return;
}
