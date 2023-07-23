**** Notes on actual implementation of features in 'remake.t' ****

Certain valuable file-system info is saved with every full-backup of each
file-system.  Specifically:
	mount-point	the directory name (at most 32 chars)
	device		device-name, less any leading /dev or /dev/dsk.
	blocks-used	number of blocks used by file-system
	i-nodes-used	number of i-nodes actually in use
	FS-blocks	number of blocks alloted to the fs
	FS-i-nodes	number of i-nodes alloted to the fs

This information is developed for individual file-system by 'HeadFor', and
is then saved as the archive label during the 'bru' backup.  The label will
be extracted and parsed during the rebuild, and will control the 'mkfs'
operations.  Since the info is stored in the archive label, it does not
interfere with other operations on the archive.

If the mount-point name has a leading '-', it means that the name was too
long to be entirely saved in the label.  The saved name is the first 32
characters of the original name, with a '-' in place of the leading '/'.
User interaction will be required to fully recover the target mount-point.
A first guess can be made by looking in the DiskInfo file for the system,
selecting all patterns that match the first 32 characters of the saved name
(after replacing its leading '-').

The device-name will have had its ancestor directories stripped, so it
should be looked for in:
	/dev/$NAME
	/dev/r$NAME
	/dev/dsk/$NAME
	/dev/rdsk/$NAME

and any units found should be selected by the user as final confirmation.
If possible, only one acceptable unit should be found in all of these
directories, simply to avoid having the user make a wrong choice.  The
precise search and confirm procedure can only be finalized after the
appropriate decisions are made for the actual Aquarius unit (and this is not
yet done).  When restoring a full backup to a naked device, take the first
name found in the search path, but ask for confirmation.  Give a 'ls -l'
description first, and if unacceptable, read a new name from the user.


The maximum size of a 'bru' archive label is 63 characters.  The maximum
number of i-nodes in a file-system is 65,535 (5 digits).  A reasonable
maximum number of blocks in a file-system is 999,999 (6 digits).  In
Aquarius systems, the device-name should fit the pattern "w??" (3
characters).  Label items must be separated by at least one space to allow
shell parsing.  Given these constraints, we observe:
	device		3 + 1 space
	blocks-used	6 + 1 space
	i-nodes-used	5 + 1 space
	total-blocks	6 + 1 space
	total-i-nodes	5 + 1 space

	total		25 + 5 spaces (30 characters total)

This leaves 33 characters to hold the mount-point name and a space
separator.  Names longer than 32 characters must be truncated to 32
characters and a '-' used to replace the leading '/'.
