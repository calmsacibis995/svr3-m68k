/* sgkey.h
 *
 */

/**********************************KEYWORDS***********************************/

/* File Strings
 *
 */
#define S_DESCFILE	"Description File:"
#define S_MACHFILE	"Machine File:"

/* Description Key
 *
 */
#define S_DESCKEY	"Description Keyword:"

/* Device Strings
 *
 */
#define S_DRVDESC	"Device Description:"
#define S_DRVNAME	"Device Name:"
#define S_DRVHDLR	"Handler Prefix:"
#define S_DRVIVS	"Interrupt Vectors Size:"
#define S_DRVMBDN	"Major Block Device Number:"
#define S_DRVMCDN	"Major Character Device Number:"
#define S_DRVMBRL	"Maximum Bus Request Level:"
#define S_DRVMDPC	"Maximum Devices Per Controller:"
#define S_DRVPRS	"Page Registers Size:"

/* Character/Block Device Mask Field Character Strings
 *
 */
#define S_BCMASK	"Block Or Character Device Handler Lines:"

#define S_DRVSTR	"Is a stream handler."
#define S_DRVSBCH \
	"Has separate open/close routines for block/character devices."
#define S_DRVTTY	"Has a tty structure."
#define S_DRVINIT	"Has an initialization handler."
#define S_DRVPFAIL	"Has a power-failure handler."
#define S_DRVOPEN	"Has an open handler."
#define S_DRVCLOSE	"Has a close handler."
#define S_DRVREAD	"Has a read handler."
#define S_DRVWRITE	"Has a write handler."
#define S_DRVIOCTL	"Has an ioctl handler."

/* Filesystem Handler Strings
 *
 */
#define S_FILSYS	"Filesystem Handler Description:"

#define S_FSHNM		"Filesystem Handler Name:"
#define S_FSHMASK	"Filesystem Handler Device Handler Lines:"

#define S_FSHIOCTL	"Has an ioctl routine."
#define S_FSHINFO	"Has an info routine."
#define S_FSHCTRL	"Has a control routine."
#define S_FSHNOTIFY	"Has a notify routine."
#define S_FSHSATTR	"Has a set attr routine."
#define S_FSHRDMAP	"Has a read map routine."
#define S_FSHFRMAP	"Has a free map routine."
#define S_FSHALMAP	"Has a allocate map routine."
#define S_FSHGDENTS	"Has a get dents routine."
#define S_FSHACCESS	"Has an access routine."
#define S_FSHSTATUS	"Has a status routine."
#define S_FSHUPDATE	"Has an update routine."
#define S_FSHCLINOD	"Has a close inode routine."
#define S_FSHOPINOD	"Has an open inode routine."
#define S_FSHGTINOD	"Has a get inode routine."
#define S_FSHUMOUNT	"Has a umount routine."
#define S_FSHMOUNT	"Has a mount routine."
#define S_FSHNAMEI	"Has a namei routine."
#define S_FSHFSTAT	"Has a status file routine."
#define S_FSHITRUNC	"Has an inode trunc routine."
#define S_FSHINIT	"Has an init routine."
#define S_FSHWRINOD	"Has a write inode routine."
#define S_FSHRDINOD	"Has a read inode routine."
#define S_FSHUPINOD	"Has an update inode routine."
#define S_FSHIREAD	"Has an inode read routine."
#define S_FSHIPUT	"Has an inode put routine."

/* Other Device/Filesystem Handler Strings
 *
 */
#define S_DRVTYPE	"Device Type Information:"
#define S_DRVCIVA	"Create interrupt vector array for this device."
#define S_DRVCCBMN \
	"Create character and/or block major device numbers for this device."
#define S_DRVCILA	"Create interrupt level array for this device."
#define S_DRVONE	"Allow only one of these devices."
#define S_DRVSCF	"Do not create a count field for this device."
#define S_DRVSIV	"Do not create an interrupt vector for this device."
#define S_DRVREQ	"This is a required device."
#define S_DRVBLOCK	"This is a block device."
#define S_DRVCHAR	"This is a character device."
#define S_DRVIDD	"This is an interrupt driven device."
#define S_DRVSVMA	"Allow one vector definition with multiple addresses."

