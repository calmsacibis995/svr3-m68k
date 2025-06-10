/*
 *  Configuration information
 */


#include	"config.h"
#include	"sys/conf.h"
#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/sysmacros.h"
#include	"sys/space.h"
#include	"sys/io.h"


extern nodev(), nulldev();
extern int fsstray(),fsnull(),*fsistray();
extern struct inode *fsinstray();
#define	notty	(struct tty *)(0)
#define	nostr	(struct streamtab *)(0)

extern cpinit();
extern s5init(), s5iput(), s5iupdat(), s5readi(), s5writei(), s5itrunc(), 
s5statf(), s5namei(), s5mount(), s5umount(), s5openi(), s5closei(), s5update(), 
s5statfs(), s5access(), s5getdents(), s5allocmap(), *s5freemap(), s5readmap(), s5setattr(), s5notify(), 
s5fcntl(), s5ioctl();
extern struct inode *s5iread(), *s5getinode();
extern m564open(), m564close(), m564read(), m564write(), m564ioctl(), m564init(); extern struct tty m564_tty[];
extern m320bopen(), m320copen(), m320bclose(), m320cclose(), m320read(), m320write(), m320ioctl(), m320init(), m320strategy(), m320print();
extern m323open(), m323close(), m323read(), m323write(), m323ioctl(), m323init(), m323strategy(), m323print();
extern m331open(), m331close(), m331read(), m331write(), m331ioctl(), m331init(); extern struct tty m331_tty[];
extern m350bopen(), m350copen(), m350bclose(), m350cclose(), m350read(), m350write(), m350ioctl(), m350init(), m350strategy(), m350print();
extern m355open(), m355close(), m355read(), m355write(), m355ioctl(), m355init(), m355strategy(), m355print();
extern m360bopen(), m360copen(), m360bclose(), m360cclose(), m360read(), m360write(), m360ioctl(), m360init(), m360strategy(), m360print();
extern msginit();
extern ramdopen(), ramdclose(), ramdread(), ramdwrite(), ramdinit(), ramdstrategy(), ramdprint();
extern sxtopen(), sxtclose(), sxtread(), sxtwrite(), sxtioctl();
extern seminit();
extern shminit();
extern m204init();
extern erropen(), errclose(), errread(), errinit();
extern mmread(), mmwrite();
extern syopen(), syread(), sywrite(), syioctl();

struct bdevsw bdevsw[] = {
/* 0*/	nodev, 	nodev, 	nodev, 	nodev,
/* 1*/	nodev, 	nodev, 	nodev, 	nodev,
/* 2*/	m320bopen,	m320bclose,	m320strategy,	m320print,
/* 3*/	m350bopen,	m350bclose,	m350strategy,	m350print,
/* 4*/	m355open,	m355close,	m355strategy,	m355print,
/* 5*/	ramdopen,	ramdclose,	ramdstrategy,	ramdprint,
/* 6*/	m360bopen,	m360bclose,	m360strategy,	m360print,
/* 7*/	m323open,	m323close,	m323strategy,	m323print,
};

struct cdevsw cdevsw[] = {
/* 0*/	m564open,	m564close,	m564read,	m564write,	m564ioctl,	m564_tty,	nostr,
/* 1*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/* 2*/	syopen,	nulldev,	syread,	sywrite,	syioctl,	notty,	nostr,
/* 3*/	nulldev,	nulldev,	mmread,	mmwrite,	nodev, 	notty,	nostr,
/* 4*/	erropen,	errclose,	errread,	nodev, 	nodev, 	notty,	nostr,
/* 5*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/* 6*/	ramdopen,	ramdclose,	ramdread,	ramdwrite,	nodev, 	notty,	nostr,
/* 7*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/* 8*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/* 9*/	sxtopen,	sxtclose,	sxtread,	sxtwrite,	sxtioctl,	notty,	nostr,
/*10*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*11*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*12*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*13*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*14*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*15*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*16*/	m355open,	m355close,	m355read,	m355write,	m355ioctl,	notty,	nostr,
/*17*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*18*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*19*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*20*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*21*/	m320copen,	m320cclose,	m320read,	m320write,	m320ioctl,	notty,	nostr,
/*22*/	m350copen,	m350cclose,	m350read,	m350write,	m350ioctl,	notty,	nostr,
/*23*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*24*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*25*/	m360copen,	m360cclose,	m360read,	m360write,	m360ioctl,	notty,	nostr,
/*26*/	m323open,	m323close,	m323read,	m323write,	m323ioctl,	notty,	nostr,
/*27*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*28*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*29*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	notty,	nostr,
/*30*/	m331open,	m331close,	m331read,	m331write,	m331ioctl,	m331_tty,	nostr,
};

