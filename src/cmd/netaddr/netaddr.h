/************************************************************************
*
*	File:		netaddr.h
*	Routine:	headers/storage
*	
*************************************************************************/


/******************************************** storage definitions */
typedef unsigned char uchar;
struct ioctlbuf {
	struct IADDR {				/* internet address */
		uchar	inet_l;
		uchar	iserial_l[3];
	} iaddr_l;
	struct EADDR {				/* ethernet address */
		uchar	mfg_code_l[3];
		uchar	enet_l;
		uchar	eserial_l[2];
	} eaddr_l;
} iocbuf;
#define	inet	iocbuf.iaddr_l.inet_l
#define iserial iocbuf.iaddr_l.iserial_l
#define eaddr	iocbuf.eaddr_l
#define mfg_code	iocbuf.eaddr_l.mfg_code_l
#define enet		iocbuf.eaddr_l.enet_l
#define eserial		iocbuf.eaddr_l.eserial_l

char	domain_buf[80];				/* current domain       */
char	inet_asc[16];				/* ascii representation */
char	inet_hex[9];				/* ascii/hex represent  */
char	enet_asc[13];				/* ascii representation */

/************************************ error messages */
char	ERR_OPEN_ENPRAM[]= 	"netaddr:  open of /dev/enpram failed\nnetaddr:   ";
char	ERR_IOCTL_ENPRAM[]= 	"netaddr:  ioctl failed - invalid address or non existant enp\nnetaddr:  ";
char	DOMAIN_DEF[]=		"Default";
char	zeros[]=		"00";
