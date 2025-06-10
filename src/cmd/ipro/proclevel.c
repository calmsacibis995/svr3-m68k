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
/***  FUNCTION:  proclevel - recursively search a file block tree  ***/
/***                         for a specific block number.          ***/
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
/***     begin process_level                                       ***/
/***                                                               ***/
/***        get a working block                                    ***/
/***                                                               ***/
/***        either return the data block number, or                ***/
/***                                                               ***/
/***        read in the indirect block,                            ***/
/***                                                               ***/
/***        pick up the correct index of the indirect block at     ***/
/***        this level, and                                        ***/
/***                                                               ***/
/***        recursively search down the file block tree for the    ***/
/***        data block number                                      ***/
/***                                                               ***/
/***     end process_level                                         ***/
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
/***  NOTES:  This is a recursive routine.                         ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/proclevel.c              ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   LONG
proclevel( verbose, mesgtype, cmdname, fsname, mesgbuf,
                        fsfd, logfp, bsize, nindir, level, longptr,
                                         blkseqno, wrkblks, wrkblkidx )

   BOOLEAN verbose;
   CHARPTR mesgtype;
   CHARPTR cmdname;
   CHARPTR fsname;
   CHARPTR mesgbuf;
   INT     fsfd;
   FILEPTR logfp;
   LONG    bsize;
   LONG    nindir;
   LONG    level;
   LONGPTR longptr;
   LONG    blkseqno;
   CHARPTR wrkblks;
   LONG    wrkblkidx;
{
   INDPTR  indirect;
   LONG    indindex;
   LONG    longpow();

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin process_level                                      */
   /*                                                          */
   /************************************************************/

   if ( ( fsfd EQ CLOSED )    OR
             ( bsize LE ZERO )     OR
                  ( nindir LE ZERO )    OR
                       ( level LT ZERO )     OR
                            ( longptr EQ NULL )   OR
                                 ( blkseqno LT ZERO )  OR
                                      ( wrkblks EQ NULL )   OR
                                                ( wrkblkidx LT ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                "proclevel: bad parameters/variables not initialized.",
                                                               logfp );

      return( M_ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get a working block                                   */
      /*                                                       */
      /*********************************************************/

   if ( (indirect = (INDPTR)( GETWRKBLK( wrkblks, WRKBLKCNT,
                                        wrkblkidx, bsize ) )) EQ NULL )
   {
      sprintf( mesgbuf, 
            "proclevel: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( M_TWO );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* either return the data block number, or               */
      /*                                                       */
      /*********************************************************/

   if ( ( level LE ZERO )  OR  ( *longptr LE ZERO ) )
   {                                      /*** fsck(1) will return ***/
      return( *longptr );                 /*** a POSSIBLE FILE     ***/
   }                                      /*** SIZE ERROR on zero  ***/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* read in the indirect block,                           */
      /*                                                       */
      /* pick up the correct index of the indirect block at    */
      /* this level, and                                       */
      /*                                                       */
      /* recursively search down the file block tree for the   */
      /* data block number                                     */
      /*                                                       */
      /*********************************************************/

   if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                               mesgbuf, fsfd, logfp, bsize, READ,
                                        (CHARPTR)indirect, *longptr ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
                                   "proclevel: read indirect block.",
                                                               logfp );

      return( M_THREE );
   }

   indindex = blkseqno / longpow( nindir, level - ONE ) % nindir;

   return( proclevel( verbose, mesgtype, cmdname, fsname,
                           mesgbuf, fsfd, logfp, bsize, nindir,
                                level - ONE, indirect + indindex,
                                      blkseqno, wrkblks, wrkblkidx ) );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end process_level                                        */
   /*                                                          */
   /************************************************************/
}
