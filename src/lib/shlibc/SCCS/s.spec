h61573
s 00027/00044/00261
d D 1.3 86/08/18 08:53:54 fnf 3 2
c Checkpoint changes at first working shared library version
e
s 00134/00191/00171
d D 1.2 86/08/11 14:22:54 fnf 2 1
c Checkpoint changes.
e
s 00362/00000/00000
d D 1.1 86/07/31 10:11:46 fnf 1 0
c Initial copy from 5.3 distribution for 3b2.
e
u
U
t
T
I 1
#ident 	"@(#)shlibc:spec	1.3"
##
## Libc_s specification file
##
#address .text 0x80040000
#address .data 0x800e0000
#target /shlib/libc_s
#branch
D 3
__doprnt 1
E 3
I 3
_doprnt 1
E 3
_bufsync 2
D 3
_cerror	3
E 3
I 3
_MaXfLoA 3
E 3
_cleanup 4
D 3
_dtop	5
_faddd	6
_fadds	7
_fcmpd	8
_fcmps	9
_fcmptd	10
_fcmpts	11
_fdivd	12
_fdivs	13
_fdtos	14
_filbuf	15
_findbuf 16
_findiop 17
_flsbuf	18
_fltod	19
_fltos	20
_fmuld	21
_fmuls	22
_fnegd	23
_fnegs	24
D 2
_fpstart 25
_fstod	26
_fsubd	27
_fsubs	28
_ftdtol	29
_ftdtou	30
_ftstol	31
_ftstou	32
_futod	33
_futos	34
_getflthw 35
_getfltsw 36
_ltostr	37
_ptod	38
_wrtchk	39
_xflsbuf 40
abs	41
access	42
atof	43
atoi	44
atol	45
brk	46
calloc	47
cfree	48
chdir	49
chmod	50
close	51
creat	52
ecvt	53
fclose	54
fcntl	55
fcvt	56
fflush	57
fgetc	58
fgets	59
fopen	60
fpgetmask 61
fpgetround 62
fpgetsticky 63
fprintf	64
fpsetmask 65
fpsetround 66
fpsetsticky 67
fputc	68
fputs	69
fread	70
free	71
freopen	72
frexp	73
fseek	74
fstat	75
fwrite	76
gcvt	77
getchar	78
getenv	79
getopt	80
getpid	81
gets	82
getw	83
ioctl	84
isatty	85
isnand	86
kill	87
ldexp	88
lseek	89
malloc	90
memccpy	91
memchr	92
memcmp	93
memcpy	94
memset	95
mktemp	96
open	97
printf	98
putchar	99
puts	100
putw	101
read	102
realloc	103
sbrk	104
setbuf	105
sighold	106
sigignore 107
signal	108
sigpause 109
sigrelse 110
sigset	111
sprintf	112
stat	113
strcat	114
strchr	115
strcmp	116
strcpy	117
strlen	118
strncat	119
strncmp	120
strncpy	121
strrchr	122
time	123
tolower	124
toupper	125
ungetc	126
unlink	127
utime	128
write	129-150
E 2
I 2
_fstod	25
_fsubd	26
_fsubs	27
_ftdtol	28
_ftdtou	29
_ftstol	30
_ftstou	31
_futod	32
_futos	33
_getflthw 34
_getfltsw 35
_ltostr	36
_ptod	37
E 3
I 3
_filbuf	5
_findbuf 6
_findiop 7
_flsbuf	8
dbcmp%% 9
_ltostr	10
dbneg%% 11
dbmul%% 12
dbadd%% 13
dbdiv%% 14
ltodb%% 15
fltodb%% 16
dbtol%% 17-37
E 3
_wrtchk	38
_xflsbuf 39
abs	40
access	41
atof	42
atoi	43
atol	44
brk	45
calloc	46
cfree	47
chdir	48
chmod	49
close	50
creat	51
ecvt	52
fclose	53
fcntl	54
fcvt	55
fflush	56
fgetc	57
fgets	58
fopen	59
fprintf	60
fputc	61
fputs	62
fread	63
free	64
freopen	65
frexp	66
fseek	67
fstat	68
fwrite	69
gcvt	70
getchar	71
getenv	72
getopt	73
getpid	74
gets	75
getw	76
ioctl	77
D 3
isatty	78
isnand	79
E 3
I 3
isatty	78-79
E 3
kill	80
ldexp	81
lseek	82
malloc	83
memccpy	84
memchr	85
memcmp	86
memcpy	87
memset	88
mktemp	89
open	90
printf	91
putchar	92
puts	93
putw	94
read	95
realloc	96
sbrk	97
D 3
setbuf	98
sighold	99
sigignore 100
signal	101
sigpause 102
sigrelse 103
sigset	104
E 3
I 3
setbuf	98-104
E 3
sprintf	105
stat	106
strcat	107
strchr	108
strcmp	109
strcpy	110
strlen	111
strncat	112
strncmp	113
strncpy	114
strrchr	115
time	116
tolower	117
toupper	118
ungetc	119
unlink	120
utime	121
write	122-150
E 2
#objects
##
##
##
##	 Imported symbols definition files
##
port/gen/ctype_def.o
port/gen/mall_def.o
port/stdio/stdio_def.o
D 2
m32/gen/gen_def.o
m32/fp/fp_def.o
E 2
I 2
m68k/gen/gen_def.o
E 2
##	 Read/Write Global data
port/gen/opt_data.o
D 2
m32/gen/m32_data.o
m32/fp/fp_data.o
E 2

