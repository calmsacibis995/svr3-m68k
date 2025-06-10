
/*
 * $Header: /rl/sys/fs/du/RCS/dulocking.c,v 1.1 86/08/20 17:12:23 dap Exp $
 * $Locker:  $
 */
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/systm.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/nami.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/message.h"


/*
 * dulocking -	file/record locking based on /usr/group
 *
 *	Included only for backward compatability reasons
 *
 * Input:
 *  ip		-	generic FS inode
 *  cmd		-	copy of uap->cmd: ignored
 *  flag	-	flags from users file descriptor
 *  offset	-	current file offset
 *  size	-	copy of uap->size: ignored
 */
dulocking(ip, cmd, flag, offset, size)
struct inode *ip;
off_t offset; {

	DUPRINT4(DB_FSS,"dulocking: ip %x cmd %x offset %x\n", ip, cmd, offset);

	remfileop(ip, flag, offset);

	DUPRINT2(DB_FSS,"dulocking: u.u_error = %d \n",u.u_error);
}
