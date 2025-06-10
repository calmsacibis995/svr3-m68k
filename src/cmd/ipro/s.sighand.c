h45621
s 00001/00001/00125
d D 1.6 86/12/18 11:39:12 norm 6 5
c 
e
s 00001/00001/00125
d D 1.5 86/12/17 17:34:17 norm 5 4
c 
e
s 00001/00001/00125
d D 1.4 86/12/17 17:16:56 norm 4 3
c 
e
s 00000/00000/00126
d D 1.3 86/11/11 22:39:47 norm 3 2
c 
e
s 00000/00000/00126
d D 1.2 86/11/11 14:52:36 norm 2 1
c 
e
s 00126/00000/00000
d D 1.1 86/11/03 09:38:39 norm 1 0
c date and time created 86/11/03 09:38:39 by norm
e
u
U
t
T
I 1
/*********************************************************************/
/*********************************************************************/
/***                                                               ***/
/***                                                               ***/
D 4
/***  (C) Copyright  Motorola, Inc.  1985, 1986                    ***/
E 4
I 4
/***  (C) Copyright  Motorola, Inc.  1985, 1986, 1987              ***/
E 4
/***  All Rights Reserved                                          ***/
/***  Motorola Confidential/Proprietary                            ***/
/***                                                               ***/
/***                                                               ***/
/***  SOFTWARE PRODUCT:  IPRO                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  FUNCTION:  sighand - signal handling function.               ***/
/***                                                               ***/
/***                                                               ***/
/***  AUTHOR:  Norman L. Nealy                                     ***/
/***           Senior Software Development Engineer                ***/
D 5
/***           Motorola Computer Systems, Inc.                     ***/
E 5
I 5
/***           Motorola Microcomputer Division                     ***/
E 5
/***           3013 South 52nd Street  MS-J3                       ***/
/***           Tempe, Arizona  85282                               ***/
/***           (602) 438-5724/5600                                 ***/
/***                                                               ***/
/***                                                               ***/
/***  ORIGINAL DATE:  November 1, 1985                             ***/
/***                                                               ***/
/***                                                               ***/
/***  ALGORITHM:                                                   ***/
/***                                                               ***/
/***     begin signal_handler                                      ***/
/***                                                               ***/
/***        reset the just caught signal if signal specified       ***/
/***                                                               ***/
/***     end signal_handler                                        ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  the passed-in signal.                        ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/sighand.c                ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


D 6
#include "./iprodefs.h"
E 6
I 6
#include "iprodefs.h"
E 6


   INT
sighand( sig )

   INT sig;
{

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin signal_handler                                     */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* reset the just caught signal if signal specified      */
      /*                                                       */
      /*********************************************************/

   if ( sig EQ SIGHUP )
   {
      signal( SIGHUP, sighand );

      fprintf( stderr,
                 "%s: sighand: hangup signal caught and reset.\n",
                                                       INFORMATIVE );
   }
   else if ( sig EQ SIGINT )
   {
      signal( SIGINT, sighand );

      fprintf( stderr,
             "%s: sighand: interrupt signal caught and reset.\n",
                                                       INFORMATIVE );
   }
   else if ( sig EQ SIGQUIT )
   {
      signal( SIGQUIT, sighand );

      fprintf( stderr,
                  "%s: sighand: quit signal caught and reset.\n",
                                                       INFORMATIVE );
   }
   else if ( sig EQ SIGALRM )
   {
      signal( SIGALRM, sighand );

      fprintf( stderr,
            "%s: sighand: alarm clock signal caught and reset.\n",
                                                       INFORMATIVE );
   }
   else if ( sig EQ SIGTERM )
   {
      signal( SIGTERM, sighand );

      fprintf( stderr,
     "%s: sighand: software terminate signal caught and reset.\n",
                                                       INFORMATIVE );
   }
   else
   {
      signal( sig, SIG_DFL );

      fprintf( stderr,
           "%s: sighand: signal, %d, caught but NOT reset.\n",
                                                  INFORMATIVE, sig );
   }

   return( sig );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end signal_handler                                       */
   /*                                                          */
   /************************************************************/
}
E 1
