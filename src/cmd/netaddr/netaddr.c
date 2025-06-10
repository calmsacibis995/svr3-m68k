/************************************************************************
*
*	File:		netaddr.c
*	Routine:	headers/storage
*	
*	Function:	This utility will extrace internet and
*			ethernet addresses from enp10 card running
*			CMC tcp/ip.  Utility is planned for use in
*			sysadm menu structure to ease installation
*			of rfs.  Internet address is that currently
*			configured in /etc/hosts file.
*
*	options:
*			-h	no header
*			-a	build inet address from ethernet address
*			-n	specify network in address build
*			-c	specify internet address for conversion
*			-{x}	help message
*
*	files/devices:	/dev/enpram
*			/usr/nserve/domain
*
************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <CMC/enpram.h>
#include "netaddr.h"
/*
/***********************************************************************
*
*	File:		netaddr.c	
*	Function:	main(argc, argv)
*	Purpose:	invocation and option processing
*
************************************************************************/

main(argc, argv)
  int argc;
  char *argv[];
{
	register c, enpram_fd, domain_fd;
	extern char *optarg;
	extern int optind, opterr;
	int header = NULL;			/* header activer flag */
	int mkaddr = NULL;			/* build inet address  */
	int fatfinger = NULL;			/* error on invocation */
	char *netid = NULL;			/* pointer to netid    */
	char *conv_addr = NULL;			/* conversion argument */
	register char *domain;			/* pointer to domain   */

	while((c = getopt(argc, argv, "han:c:")) != -1)
	{
		switch(c) {
			case 'h':
				header++;
				break;
			case 'a':
				mkaddr++;
				break;
			case 'n':
				netid = optarg;
				break;
			case 'c':
				conv_addr = optarg;
				break;
		}
	}

	/* open /usr/nserve/domain and get domain name */
	if((domain_fd = open("/usr/nserve/domain", O_RDONLY)) == -1)
		domain = DOMAIN_DEF;
	else {
		read(domain_fd, domain_buf, 80);
		domain = domain_buf;
		close(domain_fd);
	}

	/* if converting - place address in structure and
	   setup ethernet as null */
	if (conv_addr) {
		i2struc(conv_addr);
		mfg_code[0] = mfg_code[1] = mfg_code[2] = NULL;
		enet = eserial[0] = eserial[1] = NULL;
	} else {
		/* open /dev/enpram and read inet and enet addresses */
		if((enpram_fd = open("/dev/enpram", O_RDONLY)) == -1) {
			perror(ERR_OPEN_ENPRAM);
			exit(errno);
		}

		if(ioctl(enpram_fd, ENPIOC_GADDR, &iocbuf)) {
			if (!(header)) /* error output for no board */
				printf("%s %s %s %s %s\n",
				       zeros,zeros,zeros,domain,zeros);
			perror(ERR_IOCTL_ENPRAM);
			exit(errno);
		}
		close(enpram_fd);
	}

	/* perform routines based on options */
	if(mkaddr)
		makeinet(netid);

	sprintf(enet_asc,"%02x%02x%02x%02x%02x%02x",mfg_code[0],mfg_code[1],
			       mfg_code[2],enet,eserial[0],eserial[1]);

	sprintf(inet_asc,"%02d.%02d.%02d.%02d",inet,iserial[0],iserial[1],
				       iserial[2]);
	sprintf(inet_hex,"%02x%02x%02x%02x",inet,iserial[0],iserial[1],
				       iserial[2]);

	if(header) {
		printf("Ethernet address = 0x%s\n", enet_asc);
		printf("Internet address = %s\n", inet_asc);
		printf("RFS Domain name  = %s\n", domain);
		printf("Network id       = %02d\n",inet);
	} else
		printf("%s %s %s %02d %s\n",
		      inet_asc,enet_asc,domain,inet,inet_hex);
}
/*
/***********************************************************************
*
*	File:		netaddr.c	
*	Function:	makeinet(netid)
*	Purpose:	make internet address from ethernet address
*
************************************************************************/

makeinet(netid)
  char *netid;
{
	if(netid != NULL)
		inet = (char)((strtol(netid,(char **)NULL,NULL) & 0xff));
	else
		inet = enet;

	iserial[0] = (eserial[0] & 0xf0) >> 4;
	iserial[1] = (eserial[0] & 0x0f);
	iserial[2] = eserial[1];
}
/*
/***********************************************************************
*
*	File:		netaddr.c	
*	Function:	i2struc(inetaddr)
*	Purpose:	fill internet portion of structure from ascii inet
*
************************************************************************/

i2struc(addr)
  char *addr;
{
	register char *ptr;
	char t[4];

	ptr = strtok(addr,".");
	inet = (uchar)(strtol(strncpy(t,ptr,3),(char **) NULL, 10));
	ptr = strtok(NULL,".");
	iserial[0] = (uchar)(strtol(strncpy(t,ptr,3),(char **) NULL, 10));
	ptr = strtok(NULL,".");
	iserial[1] = (uchar)(strtol(strncpy(t,ptr,3),(char **) NULL, 10));
	ptr = strtok(NULL,".");
	iserial[2] = (uchar)(strtol(strncpy(t,ptr,3),(char **) NULL, 10));
}

