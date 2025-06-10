
#include "/mot/include/access.h"
#include <string.h>

main (argc, argv)
int argc;
char *argv[];
{
struct usrdev disk;

strcpy ( disk.look_for, argv[1] );
getperms (&disk);
}