##	 REST of *.o

##	Some frequently used functions, called randomly

D 2
m32/gen/abs.o
E 2
I 2
m68k/gen/abs.o
E 2
port/gen/tolower.o
port/stdio/ungetc.o
D 2
m32/gen/strcmp.o
E 2
I 2
m68k/gen/strcmp.o
E 2

##	malloc family

port/gen/calloc.o
port/gen/malloc.o

##	infrequently used, page glue

port/gen/strncat.o

##	frequently used, randomly called

port/gen/memccpy.o
D 2
m32/gen/memcpy.o
E 2
I 2
m68k/gen/memcpy.o
E 2
port/gen/memset.o
port/gen/strcat.o
D 2
m32/gen/strcpy.o
m32/gen/strlen.o
E 2
I 2
m68k/gen/strcpy.o
m68k/gen/strlen.o
E 2

##	stdio group

port/stdio/filbuf.o
port/stdio/fread.o

##	stdio group

port/stdio/fgetc.o
port/stdio/fgets.o
port/stdio/gets.o
port/stdio/getchar.o
port/stdio/getw.o

##	stdio group

port/stdio/fputs.o
port/stdio/putw.o
port/stdio/fputc.o
port/stdio/putchar.o

##	infrequently called, page glue

port/gen/toupper.o

##	stdio group

port/stdio/fwrite.o
port/stdio/flsbuf.o
port/stdio/fseek.o

##	infrequently called, page glue

port/gen/strncpy.o

##	stdio group

port/gen/isatty.o
port/stdio/fopen.o
port/stdio/findiop.o

##	stdio group (remainder)

port/stdio/puts.o

##	printf family

D 2
m32/print/fprintf.o
m32/print/printf.o
m32/print/sprintf.o
E 2
I 2
port/print/fprintf.o
port/print/printf.o
port/print/sprintf.o
E 2

##	floating point stuff, called from _dtop, _ptod and ecvt
##		fills out page

D 2
m32/gen/biglitpow.o
m32/gen/frexp.o
m32/gen/ltostr.o
E 2
I 2
m68k/gen/frexp.o
m68k/gen/ltostr.o
E 2

##	page glue

port/gen/memcmp.o

##	printf internals

D 2
m32/print/doprnt.o
E 2
I 2
port/print/doprnt.o
E 2

##	process startup functions

port/gen/mktemp.o
port/gen/getenv.o
port/gen/getopt.o
port/stdio/setbuf.o
D 2
m32/sys/signal.o
m32/fp/getflts1.o
m32/fp/getflth0.o
E 2
I 2
m68k/sys/signal.o
E 2

