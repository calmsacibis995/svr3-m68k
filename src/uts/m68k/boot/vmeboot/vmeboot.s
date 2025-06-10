#	@(#)vmeboot.s	8.4 MODIFIED
#
#	This is the system bootloader.  It implements the functionality
#	specified in the System V/68 General Disk Bootloader specification
#	with the following exceptions and additions:
#
#	1) The VMEbug software abort functionality is supported for
#	debugging purposes.
#
#	2) An object file name may be passed from the VMEbug boot
#	command to the bootloader. Either a full path name or a path name
#	which assumes the present working directory is root, may be used.
#
#	3) The default object file need not be in the root directory
#	if its path is specified as follows:
#		[<up to 13 chars>,0 ]...<up to 13 chars> 0,0
#	For example:  name: byte 's,'t,'a,'n,'d,0,'s,'a,'s,'h,0,0
#	might be used to specify the standalone shell.
#
#	4) This bootloader is compatible with the SGS2 a.out file format.
#	It will be stored on disk in cylinder (usually track) 0 by the
#	disk init program "dinit" with VMEbug-compatible pointer/size
#	information in the volume-id block.
#	
#	5) The VMEbug disk read routine is used instead of including another
#	disk read routine here. This approach depends on several values
#	from VMEbug expected in registers when the bootloader begins execution.
#	It may also be used by 120BUG or 130BUG which have different mechanisms
# 	of disk reads.
#					120BUG	      130BUG	
#	return to BUG
#	   TRAP #15 value		0		$63

#	drive # 		d0	X		X
#	controller #		d1	X		X
#	disk config.		d2	X
#	"IPLz"			d4	X  z=$03	X  z=$c0
#	addr. cntrlr		a0	X		X
#	addr. program		a1	X		X
#	addr. disk config	a2	X
#	addr. BUG read routine  a3	X
#	start of text		a5	X		X
#	end of text+1		a6	X		X
#	stack			a7	X		X
#	Sup. Mode & lvl 7	SR	X		X
#
#
#	6) The folowing information is passed to the loaded program in
#	registers:
#
#	  d0 = boot device drive number
#	  d1 = boot device controller number (VMEbug assignment)
#	  d2 = booted file inode number
#
#	7) This bootloader supports both hardware and software bad
#	track/sector handling. Byte BADFLG in the config area must be
#	set non-zero to activate software bad track handling.
#
#  Bootloader Algorithm
#
#	 Fetch parameters passed in registers
#	 Read the disk configuration block (block 0)
#	 Read (possibly) continued bad track data (block 1)
#	 Repeat while components left in search path
#	    Execute get_inode for directory 
#	    Repeat while no match && blocks left in directory
#	       Execute read_next_block to get directory data block
#	       Repeat while no match && not out of directory entries
#	          If inode number is null
#		     error terminate
#	          Compare name to directory entry
#	       end repeat
#	    end repeat
#	    If no match 
#	       error terminate
#	 end repeat
#	 Execute get_inode for load module
#	 Read header into buffer
#	 If magic is MC68MAGIC
#           Get two more blocks into buffer
#	    Move vectors from buffer to text_start
#	    Move remaining code from buffer to memory
#	    Load remaining blocks into memory
#	    If data_start != textsize + text_start
#	       move datasize bytes into position at data_start
#	    Set entry address from header
#	 Else
#	    Load all blocks into memory beginning at address 0
#	    Set entry address to 0
#	 Jump to entry address
#
#-----------------------------------------------------------------------
# Conventions & Assumptions
#
#	Block numbers run from 0-n on the disk device.
#	Inode numbers run from 1-n beginning in block 1.
#
# Register conventions:
#	a6		- frame pointer
#	a5		- buffer pointer
#	a4		- target name pointer
#	a3		- directory entry pointer
#	d7		- long block number
#	d6		- long current inode number
#	d5		- long file block number*4
#	d0-d4,a0-a2	- scratch registers
#-----------------------------------------------------------------------
# Memory Usage
#
#				|          |	^
#				|          |	|
#				|  blkno[] |  (as reqd, usually 2k)
#				|          |	|
#				|          |	v
#		FRAMEPTR (a6)->	|----------|
#				|          |	^
#				|          |	|
#				|  buffer  | BUFSIZ (3k)
#				|          |	|
#				|          |	v
#			(a5) ->	|----------|
#				|          |	^
#				|  badtrk, |	|
#				|  config, | DSKHDRSIZ (2k)
#				|   vid    |	|
#				|          |	v
#		DSKHDR --->	|----------|
#				|          |	^
#				|          |	|
#				|   boot   | BOOTSIZ (2k)
#				|   text   |	|
#				|          |	v
#		HIGH --->	|----------|
#				|          |
#				|  stack   |
#				|          |
#				v          v
#-----------------------------------------------------------------------

