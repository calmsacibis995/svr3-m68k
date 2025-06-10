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
 *	ioctl definitions for enpram driver
 */

# define mkioctl(type,value) (('type'<<8)|value)

# define ENPIOC_JUMP		mkioctl(N,1)
# define ENPIOC_RESET		mkioctl(N,2)
# define ENPIOC_FRESET		mkioctl(N,3)
# define ENPIOC_GADDR		mkioctl(N,4)
