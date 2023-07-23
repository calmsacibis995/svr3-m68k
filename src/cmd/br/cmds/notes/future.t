If B/R is running on the console,
	and run-level != 1,
		give the option of executing 'init 1'
else
	give warning that run-level isn't 1.


if low-space on '/',
	and verbose-incremental,
		don't log incremental output.
	and verbose-full
		don't log full output.


##
Both SELECTED and RESTORE should be able to read groups of file-selection
criteria from files, or from libraries.  Also, should be able to save
current criteria in selected files or libraries.  This allows the SysOp to
build up various criteria groups that describe commonly done operations,
such as backing up a certain person's directory tree, or performing other
repeated operations.

###
It should be possible to write out the FS rebuild info on the header of an
individually backed-up FS.  Perhaps this could be an option under SELECTED,
or a separate facility of FULL backup.  Or a separate top-menu choice, like
"FILE-SYSTEM backup".  This would ease such things as dumping a full FS and
reusing its disk space, as was recently done with /att/src on this machine.
Especially, recovery would be quite simple.  This especially true in light
of inability to have blanks in archive-label using SELECTED backup.

###
When restoring files, be sure to read the contents of the private B/R files,
so that restoring does not overwrite them and lose their contents.  When the
restore is complete, write the contents back out.  Also be sure to get the
dates of the 'touch' files so they can also be restored.

###
Part of the B/R release package is:
	bru
	sh (new)
	/backups

Bru can actually reside in the 'common' directory, and can be installed in
/bin by the target user.  Thus, if they want to keep the old bru (why?) they
can.  The new shell must be installed, perhaps as 'shn' in /bin...
