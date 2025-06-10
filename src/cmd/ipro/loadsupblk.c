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
/***  FUNCTION:  loadsupblk - load the super block free list       ***/
/***                          array with the numbers of the first  ***/
/***                          blocks of the free list and load     ***/
/***                          the super block free inode array     ***/
/***                          with the numbers of the first free   ***/
/***                          inodes.                              ***/
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
/***     begin load_super_block_arrays                             ***/
/***                                                               ***/
/***        load the super block free list array with the numbers  ***/
/***        of the first blocks of the free list and zero out the  ***/
/***        remaining slots, if any                                ***/
/***                                                               ***/
/***        if there are no free list sections outside of the      ***/
/***                                             super block then  ***/
/***                                                               ***/
/***           the super block free list array serves as the       ***/
/***           terminating free list section                       ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***        load the super block free inode array with the         ***/
/***        numbers of the first free inodes and zero out the      ***/
/***        remaining slots, if any                                ***/
/***                                                               ***/
/***        load the remaining section fields,                     ***/
/***                                                               ***/
/***        update the super block last modified time field, and   ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end load_super_block_arrays                               ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/loadsupblk.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
loadsupblk( verbose, mesgtype, cmdname, fsname, mesgbuf, logfp,
                    ninodesect, nfsblkcnt, ntotfreblks,
                            ntotfresect, nsupfrecnt, ntotfreinos,
                                    nvalinodes, nsupinocnt, supblkptr )

   BOOLEAN     verbose;
   CHARPTR     mesgtype;
   CHARPTR     cmdname;
   CHARPTR     fsname;
   CHARPTR     mesgbuf;
   FILEPTR     logfp;
   LONG        ninodesect;
   LONG        nfsblkcnt;
   LONG        ntotfreblks;
   LONG        ntotfresect;
   LONG        nsupfrecnt;
   LONG        ntotfreinos;
   LONG        nvalinodes;
   LONG        nsupinocnt;
   SUPERBLOCK *supblkptr;
{
   LONG        supfreidx;
   LONG        supinoidx;
   LONG        gmtime;
   LONG        time();

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin load_super_block_arrays                            */
   /*                                                          */
   /************************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                            "Loading The Super Block Arrays.", logfp );

   if ( ( ninodesect LE ZERO )   OR
             ( nfsblkcnt LE ZERO )    OR
                  ( ntotfreblks LT ZERO )  OR
                       ( ntotfresect LT ZERO )  OR
                            ( nsupfrecnt LE ZERO )   OR
                                 ( ntotfreinos LT ZERO )  OR
                                      ( nvalinodes LE ZERO )   OR
                                           ( nsupinocnt LT ZERO )   OR
                                                ( supblkptr EQ NULL ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
               "loadsupblk: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* load the super block free list array with the numbers */
      /* of the first blocks of the free list and zero out the */
      /* remaining slots, if any                               */
      /*                                                       */
      /*********************************************************/

   supblkptr->s_nfree = nsupfrecnt;

   for ( supfreidx = ZERO; supfreidx LT nsupfrecnt; supfreidx++ )
   {
      supblkptr->s_free[ supfreidx ] =
                         nfsblkcnt - ntotfresect * NICFREE - supfreidx;
   }

   for ( supfreidx = nsupfrecnt; supfreidx LT NICFREE; supfreidx++ )
   {
      supblkptr->s_free[ supfreidx ] = ZERO;
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* if there are no free list sections outside of the     */
      /*                                      super block then */
      /*                                                       */
      /*    the super block free list array serves as the      */
      /*    terminating free list section                      */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /*********************************************************/

   if ( ntotfresect EQ ZERO )
   {
      supblkptr->s_free[ ZERO ] = ZERO;
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* load the super block free inode array with the        */
      /* numbers of the first free inodes and zero out the     */
      /* remaining slots, if any                               */
      /*                                                       */
      /*********************************************************/

   supblkptr->s_ninode = nsupinocnt;

   for ( supinoidx = ZERO; supinoidx LT nsupinocnt; supinoidx++ )
   {
      supblkptr->s_inode[ supinoidx ] = nvalinodes + supinoidx + ONE;
   }

   for ( supinoidx = nsupinocnt; supinoidx LT NICINOD; supinoidx++ )
   {
      supblkptr->s_inode[ supinoidx ] = ZERO;
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* load the remaining section fields,                    */
      /*                                                       */
      /* update the super block last modified time field, and  */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   supblkptr->s_isize = ninodesect;
   supblkptr->s_fsize = nfsblkcnt;
   supblkptr->s_tfree = ntotfreblks;
   supblkptr->s_tinode = ntotfreinos;

   time( &gmtime );

   supblkptr->s_time = gmtime;

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end load_super_block_arrays                              */
   /*                                                          */
   /************************************************************/
}
