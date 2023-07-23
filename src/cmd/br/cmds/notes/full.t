**** Notes on full backups ****

A full backup is done on each individual file-system.  Each one is placed on
its own tape.  Each tape contains the files, but also has enough info to
remake the filesystem, should it become totally hosed.  This basically
amounts to the device-name and the partition size, and the current allotment
used by the file-system.  This information is called the "environment", to
distinguish it from the filesystem itself.

The environment cannot simply be saved on the tape as a file found in the
filesystem.  Restoral would then require extraction of the single file, but
bru will scan all the rest of the tape looking for a later version.  This is
not good.

If the tape can be made to do "no-rewind", then the environment can be
written in its own archive first, followed by the filesystem archive.  This
is still only one file-system per tape, and is called a "dual-archive".

Another alternative is to stash the information in the bru header somewhere
(e.g. in the "label"), then modify bru so it simply tells about the archive
header.  Restoring the environment would then consist of reading and parsing
the archive header, and restoring the filesystem would read the files.  The
bru modification should only read the header information, which should be
quite fast.  If "no-rewind" ever became available on tape, and we switched
to more than one file-system per tape, bru would not be able to rewind to
the beginning of a logical archive.  This solution would then become
unusable, and the only choice left is the dual-archive.

***
'Bru' has been modified as of Jan 16 to do the "label thing".  It still only
allows one full backup per tape, since the tape can't rewind to a specific
point.  The environment and reconstruction detail are more fully described
in "rebuild.t" and "remake.t".
***

Note that another value worth saving is the current number of blocks and
i-nodes used.  Then if a file-system was reconstructed on a different
device, we can check to make sure the whole thing will fit before we start
taking it off.  All this without forcing the new device's partition to be
exactly the same size as that of the original device.

---

On making a full backup, each file-system should be validated using
'fsck'.  This ensures that no insane files are written to an archive.  Since
backups should only be done in single-user state, all file-systems can be
made quiescent.  Perhaps the most affected function will be the unattended
incremental backup, since it must take the system to single-user state, run
the backup, then return to multi-user state.  It must do all of this while
unattended, usually during the wee hours of the morning.

The user's manual must explain the various levels of file-system recovery,
definitely including the following:
	- recovering partial file-systems from a full backup + incremental
	- restoring entire file-systems to their original device
	- restoring entire file-systems to a different device
	- restoring complete groups of file-systems using the memory-based
	  file-system toolkit

It must also explain the principles of backups (full & incremental), and the
reasons for off-site backups.  Several suggested ways of managing
different-sized installations should be given, along with REASONS for the
various choices made.  Be helpful, honest, and occasionally witty, not cold,
authoritative, or condescending.  Give reasons whenever possible, even if
they must be placed in footnotes.

-----

When a full backup is in progress, and a single FS fails to fsck or backup,
rotate its name to the bottom of the list of "not-done" filesystems.  This
defers the funky behavior to the end, and all sane file-systems are backed
up safely first.

