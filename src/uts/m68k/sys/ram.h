/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/
/* @(#)ram.h	6.1	 */

/* initial probe structure */

struct probe {
	char	*probe_addr;	/* address to be probed */
	short	probe_byte;	/* byte to write into probe_addr */
};

struct ram {
	int	dummy1;
	int	dummy2;
	int	dummy3;
	int	dummy4;
	};

/* (probe_addr is only read from if read if probe_byte < 0) */
