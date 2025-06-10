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
/***  FUNCTION:  loadinotbl - load the inode table with            ***/
/***                          destination inode numbers of all     ***/
/***                          valid inodes.                        ***/
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
/***     begin load_inode_table                                    ***/
/***                                                               ***/
/***        get a working block                                    ***/
/***                                                               ***/
/***        for each inode block do                                ***/
/***                                                               ***/
/***           read in the inode block and                         ***/
/***                                                               ***/
/***           for each inode in the inode block do                ***/
/***                                                               ***/
/***              if the inode is free then                        ***/
/***                                                               ***/
/***                 skip the inode                                ***/
/***                                                               ***/
/***              else if the inode is a directory then            ***/
/***                                                               ***/
/***                 keep a running size of the largest directory  ***/
/***                 and load the inode number into the            ***/
/***                 directory-list table                          ***/
/***                                                               ***/
/***              endif                                            ***/
/***                                                               ***/
/***              regardless, load the source inode number and     ***/
/***              the inode modification time of the valid inode   ***/
/***              into the inode table                             ***/
/***                                                               ***/
/***           end for each inode in the inode block               ***/
/***                                                               ***/
/***        end for each inode block                               ***/
/***                                                               ***/
/***        terminate the valid entries in the inode and           ***/
/***        directory-list tables (defensive measures even though  ***/
/***        the tables were initialized to all zeroes)             ***/
/***                                                               ***/
/***        determine the block count of the largest directory     ***/
/***        and sort the inode table with respect to ascending     ***/
/***        modification times (skipping the first position and    ***/
/***        the first two inodes, the first inode is reserved and  ***/
/***        the second inode is the root directory, and must not   ***/
/***        be disturbed)                                          ***/
/***                                                               ***/
/***        load the destination inode numbers into the inode      ***/
/***        table                                                  ***/
/***                                                               ***/
/***        return failure on bad inode processing count or        ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end load_inode_table                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  the first entry is always null since the       ***/
/***                inode table contains real inode numbers, i.e.  ***/
/***                greater than zero.                             ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/loadinotbl.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
loadinotbl( verbose, mesgtype, cmdname, fsname, mesgbuf,
               fsfd, logfp, inodetable, dirlsttbl, bsize, inopb,
                  inoblkcnt, valinodes, lrgdirptr, wrkblks, wrkblkidx )

   BOOLEAN    verbose;
   CHARPTR    mesgtype;
   CHARPTR    cmdname;
   CHARPTR    fsname;
   CHARPTR    mesgbuf;
   INT        fsfd;
   FILEPTR    logfp;
   INODEINFO *inodetable;
   INO_T     *dirlsttbl;
   LONG       bsize;
   LONG       inopb;
   LONG       inoblkcnt;
   LONG       valinodes;
   LONGPTR    lrgdirptr;
   CHARPTR    wrkblks;
   LONG       wrkblkidx;
{
   LONG       inoblkno;
   INODEPTR   inodeblock;
   LONG       inoblkidx;
   LONG       inum;
   INT        mode;
   LONG       dirlstidx  = ZERO;
   LONG       valinocnt  = ZERO;
   DOUBLE     ceil();
   INT        inodecmp();
   LONG       inotblidx;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin load_inode_table                                   */
   /*                                                          */
   /************************************************************/

   sprintf( mesgbuf, "Loading The %s Table.", INOTBLNAME );

   printmesg( verbose, INFORMATIVE, cmdname, fsname, mesgbuf, logfp );

   if ( ( fsfd EQ CLOSED )      OR
           ( inodetable EQ NULL )  OR
              ( dirlsttbl EQ NULL )   OR
                 ( bsize LE ZERO )       OR
                    ( inopb LE ZERO )       OR
                       ( inoblkcnt LE ZERO )   OR
                          ( valinodes LE ZERO )   OR
                             ( lrgdirptr EQ NULL )   OR
                                ( wrkblks EQ NULL )     OR
                                   ( wrkblkidx LT ZERO )   OR
                                      ( dirlstidx NE ZERO )   OR
                                                ( valinocnt NE ZERO ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
               "loadinotbl: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get a working block                                   */
      /*                                                       */
      /*********************************************************/

   if ( (inodeblock = (INODEPTR)( GETWRKBLK( wrkblks, WRKBLKCNT,
                                        wrkblkidx, bsize ) )) EQ NULL )
   {
      sprintf( mesgbuf, 
           "loadinotbl: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* for each inode block do                               */
      /*                                                       */
      /*********************************************************/

   for ( inoblkno = ZERO; inoblkno LT inoblkcnt; inoblkno++ )
   {
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* read in the inode block and                        */
         /*                                                    */
         /* for each inode in the inode block do               */
         /*                                                    */
         /******************************************************/

      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize,
                                        READ, (CHARPTR)inodeblock,
                                               INOBLKOFF + inoblkno ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                                       "loadinotbl: read inode block.",
                                                               logfp );

         return( FALSE );
      }

      for ( inoblkidx = ZERO,
                      inum = inoblkno * inopb + inoblkidx + ONE;
                                    inoblkidx LT inopb;
                                                    inoblkidx++,
                                                               inum++ )
      {
            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* if the inode is free then                       */
            /*                                                 */
            /*    skip the inode                               */
            /*                                                 */
            /***************************************************/

         if ( (mode = inodeblock[ inoblkidx ].di_mode) EQ ZERO )
         {
            continue;
         }

            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* else if the inode is a directory then           */
            /*                                                 */
            /*    keep a running size of the largest directory */
            /*    and load the inode number into the           */
            /*    directory-list table                         */
            /*                                                 */
            /***************************************************/

         if ( isdir( mode ) )
         {
            *lrgdirptr = MAX2( *lrgdirptr,
                                     inodeblock[ inoblkidx ].di_size );

            dirlsttbl[ dirlstidx++ ] = inum;
         }

            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* endif                                           */
            /*                                                 */
            /* regardless, load the source inode number and    */
            /* the inode modification time of the valid inode  */
            /* into the inode table                            */
            /*                                                 */
            /***************************************************/

         inodetable[ ++valinocnt ].xchgsrc = inum;

         inodetable[ valinocnt ].modtime =
                                      inodeblock[ inoblkidx ].di_mtime;
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /*    end for each inode in the inode block              */
      /*                                                       */
      /* end for each inode block                              */
      /*                                                       */
      /* terminate the valid entries in the inode and          */
      /* directory-list tables (defensive measures even though */
      /* the tables were initialized to all zeroes)            */
      /*                                                       */
      /* determine the block count of the largest directory    */
      /* and sort the inode table with respect to ascending    */
      /* modification times (skipping the first position and   */
      /* the first two inodes, the first inode is reserved and */
      /* the second inode is the root directory, and must not  */
      /* be disturbed)                                         */
      /*                                                       */
      /*********************************************************/

   inodetable[ valinocnt + ONE ].xchgsrc = ZERO;

   dirlsttbl[ dirlstidx ] = ZERO;

   *lrgdirptr = (LONG)ceil( (DOUBLE)( *lrgdirptr ) / bsize );

   qsort( (CHARPTR)( inodetable + THREE ), valinocnt - TWO,
                                       sizeof( INODEINFO ), inodecmp );

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* load the destination inode numbers into the inode     */
      /* table                                                 */
      /*                                                       */
      /*********************************************************/

   for ( inotblidx = ONE;
            (inum = inodetable[ inotblidx ].xchgsrc) NE ZERO;
                                                          inotblidx++ )
   {
      inodetable[ inum ].xchgdst = inotblidx;

      inodetable[ inum ].dirdest = inotblidx;
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* return failure on bad inode processing count or       */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   if ( valinocnt NE valinodes )
   {
      sprintf( mesgbuf, 
                 "loadinotbl: valid inode count mismatch %ld %ld.",
                                                valinocnt, valinodes );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end load_inode_table                                     */
   /*                                                          */
   /************************************************************/
}
