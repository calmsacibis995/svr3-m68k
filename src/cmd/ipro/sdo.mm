.nr N 2
.DS C
\fBSmart Disk Optimizer

(SDO)\fR
.DE


.P
\fBSmart Disk Optimizer\fR
is a UNIX* based program that optimizes UNIX disk
filesystems for performance, verification, and reoptimization purposes.
\fBSmart Disk Optimizer\fR
consists of 10,000 lines of highly documented 'C' language code
and is designed with portability in mind.
\fBSmart Disk Optimizer\fR
is targeted for Motorola's Series 20 System V/68 machines
running UNIX Release V.3 but
runs on most any virtual memory UNIX system.

.P
\fBSmart Disk Optimizer\fR
.BL
.LI
arranges the file data blocks
and the free blocks of the filesystems to
be (logically) contiguous on the disk.
This optimizes disk I/O especially
when used in conjunction with "smart"
disk sub-systems that employ track caching and sequential track
read-ahead schemes.
.LI
operates in-place.
This eliminates the need for another disk or disk section especially
when only one disk is available.
.LI
sorts the files
on ascending modification times.
This statistically minimizes
the time to reoptimize the disk at a later time.
.LI
sorts directories on descending modification times.
This helps to minimize file name search times.
.LI
increases/decreases the number of inode/free blocks in the filesystems
by command line option.
This helps the system administrator
in balancing the filesystem with regards
to the average file size.
.LI
erases all unused areas in the filesystem.
This provides some additional system security in that there is no
"old" data present.
.LI
handles filesystem block sizes from 512 bytes to 8K bytes.
.LI
provides linear verification of the optimizations.
.LI
provides file content verification before and after optimizations.
.LE


.P
Please contact your Motorola Sales Representative for more information
concerning
\fBSmart Disk Optimizer\fR
and other Motorola products.
.P









*UNIX is a registered trademark of AT&T Bell Laboratories, Inc.
