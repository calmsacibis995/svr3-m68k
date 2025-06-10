/*********************************************************************/
/*********************************************************************/
/***                                                               ***/
/***                                                               ***/
/***  (C) Copyright  Motorola, Inc.  1985, 1986, 1987              ***/
/***  All Rights Reserved                                          ***/
/***  Motorola Confidential/Proprietary                            ***/
/***                                                               ***/
/***                                                               ***/
/***  SOFTWARE PRODUCT:  IPRO                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  FUNCTION:  cleanup - close the filesystem and free up all    ***/
/***                       allocated tables.                       ***/
/***                                                               ***/
/***                                                               ***/
/***  AUTHOR:  Norman L. Nealy                                     ***/
/***           Senior Software Development Engineer                ***/
/***           Motorola Microcomputer Division                     ***/
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
/***     begin cleanup                                             ***/
/***                                                               ***/
/***        close the filesystem if it is open,                    ***/
/***                                                               ***/
/***        free up each table that is allocated, and              ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end cleanup                                               ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/cleanup.c                ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
cleanup( verbose, mesgtype, cmdname, fsname, mesgbuf, fsfd, logfp,
                          tbl1ptr, tbl2ptr, tbl3ptr, tbl4ptr, tbl5ptr )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR mesgbuf;
   INT     fsfd;
   FILEPTR logfp;
   CHARPTR tbl1ptr;
   CHARPTR tbl2ptr;
   CHARPTR tbl3ptr;
   CHARPTR tbl4ptr;
   CHARPTR tbl5ptr;
{

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin cleanup                                            */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* close the filesystem if it is open,                   */
      /*                                                       */
      /*********************************************************/

   if ( fsfd NE CLOSED )
   {
      close( fsfd );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* free up each table that is allocated, and             */
      /*                                                       */
      /*********************************************************/

   if ( tbl1ptr NE NULL )
   {
      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, logfp, DEALLOC_MEM, NULL,
                                    NULL, tbl1ptr, NULL, NULL, NULL ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                              "cleanup: can't free table #1.", logfp );

         return( FALSE );
      }
   }

   if ( tbl2ptr NE NULL )
   {
      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, logfp, DEALLOC_MEM, NULL,
                                    NULL, tbl2ptr, NULL, NULL, NULL ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                              "cleanup: can't free table #2.", logfp );

         return( FALSE );
      }
   }

   if ( tbl3ptr NE NULL )
   {
      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, logfp, DEALLOC_MEM, NULL,
                                    NULL, tbl3ptr, NULL, NULL, NULL ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                              "cleanup: can't free table #3.", logfp );

         return( FALSE );
      }
   }

   if ( tbl4ptr NE NULL )
   {
      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, logfp, DEALLOC_MEM, NULL,
                                    NULL, tbl4ptr, NULL, NULL, NULL ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                              "cleanup: can't free table #4.", logfp );

         return( FALSE );
      }
   }

   if ( tbl5ptr NE NULL )
   {
      if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, logfp, DEALLOC_MEM, NULL,
                                    NULL, tbl5ptr, NULL, NULL, NULL ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                              "cleanup: can't free table #5.", logfp );

         return( FALSE );
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end cleanup                                              */
   /*                                                          */
   /************************************************************/
}
