/*********************************************************************/
/*********************************************************************/
/***                                                               ***/
/***                                                               ***/
/***  (C) Copyright  Motorola, Inc.  1985, 1986, 1987              ***/
/***  All Rights Reserved                                          ***/
/***  Motorola Confidential/Proprietary                            ***/
/***                                                               ***/
/***                                                               ***/
/***  FILE:  IPRO definitions header file.                         ***/
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
/***  CONTENTS:                                                    ***/
/***                                                               ***/
/***     begin IPRO_definitions                                    ***/
/***                                                               ***/
/***        include files                                          ***/
/***                                                               ***/
/***        equality operators                                     ***/
/***                                                               ***/
/***        boolean operators                                      ***/
/***                                                               ***/
/***        bitwise operators                                      ***/
/***                                                               ***/
/***        boolean values                                         ***/
/***                                                               ***/
/***        numeric constants                                      ***/
/***                                                               ***/
/***        file seek points                                       ***/
/***                                                               ***/
/***        inode disk block address constants                     ***/
/***                                                               ***/
/***        miscellaneous                                          ***/
/***                                                               ***/
/***        standard data types                                    ***/
/***                                                               ***/
/***        inode types                                            ***/
/***                                                               ***/
/***        directory types                                        ***/
/***                                                               ***/
/***        boot/super/indirect/free block types                   ***/
/***                                                               ***/
/***        file status types                                      ***/
/***                                                               ***/
/***        min/max/sqr/cube/range functions                       ***/
/***                                                               ***/
/***        inode table structure type                             ***/
/***                                                               ***/
/***        block table structure type                             ***/
/***                                                               ***/
/***        message type strings                                   ***/
/***                                                               ***/
/***        command line option constants                          ***/
/***                                                               ***/
/***        file operations                                        ***/
/***                                                               ***/
/***        default message strings                                ***/
/***                                                               ***/
/***        table names                                            ***/
/***                                                               ***/
/***        memory operation types                                 ***/
/***                                                               ***/
/***        time values                                            ***/
/***                                                               ***/
/***     end IPRO_definitions                                      ***/
/***                                                               ***/
/***                                                               ***/
/***  SOURCE LOCATION:  /usr/src/cmd/ipro/iprodefs.h               ***/
/***                                                               ***/
/***                                                               ***/
/*********************************************************************/
/*********************************************************************/


   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* begin IPRO_definitions                                   */
   /*                                                          */
   /************************************************************/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* include files                                         */
      /*                                                       */
      /*********************************************************/

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5dir.h>
#include <sys/fs/s5fblk.h>
#include <sys/fs/s5filsys.h>
#include <sys/ino.h>
#include <sys/fs/s5inode.h>
#include <sys/signal.h>
#include <sys/stat.h>

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* equality operators                                    */
      /*                                                       */
      /*********************************************************/

#define EQ		==
#define NE		!=

#define LT		<
#define LE		<=

#define GT		>
#define GE		>=

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* boolean operators                                     */
      /*                                                       */
      /*********************************************************/

#define AND		&&
#define OR		||
#define XOR( a, b )	( ((a) AND NOT (b))  OR  ((b) AND NOT (a)) )
#define NOT		!

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* bitwise operators                                     */
      /*                                                       */
      /*********************************************************/

#define BITAND		&
#define BITOR		|
#define BITXOR		^
#define BITNOT		~
#define LSHIFT		<<
#define RSHIFT		>>

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* boolean values                                        */
      /*                                                       */
      /*********************************************************/

#define FALSE		( ZERO )
#define TRUE		( NOT FALSE )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* numeric constants                                     */
      /*                                                       */
      /*********************************************************/

