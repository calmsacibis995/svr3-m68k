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
/***  SUBROUTINE:  terminate - close the filesystem, free up all   ***/
/***                           allocated tables, and exit the      ***/
/***                           program with the passed-in status.  ***/
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
/***     begin terminate_program                                   ***/
/***                                                               ***/
/***        close the indirect cmd line/std input file, if open,   ***/
/***                                                               ***/
/***        cleanup,                                               ***/
/***                                                               ***/
/***        update all system disks, and                           ***/
/***                                                               ***/
/***        exit with the passed-in status                         ***/
/***                                                               ***/
/***     end terminate_program                                     ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/terminate.c              ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   VOID
terminate( verbose, mesgtype, cmdname, fsname,
                     mesgbuf, fsfd, logfp, fp, tbl1ptr, tbl2ptr,
                                 tbl3ptr, tbl4ptr, tbl5ptr, exitvalue )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR mesgbuf;
   INT     fsfd;
   FILEPTR logfp;
   FILEPTR fp;
   CHARPTR tbl1ptr;
   CHARPTR tbl2ptr;
   CHARPTR tbl3ptr;
   CHARPTR tbl4ptr;
   CHARPTR tbl5ptr;
   INT     exitvalue;
{

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin terminate_program                                  */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* close the indirect cmd line/std input file, if open,  */
      /*                                                       */
      /* cleanup,                                              */
      /*                                                       */
      /* update all system disks, and                          */
      /*                                                       */
      /* exit with the passed-in status                        */
      /*                                                       */
      /*********************************************************/

   if ( fp NE NULL )
   {
      fclose( fp );
   }

   if ( NOT cleanup( verbose, mesgtype, cmdname, fsname,
                               mesgbuf, fsfd, logfp, tbl1ptr, tbl2ptr,
                                          tbl3ptr, tbl4ptr, tbl5ptr ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                                 "terminate: failed cleanup.", logfp );
   }

   if ( exitvalue NE NORMAL )
   {
      sprintf( mesgbuf, "terminate: exiting with status, %ld.",
                                                           exitvalue );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );
   }

   if ( logfp NE NULL )
   {
      fclose( logfp );
   }

   sync();
   sync();
   sync();

   exit( exitvalue );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end terminate_program                                    */
   /*                                                          */
   /************************************************************/
}
