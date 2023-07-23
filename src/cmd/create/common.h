/*
	common.h -- header file common to create and upgrade

	Revision history:
		DS	2-Jun-86	added create_ver_rev_id_fmt
		DS	9-May-86	changed size of eov_blk from BSIZE/4 to constant 1024/4 
		DS	29-Apr-86	original VME version (David Schwartz)
*/

#define REVISION 1
#define VERSION 3
/* ALL RELEASES OF CREATE OR UPGRADE WITH THE SAME VERSION # MUST HAVE THE
** SAME INTERNAL STRUCTURE.
*/

#ifndef MOD_misc
	static int _version = VERSION; 
	static int _revision = REVISION;
#	ifdef UPDATE
		static int _update = UPDATE;
#	else
		static int _update = 101;
#	endif
#endif

#ifdef RELEASE
	static char _copyright[] =
		"Copyright 1984,86 Motorola Computer Systems, Inc.";
#else
	static char _copyright[] =
		"Motorola Computer Systems Corp. proprietary software";
#endif

#define FALSE 0
#define TRUE 1
#define MIN(x,y) (x<y?x:y)
#define MAX(x,y) (x>y?x:y)
#define NELTS(x) (sizeof(x)/sizeof(x[0]))
#define CLEAR	system("tput clear 2>/dev/null")

/* One block of this pattern is written prior to the checksum block */
/* to signify the logical end of volume. */
#define EOV_FLAG	0x9c5d30a7

extern int errno;

#ifndef MOD_checksum
	/* things in module checksum */ 
	extern unsigned long checksum();
	extern void make_crctable();
#endif

#ifdef MOD_misc
	/* things in module misc */
	char *media_name;
	char media_type;
	int  can_format_media, blocks_per_volume;
#else
	extern char ask();
	extern int request();
	extern int read_blk();
	extern int write_blk();
	extern void fatal1();
	extern unsigned long get_media_size();
	extern char *media_name;	/* name of media type */
	extern char media_type;	/* [f,1,c,t,9] for floppy, 1.2MB floppy, */
							/* disk cart, streamer tape, 9-track tape */
	extern int can_format_media; /* TRUE if this media_type can be formatted */
	extern int blocks_per_volume;
	extern int get_bsize();
#endif


				/******   global declarations  ******/

unsigned long crctable[256];
char crcbuf[1024];
static unsigned long disclimit, cnt, crc;
static char yes[] = "yes", no[] = "no";

FILE *in_stream;	/* input stream file descriptor (via popen) */
int media;			/* output media file pointer (via open) */
char *devname;		/* media device name */

/* the fmt's here are for use in sprintf commands */
static char audit_dir_name[] = "/usr/AUDITS";
static char make_audit_dir_cmd[] = "mkdir /usr/AUDITS";
static char toc_file_fmt[] = "/usr/AUDITS/%s.ls";
static char create_audit_file_fmt[] = "/usr/AUDITS/%s.cr0";
static char upgrade_audit_file_fmt[] = "/usr/AUDITS/%s.crc";
static char create_ver_rev_id_fmt[] = "create R%s.%s";
static char toc_file[80];
static char audit_file[80];

struct HDR			/* holds info to write at beginning of each disk */
{
	char create_id[32];	/* an ascii version/rev identifier for create */
	int version;		/* which version of create was used */
	int revision;		/* which revision of create was used */
	int vol_num;		/* which volume this is */
	int max_volumes;	/* max number of volumes needed by this product */
	int media_blk_cnt;	/* number of blocks needed */
	int src_bsize;		/* bytes/block in source system [512,1024] */
	int fs_blk_cnt;		/* num blocks used by product in source file system */
	int fs_inode_cnt;	/* number of inodes needed in source file system */
	char prodname[32];	/* product name */
	char rel_id[32];	/* release identifier */
	char audit_name[11];/* name used for auditing needs */
	char pre_cmds[128];	/* commands to be executed at beginning of Upgrade */
	char destdir[128];	/* the product's destination directory name */
	char post_cmds[128];/* commands to be executed at end of Upgrade */
} hdr;

static char buf_io[1024];	/* buffer for reading */
static char cmd[128];		/* general use buffer */
unsigned long eov_blk[1024/4];		/* used for end-of-volume flag */
