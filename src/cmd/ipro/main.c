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
/***  PROGRAM:  IPRO - An In-Place ReOrganizer Of UNIX*            ***/
/***                   Filesystems.                                ***/
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
/***     begin IPRO                                                ***/
/***                                                               ***/
/***        trap all standard signals so that IPRO does not        ***/
/***        terminate during a reorganization,                     ***/
/***                                                               ***/
/***        display program copyright/trademark notices, and       ***/
/***                                                               ***/
/***        wait for them to be read                               ***/
/***                                                               ***/
/***        for each command line option found do                  ***/
/***                                                               ***/
/***           if option is verbose mode then                      ***/
/***                                                               ***/
/***              turn on all informative output                   ***/
/***                                                               ***/
/***           else if option is process command line as           ***/
/***                                        filesystem names then  ***/
/***                                                               ***/
/***              set up for filesystem names on the command line  ***/
/***                                                               ***/
/***           else if option is process command line as names of  ***/
/***                       files containing filesystem names then  ***/
/***                                                               ***/
/***              set up for names of files containing filesystem  ***/
/***              names on the command line                        ***/
/***                                                               ***/
/***           else if option is process standard input as         ***/
/***                                        filesystem names then  ***/
/***                                                               ***/
/***              set up for filesystem names on standard input    ***/
/***                                                               ***/
/***           else if option is process standard input as names   ***/
/***                    of files containing filesystem names then  ***/
/***                                                               ***/
/***              set up for names of files containing filesystem  ***/
/***              names on standard input                          ***/
/***                                                               ***/
/***           else if the option is change inode block count      ***/
/***                                                         then  ***/
/***                                                               ***/
/***              pick up the change in the inode block count      ***/
/***                                                               ***/
/***           else if the option is change free block count then  ***/
/***                                                               ***/
/***              pick up the change in the free block count       ***/
/***                                                               ***/
/***           else if the option is log all activity then         ***/
/***                                                               ***/
/***              open the log file in append mode                 ***/
/***                                                               ***/
/***           else                                                ***/
/***                                                               ***/
/***              option violates command line specification       ***/
/***              and exit IPRO                                    ***/
/***                                                               ***/
/***           endif                                               ***/
/***                                                               ***/
/***        set up implicit default processing, i.e. process       ***/
/***        command line arguments as names of filsystems to be    ***/
/***        reorganized                                            ***/
/***                                                               ***/
/***        process the command line, if selected                  ***/
/***                                                               ***/
/***           NOTE:  The command line is presently processed      ***/
/***                  ahead of the standard input.                 ***/
/***                                                               ***/
/***        process the standard input, if selected                ***/
/***                                                               ***/
/***        resynchronize the disks one last time and              ***/
/***                                                               ***/
/***        exit IPRO normally                                     ***/
/***                                                               ***/
/***     end IPRO                                                  ***/
/***                                                               ***/
/***                                                               ***/
/***  EXIT VALUES:   0 - normal.                                   ***/
/***                                                               ***/
/***                 1 - variables not initialized.                ***/
/***                                                               ***/
/***                 2 - multiple verbose modes detected.          ***/
/***                                                               ***/
/***                 3 - multiple cmdline filsys modes detected.   ***/
/***                                                               ***/
/***                 4 - cmdline filsys/name mode conflict.        ***/
/***                                                               ***/
/***                 5 - multiple cmdline filnam modes detected.   ***/
/***                                                               ***/
/***                 6 - cmdline filnam/sys mode conflict.         ***/
/***                                                               ***/
/***                 7 - multiple stdinput filsys modes detected.  ***/
/***                                                               ***/
/***                 8 - stdinput filsys/nam mode conflict.        ***/
/***                                                               ***/
/***                 9 - multiple stdinput filnam modes detected.  ***/
/***                                                               ***/
/***                10 - stdinput filnam/sys mode conflict.        ***/
/***                                                               ***/
/***                11 - multiple inode block modes detected.      ***/
/***                                                               ***/
/***                12 - delta inode block value not numeric.      ***/
/***                                                               ***/
/***                13 - multiple free block modes detected.       ***/
/***                                                               ***/
/***                14 - delta free block value not numeric.       ***/
/***                                                               ***/
/***                15 - multiple log file modes detected.         ***/
/***                                                               ***/
/***                16 - can't open log file in append mode.       ***/
/***                                                               ***/
/***                17 - option violates cmdline specification.    ***/
/***                                                               ***/
/***                18 - can't open file on cmdline.               ***/
/***                                                               ***/
/***                19 - no operation selected for cmdline.        ***/
/***                                                               ***/
/***                20 - can't open file on stdinput.              ***/
/***                                                               ***/
/***                21 - no operation selected for stdinput.       ***/
/***                                                               ***/
/***                                                               ***/
/***  NOTES:  All unused space in the filesystems being            ***/
/***          reorganized is zeroed out.                           ***/
/***                                                               ***/
/***                                                               ***/
/***  ASSUMPTIONS:  the inodes have to be reordered so that the    ***/
/***                inode processing sequence during the block     ***/
/***                table loading need not depend upon the         ***/
/***                contents of the inode table.  the inode and    ***/
/***                block tables then do not have to coexist       ***/
/***                simultaneously.                                ***/
/***                                                               ***/
/***                the list of directories is picked up during    ***/
/***                the first inode scan while loading the inode   ***/
/***                table.                                         ***/
/***                                                               ***/
/***                the directories must be compacted after the    ***/
/***                inode reordering because the inode numbers in  ***/
/***                the directories need to reflect the inode      ***/
/***                reordering.                                    ***/
/***                                                               ***/
/***                the directories must be compacted before the   ***/
/***                inode table is freed because the inode table   ***/
/***                contains the inode renumbering information.    ***/
/***                                                               ***/
/***                the blocks must be reordered before the free   ***/
/***                list is loaded to prevent data and indirect    ***/
/***                blocks from being overwritten.                 ***/
/***                                                               ***/
/***                verbose - is initially set, and allows/        ***/
/***                          disallows informational output.      ***/
/***                                                               ***/
/***                cmdname - points to the name of this program.  ***/
/***                                                               ***/
/***                fsname - points to the name of the filesystem  ***/
/***                         currently being reorganized.          ***/
/***                                                               ***/
/***                deltainoblks - contains the inode block        ***/
/***                               difference between the present  ***/
/***                               and new filesystems.            ***/
/***                                                               ***/
/***                deltafreblks - contains the free block         ***/
/***                               difference between the present  ***/
/***                               and new filesystems.            ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/main.c                   ***/
/***                                                               ***/
/***                                                               ***/
/***  OBJECT LOCATION:  /etc/ipro                                  ***/
/***                                                               ***/
/***                                                               ***/
/***  *UNIX is a registered trademark of AT&T Bell Laboratories,   ***/
/***         Inc.                                                  ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


#include "iprodefs.h"


   INT
main( argc, argv )

   INT              argc;
   CHARPTR          argv[];
{
   INT              sighand();
   CHARPTR          cmdname;
   INT              optchar;
   BOOLEAN          verbose      = FALSE;
   BOOLEAN          vflag        = FALSE;
   BOOLEAN          lflag        = FALSE;
   BOOLEAN          fflag        = FALSE;
   BOOLEAN          iflag        = FALSE;
   LINEBUF          mesgbuf;
   EXTERNAL INT     optind;
   EXTERNAL CHARPTR optarg;
   LONG             fsindex;
   LINEBUF          fsname;
   LINEBUF          clname;
   LINEBUF          siname;
   LONG             deltainoblks = ZERO;
   LONG             deltafreblks = ZERO;
   BOOLEAN          cmdline      = FALSE;
   BOOLEAN          clfilsys     = FALSE;
   BOOLEAN          clfilnam     = FALSE;
   BOOLEAN          stdinput     = FALSE;
   BOOLEAN          sifilsys     = FALSE;
   BOOLEAN          sifilnam     = FALSE;
   FILEPTR          fopen();
   FILEPTR          fp           = NULL;
   FILEPTR          logfp        = NULL;

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin IPRO                                               */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* trap all standard signals so that IPRO does not       */
      /* terminate during a reorganization,                    */
      /*                                                       */
      /* display program copyright/trademark notices, and      */
      /*                                                       */
      /* wait for them to be read                              */
      /*                                                       */
      /*********************************************************/

   signal( SIGHUP, sighand );
   signal( SIGINT, sighand );
   signal( SIGQUIT, sighand );
   signal( SIGALRM, sighand );
   signal( SIGTERM, sighand );

   sync();
   sync();
   sync();

   fprintf( stderr,
                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" );

   prcenter( stderr,
                   "(C) Copyright  Motorola, Inc.  1985, 1986, 1987" );
   prcenter( stderr, "All Rights Reserved" );
   prcenter( stderr, "Motorola Confidential/Proprietary" );
   fprintf( stderr, "\n" );
   prcenter( stderr, "IPRO" );
   prcenter( stderr, "An In-Place ReOrganizer of UNIX* Filesystems" );
   sprintf( mesgbuf, "Release %s", IPRORELNO );
   prcenter( stderr, mesgbuf );
   fprintf( stderr, "\n" );
   prcenter( stderr,
   "*UNIX is a registered trademark of AT&T Bell Laboratories, Inc." );
   fprintf( stderr, "\n\n\n\n\n\n\n" );

   sleep( TEN );

   cmdname = argv[ COMMAND ];

   if ( ( verbose NE FALSE )      OR
           ( vflag NE FALSE )        OR
              ( lflag NE FALSE )        OR
                 ( fflag NE FALSE )        OR
                    ( iflag NE FALSE )        OR
                       ( deltainoblks NE ZERO )  OR
                          ( deltafreblks NE ZERO )  OR
                             ( cmdline NE FALSE )      OR
                                ( clfilsys NE FALSE )     OR
                                   ( clfilnam NE FALSE )     OR
                                      ( stdinput NE FALSE )     OR
                                         ( sifilsys NE FALSE )    OR
                                            ( sifilnam NE FALSE )   OR
                                               ( fp NE NULL )        OR
                                                    ( logfp NE NULL ) )
   {
      printmesg( TRUE, PROCEDURAL, cmdname, NULL,
                   "main: bad parameters/variables not initialized.",
                                                               logfp );

      terminate( TRUE, PROCEDURAL, cmdname, NULL,
                            mesgbuf, CLOSED, logfp, fp, NULL,
                                         NULL, NULL, NULL, NULL, ONE );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* for each command line option found do                 */
      /*                                                       */
      /*********************************************************/

   while ( (optchar = getopt( argc, argv, OPTNLTRS )) NE EOF )
   {
         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* if option is verbose mode then                     */
         /*                                                    */
         /*    turn on all informative output                  */
         /*                                                    */
         /******************************************************/

      if ( optchar EQ VERBOSE )
      {
         if ( vflag )
         {
            sprintf( mesgbuf,
                  "main: ref. -%c option: multiple -%c options found.",
                                                    VERBOSE, VERBOSE );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                               mesgbuf, CLOSED, logfp, fp, NULL,
                                         NULL, NULL, NULL, NULL, TWO );
         }

         vflag = TRUE;

         verbose = TRUE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* else if option is process command line as          */
         /*                              filesystem names then */
         /*                                                    */
         /*    set up for filesystem names on the command line */
         /*                                                    */
         /******************************************************/

      else if ( optchar EQ CLFILSYS )
      {
         if ( clfilsys )
         {
            sprintf( mesgbuf,
                  "main: ref. -%c option: multiple -%c options found.",
                                                  CLFILSYS, CLFILSYS );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                       NULL, NULL, NULL, NULL, THREE );
         }

         if ( clfilnam )
         {
            sprintf( mesgbuf,
             "main: ref. -%c option: -%c or -%c allowed but not both.",
                                        CLFILSYS, CLFILSYS, CLFILNAM );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                        NULL, NULL, NULL, NULL, FOUR );
         }

         cmdline = TRUE;

         clfilsys = TRUE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* else if option is process command line as names of */
         /*             files containing filesystem names then */
         /*                                                    */
         /*    set up for names of files containing filesystem */
         /*    names on the command line                       */
         /*                                                    */
         /******************************************************/

      else if ( optchar EQ CLFILNAM )
      {
         if ( clfilnam )
         {
            sprintf( mesgbuf,
                  "main: ref. -%c option: multiple -%c options found.",
                                                  CLFILNAM, CLFILNAM );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                        NULL, NULL, NULL, NULL, FIVE );
         }

         if ( clfilsys )
         {
            sprintf( mesgbuf,
             "main: ref. -%c option: -%c or -%c allowed but not both.",
                                        CLFILNAM, CLFILNAM, CLFILSYS );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                               mesgbuf, CLOSED, logfp, fp, NULL,
                                         NULL, NULL, NULL, NULL, SIX );
         }

         cmdline = TRUE;

         clfilnam = TRUE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* else if option is process standard input as        */
         /*                              filesystem names then */
         /*                                                    */
         /*    set up for filesystem names on standard input   */
         /*                                                    */
         /******************************************************/

      else if ( optchar EQ SIFILSYS )
      {
         if ( sifilsys )
         {
            sprintf( mesgbuf,
                  "main: ref. -%c option: multiple -%c options found.",
                                                  SIFILSYS, SIFILSYS );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                       NULL, NULL, NULL, NULL, SEVEN );
         }

         if ( sifilnam )
         {
            sprintf( mesgbuf,
             "main: ref. -%c option: -%c or -%c allowed but not both.",
                                        SIFILSYS, SIFILSYS, SIFILNAM );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                       NULL, NULL, NULL, NULL, EIGHT );
         }

         stdinput = TRUE;

         sifilsys = TRUE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* else if option is process standard input as names  */
         /*          of files containing filesystem names then */
         /*                                                    */
         /*    set up for names of files containing filesystem */
         /*    names on standard input                         */
         /*                                                    */
         /******************************************************/

      else if ( optchar EQ SIFILNAM )
      {
         if ( sifilnam )
         {
            sprintf( mesgbuf,
                  "main: ref. -%c option: multiple -%c options found.",
                                                  SIFILNAM, SIFILNAM );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                        NULL, NULL, NULL, NULL, NINE );
         }

         if ( sifilsys )
         {
            sprintf( mesgbuf,
             "main: ref. -%c option: -%c or -%c allowed but not both.",
                                        SIFILNAM, SIFILNAM, SIFILSYS );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                               mesgbuf, CLOSED, logfp, fp, NULL,
                                         NULL, NULL, NULL, NULL, TEN );
         }

         stdinput = TRUE;

         sifilnam = TRUE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* else if the option is change inode block count     */
         /*                                               then */
         /*                                                    */
         /*    pick up the change in the inode block count     */
         /*                                                    */
         /******************************************************/

      else if ( optchar EQ DELTA_INODEBLKS )
      {
         if ( iflag )
         {
            sprintf( mesgbuf,
                  "main: ref. -%c option: multiple -%c options found.",
                                    DELTA_INODEBLKS, DELTA_INODEBLKS );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                      NULL, NULL, NULL, NULL, ELEVEN );
         }

         if ( sscanf( optarg, "%ld", &deltainoblks ) NE ONE )
         {
            sprintf( mesgbuf,
       "main: ref. -%c option: delta inode blocks %s must be numeric.",
               DELTA_INODEBLKS, ( optarg EQ NULL ) ? NULSTR : optarg );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                      NULL, NULL, NULL, NULL, TWELVE );
         }

         iflag = TRUE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* else if the option is change free block count then */
         /*                                                    */
         /*    pick up the change in the free block count      */
         /*                                                    */
         /******************************************************/

      else if ( optchar EQ DELTA_FREEBLKS )
      {
         if ( fflag )
         {
            sprintf( mesgbuf,
                  "main: ref. -%c option: multiple -%c options found.",
                                      DELTA_FREEBLKS, DELTA_FREEBLKS );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                             mesgbuf, CLOSED, logfp, fp, NULL,
                                    NULL, NULL, NULL, NULL, THIRTEEN );
         }

         if ( sscanf( optarg, "%ld", &deltafreblks ) NE ONE )
         {
            sprintf( mesgbuf,
        "main: ref. -%c option: delta free blocks %s must be numeric.",
                DELTA_FREEBLKS, ( optarg EQ NULL ) ? NULSTR : optarg );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                             mesgbuf, CLOSED, logfp, fp, NULL,
                                    NULL, NULL, NULL, NULL, FOURTEEN );
         }

         fflag = TRUE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* else if the option is log all activity then        */
         /*                                                    */
         /*    open the log file in append mode                */
         /*                                                    */
         /******************************************************/

      else if ( optchar EQ LOGFILE )
      {
         if ( lflag )
         {
            sprintf( mesgbuf,
                  "main: ref. -%c option: multiple -%c options found.",
                                                    LOGFILE, LOGFILE );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                             mesgbuf, CLOSED, logfp, fp, NULL,
                                     NULL, NULL, NULL, NULL, FIFTEEN );
         }

         if ( (logfp = fopen( optarg, FP_APPEND )) EQ NULL )
         {
            sprintf( mesgbuf,
                     "main: ref. -%c option: can't open log file %s.",
                       LOGFILE, ( optarg EQ NULL ) ? NULSTR : optarg );

            printmesg( TRUE, PROCEDURAL, cmdname, NULL, mesgbuf,
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, NULL,
                             mesgbuf, CLOSED, logfp, fp, NULL,
                                     NULL, NULL, NULL, NULL, SIXTEEN );
         }

         lflag = TRUE;
      }

         /******************************************************/
         /******************************************************/
         /*                                                    */
         /* else                                               */
         /*                                                    */
         /*    option violates command line specification      */
         /*    and exit IPRO                                   */
         /*                                                    */
         /* endif                                              */
         /*                                                    */
         /******************************************************/

      else
      {
         printmesg( TRUE, PROCEDURAL, cmdname, NULL,
                   "main: option violates command line specification.",
                                                               logfp );

         terminate( TRUE, PROCEDURAL, cmdname, NULL,
                            mesgbuf, CLOSED, logfp, fp, NULL,
                                   NULL, NULL, NULL, NULL, SEVENTEEN );
      }
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* set up implicit default processing, i.e. process      */
      /* command line arguments as names of filsystems to be   */
      /* reorganized                                           */
      /*                                                       */
      /*********************************************************/

   if ( ( NOT cmdline )  AND  ( NOT stdinput ) )
   {
      cmdline = TRUE;

      clfilsys = TRUE;
   }

   printmesg( verbose, INFORMATIVE, cmdname, NULL, "Begin IPRO.",
                                                               logfp );

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* process the command line, if selected                 */
      /*                                                       */
      /*    NOTE:  The command line is presently processed     */
      /*           ahead of the standard input.                */
      /*                                                       */
      /*********************************************************/

   if ( cmdline )
   {
      printmesg( verbose, INFORMATIVE, cmdname, NULL,
                             "Begin Command Line Processing.", logfp );

      for ( fsindex = optind; fsindex LT argc; fsindex++ )
      {
         strcpy( clname, argv[ fsindex ] );

         if ( clfilsys )
         {
            procfilsys( verbose, cmdname, clname, mesgbuf, logfp,
                                      fp, deltainoblks, deltafreblks );
         }
         else if ( clfilnam )
         {
            if ( (fp = fopen( clname, FP_READ )) EQ NULL )
            {
               sprintf( mesgbuf,
                           "main: can't open file %s on command line.",
                                                              clname );

               printmesg( TRUE, PROCEDURAL, cmdname, clname, mesgbuf,
                                                               logfp );

               terminate( TRUE, PROCEDURAL, cmdname, clname,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                   NULL, NULL, NULL, NULL, EIGHTTEEN );
            }

            while ( fscanf( fp, "%s", fsname ) NE EOF )
            {
               procfilsys( verbose, cmdname, fsname, mesgbuf, logfp,
                                      fp, deltainoblks, deltafreblks );
            }

            fclose( fp );

            fp = NULL;
         }
         else
         {
            printmesg( TRUE, PROCEDURAL, cmdname, clname,
            "main: no operation selected for command line processing.",
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, clname,
                              mesgbuf, CLOSED, logfp, fp, NULL,
                                    NULL, NULL, NULL, NULL, NINETEEN );
         }
      }

      printmesg( verbose, INFORMATIVE, cmdname, NULL,
                               "End Command Line Processing.", logfp );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* process the standard input, if selected               */
      /*                                                       */
      /*********************************************************/

   if ( stdinput )
   {
      printmesg( verbose, INFORMATIVE, cmdname, NULL,
                           "Begin Standard Input Processing.", logfp );

      while ( scanf( "%s", siname ) NE EOF )
      {
         if ( sifilsys )
         {
            procfilsys( verbose, cmdname, siname, mesgbuf, logfp,
                                      fp, deltainoblks, deltafreblks );
         }
         else if ( sifilnam )
         {
            if ( (fp = fopen( siname, FP_READ )) EQ NULL )
            {
               sprintf( mesgbuf,
                         "main: can't open file %s on standard input.",
                                                              siname );

               printmesg( TRUE, PROCEDURAL, cmdname, siname, mesgbuf,
                                                               logfp );

               terminate( TRUE, PROCEDURAL, cmdname, siname,
                               mesgbuf, CLOSED, logfp, fp, NULL,
                                      NULL, NULL, NULL, NULL, TWENTY );
            }

            while ( fscanf( fp, "%s", fsname ) NE EOF )
            {
               procfilsys( verbose, cmdname, fsname, mesgbuf, logfp,
                                      fp, deltainoblks, deltafreblks );
            }

            fclose( fp );

            fp = NULL;
         }
         else
         {
            printmesg( TRUE, PROCEDURAL, cmdname, siname,
          "main: no operation selected for standard input processing.",
                                                               logfp );

            terminate( TRUE, PROCEDURAL, cmdname, siname,
                             mesgbuf, CLOSED, logfp, fp, NULL,
                                   NULL, NULL, NULL, NULL, TWENTYONE );
         }
      }

      printmesg( verbose, INFORMATIVE, cmdname, NULL,
                             "End Standard Input Processing.", logfp );
   }

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* resynchronize the disks one last time and             */
      /*                                                       */
      /* exit IPRO normally                                    */
      /*                                                       */
      /*********************************************************/

   printmesg( verbose, INFORMATIVE, cmdname, NULL, "End IPRO.",
                                                               logfp );
   sync();
   sync();
   sync();

   terminate( verbose, INFORMATIVE, cmdname, NULL,
                         mesgbuf, CLOSED, logfp, fp, NULL,
                                      NULL, NULL, NULL, NULL, NORMAL );

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end IPRO                                                 */
   /*                                                          */
   /************************************************************/
}
