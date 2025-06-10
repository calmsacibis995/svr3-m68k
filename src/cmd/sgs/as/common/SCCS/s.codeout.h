h31381
s 00000/00000/00034
d D 1.6 86/09/16 15:28:10 fnf 6 5
c Changes to make ident directive work.
e
s 00000/00000/00034
d D 1.5 86/09/15 14:01:15 fnf 5 4
c Changes to remove multisegment capability for "init" and "lib" sections.
e
s 00000/00000/00034
d D 1.4 86/09/11 13:01:47 fnf 4 3
c Finish work for shared libraries, checkpoint changes.
e
s 00000/00000/00034
d D 1.3 86/08/18 08:48:00 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00000/00000/00034
d D 1.2 86/08/07 08:03:04 fnf 2 1
c Checkpoint changes for shared lib version.  Still does not work
c but at least accepts "init" and "lib" directives without crashing now...
e
s 00034/00000/00000
d D 1.1 86/08/04 09:37:59 fnf 1 0
c Start with baseline 5.2 68000 assembler.
e
u
U
t
T
I 1
/*	@(#)codeout.h	6.1		*/
#if ONEPROC
#define TBUFSIZE 512
#else
#define TBUFSIZE 10
#endif

typedef struct {
	long cvalue;
#if VAX
	unsigned char caction;
	unsigned char cnbits;
	int cindex;
#else
	unsigned short caction;
	unsigned short cindex;
	unsigned short cnbits;
#endif
} codebuf;

typedef struct symentag {
	long stindex;
#if FLDUPDT
	long fcnlen;
#endif
	long fwdindex;
	struct symentag *stnext;
} stent;

typedef struct {
	long relval;
	char *relname;
	short reltype;
} prelent;
E 1
