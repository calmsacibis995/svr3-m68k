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
 * This file contains defines and such that are specific to individual
 * systems.
 *
 * Before including this file, TARGET_SYSTEM should have been #defined
 * to the name of the system we're running on.
 */

/* Target-system specific values follow. */

#if TARGET_SYSTEM == VALID
#define void	int
#endif
