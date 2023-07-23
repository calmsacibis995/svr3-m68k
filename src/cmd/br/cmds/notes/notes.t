The disk backup system will rely on the disk-pic menus for selecting the
file-system for backup.  This is only when the automatica defualt scheduled
procedures will not be used.  An alternative is to run disk-pic, but have it
sort the disk-info based on last time of backup.  This may be difficult, at
least for the early releases, but should be a goal to try for.

The disk backup system (BU) will keep information about backups in a private
directory. It will retain the date of each file-system's last full backup,
and the date of the last incremental.  Incremental backups will always
consist of the files changed since the last full backup.  Thus, the size of
an incremental will increase or remain the same as time goes by, at least
until the next full backup.

The file-system information will consist of a file with the same name as the
file-system.  Or, a file-system name can be specified in the Disk-Pic file,
and this name will then be used as the name of the time-file.  The
modification date of this file will indicate the time of the last backup.
It is unclear whether incrementals should be done for the all file-systems as
a whole, or for each file-system individually.  The latter may be too large
to fit on a tape.  The former complicates restorals and the automatic
time-trackings, since a separate time-file must be kept for fulls and for
incrementals.

Since bru can estimate the size of a given archive (quite accurately), the
operator will be permitted to estimate the size of the current backup at any
time.  This should be done at a low priority, to keep from hogging machine
resources during prime-time.  Of course, if set up for unattended operation,
and using different-sized archive media, the estimates on number of volumes
may be wrong.

The preferred way of doint this sort of thing is an action called
"next-backup", which will tell the user which is the next thing that should
be backed up, how big it will be, when it should be done, how many archive
volumes, and anything else necessary to know about the backup without
actually doing it.  This should be invokable as a command and as a menu
item.  The menu item should simply invoke the command (or a variant thereof)
and present the resulting output in a "menu-ish" manner.

At a minimum, the user should be able to make full and incremental backups
of everything on the system, automatically recording the backup time and
other relevant information.  This will be the first pass, and will probably
NOT have menus initially.  Also, there should be at least a simple version of
"next-backup" that tells what the next action will be and when.

It is reasonable to assume that the shell used to run shell files will be
5.2.2 Bourne shell.  This means that functions can be defined by the
high-level file, then used by lower-level actions.  Should be very speedy
and convenient for things like debug.

Incremental and full backups should be taken as relative, while located in
/.  Individual file-system backups should be taken as relative while located
in the root of the file-system.  The latter will allow restoral of files
saved in one file-system to another file-system.  Ideally, fulls and
incrementals should make a similar arrangement, but this makes them
sensitive to correct labeling and proper restoral.  It's too easy to make a
mistake and restore something in the wrong order, and therefore in the wrong
place.
