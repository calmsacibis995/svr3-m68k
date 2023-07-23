---- The Files used by the backup facility ----

The backup facility maintains several files, which it uses keep track of
when backups were done, and when to do the next one, etc.  These are listed
below, along with their content (if any) and use.

LastIncr
M-time = time of last incremental backup.
contents = none 
used by all the incremental backup actions.  Touched each time an
incremental is finished.

IncrAt
contents = time to schedule incrementals for
Contains time of day for scheduling deferred incrementals.  If the file is
found to be empty, it is filled with "02:00", representing 2am.  The form of
the file is a simple hour and minute of a time of day.  The hour is
expressed in 24-hour form, with 00 being 12am (midnite).  A space may be
used instead of a colon to separate the hour from the minute, but a
separator is required.  The user may edit this file (using any text editor)
to change the time for deferred incrementals.

LastEst
M-time = time of last incremental estimate.
contents = #volumes, #Kbytes, #files, #archive-blocks for last estimate
used by all incremental actions.  New contents written after each estimate
is made, which also changes the modified time.  When a deferred incremental
is scheduled, if LastIncr is more recent than LastEst, an estimate is
automatically performed then, before the incremental is scheduled.  This
helps prevent scheduling of incrementals that are too large to fit on a
single archive medium.

LastFull
M-time = no particular significance
contents = date & time of last full backup.
Contents is:
	year month day hour minute second monthname
of last full backup.  Each item is a number, blank separated; except
monthname is a 3-character string representing the month.  Checked by
'SinceFull'.  If no file exists, it will create the file and set its
contents to 1 Jan 1970.  If the file exists, but is empty, it uses the
M-time of the file, and writes that date to the file contents.  Otherwise,
the file is assumed to have valid contents which constitute the date of the
last full backup.

Archive
M-time = time the archive device description was last changed
contents = archive device description, as follows:
	<archive-name> <other bru options>

Words are separated by white-space.  The archive-change item writes like:
	name  -b nnn  -s nnn [-p]

Where nnn is a bru size-expression (a number with or without [KkMm]).
The '-p' is optional, and indicates that blocks should be skipped by reading
instead of seeking.

Actually parsing the items out is not dependent on specific ordering, but
looks for the 'bru' flags, just in case the user edits the file with a
normal editor and changes the order of the items.  Contents written when
"Save-description" item is chosen.  Read by most things that deal with
archive.  The only content requirement is that the archive name be first,
followed by the remaining bru options.