# Data Structures
	set	BLKSIZ,1024		# no. bytes/block
#df	set	HDRSIZ,168		# bytes in header
	set	MC68MAGIC,0520		# expected file type
	set	BOOTSIZ,2048
	set	INOSIZ,64		# no. bytes/inode entry
	set	INOBLK,BLKSIZ/INOSIZ	# no. inodes/disc block
	set	INOMSK,0xf		# inode to block mask
	set	NAMSIZ,14		# no. bytes in name field of dir entry
	set	DIRSIZ,16		# size of directory entry, bytes
	set	ROOTINO,2		# root dir inode no.
	set	PATHLIM,18		# char limit for specified path
		# make sure default name (name:... ) defines PATHLIM chars
	set	FILL,0
	set	CLR,0
# Inode structure
	set	inode,0
	set	mode,inode
	set	nlink,mode+2
	set	uid,nlink+2
	set	gid,uid+2
	set	size,gid+2
	set	faddr,size+4
	set	time,faddr+40
# Header structure
	set	filehdr,0
	set	f_magic,filehdr		# long magic number
	set	tsize,f_magic+24	# long size of text
	set	dsize,tsize+4		# long size of data
	set	entry,dsize+8		# long entry point
	set	text_start,entry+4	# long base of text
	set	data_start,text_start+4	# long base of data
	set	text_scnptr,data_start+24# long file offset of text section
					# other header info not used
# Winchester Disk Constants		(wdc = winchester disk controller)
	set	SPB,4			# 4 sectors (256 byte) per block
	set	BPT,8			# logical (1024) blocks on track
# Other Constants, Variables and Offsets 
	set	HIGH,0xf0000
	set	DSKHDRSIZ,BLKSIZ*2	# vid,config,bad-track see note below
	set	DSKHDR,HIGH+BOOTSIZ	# pos. of Disk Header Block (Block 0)
	set	VIDBTL,DSKHDR+150	# bad track count (short)
	set	VIDRFS,DSKHDR+156	# root file system offset
	set	IOSATW,DSKHDR+265	# attribute word
	set	BADFLG,DSKHDR+154	# software bad track flag
	set	BADTAB,DSKHDR+768	# bad track table (tree order)
	set	STACK,HIGH
	set	BUFSIZ,BLKSIZ*3		# 3k buffer area
	set	FRAMEPTR,HIGH+BOOTSIZ+DSKHDRSIZ+BUFSIZ	# data reference ptr 
	set	NT,4*266		# blkno[null terminator]
	set	SAVREGS,0x71e		# d5-d7,a3-a6
	set	GETREGS,0x78e0		# d5-d7,a3-a6
	set	VECS,0x400		# Vector area is 1k for 68k machines
	set	BLK2INDEX,8		# byte offset of 3rd long in blkno[]

	
	set	DSKRD,0x0010		# Disk read routine
	set 	RT120,0			# 120BUG return to BUG 
	set	RT130,0x63		# 130BUG return to BUG
	set	OUT120,0x5		# 120BUG outln routine no LF/CR
	set	OUT130,0x21		# 130BUG outln routine no LF/CR
	set	OUT120LF,0x2		# 120BUG outln routine
	set	OUT130LF,0x22		# 130BUG outln routine

	set	INDIR1,4*10		# offset to single indirect block
	set	INDIR2,4*11		# offset to double indirect block
	set	BLKNO266,4*266		# offset to blkno[266]



# Bootloader Entry Info Expected by VMEbug

	org	0
boot:

	set	T,boot+BOOTSIZ+BUFSIZ+DSKHDRSIZ	# Text offset from frame pointer

	long	STACK			# initial stack pointer
	long	bootup			# bootloader entry

# Default load module name

name:	byte	's,'t,'a,'n,'d,0
	byte	's,'y,'s,'V,'6,'8
	byte	0,0,0,0,0,0		# PATHLIM chars defined in all
	byte	0,0			# double null terminator

	even	
ipl: 	byte	CLR,'I,'P,'L		# "IPL string for comparison with %d4