#define S_DRVDATA	"Device Configuration Tables Declarations:"
#define S_DRVCTSD	"Device config table structure declaration:"


/* Board Description Strings
 *
 */
#define S_DEVDESC	"Board Description:"
#define S_DEVTYPE	"Device Type:"
#define S_DEVBRL	"Bus Request Level:"
#define S_DEVADDR	"Address:"
#define S_DEVIVL	"Interrupt Vector Location:"
#define S_DEVNOD	"Number Of Devices:"
#define S_DEVDNOD	"Default Number Of Devices:"

/* System Devices Specification Strings
 *
 */
#define S_SYSDEV	"System Devices Specification:"
#define S_SDROOT	"Root Device Name:"
#define S_SDPIPE	"Pipe Device Name:"
#define S_SDDUMP	"Dump Device Name:"
#define S_SDSWAP	"Swap Device Name:"

#define S_MINOR		"Minor Device Number Specification"
#define S_MROOT		"Root Minor Device Number:"
#define S_MPIPE		"Pipe Minor Device Number:"
#define S_MDUMP		"Dump Minor Device Number:"
#define S_MSWAP		"Swap Minor Device Number:"
#define S_MSWPLO	"Lowest Disk Block In Swap Area:"
#define S_MNSWAP	"Number Of Disk Blocks In Swap Area:"

/* Parameter Description Strings
 *
 */
#define S_PARDESC	"Parameter Description:"
#define S_PARNAME	"Parameter Name:"
#define S_PARCONF	"Conf.c Name:"
#define S_PARDVAL	"Default Value:"

/* Microprocessor Specification Strings
 *
 */
#define S_MPU		"MPU Specification:"
#define S_MPUNUM	"MPU Type:"

/* Non-Unique Driver Specification Strings
 *
 */
#define S_FORCE		"Non-Unique Driver Specification:"
#define S_FRID		"Unique Identifier:"

/* Memory Probe Specification Strings
 *
 */
#define S_PROBE		"Memory Probe Specification:"
#define S_PRBADDR	"Address To Probe:"
#define S_PRBVAL	"Assign Value:"

/* Alien Handler Entry Specification
 *
 */
#define S_ALIEN		"Alien Handler Entry Specification:"
#define S_ALVEC		"Alien Exception Vector Address:"
#define S_ALADDR	"Alien Entry Point:"
#define S_ALNAME	"Alien Handler Name:"

/* Multiple Handler Specification Strings
 *
 */
#define S_DUP		"Multiple Handler Specification:"
#define S_DUPINTRET	"On 0, go to the normal interrupt return point."
#define S_DUPALLTRAPS	"On 0, go to the normal trap return point."
#define S_DUPBEQRTS	"On 0, go to the branch equal return point."
#define S_DUPARG	"Argument is to be passed to handler."
#define S_DUPVEC	"Duplicate's Exception Vector:"
#define S_DUPHDLR	"Duplicate's Name:"
#define S_DUPARGVAL	"Optional Handler Argument:"

/* Memory Configuration Specification Strings
 *
 */
#define S_RAM		"Memory Configuration Specification:"
#define S_RAMNOPARTY	"Memory has no parity check."
#define S_RAMSINGLE	"A single memory block may exist."
#define S_RAMMULTIPLE	"Multiple memory blocks may exist."
#define S_RAMLOW	"Low Memory Address:"
#define S_RAMHIGH	"High Memory Address:"
#define S_RAMSIZE	"Memory Block Size:"

/* Machine Description Strings
 *
 */
#define S_CNFGFILE "Select Description File:"
#define S_CNFGVERS "Select File's Version:"
#define S_CNFGITEM "Select Description:"
#define S_CNFGVALU "Value:"