#define M_TWENTY	( -20L )
#define M_NINETEEN	( -19L )
#define M_EIGHTTEEN	( -18L )
#define M_SEVENTEEN	( -17L )
#define M_SIXTEEN	( -16L )
#define M_FIFTEEN	( -15L )
#define M_FOURTEEN	( -14L )
#define M_THIRTEEN	( -13L )
#define M_TWELVE	( -12L )
#define M_ELEVEN	( -11L )
#define M_TEN		( -10L )
#define M_NINE		(  -9L )
#define M_EIGHT		(  -8L )
#define M_SEVEN		(  -7L )
#define M_SIX		(  -6L )
#define M_FIVE		(  -5L )
#define M_FOUR		(  -4L )
#define M_THREE		(  -3L )
#define M_TWO		(  -2L )
#define M_ONE		(  -1L )
#define ZERO		(   0L )
#define ONE		(   1L )
#define TWO		(   2L )
#define THREE		(   3L )
#define FOUR		(   4L )
#define FIVE		(   5L )
#define SIX		(   6L )
#define SEVEN		(   7L )
#define EIGHT		(   8L )
#define NINE		(   9L )
#define TEN		(  10L )
#define ELEVEN		(  11L )
#define TWELVE		(  12L )
#define THIRTEEN	(  13L )
#define FOURTEEN	(  14L )
#define FIFTEEN		(  15L )
#define SIXTEEN		(  16L )
#define SEVENTEEN	(  17L )
#define EIGHTTEEN	(  18L )
#define NINETEEN	(  19L )
#define TWENTY		(  20L )
#define TWENTYONE	(  21L )
#define TWENTYTWO	(  22L )
#define TWENTYTHREE	(  23L )
#define TWENTYFOUR	(  24L )
#define TWENTYFIVE	(  25L )
#define TWENTYSIX	(  26L )
#define TWENTYSEVEN	(  27L )
#define TWENTYEIGHT	(  28L )
#define TWENTYNINE	(  29L )
#define THIRTY		(  30L )
#define THIRTYONE	(  31L )
#define THIRTYTWO	(  32L )
#define THIRTYTHREE	(  33L )
#define THIRTYFOUR 	(  34L )
#define THIRTYFIVE 	(  35L )
#define THIRTYSIX	(  36L )
#define THIRTYSEVEN	(  37L )
#define THIRTYEIGHT	(  38L )
#define THIRTYNINE	(  39L )
#define FORTY		(  40L )
#define FORTYONE	(  41L )
#define FORTYTWO	(  42L )
#define FORTYTHREE	(  43L )
#define FORTYFOUR 	(  44L )
#define FORTYFIVE 	(  45L )
#define FORTYSIX	(  46L )
#define FORTYSEVEN	(  47L )
#define FORTYEIGHT	(  48L )
#define FORTYNINE	(  49L )
#define FIFTY		(  50L )
#define FIFTYONE	(  51L )
#define FIFTYTWO	(  52L )
#define FIFTYTHREE	(  53L )
#define FIFTYFOUR 	(  54L )
#define FIFTYFIVE 	(  55L )
#define FIFTYSIX	(  56L )
#define FIFTYSEVEN	(  57L )
#define FIFTYEIGHT	(  58L )
#define FIFTYNINE	(  59L )
#define SIXTY		(  60L )
#define SIXTYONE	(  61L )
#define SIXTYTWO	(  62L )
#define SIXTYTHREE	(  63L )
#define SIXTYFOUR 	(  64L )
#define SIXTYFIVE 	(  65L )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* file seek points                                      */
      /*                                                       */
      /*********************************************************/

#define FILEBEGIN	( ZERO )
#define FILECURRENT	( ONE )
#define FILEEND		( TWO )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* inode disk block address constants                    */
      /*                                                       */
      /*********************************************************/

#ifndef NADDR
#define NADDR		( THIRTEEN )
#endif

#define IND_3		( NADDR - ONE )
#define IND_2		( NADDR - TWO )
#define IND_1		( NADDR - THREE )
#define DIRECT		( NADDR - THREE )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* miscellaneous                                         */
      /*                                                       */
      /*********************************************************/

#define INVALID		( M_ONE )
#define CLOSED		( M_ONE )
#define COMMAND		( ZERO )
#define NORMAL		( ZERO )
#define NULSTR		( "" )
#define MAXLINE		( 512L )
#define SCRNWDTH	( 80L )
#define IPRORELNO	( "1.0" )
#define BITSPERCHAR	( EIGHT )
#define DSKADDRSZ	( THREE )
#define MAXFSBLKS\
         ( (LONG)( ( ONE LSHIFT ( DSKADDRSZ * BITSPERCHAR ) ) - ONE ) )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* standard data types                                   */
      /*                                                       */
      /*********************************************************/