#
# Error messages
#

	even
vmerr:	byte	'V,'M,'E,'b
	byte	'o,'o,'t,' 
	byte	'e,'r,'r,'o
	byte	'r,':,' ,' 
vmend:	byte	0,0

	even
iplerr:	byte	'I,'P,'L,' 
	byte	'n,'o,'t,' 
	byte	's,'p,'e,'c
	byte	'i,'f,'e,'d
iplend: byte	0,0

	even
objerr:	byte	'C,'a,'n,''
	byte	't,' ,'f,'i
	byte	'n,'d,' ,'o
	byte	'b,'j,'e,'c
	byte	't,' ,'f,'i
	byte	'l,'e,'.,' 
objend:	byte	0,0

	even
magerr:	byte	'U,'n,'k,'n
	byte	'o,'w,'n,' 
	byte	'm,'a,'g,'i
	byte	'c,' ,'n,'u
	byte	'm,'b,'e,'r
	byte	'.,'.,'.,'a
	byte	't,'t,'e,'m
	byte	'p,'t,'i,'n
	byte	'g,' ,'e,'x
	byte	'e,'c,'u,'t
	byte	'i,'o,'n,'.
magend:	byte	0,0

	even
nulerr:	byte	'N,'u,'l,'l
	byte	' ,'o,'b,'j
	byte	'e,'c,'t,' 
	byte	'f,'i,'l,'e
	byte	'.,' ,' ,' 
nulend:	byte	0,0

	even
rderr:	byte	'R,'e,'a,'d
	byte	' ,'e,'r,'r
	byte	'o,'r,'.,' 
rdend:	byte	0,0


# Local storage

	even
bootinod: long	0			# inode number of loaded file
halt:	byte	0,0			# local halt flag (fill)

# Arguments received from VMEbug

dev:	byte	CLR			# boot device drive number
ctlr:	byte	CLR			# controller number
Tnum:	byte	CLR			# flag - which number for return to BUG
					#   0 = 130BUG -  0x63
					# <>0 = 120BUG -  0x0
IOtype: byte	CLR			# type of disk i/o supported by BUG
					#   0 = routine addr. in a3
					# <>0 = TRAP #15 - registers unchanged	
				
	even				#  values used by 120BUG
cfcode:	long	CLR			# disk configuration code (byte 5)
cfadr:	long	CLR			# configuration data address
ioadr:	long	CLR			# controller io address
readr:	long	CLR			# read routine address

	even
dskpkt:					# disk i/o packet for use by 130BUG
ctlLUN: byte	CLR			#  Controller LUN ( first 4 bits are 0 )
devLUN: byte	CLR			#  Device LUN
stat:	short	CLR			#  Status Word
memadd:	long	CLR			#  Memory Address
secnum:	long	CLR			#  Sector Number
numtf:	short	CLR			#  Number of Sectors to Transfer
	byte	CLR			#  fill
addmod:	byte	CLR			#  Addr. Modifier - not used by MVME320
	

# Bootloader entry


bootup:
	mov.l	%a6,%a4			# save end of name pointer
	mov.l	&FRAMEPTR,%a6		# set data reference pointer

	btst	&0,%d4			# determine which facilities the BUG
	beq.b	boot01			#    provides
	mov.b	&-1,Tnum-T(%a6)	
boot01:
	btst	&3,%d4			#  which type of disk i/o
	beq.b	boot02
	mov.b	&-1,IOtype-T(%a6)
boot02:
	lsr.l	&8,%d4			# pick up the chars. "IPL"
	lea.l	ipl,%a1
	cmp.l	%d4,(%a1)

	beq.b	boot03			# if not there, die
	lea.l	iplerr,%a0
	lea.l	iplend,%a1
	bsr	perror
	bra	die

boot03:	mov.b	%d0,dev-T(%a6)		# save boot device drive number
	mov.b	%d0,devLUN-T(%a6)	#   also in disk i/o packet
	mov.b	%d1,ctlr-T(%a6)		# save controller number
	mov.b	%d1,ctlLUN-T(%a6)	#   also in disk i/o packet
	mov.w	&SPB,numtf-T(%a6)	# sectors per transfer in i/o packet
	mov.l	%d2,cfcode-T(%a6)	# save disk configuration code
	mov.l	%a0,ioadr-T(%a6)	# save controller io address
	mov.l	%a2,cfadr-T(%a6)	# save configuration data adr
	mov.l	%a3,readr-T(%a6)	# store entry to bug's read routine
	mov.l	%a5,%a3			# save name pointer
