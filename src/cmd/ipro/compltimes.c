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
/***  FUNCTION:  compltimes - print estimated completion times.    ***/
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
/***     begin print_estimated_completion_times                    ***/
/***                                                               ***/
/***        print the number of out-of-place, in-use blocks,       ***/
/***                                                               ***/
/***        print the number of free blocks,                       ***/
/***                                                               ***/
/***        print the number of new inode blocks,                  ***/
/***                                                               ***/
/***        print the number of old filesystem blocks,             ***/
/***                                                               ***/
/***        print the estimated completion time (ensuring that     ***/
/***        the estimated completion time will not be the same as  ***/
/***        the current time for a small number of out-of-place    ***/
/***        blocks), and                                           ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end print_estimated_completion_times                      ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/compltimes.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
compltimes( verbose, mesgtype, cmdname, fsname, mesgbuf,
                       logfp, inodesect, fsblkcnt, ninodesect,
                                   nfsblkcnt, ntotfreblks, outofplace )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR mesgbuf;
   FILEPTR logfp;
   LONG    inodesect;
   LONG    fsblkcnt;
   LONG    ninodesect;
   LONG    nfsblkcnt;
   LONG    ntotfreblks;
   LONG    outofplace;
{
   LONG    ct;
   LONG    ect;
   LONG    hour;
   LONG    minute;
   LONG    second;
   LONG    ioblkcnt;
   LONG    inoblkdiff;
   LONG    fsblkdiff;
   LONG    printtime();

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin print_estimated_completion_times                   */
   /*                                                          */
   /************************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                       "Printing Estimated Completion Times.", logfp );

   if ( ( inodesect LE ZERO )    OR
               ( fsblkcnt LE ZERO )     OR
                      ( ninodesect LE ZERO )   OR
                             ( nfsblkcnt LE ZERO )    OR
                                    ( ntotfreblks LT ZERO )  OR
                                               ( outofplace LT ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
               "compltimes: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* print the number of out-of-place, in-use blocks,      */
      /*                                                       */
      /* print the number of free blocks,                      */
      /*                                                       */
      /* print the number of new inode blocks,                 */
      /*                                                       */
      /* print the number of old filesystem blocks,            */
      /*                                                       */
      /*********************************************************/

   sprintf( mesgbuf, "OUT-OF-PLACE, IN-USE BLOCKS: %8ld", outofplace );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   sprintf( mesgbuf, "FREE BLOCKS:                 %8ld",
                                                         ntotfreblks );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   ioblkcnt = TWO * outofplace + ntotfreblks;

   if ( (inoblkdiff = ninodesect - inodesect) GT ZERO )
   {
      ioblkcnt += inoblkdiff;
   }
   else
   {
      inoblkdiff = ZERO;
   }

   if ( (fsblkdiff = fsblkcnt - nfsblkcnt) GT ZERO )
   {
      ioblkcnt += fsblkdiff;
   }
   else
   {
      fsblkdiff = ZERO;
   }

   sprintf( mesgbuf, "NEW INODE BLOCKS:            %8ld", inoblkdiff );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   sprintf( mesgbuf, "OLD FILESYSTEM BLOCKS:       %8ld", fsblkdiff );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* print the estimated completion time (ensuring that    */
      /* the estimated completion time will not be the same as */
      /* the current time for a small number of out-of-place   */
      /* blocks), and                                          */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   ct = printtime( FALSE, mesgtype, cmdname, fsname, mesgbuf, logfp );

   ect = (LONG)( ct + ioblkcnt / TWO / MICRO_BPM * SECPMIN +
                               ( ( ioblkcnt GT ZERO ) ? ONE : ZERO ) );

   hour = ect % SECPDAY / SECPHOUR;

   minute = ect % SECPHOUR / SECPMIN;

   second = ect % SECPMIN;

   sprintf( mesgbuf,
           "ESTIMATED COMPLETION TIME (MICROPOLIS): %02ld:%02ld:%02ld",
                                                hour, minute, second );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   ect = (LONG)( ct + ioblkcnt / TWO / SMD_BPM * SECPMIN +
                             ( ( ioblkcnt GT ZERO ) ? ONE : ZERO ) );

   hour = ect % SECPDAY / SECPHOUR;

   minute = ect % SECPHOUR / SECPMIN;

   second = ect % SECPMIN;

   sprintf( mesgbuf,
           "ESTIMATED COMPLETION TIME (SMD):        %02ld:%02ld:%02ld",
                                                hour, minute, second );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end print_estimated_completion_times                     */
   /*                                                          */
   /************************************************************/
}