typedef int		INT;
typedef INT	       *INTPTR;
typedef ushort		USHORT;
typedef unsigned	UNSIGNED;
typedef ino_t		INO_T;
typedef time_t		TIME_T;
typedef char		CHAR;
typedef CHAR	       *CHARPTR;
typedef long		LONG;
typedef LONG	       *LONGPTR;
typedef LONG		INDIRECT;
typedef INDIRECT       *INDPTR;
typedef double		DOUBLE;
typedef INT		VOID;
typedef LONG		BOOLEAN;
typedef CHAR		LINEBUF[ MAXLINE ];
typedef struct tm	TIME;
typedef TIME	       *TIMEPTR;
typedef FILE	       *FILEPTR;
#define EXTERNAL	extern
#define STATIC		static

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* inode types                                           */
      /*                                                       */
      /*********************************************************/

typedef struct dinode	INODE;
#define INODESIZE	( sizeof( INODE ) )
typedef INODE	       *INODEPTR;
typedef LONG		INOADDRBUF[ NADDR ];
#define INOBLKOFF	( TWO )
#define MAXINOBLKS\
           ( (LONG)( ( ONE LSHIFT ( sizeof( USHORT ) * BITSPERCHAR ) )\
                                                  - INOBLKOFF - ONE ) )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* directory types                                       */
      /*                                                       */
      /*********************************************************/

typedef struct direct	DIRECTORY;
#define DIRSIZE		( sizeof( DIRECTORY ) )
typedef DIRECTORY      *DIRPTR;

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* boot/super/indirect/free block types                  */
      /*                                                       */
      /*********************************************************/

#define BOOTBLKOFF	( ZERO )
#define SUPBLKOFF	( ONE )
typedef struct filsys	SUPERBLOCK;
typedef struct fblk	FREEBLOCK;

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* file status types                                     */
      /*                                                       */
      /*********************************************************/

typedef struct stat	STAT;
typedef STAT	       *STATPTR;

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* min/max/sqr/cube/range functions                      */
      /*                                                       */
      /*********************************************************/

#define MIN2( a, b )	( (( a ) LE ( b )) ? ( a ) : ( b ) )
#define MAX2( a, b )	( (( a ) GE ( b )) ? ( a ) : ( b ) )
#define SQR( x )	( ( x ) * ( x ) )
#define CUBE( x )	( ( x ) * SQR( ( x ) ) )
#define INRANGE( low, test, high )\
                 ( (( test ) GE ( low ))  AND  (( test ) LE ( high )) )

#define XCHG( x1, x2, t )  t = x1;  x1 = x2;  x2 = t;

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* inode table structure type                            */
      /*                                                       */
      /*********************************************************/

struct inodeinfo
{
   LONG   modtime;
   USHORT xchgsrc;
   USHORT xchgdst;
   USHORT dirdest;
};

typedef struct inodeinfo INODEINFO;

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* block table structure type                            */
      /*                                                       */
      /*********************************************************/

typedef LONG BLOCKINFO;

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* message type strings                                  */
      /*                                                       */
      /*********************************************************/

#define INFORMATIVE	( "INFO" )
#define PROCEDURAL	( "PROC" )
#define FATAL		( "FATAL" )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* command line option constants                         */
      /*                                                       */
      /*********************************************************/

#define VERBOSE		( 'v' )
#define CLFILSYS	( 'c' )
#define CLFILNAM	( 'C' )
#define SIFILSYS	( 's' )
#define SIFILNAM	( 'S' )
#define DELTA_INODEBLKS	( 'i' )
#define DELTA_FREEBLKS	( 'f' )
#define LOGFILE		( 'l' )
#define OPTNLTRS	( "vcCsSi:f:l:" )   /*** for getopt() ***/

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* file operations                                       */
      /*                                                       */
      /*********************************************************/

#define READ		( O_RDONLY )
#define WRITE		( O_WRONLY )
#define UPDATE		( O_RDWR )

#define FP_READ		( "r" )
#define FP_WRITE	( "w" )
#define FP_APPEND	( "a" )