struct fstypsw fstypsw[] = {
	fsstray,fsstray,fsinstray,fsstray,fsstray,fsstray,fsstray,fsstray,
	fsstray,fsstray,fsstray,fsstray,fsinstray,fsstray,fsstray,fsstray,
	fsstray,fsstray,fsstray,fsstray,fsistray,fsstray,fsstray,fsstray,
	fsstray,fsstray,fsstray,fsstray,fsstray,fsstray,fsstray,fsstray,

	s5init, s5iput, s5iread, fsstray,s5iupdat, s5readi, s5writei, s5itrunc,
 	s5statf, s5namei, s5mount, s5umount, s5getinode, s5openi, s5closei, s5update,
 	s5statfs, s5access, s5getdents, s5allocmap, s5freemap, s5readmap, s5setattr, s5notify,
 	s5fcntl, fsstray,s5ioctl, fsstray,fsstray,fsstray,fsstray,fsstray,
};

#define	NFSTYPE	2
short	 nfstyp = {NFSTYPE};
int	bdevcnt = 8;
int	cdevcnt = 31;

char MAJOR[31] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,};
char MINOR[31] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

dev_t	rootdev = makedev(7, 0);
dev_t	pipedev = makedev(7, 0);
dev_t	dumpdev = makedev(3, 0);
extern m350dump();
int	(*dump)() = m350dump;
int	dump_addr = 0xffff5000;
dev_t	swapdev = makedev(7, 1);
daddr_t	swplo = 1;
int	nswap = 17279;


int	m320_addr[] = {
	0xffffb000,
	0xffffac00,
};
int	m320_cnt = 64;
int	m320_ivec[] = {
	0x3f4,
	0x3f0,
};
int	m320_ilev[] = {
	5,
	5,
};

int	m350_addr[] = {
	0xffff5000,
	0xffff5100,
};
int	m350_cnt = 2;
int	m350_ivec[] = {
	0x3cc,
	0x3c8,
};
int	m350_ilev[] = {
	3,
	3,
};
int	m350_bmaj = 3;
int	m350_cmaj = 22;

int	m355_addr[] = {
	0xffff0800,
};
int	m355_cnt = 1;
int	m355_ivec[] = {
	0x11c,
};
int	m355_ilev[] = {
	5,
};
int	m355_bmaj = 4;
int	m355_cmaj = 16;

int	ramd_cnt = 1;

int	m360_addr[] = {
	0xffff0c00,
	0xffff0e00,
};
int	m360_cnt = 64;
int	m360_ivec[] = {
	0x3e4,
	0x3dc,
};
int	m360_ilev[] = {
	5,
	5,
};
int	m360_bmaj = 6;
int	m360_cmaj = 25;

int	m323_addr[] = {
	0xffffa000,
};
int	m323_cnt = 4;
int	m323_ivec[] = {
	0x324,
};
int	m323_ilev[] = {
	3,
};

int	m564_addr[] = {
	0xfffb0040,
};
int	m564_cnt = 2;
int	m564_ivec[] = {
	0x110,
};
struct	tty	m564_tty[2];




int	sxt_cnt = 6;

int	m331_addr[] = {
	0xffff6000,
	0xffff6100,
	0xffff6200,
	0xffff6300,
};
int	m331_cnt = 32;
int	m331_ivec[] = {
	0x3bc,
	0x3b8,
	0x3b4,
	0x3b0,
};
int	m331_ilev[] = {
	2,
	2,
	2,
	2,
};
struct	tty	m331_tty[32];

int	(*pwr_clr[])() = 
{
	(int (*)())0
};

int	(*io_init[])() = 
{
	m320init,
	m350init,
	m355init,
	ramdinit,
	m360init,
	m323init,
	m564init,
	errinit,
	m331init,
	cpinit,
	msginit,
	seminit,
	shminit,
	m204init,
	(int (*)())0
};

#include "sys/ram.h"

char sysdefr;
struct ram ram_tbl[] = {
	{0x0, 0x3c00, 0x3c00, 0x1}
};

unsigned ram_nbr = sizeof(ram_tbl)/sizeof(struct ram);


struct probe probe_tbl[] = {
	{(char*)0xfffc0000, 0xf1},
	{(char*)0, 0}
};

short sysdefp = 0;
