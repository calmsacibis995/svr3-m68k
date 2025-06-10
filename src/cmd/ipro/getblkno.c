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
/***  FUNCTION:  getblkno - get the number of a file block as a    ***/
/***                        function of the block's sequence       ***/
/***                        number within the file.                ***/
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
/***     begin get_file_block_number                               ***/
/***                                                               ***/
/***        check the data block count and block sequence number   ***/
/***        for validity                                           ***/
/***                                                               ***/
/***        if the sequence number describes a direct block then   ***/
/***                                                               ***/
/***           return the block number from the direct block       ***/
/***           addresses in the inode                              ***/
/***                                                               ***/
/***        else if the sequence number describes a block under    ***/
/***                          the first inode indirect block then  ***/
/***                                                               ***/
/***           seek down the inode's first indirect block tree     ***/
/***           and return the block number                         ***/
/***                                                               ***/
/***        else if the sequence number describes a block under    ***/
/***                         the second inode indirect block then  ***/
/***                                                               ***/
/***           seek down the inode's second indirect block tree    ***/
/***           and return the block number                         ***/
/***                                                               ***/
/***        else if the sequence number describes a block under    ***/
/***                         the third inode indirect block then   ***/
/***                                                               ***/
/***           seek down the inode's third indirect block tree     ***/
/***           and return the block number                         ***/
/***                                                               ***/
/***        else                                                   ***/
/***                                                               ***/
/***           the block sequence number is out-of-range and       ***/
/***           return an error                                     ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***     end get_file_block_number                                 ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  less than 0 - error.                         ***/
/***                                                               ***/
/***                  0 - either a data or indirect block number   ***/
/***                      was found to be zero.                    ***/
/***                                                               ***/
/***                  greater than 0 - legitimate block number.    ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/getblkno.c               ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   LONG
getblkno( verbose, mesgtype, cmdname, fsname, mesgbuf,
                   fsfd, logfp, bsize, nindir, longbufptr,
                              datblkcnt, blkseqno, wrkblks, wrkblkidx )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR mesgbuf;
   INT     fsfd;
   FILEPTR logfp;
   LONG    bsize;
   LONG    nindir;
   LONGPTR longbufptr;
   LONG    datblkcnt;
   LONG    blkseqno;
   CHARPTR wrkblks;
   LONG    wrkblkidx;
{
   LONG    proclevel();

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin get_file_block_number                              */
   /*                                                          */
   /************************************************************/

   if ( ( fsfd EQ CLOSED )      OR
             ( bsize LE ZERO )       OR
                  ( nindir LE ZERO )      OR
                       ( longbufptr EQ NULL )  OR
                            ( datblkcnt LE ZERO )   OR
                                 ( blkseqno LT ZERO )    OR
                                      ( wrkblks EQ NULL )     OR
                                                ( wrkblkidx LT ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                 "getblkno: bad parameters/variables not initialized.",
                                                               logfp );

      return( M_ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* check the data block count and block sequence number  */
      /* for validity                                          */
      /*                                                       */
      /*********************************************************/

   if ( datblkcnt LE ZERO )
   {
      sprintf( mesgbuf,
                 "getblkno: data block count, %ld, is <= 0 %ld.",
                                                 datblkcnt, blkseqno );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( M_TWO );
   }

   if ( NOT INRANGE( ZERO, blkseqno, ( datblkcnt - ONE ) ) )
   {
      sprintf( mesgbuf, 
      "getblkno: block sequence number, %ld, must be >= 0 and <= %ld.",
                                           blkseqno, datblkcnt - ONE );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( M_THREE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if the sequence number describes a direct block then  */
      /*                                                       */
      /*    return the block number from the direct block      */
      /*    addresses in the inode                             */
      /*                                                       */
      /*********************************************************/

   if ( blkseqno LT DIRECT )
   {
      return( proclevel( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, fsfd, logfp, bsize,
                                 nindir, ZERO, longbufptr + blkseqno,
                                      blkseqno, wrkblks, wrkblkidx ) );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the sequence number describes a block under   */
      /*                   the first inode indirect block then */
      /*                                                       */
      /*    seek down the inode's first indirect block tree    */
      /*    and return the block number                        */
      /*                                                       */
      /*********************************************************/

   else if ( (blkseqno -= DIRECT) LT nindir )
   {
      return( proclevel( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, fsfd, logfp, bsize,
                                 nindir, ONE, longbufptr + IND_1,
                                      blkseqno, wrkblks, wrkblkidx ) );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the sequence number describes a block under   */
      /*                  the second inode indirect block then */
      /*                                                       */
      /*    seek down the inode's second indirect block tree   */
      /*    and return the block number                        */
      /*                                                       */
      /*********************************************************/

   else if ( (blkseqno -= nindir) LT SQR( nindir ) )
   {
      return( proclevel( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, fsfd, logfp, bsize,
                                 nindir, TWO, longbufptr + IND_2,
                                      blkseqno, wrkblks, wrkblkidx ) );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else if the sequence number describes a block under   */
      /*                  the third inode indirect block then  */
      /*                                                       */
      /*    seek down the inode's third indirect block tree    */
      /*    and return the block number                        */
      /*                                                       */
      /*********************************************************/

   else if ( (blkseqno -= SQR( nindir )) LT CUBE( nindir ) )
   {
      return( proclevel( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, fsfd, logfp, bsize,
                                 nindir, THREE, longbufptr + IND_3,
                                      blkseqno, wrkblks, wrkblkidx ) );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* else                                                  */
      /*                                                       */
      /*    the block sequence number is out-of-range and      */
      /*    return an error                                    */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /*********************************************************/

   else
   {
      sprintf( mesgbuf,
                 "getblkno: block sequence number %ld too large.",
                          blkseqno + DIRECT + nindir + SQR( nindir ) );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( M_FOUR );
   }

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end get_file_block_number                                */
   /*                                                          */
   /************************************************************/
}