#define FP_RD_UPDATE	( "r+" )
#define FP_WR_UPDATE	( "w+" )
#define FP_AP_UPDATE	( "a+" )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* default message strings                               */
      /*                                                       */
      /*********************************************************/

#define DEFTBLNAME	( "default" )
#define DEFMSGTYPE	( INFORMATIVE )
#define DEFCMDNAME	( "ipro" )
#define DEFFSNAME	( "default" )
#define DEFMESSAGE	( "default" )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* table names                                           */
      /*                                                       */
      /*********************************************************/

#define INOTBLNAME	( "INODE" )
#define DIRLSTNAME	( "DIRECTORY-LIST" )
#define DIRBLKNAME	( "DIRECTORY-BLOCK" )
#define DIRTBLNAME	( "DIRECTORY" )
#define BLKTBLNAME	( "BLOCK" )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* memory operation types                                */
      /*                                                       */
      /*********************************************************/

#define ALLOC_MEM	( ZERO )
#define INIT_MEM	( ONE )
#define ALLOC_INIT_MEM	( TWO )
#define DEALLOC_MEM	( THREE )
#define COPY_MEM	( FOUR )
#define COPY_INIT_MEM	( FIVE )
#define XCHG_MEM	( SIX )

      /*********************************************************/
      /*********************************************************/
      /*                                                       */
      /* time values                                           */
      /*                                                       */
      /*********************************************************/

#define MICRO_BPM	( 1045.0 )  /***  880.0 w/ipro overhead    ***/
#define SMD_BPM		( 1305.0 )  /*** 1090.0 w/ipro overhead    ***/
                                    /*** determined empirically,   ***/
                                    /*** these constants are       ***/
                                    /*** machine-dependent and are ***/
                                    /*** for completion time       ***/
                                    /*** reporting purposes only;  ***/
                                    /*** change them with the      ***/
                                    /*** port.  they are rough     ***/
                                    /*** guesstimates of the       ***/
                                    /*** number of out-of-place    ***/
                                    /*** blocks that can be        ***/
                                    /*** reordered per minute on   ***/
                                    /*** Micropolis and SMD        ***/
                                    /*** drives on a MOTOROLA      ***/
                                    /*** Series 20 respectively.   ***/
                                    /*** also, leave them as       ***/
                                    /*** floating-point constants. ***/

#define SECPMIN		( SIXTY )
#define MINPHOUR	( SIXTY )
#define SECPHOUR	( SECPMIN * MINPHOUR )
#define HOURPDAY	( TWENTYFOUR )
#define SECPDAY		( SECPHOUR * HOURPDAY )

#ifndef Fs16b
#define Fs16b		( FIVE )
#endif

#ifndef Fs8b
#define Fs8b		( FOUR )
#endif

#ifndef Fs4b
#define Fs4b		( THREE )
#endif

#define MRIOPHYSBLKS	( TWO )
#define PHYSBLKSZ	( 512L )
#define OBSIZE		( 512L )
#define TMPBLKSZ	( 1024L )
#define V68SUPOFF	( 512L )
#define WRKBLKCNT	( EIGHT )

#define GETWRKBLK( wb, wbc, wbi, bs )                                 \
(CHARPTR)(                                                            \
   ( ( ( wb ) NE NULL )                            AND                \
   ( ( wbc ) GT ZERO )                             AND                \
   ( INRANGE( ZERO, ( wbi ), ( wbc ) - ONE ) )     AND                \
   ( ( bs ) GT ZERO ) )                            ?                  \
   ( (CHARPTR)( wb ) + wbi++ * ( bs ) )            :                  \
   ( NULL )                                                           \
)

#define PUTWRKBLK( wb, wbc, wbi, bs )                                 \
(CHARPTR)(                                                            \
   ( ( ( wb ) NE NULL )                            AND                \
   ( ( wbc ) GT ZERO )                             AND                \
   ( INRANGE( ONE, ( wbi ), ( wbc ) ) )            AND                \
   ( ( bs ) GT ZERO ) )                            ?                  \
   ( wbi--, NULL )                                 :                  \
   ( ONE )                                                            \
)

   /************************************************************/
   /************************************************************/
   /*                                                          */
   /* end IPRO_definitions                                     */
   /*                                                          */
   /************************************************************/
