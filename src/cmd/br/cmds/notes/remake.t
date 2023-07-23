**** Notes on reconstructing a complete fs from a full backup ****

There are several distinct steps in remaking a destroyed file-system (fs).
Some may be omitted depending on the extent of the damage.  Basically, the
following must be done, given a disk drive on which to begin:

1. Format the disk, avoiding bad blocks.
2. Partition the disk, giving sizes that will allow the desired FS to
	be restored.
3. Choose a partition and run 'mkfs' on it, allowing sufficient space to
	restore the desired file-system.
4. Examine the archive header to be sure the fs will fit where requested.
5. Mount the empty fs in a convenient location, then cd to it.
6. Restore the fs from the full-backup archive.
7. If not mounted in the final desired place, mount it there.
8. Apply the latest incremental, but only to the newly restored fs.

Since each fs is backed up with 'mkfs' information (# blocks, # i-nodes,
etc.), and this information can be quickly retrieved from the archive, it
should be used to control steps 2-4.  An alternative is to simply display
the information for step 2, then have the B/R program do everything in steps
3-8, as instructed by the operator.  This puts the partitioning of the
entire disk totally in the hands of the operator, who (it is hoped) has
better heuristics and can make more intelligent decisions about partition
sizes than a simple computer program.  To make this archive info available
without resorting to a lengthy series of menus, a single command should read
the archive and display this info in a convenient format.

Each step above has particular requirements for robustness.  These will be
individually described...

Step 3 requires that a failure of 'mkfs' allow the operator to try again,
perhaps on a different partition.  Also, a happy medium between the actual
blocks/i-nodes used and those allocated in the original fs must be struck if
the target partition is not as large as the original partition.

Step 4 should be a moot point, as the archive-info is actually used to
reconstruct an acceptable file-system.

Step 5 can mount the empty fs on a convenient directory made expressly for
the purpose of recovery, or it can be mounted in its final destination.  The
latter requires that file-systems be restored in a certain sequence, though,
in case the full system has file-systems mounted hierarchically.  The former
is thus preferable, since it only requires the presence of '/', a given.

Step 6 is accomplished by bru (at the lowest level), and should be at least
a bit verbose, so the user can see that something is happening.  It may even
be desireable to permit 'bru' interaction with the user in case of archive
media errors or media changes.

Step 7 can be done whether the full restore of step 6 was partially or
entirely successful.  The partial success case should ask for user
confirmation before proceeding.  If more than one fs is being recovered,
steps 7 and 8 can be deferred and applied to the complete system.  In fact,
the normal system boot procedure should be invoked, mounting all
file-systems in their normal places.  Then the last incremental can be
applied, updating all file-systems in the process.  That is, steps 7 & 8
should only be done in the end, after all file-systems have been restored
from full backups.

Step 8 is done with the latest incremental backup, but is accomplished using
a variation of SELECTED RESTORE.  Specifically, the DIR will be '/', and the
CRITERIA will select all files from that file-system.  This info is
automatically jammed in when a fs is restored, but can still be altered by
the user to permit additional selection criteria, such as only restoring a
certain user's files.  Since incrementals are taken from '/' (but relative),
all file-systems must be mounted in the same places as when the incremental
was taken.  Also, the current directory must be '/', the same as when the
incremental was taken.

It is unclear if the above process can be applied to '/', but the
memory-based file-system should permit it.  Thus, one could presumably
recover from a crash of every file-system.  Before this claim is widely
promulgated, though, it should be exhaustively verified.
