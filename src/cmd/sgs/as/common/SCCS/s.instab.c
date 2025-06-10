h02137
s 00000/00000/00009
d D 1.6 86/09/16 15:28:20 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00009
d D 1.5 86/09/15 14:01:28 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00009
d D 1.4 86/09/11 13:01:56 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00009
d D 1.3 86/08/18 08:48:07 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00009
d D 1.2 86/08/07 08:03:13 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00009/00000/00000
d D 1.1 86/08/04 09:38:02 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)instab.c	6.1		*/
#include "systems.h"
#include "symbols.h"
#include <instab.h>
#include <parse.h>
instr instab[] = {
#include "ops.out"
0
};
E 1
