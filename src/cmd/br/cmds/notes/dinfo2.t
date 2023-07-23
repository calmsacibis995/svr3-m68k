Notes on DiskInfo made after the watershed Nov 15 meeting.

Diskformat type will come from a text file that associates disk-size with
dev-type.  One entry for each size.  Each size corresponds to a unique type
or model of drive.  The controller can only return the disk size, so this is
how the correlation between size and type is made.

Another disk file associates disk use with disk partition.  This is simply a
map of how each partition is used (file-system, block, swap, etc.).  It is
not clear if this will be necessary, but may be useful when making the
Disk-picture, to associate a meaningful name for unused partitions, or ones
used as things other than file-systems or swap areas.  Perhaps it should
simply be used by Disk-pict itself, and not be part of the Enq system.

When a backup is made, the Enquirer (Enq) is used to get the info on the
file-system being backed up.  This is saved in "/", or in the root of the
file-system itself, depending upon whether an incremental or a full is being
done.  The Enquirer file is written to the tape, using a single invocation
of bru, without rewinding.  The  file is erased, then the file-systems are
backed up, using another invocation of bru, still without rewinding.

An incremental backup will consist of all the Enq files in a single archive,
followed by an incremental of the whole hierarchy, starting from /.  

A full will backup each file-system separately, to improve the ease of
restoral.  In this case, each file-system backup will have its Enq file in
an archive, followed by the whole file-system.  Bru will be invoked with
"-m", so mounted file-systems will not be entered.  This means that files
hidden under mounted file-systems will not be backed up.  Full backups can
only be done on quiescent file-systems.  It may be convenient to unmount
each FS from its normal place, and mount it on a directory reserved just for
backing up "the current file-system".

?????
If the order of backing things up is significant, how does one determine the
order, since Enq does not have any such information?  A text file could be
perused, or the backup process could simply start at the file-systems with
the most number of slashes, working back and unmounting along the way.
?????

The Enquirer will take a command-line that specifies which info to show, and
how it is to be selected.  Enq itself will only do simple kinds of
include/exclude.  Awk and shell scripts will do more complex things, like
selecting only those items with certain ranges of values for numeric fields.
The basis enq capabilities are simple include & exclude of items, based on
the settings of the various bit flags.  Certain fields can be chosen for
display, based on the inclusion or exclusion of certain codes from a display
format string (after printf?).  By default, all fields will be displayed in
a default order.  Both include and exclude may be specified at the same
time, as long as they are on different fields.  e.g.
	-xyz	= exclude "X" "Y" "Z"
	+abc	= include "A" "B" "C"
	=pdq	= print in format "P" "D" "Q"
It may be possible to simply have the print format be the default argument,
since the include & exclude parameters will have leading symbols.




????
Will the raw wini device need to be public read? if so, does this violate
security of data?  what about to certain potential customers?  
?????
