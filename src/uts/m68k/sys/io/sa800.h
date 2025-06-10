/*	@(#)sa800.h	6.5		*/
/*
 * Paritioning for floppy disks supported by sa800 standard compatible
 * controllers.
 */

/* defines partition 0 for single sided and partition 1 for double sided */
/* This form now obsolete but maintained for compatibility old drivers (ud.c) */
#define FLOPY\
	500,	0,\
	1000,	0,\
	0,0,0,0,0,0,0,0,0,0,0,0,

#define SA800FL\
	 988,	1,	/* double sided, single density */	\
	 487,	2,	/* single sided, single density */	\
	1976,	1,	/* double sided, double density */	\
	 975,	2,	/* single sided, double density */	\
	   0,	0,	/* unused */				\
	   0,	0,	/* unused */				\
	   0,	0,	/* unused */				\
	2002,	0,	/* largest floppy size */


/*
 * defines partitions 0 and 7 for double sided, single density
 * 8-inch floppies, supported by the id driver (mvme319).
 */
#define IDSA800FL\
	 988,	13, \
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
  	 1001,	 0,


/*
 * defines partition 0 and 7 for CIPHER DATA PRODUCTS
 * FloppyTape (STREAMER) supported by the id driver (mvme319)
 * (interface is sa800 compatible)
 */
#define FTAPE\
	46848,	32,\
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
	47040,	0,


/* space holder in table for unused minor device */
/* used in conjunction with FTAPE which uses up two minor devs on vme319 */
#define NODISK\
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
