/*		Copyright (c) 1985 AT&T		*/
/*		All Rights Reserved		*/

/*	@(#)sa400.h	6.3		*/
/*
 * Paritioning for floppy disks supported by sa400 standard compatible
 * controllers.
 */

/* WFLOPPY is defined for backward compatibility with the VME/10 */
/* defines partition 0 for double sided double density */
#define WFLOPPY\
	1264,	12,\
	0,0,0,0,0,0,0,0,0,0,0,0,1276,0,

/*
 *	sa400 standard compatible floppy
 */

#define	SA400FL\
	 632,	1,	/* double sided, single density */	\
	 316,	1,	/* single sided, single density */	\
	1264,	1,	/* double sided, double density */	\
	 632,	1,	/* single sided, double density */	\
	   0,	0,	/* unused */				\
	   0,	0,	/* unused */				\
	   0,	0,	/* unused */				\
	1276,	0,	/* old style double sided, double density */