##	printf internals, floating pt

D 2
m32/gen/dtop.o
m32/gen/ecvt.o
E 2
I 2
m68k/gen/ecvt.o
E 2
port/gen/gcvt.o
I 3
m68k/crt/dbcmp.o
m68k/crt/dbneg.o
m68k/crt/dbmul.o
m68k/crt/dbadd.o
m68k/crt/dbdiv.o
m68k/crt/ltodb.o
m68k/crt/fltodb.o
m68k/crt/dbtol.o
m68k/crt/table.o
m68k/crt/fpdata.o
E 3

##	misc. stuff, infrequently, randomly called
##		glue to cross page boundary

port/gen/atol.o
port/gen/atoi.o
port/gen/memchr.o
port/gen/strchr.o
port/gen/strncmp.o
port/gen/strrchr.o

##	system calls

D 2
m32/crt/cerror.o
m32/sys/close.o
m32/sys/access.o
m32/sys/chdir.o
m32/sys/chmod.o
m32/sys/creat.o
m32/sys/fcntl.o
m32/sys/fstat.o
m32/sys/getpid.o
m32/sys/ioctl.o
m32/sys/kill.o
m32/sys/lseek.o
m32/sys/open.o
m32/sys/read.o
m32/sys/sbrk.o
m32/sys/stat.o
m32/sys/time.o
m32/sys/unlink.o
m32/sys/utime.o
m32/sys/write.o
E 2
I 2
m68k/crt/cerror.o
m68k/sys/close.o
m68k/sys/access.o
m68k/sys/chdir.o
m68k/sys/chmod.o
m68k/sys/creat.o
m68k/sys/fcntl.o
m68k/sys/fstat.o
m68k/sys/getpid.o
m68k/sys/ioctl.o
m68k/sys/kill.o
m68k/sys/lseek.o
m68k/sys/open.o
m68k/sys/read.o
m68k/sys/sbrk.o
m68k/sys/stat.o
m68k/sys/time.o
m68k/sys/unlink.o
m68k/sys/utime.o
m68k/sys/write.o
E 2

D 2
##	double fp

m32/fp/dround.o
m32/fp/dspecial.o
m32/fp/fdtos.o
m32/fp/fltod.o
m32/fp/ftdtol.o
m32/fp/ftdtou.o
m32/fp/faddd.o
m32/fp/fcmpd.o
m32/fp/fdivd.o
m32/fp/fmuld.o
m32/fp/fnegd.o

##	fp util

m32/fp/fpgetmask.o
m32/fp/fpgetsticky.o
m32/fp/isnand.o
m32/fp/fpgetrnd.o

##	single fp

m32/fp/sround.o
m32/fp/sspecial.o
m32/fp/fltos.o
m32/fp/fstod.o
m32/fp/ftstol.o
m32/fp/ftstou.o
m32/fp/fadds.o
m32/fp/fcmps.o
m32/fp/fdivs.o
m32/fp/fmuls.o
m32/fp/fnegs.o

E 2
##	fp masks, misc

D 2
m32/gen/atof.o
m32/gen/ldexp.o
m32/gen/ptod.o
m32/fp/fpsetmask.o
m32/fp/fpsetrnd.o
m32/fp/fpsetsticky.o
m32/fp/fpstart1.o
E 2
I 2
port/gen/atof.o
m68k/gen/ldexp.o
E 2

#init ctype_def.o
_libc__ctype		_ctype
D 2

#init fp_def.o
_libc__getflthw		_getflthw
E 2

#init gen_def.o
_libc__cleanup		_cleanup
_libc_end		end
_libc_environ		environ

#init mall_def.o
_libc_malloc		malloc
_libc_free		free
_libc_realloc		realloc
	
#init stdio_def.o
_libc__sibuf		_sibuf
_libc__sobuf		_sobuf
_libc__smbuf		_smbuf
_libc__iob		_iob
_libc__lastbuf		_lastbuf
_libc__bufendtab	_bufendtab
E 1
