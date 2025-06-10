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
/***  FUNCTION:  fsblockio - seek into the filesystem and either   ***/
/***                         read or write one block of data.      ***/
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
/***     begin seek_and_then_read_or_write_a_block                 ***/
/***                                                               ***/
/***        seek into the filesystem and                           ***/
/***                                                               ***/
/***        either read a block,                                   ***/
/***                                                               ***/
/***        write a block, or                                      ***/
/***                                                               ***/
/***        return an error on bad operation type, or              ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end seek_and_then_read_or_write_a_block                   ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  This routine is NOT for general use.  It assumes     ***/
/***          that all reads and writes return exactly the number  ***/
/***          of bytes to be read or written, respectively.  This  ***/
/***          suits IPRO perfectly but does not reflect standard   ***/
/***          reality.                                             ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  all input/output will be single, whole-block   ***/
/***                reads/writes.                                  ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/fsblockio.c              ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
fsblockio( verbose, mesgtype, cmdname, fsname, mesgbuf,
                          fsfd, logfp, bsize, operation, buf, fsblkno )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR mesgbuf;
   INT     fsfd;
   FILEPTR logfp;
   LONG    bsize;
   LONG    operation;
   CHARPTR buf;
   LONG    fsblkno;
{
   LONG    offset;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin seek_and_then_read_or_write_a_block                */
   /*                                                          */
   /************************************************************/

   if ( ( fsfd EQ CLOSED )     OR
                  ( bsize LE ZERO )      OR
                            ( operation LT ZERO )  OR
                                      ( buf EQ NULL )        OR
                                                  ( fsblkno LT ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                "fsblockio: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* seek into the filesystem and                          */
      /*                                                       */
      /*********************************************************/

   offset = fsblkno * bsize;

   if ( lseek( fsfd, offset, FILEBEGIN ) NE offset )
   {
      sprintf( mesgbuf, "fsblockio: bad seek on block number %ld %ld.",
                                                      fsblkno, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* either read a block,                                  */
      /*                                                       */
      /*********************************************************/

   if ( operation EQ READ )
   {
      if ( read( fsfd, buf, bsize ) NE bsize )
      {
         sprintf( mesgbuf,
                    "fsblockio: bad read on block number %ld %ld.",
                                                      fsblkno, bsize );

         printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

         return( FALSE );
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* write a block, or                                     */
      /*                                                       */
      /*********************************************************/

   else if ( operation EQ WRITE )
   {
      if ( write( fsfd, buf, bsize ) NE bsize )
      {
         sprintf( mesgbuf,
                    "fsblockio: bad write on block number %ld %ld.",
                                                      fsblkno, bsize );

         printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

         return( FALSE );
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* return an error on bad operation type, or             */
      /*                                                       */
      /*********************************************************/

   else
   {
      sprintf( mesgbuf, "fsblockio: bad operation type %ld %ld %ld.",
                                           operation, fsblkno, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
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
   /* end seek_and_then_read_or_write_a_block                  */
   /*                                                          */
   /************************************************************/
}
