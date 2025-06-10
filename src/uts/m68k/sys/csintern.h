
/* @(#)csintern.h	1.7 */
/* "@(#) Copyright (C) 1985  by  Four Phase (ISG) of Motorola, Inc." */

#ifndef csintern_h
#define csintern_h

/* Define the standard internal char set translation tables which are
   used to convert to the Motorola Private char set 040, or to convert
   towards a standard representation avoinding char sets 040, 360 and 361.

   These are compiled by csinit(3) from the internal char set translation
   tables 'topri' and 'tostd'.  
 */

/*#include <sys/cstty.h> */

#include "cs_topri.h"

#include "cs_tostd.h"

#endif /* csintern_h */