boot0:
	clr.l	VIDRFS			# make root offset zero
	clr.w	VIDBTL			# make bad track count zero
	clr.l	%d7			# disk block 0
	mov.l	&DSKHDR,%a5		# read into DSKHDR
	bsr	rdblk			# do the read of the disk config block

# Note: Second block read is needed only for devices with > 64 bad tracks

	mov.l	VIDRFS,-(%sp)		# save root filesys
        mov.w	VIDBTL,-(%sp)		# save bad track count
	clr.l	VIDRFS			# zero again
	clr.w	VIDBTL			# ...
	mov.l	&1,%d7			# disk block 1
	add.l	&BLKSIZ,%a5		# second block area
	bsr	rdblk			# read second block
	mov.w	(%sp)+,VIDBTL		# restore bad track count
	mov.l	(%sp)+,VIDRFS		# restore root filesys

	tst.b	BADFLG			# nature of bad track support?
	blt.b	boot1			# less than zero, is software
	clr.w	VIDBTL			# clear count, no translation
boot1:

	lea.l	-BUFSIZ(%a6),%a5	# set buffer pointer

# Get object file name if passed from boot command

	cmp.l	%a3,%a4			# if default name is OK
	beq.b	boot8			#    go get started
	cmp.b	(%a3),&';		# halt option only?
	bne.b	boot2			# no
	add.l	&1,%a3			# skip the ;
	lea.l	name+PATHLIM,%a0	# fudge ptr for terminate
	bra.b	boot7			# go do option
boot2:
	cmp.b	(%a3),&'/		# skip leading /
	bne.b	boot3
	add.l	&1,%a3
boot3:
	mov.w	&PATHLIM-1,%d0		# move up to PATHLIM chars (-1 for dbcc)
	lea.l	name,%a0		# point to search key area

boot4:
	mov.b	(%a3)+,%d1		# get character
	cmp.b	%d1,&';			# semicolon marks end of name
	beq.b	boot7			# got one
	cmp.b	%d1,&'/			# change / to null
	bne.b	boot5
	clr.b	%d1
boot5:
#	cmp.b	%d1,&'A			# map UPPER to lower
#	blo.b	boot6
#	cmp.b	%d1,&'Z
#	bhi.b	boot6
#	add.b	&'a-'A,%d1
#boot6:
	mov.b	%d1,(%a0)+		# save this one
	cmp.l	%a3, %a4		# until all are fetched
	dbhs	%d0, boot4		#    || count run out
	bra.b	boot9			# go end string
boot7:
	cmp.l	%a3,%a4			# nothing after ; ?
	beq.b	boot9
	mov.b	(%a3),%d1		# get option char
	and.b	&0xDF,%d1		# lower-case off
	cmp.b	%d1,&'H			# halt?
	bne.b	boot9
	mov.b	&-1,halt-T(%a6)		# set flag
boot9:
	clr.b	(%a0)+			# double null terminate path
	clr.b	(%a0)

boot8:
	mov.l	&ROOTINO,%d6		# inode for root directory (inode 2)
	lea.l	name,%a4		# Point to search key

# Repeat while components left in search path

boot10:	bsr	iget			# Get_inode for directory
	clr.l	%d5			# Clear file_block_number

#    Repeat while no match && blocks left in directory

boot11:	tst.l	0(%a6,%d5.l)		# If blkno[file_block_no] is null
	bne.b	nerr11			#    out of blocks & out of luck
	lea.l	objerr,%a0
	lea.l	objend,%a1
	bsr	perror
	bra.w	die

nerr11:	bsr	rdnblk			# read_next_block get dir data block
	mov.l	%a5,%a3			# reset directory entry pointer

#       Repeat while no match && not out of directory entries

	lea.l	BLKSIZ(%a5),%a2		# Point above 1k block
boot12:	cmp.l	%a3,%a2			# If entry pointer beyond buffer
	bhs.b	boot11			#    go try another block
	mov.w	(%a3)+,%d6		# Get inode number
	mov.l	%a3,%a0			# Point to directory entry
	add.l	&NAMSIZ,%a3
	tst.w	%d6			# If inode number is null
	beq.b	boot12			#    go try another entry

#          Compare name to directory entry

	mov.w	&NAMSIZ-1,%d0		# Check up to NAMSIZ chars (-1 for dbcc)
	mov.l	%a4,%a1			# Point to start of current component
