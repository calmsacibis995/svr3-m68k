The disk database, currently known as "disktab" will be a text file that
describes the known disks/file-systems on the machine.  It will be the file
"/etc/DiskInfo", will have 644 permissions, and will be owned by "bin".
Each line will describe a disk or file-system on the machine.  Since it is
pure ASCII text, it can be easily parsed by shell, awk, or sed scripts, and
also by C programs.

Each line will describe a single unit: a disk, file-system, swap-area, etc.
The comment character is "#" and any line that starts with a "#" will be
ignored.  The "#" character only has this meaning when it is the first
character on the line.  There is no escape for this.  All blank lines will
also be ignored.  Any other line is assumed to hold meaningful information.
All fields should be present for every unit, but any which are missing are
assumed to indicate "no" or "none".  The conventional method for doing this
is any string starting with a "-".  For instance, a partition which is never
to be mounted would have a mount-permissions field of "-".

Fields are contiguous string of ASCII characters separated by white-space
(tabs or blanks).  A new-line character terminates each line.  The fields
are arranged as follows:
	device-path	- pathname to device
	mount-point	- default mount-point (directory) or other ident
	mount-flag	- "+" = mount at boot, "-" = don't
	mount-perm	- "-"=don't mount, "W"=R/W, "R"=read-only
	fs-name		- file-system name from super-block, if any
	media-size	- actual size of media or partition
	fs-size		- file-system size (always <= media-size)
	backup-flag	- "-" if no auto-backup, else backup type
	disk-type	- device type for 'diskfmt'

Any field that starts with a "-" is assumed to mean "no" or "none".  For
flags, "+" is only the preferred symbol.  Anything other than a string
starting with "-" will symbolize "affirmative".

While many of the fields are related to mounting file-systems, this table
can be used to describe devices which are not mounted.  This is done simply
by entering "-" for the unused field.  Thus, raw block devices can be
included in the auto-backup procedure, but not mounted as a file-system.

The backup-flag is used to tell the backup system whether or not to include
this file-system as part of the automatic backup process or not.  If not a
"-", it tells how the device is to be backed up: as a file-system, as a
block device, as a binary image, etc.  Initial versions of the backup system
will recognize the following characters:
	[Ff]	- backup using the mount-point name (i.e. as a file-system)
	[Rr]	- backup as a raw device
	"-"	- do not automatically backup
