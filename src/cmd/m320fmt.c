/*  @(#)m320fmt.c	6.1    */
/*
  This utility program invokes the format facilities of the mvme320
  driver through ioctl. It is intended to be used both as a free-standing
  facility for formatting floppy disks and as the format stage of
  program dinit.

  usage:

    m320fmt [hard_disk_enable -h <heads> -c <cylinders>] <raw_device> [-F]

    The phrase "hard_disk_enable" must appear literally as shown to enable
    the program to format a hard disk. The head and cylinder counts are
    required for hard disk formatting and ignored (defaults 2 and 80) for
    floppy disks. For a hard disk the <raw_device> may be any device which
    maps to the correct controller and physical unit. For a floppy the
    <raw_device> must map correctly AND refer to a volume partition (slice)
    which spans the entire volume. Conventionally this is slice 7.

    The -F option is included for compatibility with dinit. Given the intent
    of its use there, it effectively no-ops the m320fmt call.
*/

#include "stdio.h"
#include "fcntl.h"
#include "sys/types.h"
#include "sys/stat.h"

/* file status request buffer */
static struct stat statbuf;

/* mvme320 driver ioctl function codes */
#define M320FMTT 1  /* format track (containing block number) */
#define M320FMTV 2  /* format volume */

/* options switches and variables */
static int hdenable = 0;
static int Fopt = 0;
static char *devp = 0;

/* args buffer passed to ioctl */
static int args [2] = {0,0};
#define block args[0]
#define heads args[0]
#define cyls  args[1]

/* options error message */
static invalid ()
{
  fprintf (stderr, "m320fmt: invalid options\n");
  exit (1);
}

/* get a numeric value for option */
static int getv (argv, rslt)
  char **argv;
  int *rslt;
{
  int x = 0;
  char *p;

  p = *argv + 2;  /* point to char following -<letter> */
  if (*p == '\0')  /* if end of string ... */
  {
    p = *(++argv);  /* step to next *argv */
    x = 1;  /* set return */
  }
  if (p == 0 || sscanf (p, "%d", rslt) < 1)  /* get val, error check */
    invalid ();
  return x;  /* x = 1 tells caller that another *argv was used */
}

/* parse line, get options */
static getopts (argc, argv)
  int argc;
  char **argv;
{
  if (*(++argv) == 0)
    return;
  if (strcmp (*argv, "hard_disk_enable") == 0)
  {
    hdenable = 1;
    ++argv;
  }
  while (*argv != 0)
  {
    if (strncmp (*argv, "-h", 2) == 0)
      argv += getv (argv, &heads);
    else if (strncmp (*argv, "-c", 2) == 0)
      argv += getv (argv, &cyls);
    else if (strcmp (*argv, "-F") == 0)
      Fopt = 1;
    else if (**argv != '-')
      devp = *argv;
    else
      invalid ();
    ++argv;
  }
}

/* ioctl call with error and message */
static iocon (fd, cmd, args)
  int fd;
  int cmd;
  int *args;
{
  if (ioctl (fd, cmd, args) == -1)
  {
    fprintf (stderr, "m320fmt: i/o error during format operation\n");
    exit (2);
  }
}

main (argc, argv)
  int argc;
  char **argv;
{
  int fd, mode, isflop, trk;

  /* get options */
  getopts (argc, argv);

  /* no device ? */
  if (devp == 0)
  {
    fprintf (stderr, "m320fmt: missing device\n");
    exit (1);
  }

  /* mysterious F option */
  if (Fopt)
    exit (0);

  /* open device, get status */
  if ((fd = open (devp, O_WRONLY)) == -1 || fstat (fd, &statbuf) == -1)
  {
    fprintf (stderr, "m320fmt: can't open %s\n", devp);
    exit (1);
  }

  /* must be char special or block special */
  mode = statbuf.st_mode & 0170000;
  if (mode == 020000 || mode == 060000)
    /* ok */;
  else
  {
    fprintf (stderr, "m320fmt: %s is not a valid device (mode %o)\n",
        devp, statbuf.st_mode);
    exit (1);
  }

  /* set flag for floppy by unit number in minor device */
  isflop = ((statbuf.st_rdev >> 3) & 3) > 1 ? 1 : 0;

  /* if hard disk enable, not a floppy and heads/cyls valued */
  if (hdenable && !isflop && heads > 0 && cyls > 0)
  {
    fprintf (stderr,
        "m320fmt: WARNING - controller will remain busy until format ends\n");
    /* format volume which also writes head/cyl info */
    iocon (fd, M320FMTV, args);
    exit (0);
  }

  /* if no hard disk enable and it is a floppy */
  else if (!hdenable && isflop)
  {
    /* issue warning if not slice 7 in minor device */
    if (statbuf.st_rdev & 7 != 7)
      fprintf (stderr,
          "m320fmt: WARNING - floppy device is not slice 7\n");
    /* track 0 has only 4 blocks */
    block = 0;
    iocon (fd, M320FMTT, args);
    /* remaining 159 tracks have 8 blocks */
    block = 4;
    for (trk = 1; trk < 160; ++trk)
    {
      /* track-by-track to prevent controller lockout */
      iocon (fd, M320FMTT, args);
      block += 8;
    }
    exit (0);
  }

  /* options error */
  else
    invalid ();
}