boot13:	cmp.b	(%a0)+,(%a1)+		# If characters not equal
	bne.b	boot12			#    go try another entry
	tst.b	-1(%a1)			# If null terminator found
boot14:	dbeq	%d0,boot13		#    || count run out
					#         drop thru we have a match

#       end repeat
#    end repeat
# until end of path is reached

	ext.l	%d6			# Make inode number long
	mov.l	%a1,%a4			# Advance current component ptr
	tst.b	(%a4)			# Check for second null terminator
	bne.b	boot10			#   at end of path

# Found the object module

	mov.l	%d6,bootinod-T(%a6)	# save the inode number
	bsr	iget			# Get_inode for load module
	clr.l	%d5			# Set file_block_number to 0
	bsr	rdnblk			# Get first block
	beq.b	boot15			# Terminate on null file
	lea.l	nulerr,%a0
	lea.l	nulend,%a1
	bsr	perror
	bra.w	die

boot15:	cmp.w	f_magic(%a5),&MC68MAGIC	# If magic number OK
	beq.b	boot20			#    go use header info

# Assume image with entry at location 0

	lea.l	magerr,%a0
	lea.l	magend,%a1
	bsr	perror
	clr.l	%d5			# Reset file_block_number to 0
	sub.l	%a5,%a5			# Set bufptr to location zero
	bsr	load			# Load all blocks into memory
	sub.l	%a2,%a2			# Set entry address to loc 0
	clr.b	halt-T(%a6)		# can't halt if loaded over bug data
	bra	boot50			# Go execute image

# Magic number recognized, use header info

boot20: # Get next two blocks into buffer
	add.l	&BLKSIZ,%a5		# move destination pointer
	bsr	rdnblk			# read next block
	beq.b	boot21			# terminate if block doesn't exist
	lea.l	rderr,%a0
	lea.l	rdend,%a1
	bsr	perror
	bra.w	die
boot21:	cmp.l	%d5,&BLK2INDEX		# is next block index pointing to blk 2?
	beq.b	boot20			# gets file blocks 0,1 & 2 in the buffer
	sub.l	&BLKSIZ*2,%a5		# restore buffer pointer

boot30:	# Move vectors to text_start

	mov.w	&VECS/4-1,%d0		# longs to move (-1 for dbra)
	mov.l	text_start(%a5),%a0	# to text_start
	mov.l	text_scnptr(%a5),%a1	# get HDRSIZ
	add.l	%a5,%a1			# offset into buffer
boot31:	mov.l	(%a1)+,(%a0)+		# move vectors
	dbra	%d0,boot31

	mov.w	&(BUFSIZ-VECS)/4-1,%d0	# longs to move (-1)
	mov.l	text_scnptr(%a5),%d1
	asr.l	&2,%d1
	sub.w	%d1,%d0			# - HDRSIZ / 4
boot32:	mov.l	(%a1)+,(%a0)+		# move code above fill
	dbra	%d0,boot32

	# move remaining blocks into memory

	mov.l	%a5,%a4			# save buffer ptr
	mov.l	%a0,%a5			# starting where we left off
	bsr.b	load			# load remaining blocks
	mov.l	%a4,%a5			# restore buffer ptr

	mov.l	text_start(%a5),%a0	# if text_start + textsize
	add.l	tsize(%a5),%a0
	mov.l	data_start(%a5),%a1
	cmp.l	%a0,%a1			#  == data_start
	beq.b	boot37			#    go skip data movement

	# move data into position

	mov.l	dsize(%a5),%d0
	add.l	%d0,%a0			# from text_start + tsize + dsize
	add.l	%d0,%a1			#   to data_start + dsize
	lsr.l	&2,%d0			# dsize/4 longs to move
	sub.w	&1,%d0			#    (-1 for dbra)
boot35:	mov.l	-(%a0),-(%a1)		# in reverse order
	dbra	%d0,boot35

boot37:
	mov.l	entry(%a5),%a2		# get entry address
	
boot50:

# pass drive and controller number to operating system

	clr.l	%d0
	mov.b	dev-T(%a6),%d0		# boot device drive number
	clr.l	%d1
	mov.b	ctlr-T(%a6),%d1		# controller number
	mov.l	bootinod-T(%a6),%d2	# inode of file loaded

