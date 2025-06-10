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
/***  FUNCTION:  inoreorder - reorder the inodes with respect to   ***/
/***                          their time of last modification.     ***/
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
/***     begin reorder_inodes                                      ***/
/***                                                               ***/
/***        get two working blocks                                 ***/
/***                                                               ***/
/***        for each inode block do                                ***/
/***                                                               ***/
/***           if the new primary inode block is the same as the   ***/
/***                          previous secondary inode block then  ***/
/***                                                               ***/
/***              primary <-- secondary                            ***/
/***                                                               ***/
/***              secondary <-- invalid                            ***/
/***                                                               ***/
/***              primarystat <-- secondarystat                    ***/
/***                                                               ***/
/***           else                                                ***/
/***                                                               ***/
/***              read in the primary block                        ***/
/***                                                               ***/
/***              primarystat <-- clean                            ***/
/***                                                               ***/
/***           endif                                               ***/
/***                                                               ***/
/***           for each inode (sequential) in the primary block    ***/
/***                                                           do  ***/
/***                                                               ***/
/***              pick up the destination of the inode             ***/
/***                                                               ***/
/***              mark the inode as processed                      ***/
/***                                                               ***/
/***              save (ripple) the inode                          ***/
/***                                                               ***/
/***              if the inode is in-place then                    ***/
/***                                                               ***/
/***                 skip the inode                                ***/
/***                                                               ***/
/***              endif                                            ***/
/***                                                               ***/
/***              if the inode will be in the free inode range     ***/
/***                                                         then  ***/
/***                 zero the inode                                ***/
/***                                                               ***/
/***                 mark the primary inode block dirty            ***/
/***                                                               ***/
/***              endif                                            ***/
/***                                                               ***/
/***              if the inode is free/processed then              ***/
/***                                                               ***/
/***                 skip the inode                                ***/
/***                                                               ***/
/***              endif                                            ***/
/***                                                               ***/
/***              for each element in the ripple insertion inode   ***/
/***                                                     chain do  ***/
/***                                                               ***/
/***                 pick up the number and index of the inode's   ***/
/***                 destination and                               ***/
/***                                                               ***/
/***                 if the destination of the inode is in the     ***/
/***                                     primary inode block then  ***/
/***                                                               ***/
/***                    set up the virtual block pointer to the    ***/
/***                    primary block and                          ***/
/***                                                               ***/
/***                    mark the primary block dirty               ***/
/***                                                               ***/
/***                 else                                          ***/
/***                                                               ***/
/***                    if the inode destination block is          ***/
/***                             not the same as the secondary     ***/
/***                                             inode block then  ***/
/***                                                               ***/
/***                       if the secondary block is valid and     ***/
/***                                            marked dirty then  ***/
/***                                                               ***/
/***                          write out the secondary block        ***/
/***                                                               ***/
/***                       endif                                   ***/
/***                                                               ***/
/***                       set the secondary block number to the   ***/
/***                       the inode destination block number,     ***/
/***                                                               ***/
/***                       read in the new secondary block, and    ***/
/***                                                               ***/
/***                       mark the secondary block clean          ***/
/***                                                               ***/
/***                    endif                                      ***/
/***                                                               ***/
/***                    set up the virtual block pointer to the    ***/
/***                    secondary block and                        ***/
/***                                                               ***/
/***                    mark the secondary block dirty             ***/
/***                                                               ***/
/***                 endif                                         ***/
/***                                                               ***/
/***                 if the destination inode is free/processed    ***/
/***                                                         then  ***/
/***                                                               ***/
/***                    the end of the inode chain has been found  ***/
/***                                                               ***/
/***                 endif                                         ***/
/***                                                               ***/
/***                 mark the destination inode as processed       ***/
/***                                                               ***/
/***                 save (ripple) the destination inode and       ***/
/***                                                               ***/
/***                 put (insert) the inode to its destination     ***/
/***                                                               ***/
/***                 inode # <-- destination inode #               ***/
/***                                                               ***/
/***              end for each element in the inode chain          ***/
/***                                                               ***/
/***              put (insert) the last inode in the chain to its  ***/
/***              destination                                      ***/
/***                                                               ***/
/***           end for each inode in the primary block do          ***/
/***                                                               ***/
/***           if the secondary block is invalid then              ***/
/***                                                               ***/
/***              copy the primary block to the secondary block,   ***/
/***                                                               ***/
/***              renumber the secondary block, and                ***/
/***                                                               ***/
/***              mark the secondary block clean                   ***/
/***                                                               ***/
/***           else if the primary block is dirty then             ***/
/***                                                               ***/
/***              write out the primary block                      ***/
/***                                                               ***/
/***           endif                                               ***/
/***                                                               ***/
/***        end for each inode block                               ***/
/***                                                               ***/
/***        if the secondary block is valid and dirty then         ***/
/***                                                               ***/
/***           write out the secondary block                       ***/
/***                                                               ***/
/***        endif                                                  ***/
/***                                                               ***/
/***        return success                                         ***/
/***                                                               ***/
/***     end reorder_inodes                                        ***/
/***                                                               ***/
/***                                                               ***/
/***  RETURN VALUES:  true - success, no errors detected.          ***/
/***                                                               ***/
/***                  false - failure, error detected in this      ***/
/***                          routine or erroneous value returned  ***/
/***                          from an invoked routine.             ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  the inode table is loaded and sorted.          ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/inoreorder.c             ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   BOOLEAN
inoreorder( verbose, mesgtype, cmdname, fsname, mesgbuf,
                    fsfd, logfp, inodetable, bsize, inopb,
                             inoblkcnt, valinodes, wrkblks, wrkblkidx )

   BOOLEAN    verbose;
   CHARPTR    mesgtype;
   CHARPTR    cmdname;
   CHARPTR    fsname;
   CHARPTR    mesgbuf;
   INT        fsfd;
   FILEPTR    logfp;
   INODEINFO *inodetable;
   LONG       bsize;
   LONG       inopb;
   LONG       inoblkcnt;
   LONG       valinodes;
   CHARPTR    wrkblks;
   LONG       wrkblkidx;
{
   INODEPTR   primary;
   INODEPTR   secondary;
   INODEPTR   temporary;
   INODE      inode;
   INODEPTR   inoptr      = &inode;
   INODEPTR   copyptr;
   LONG       inum1;
   LONG       inum2;
   LONG       primeno;
   LONG       secondno    = INVALID;
   LONG       primeidx;
   LONG       newblkno;
   LONG       newblkidx;
   BOOLEAN    primedirty;
   BOOLEAN    seconddirty = FALSE;
   LONG       inotblidx;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin reorder_inodes                                     */
   /*                                                          */
   /************************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, fsname,
                                     "Reordering The Inodes.", logfp );

   if ( ( fsfd EQ CLOSED )       OR
           ( inodetable EQ NULL )   OR
              ( bsize LE ZERO )        OR
                 ( inopb LE ZERO )        OR
                    ( inoblkcnt LE ZERO )    OR
                       ( valinodes LE ZERO )    OR
                          ( wrkblks EQ NULL )      OR
                             ( wrkblkidx LT ZERO )    OR
                                ( inoptr NE &inode )     OR
                                   ( secondno NE INVALID )  OR
                                             ( seconddirty NE FALSE ) )
   {
      printmesg( TRUE, mesgtype, cmdname, fsname,
               "inoreorder: bad parameters/variables not initialized.",
                                                               logfp );

      return( FALSE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* get two working blocks                                */
      /*                                                       */
      /*********************************************************/

   if ( (primary = (INODEPTR)( GETWRKBLK( wrkblks, WRKBLKCNT,
                                        wrkblkidx, bsize ) )) EQ NULL )
   {
      sprintf( mesgbuf, 
           "inoreorder: can't get first working block %ld of %ld %ld.",
                                   wrkblkidx - ONE, WRKBLKCNT, bsize );

      printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf, logfp );

      return( FALSE );
   }

   if ( (secondary = (INODEPTR)( GETWRKBLK( wrkblks, WRKBLKCNT,
                                        wrkblkidx, bsize ) )) EQ NULL )
   {
      sprintf( mesgbuf, 
          "inoreorder: can't get second working block %ld of %ld %ld.",
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

   for ( primeno = ZERO; primeno LT inoblkcnt; primeno++ )
   {
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* if the new primary inode block is the same as the  */
         /*                previous secondary inode block then */
         /*                                                    */
         /*    primary <-- secondary                           */
         /*                                                    */
         /*    secondary <-- invalid                           */
         /*                                                    */
         /*    primarystat <-- secondarystat                   */
         /*                                                    */
         /* else                                               */
         /*                                                    */
         /*    read in the primary block                       */
         /*                                                    */
         /*    primarystat <-- clean                           */
         /*                                                    */
         /* endif                                              */
         /*                                                    */
         /******************************************************/

      if ( primeno EQ secondno )
      {
         XCHG( primary, secondary, temporary );

         secondno = INVALID;

         primedirty = seconddirty;
      }
      else if ( NOT fsblockio( verbose, mesgtype, cmdname,
                                   fsname, mesgbuf, fsfd, logfp,
                                         bsize, READ, (CHARPTR)primary,
                                                 INOBLKOFF + primeno) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                            "inoreorder: read primary block.", logfp );

         return( FALSE );
      }
      else
      {
         primedirty = FALSE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* for each inode (sequential) in the primary block   */
         /*                                                 do */
         /*                                                    */
         /******************************************************/

      for ( primeidx = ZERO,
                    inotblidx = primeno * inopb + primeidx + ONE;
                                 primeidx LT inopb;
                                               primeidx++,
                                                          inotblidx++ )
      {
            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* pick up the destination of the inode            */
            /*                                                 */
            /* mark the inode as processed                     */
            /*                                                 */
            /* save (ripple) the inode                         */
            /*                                                 */
            /* if the inode is in-place then                   */
            /*                                                 */
            /*    skip the inode                               */
            /*                                                 */
            /* endif                                           */
            /*                                                 */
            /***************************************************/

         inum1 = inodetable[ inotblidx ].xchgdst;

         inodetable[ inotblidx ].xchgdst = ZERO;

         if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, logfp, COPY_MEM, NULL, NULL,
                                (CHARPTR)( primary + primeidx ),
                                   (CHARPTR)inoptr, INODESIZE, NULL ) )
         {
            sprintf( mesgbuf, "inoreorder: ripple inode %ld.",
                                                           inotblidx );

            printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

            return( FALSE );
         }

         if ( inum1 EQ inotblidx )
         {
            continue;
         }

            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* if the inode will be in the free inode range    */
            /*                                            then */
            /*    zero the inode                               */
            /*                                                 */
            /*    mark the primary inode block dirty           */
            /*                                                 */
            /* endif                                           */
            /*                                                 */
            /***************************************************/

         if ( inotblidx GT valinodes )
         {
            if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                                  mesgbuf, logfp, INIT_MEM, NULL, NULL,
                                       (CHARPTR)( primary + primeidx ),
                                              NULL, INODESIZE, NULL ) )
            {
               sprintf( mesgbuf,
                         "inoreorder: null free range inode %ld.",
                                                           inotblidx );

               printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

               return( FALSE );
            }

            primedirty = TRUE;
         }

            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* if the inode is free/processed then             */
            /*                                                 */
            /*    skip the inode                               */
            /*                                                 */
            /* endif                                           */
            /*                                                 */
            /***************************************************/

         if ( inum1 EQ ZERO )
         {
            continue;
         }

            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* for each element in the ripple insertion inode  */
            /*                                        chain do */
            /*                                                 */
            /***************************************************/

         while ( TRUE )
         {
               /************************************************/
               /************************************************/
               /*                                              */
               /* pick up the number and index of the inode's  */
               /* destination and                              */
               /*                                              */
               /* if the destination of the inode is in the    */
               /*                     primary inode block then */
               /*                                              */
               /*    set up the virtual block pointer to the   */
               /*    primary block and                         */
               /*                                              */
               /*    mark the primary block dirty              */
               /*                                              */
               /************************************************/

            newblkno = ( inum1 - ONE ) / inopb;

            newblkidx = ( inum1 - ONE ) % inopb;

            if ( newblkno EQ primeno )
            {
               copyptr = primary;

               primedirty = TRUE;
            }

               /************************************************/
               /************************************************/
               /*                                              */
               /* else                                         */
               /*                                              */
               /************************************************/

            else
            {
                  /*********************************************/
                  /*********************************************/
                  /*                                           */
                  /* if the inode destination block is         */
                  /*          not the same as the secondary    */
                  /*                          inode block then */
                  /*                                           */
                  /*    if the secondary block is valid and    */
                  /*                         marked dirty then */
                  /*                                           */
                  /*       write out the secondary block       */
                  /*                                           */
                  /*    endif                                  */
                  /*                                           */
                  /*    set the secondary block number to the  */
                  /*    the inode destination block number,    */
                  /*                                           */
                  /*    read in the new secondary block, and   */
                  /*                                           */
                  /*    mark the secondary block clean         */
                  /*                                           */
                  /* endif                                     */
                  /*                                           */
                  /* set up the virtual block pointer to the   */
                  /* secondary block and                       */
                  /*                                           */
                  /* mark the secondary block dirty            */
                  /*                                           */
                  /*********************************************/

               if ( newblkno NE secondno )
               {
                  if ( ( secondno NE INVALID )  AND  ( seconddirty ) )
                  {
                     if ( NOT fsblockio( verbose, mesgtype, cmdname,
                                          fsname, mesgbuf, fsfd,
                                           logfp, bsize, WRITE,
                                             (CHARPTR)secondary,
                                               INOBLKOFF + secondno ) )
                     {
                        printmesg( TRUE, mesgtype, cmdname, fsname,
                                  "inoreorder: write secondary block.",
                                                               logfp );

                        return( FALSE );
                     }

                     secondno = INVALID;
                  }

                  secondno = newblkno;

                  if ( NOT fsblockio( verbose, mesgtype, cmdname,
                                        fsname, mesgbuf, fsfd,
                                          logfp, bsize, READ,
                                            (CHARPTR)secondary,
                                               INOBLKOFF + secondno ) )
                  {
                     printmesg( TRUE, mesgtype, cmdname, fsname,
                               "inoreorder: read new secondary block.",
                                                               logfp );

                     return( FALSE );
                  }

                  seconddirty = FALSE;
               }

               copyptr = secondary;

               seconddirty = TRUE;
            }

               /************************************************/
               /************************************************/
               /*                                              */
               /* endif                                        */
               /*                                              */
               /* if the destination inode is free/processed   */
               /*                                         then */
               /*                                              */
               /*    the end of the inode chain has been found */
               /*                                              */
               /* endif                                        */
               /*                                              */
               /* mark the destination inode as processed      */
               /*                                              */
               /* save (ripple) the destination inode and      */
               /*                                              */
               /* put (insert) the inode to its destination    */
               /*                                              */
               /* inode # <-- destination inode #              */
               /*                                              */
               /************************************************/

            if ( (inum2 = inodetable[ inum1 ].xchgdst) EQ ZERO )
            {
               break;
            }

            inodetable[ inum1 ].xchgdst = ZERO;

            if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                               mesgbuf, logfp, XCHG_MEM, NULL, NULL,
                                 (CHARPTR)( copyptr + newblkidx ),
                                   (CHARPTR)inoptr, INODESIZE, NULL ) )
            {
               sprintf( mesgbuf,
                         "inoreorder: ripple/insert inode %ld.",
                                                           inotblidx );

               printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

               return( FALSE );
            }

            inum1 = inum2;
         }

            /***************************************************/
            /***************************************************/
            /*                                                 */
            /* end for each element in the inode chain         */
            /*                                                 */
            /* put (insert) the last inode in the chain to its */
            /* destination                                     */
            /*                                                 */
            /***************************************************/

         if ( NOT memops( verbose, mesgtype, cmdname, fsname,
                             mesgbuf, logfp, COPY_MEM,
                                 NULL, NULL, (CHARPTR)inoptr,
                                      (CHARPTR)( copyptr + newblkidx ),
                                                    INODESIZE, NULL ) )
         {
            sprintf( mesgbuf, "inoreorder: insert chain tail %ld.",
                                                           inotblidx );

            printmesg( TRUE, mesgtype, cmdname, fsname, mesgbuf,
                                                               logfp );

            return( FALSE );
         }
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* end for each inode in the primary block do         */
         /*                                                    */
         /* if the secondary block is invalid then             */
         /*                                                    */
         /*    copy the primary block to the secondary block,  */
         /*                                                    */
         /*    renumber the secondary block, and               */
         /*                                                    */
         /*    mark the secondary block clean                  */
         /*                                                    */
         /* else if the primary block is dirty then            */
         /*                                                    */
         /*    write out the primary block                     */
         /*                                                    */
         /* endif                                              */
         /*                                                    */
         /******************************************************/

      if ( secondno EQ INVALID )
      {
         XCHG( primary, secondary, temporary );

         secondno = primeno;

         seconddirty = primedirty;
      }
      else if ( primedirty )
      {
         if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                   mesgbuf, fsfd, logfp, bsize,
                                         WRITE, (CHARPTR)primary,
                                                INOBLKOFF + primeno ) )
         {
            printmesg( TRUE, mesgtype, cmdname, fsname,
                           "inoreorder: write primary block.", logfp );

            return( FALSE );
         }
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* end for each inode block                              */
      /*                                                       */
      /* if the secondary block is valid and dirty then        */
      /*                                                       */
      /*    write out the secondary block                      */
      /*                                                       */
      /* endif                                                 */
      /*                                                       */
      /* return success                                        */
      /*                                                       */
      /*********************************************************/

   if ( ( secondno NE INVALID )  AND  ( seconddirty ) )
   {
      if ( NOT fsblockio( verbose, mesgtype, cmdname, fsname,
                                 mesgbuf, fsfd, logfp, bsize,
                                        WRITE, (CHARPTR)secondary,
                                               INOBLKOFF + secondno ) )
      {
         printmesg( TRUE, mesgtype, cmdname, fsname,
                   "inoreorder: write final secondary block.", logfp );

         return( FALSE );
      }

      secondno = INVALID;
   }

   return( TRUE );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end reorder_inodes                                       */
   /*                                                          */
   /************************************************************/
}