# Jump to entry address

	tst.b	halt-T(%a6)		# halt option?
	beq.b	boot52			# no

	tst.b	Tnum-T(%a6)		# which return trap number to use
	beq.b	boot51

	trap	&15			# go to 120BUG
	short	RT120			
	bra.b	boot52

boot51:
	trap	&15			# go to 130BUG
	short	RT130


boot52:
	jmp	(%a2)			# Jump to entry address

# Subroutine load
#
# Loads file blocks specified by array blkno[] starting with block
# blkno[file_block_no], into successive locations in memory beginning
# at start_loc.
#		d5 = long file_block_no*4
#		a5 = start_loc
#		d5, a5 modified
#

load:					# Repeat while not out of blocks
	bsr	rdnblk			#    Execute read_next_block 
	tst.l	0(%a6,%d5.l)
	beq.b	load1
	add.l	&BLKSIZ,%a5		#    bufptr = bufptr + BLKSIZ
	bra.b	load
load1:					# end repeat
	rts				# return

# Subroutine get_inode
#
#  Gets inode whose number is in d6
#  It assumes a5 is pointing to the buffer to be used
#  and that it is at least 2K in size, one for the inode and
#  one for indirect block temporary storage.
#  Generates a list of the block numbers in the file (64mb max),
#  fetching the first and second level indirect blocks if necessary.
#  The result is the array blkno[] containing all the block numbers
#  for the file.
# 		d6 = unsigned long inodenumber and is modified
#		a5 = buffer pointer
#		d7 modified

iget:
	movm.l	&SAVREGS,-(%sp)		# save regs

# Get block containing specified inode

	add.l	&31,%d6			# inode_blk = (inode_no + 31) / 16
	mov.l	%d6,%d7
	lsr.l	&4,%d7
	bsr.b	rdblk			# read_block(inode_blk  buffer_ptr)
					# into lower half of 2k buffer
	
# extract first 12 block numbers from inode disk address field

	and.l	&INOMSK,%d6		# offset = (inode_no+31)mod 16)*64
	lsl.l	&6,%d6		
	lea.l	faddr(%a5,%d6.l),%a0	# faddr + buffer addr +  offset
	mov.l	%a6,%a1			# point to blkno[0] 
	mov.l	&11,%d0			# 12 addresses to unpack (-1 for dbra)
iget1:
	clr.b	(%a1)+			# convert 3 byte field to a long
	mov.b	(%a0)+,(%a1)+
	mov.b	(%a0)+,(%a1)+
	mov.b	(%a0)+,(%a1)+
	dbra	%d0,iget1		# til done

	clr.l	BLKNO266(%a6)		# set default terminator 

# if a double indirect block exists, get double indirects first

	mov.l	INDIR2(%a6),%d7		# if blkno[11] != 0
	beq.b	iget8

	lea.l	BLKSIZ(%a5),%a5		# use upper half of 2k buffer
	bsr.b	rdblk			# for list of dbl indirects
	mov.l	%a5,%a3			# a3 points to next entry
	lea.l	BLKNO266(%a6),%a5	# start loading them at &blkno[266]

# for each double indirect block entry, read the block into the blkno array

iget3:
	mov.l	(%a3)+,%d7		# if block number is non null
	beq.b	iget6
	bsr.b	rdblk			# read block into the blkno array
	lea.l	BLKSIZ(%a5),%a5		# move 1k further up in blkno[]
	bra.b	iget3

iget6:
	clr.l	(%a5)			#  set terminating null in blkno[]

# if single indirect block exists, read( indirect_block  &blkno[10])

iget8:
	mov.l	INDIR1(%a6),%d7		# if blkno[10] != 0
	beq.b	iget9
	lea.l	INDIR1(%a6),%a5		#  point to &blkno[10]
	bsr.b	rdblk			#  read( indirect_block  &blkno[10])

iget9:
	movm.l	(%sp)+,&GETREGS		# restore
	rts				# return

# Subroutine read_next_block
#
#	Reads block number blkno[file_block_no] if it is not null.
#	It assumes a5 is pointing to the buffer to be used.
#	It postincrements file_block_no.
#	Upon return, the zero flag will be set if a block was read.
#		d5 = long file_block_no*4
#		a5 = buffer pointer
#		d7 modified

rdnblk:
	mov.l	0(%a6,%d5.l),%d7	# get blkno[file_block_no] 
	bne.b	rdn1			# exit if null
	mov.w	&CLR,%cc		# clear zero flag in ccr
	rts
rdn1:	add.l	&4,%d5			# increment file_block_no
#	bra.b	rdblk			# read block and return

# Subroutine read_block
#
#	Reads block number d7 into buffer pointed to by a5
#		 d7 = long block number
#	(block numbers begin at 0 and each block contains 1024 bytes)
#		 a5 = long buffer pointer
#	(buffer size is 1024 bytes)
#	Returns zero flag set on successful io, terminates on bad io.

rdblk:					# wdc = winchester disk controller

	movm.l	&SAVREGS,-(%sp)

	add.l	VIDRFS,%d7		# add root filesystem offset

# software bad track support

	mov.w	VIDBTL,%d0		# get bad track count
	beq.b	rb11			# zero, skip whole thing

	mov.l	%d7,%d1			# make a copy
	divu.w	&BPT,%d1		# rel block (hi), track (low)

	lea.l	BADTAB,%a0		# start of bad track table
	sub.w	&1,%d0			# adjust for dbf
rb12:	cmp.w	%d1,(%a0)		# track in this entry?
	beq.b	rb13			# yes
	add.l	&4,%a0			# step ptr
	dbf	%d0,rb12		# loop
	bra.b	rb11			# no hit, done

rb13:
	clr.w	%d1			# rel block (in hi) ...
	swap.w	%d1			# ... to long
	mov.w	2(%a0),%d7		# alternate track
	mulu.w	&BPT,%d7		# to blocks
	add.l	%d1,%d7			# add rel block back

rb11:
	tst.b	IOtype-T(%a6)		# determine which type of i/o
	bne.b	rb8
	bsr	rdbreg			# register i/o
	bra	rb9
rb8:
	bsr	rdbwT			# TRAP #15 i/o
rb9:

	movm.l	(%sp)+,&GETREGS		# zero flag is set on good io
	beq.b	rb10			# die if not successful
	mov.w	&0x1,%cc		# set flag in ccr
rb10:	rts				# otherwise return

#**********************************************************************
# Subroutine using the BUG's Disk Read routine who's address
# will be passed in a register from the BUG as in the 120BUG.
# 

rdbreg:

# calculate and load arguments for read command to 120BUG
	
	mov.l	%a5,%d2			# buffer address
	mov.l	%d7,%d3
	lsl.l	&2,%d3			# starting sector = block * 4
	mov.l	&SPB,%d0		# sectors per block
	mov.l	cfcode-T(%a6),%d1	# disk configuration code (byte 5)
	mov.l	cfadr-T(%a6),%a2	# configuration data address
	mov.l	ioadr-T(%a6),%a0	# controller io address
	mov.l	readr-T(%a6),%a1	# read routine address

# read a block

	jsr	(%a1)			# 120BUG read disk routine

	rts				# return

# Subroutine read_block using the TRAP #15 mechanism
# and the disk packet protocol as in 130BUG
#
rdbwT:

# calculate and load arguments for read command to 130bug
	
	mov.l	%a5,memadd-T(%a6)	# buffer address
	lsl.l	&2,%d7			# starting sector = block * 4
	mov.l	%d7,secnum-T(%a6)	# beginning sector number
	lea.l	dskpkt-T(%a6),%a1

# read a block

	pea	(%a1)			# 130BUG read disk routine
	trap 	&15
	short	DSKRD

	rts				# return


# Subroutine perror using TRAP #15 to print a one line
# message to console 
#
perror:
	movm.l	&SAVREGS,-(%sp)
	tst.b	IOtype-T(%a6)		# determine which type of i/o
	bne.b	per130			# 130BUG
	mov.l	&vmerr,%a5
	mov.l	&vmend,%a6
	trap 	&15
	short	OUT120
	mov.l	%a0,%a5			# start of string
	mov.l	%a1,%a6			# string length + 1
	trap 	&15
	short	OUT120LF
	bra.b	perrts

per130:	movm.l	&0x00c0,-(%sp)		# %a0,%a1
	pea	vmend
	pea	vmerr
	trap 	&15
	short	OUT130
	trap 	&15
	short	OUT130LF

perrts:	movm.l	(%sp)+,&GETREGS		# restore
	rts

#
#	die - go back to BUG
#

die:	tst.b	Tnum-T(%a6)		# which return trap number to use
	beq.b	die1

	trap	&15			# go to 120BUG
	short	RT120			

die1:
	trap	&15			# go to 130BUG
	short	RT130

