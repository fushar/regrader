/*
 *	UCW Library -- Syncing Directories
 *
 *	(c) 2004--2005 Martin Mares <mj@ucw.cz>
 */

#include "ucw/lib.h"

#include <fcntl.h>
#include <unistd.h>

void
sync_dir(const char *name)
{
  int fd = open(name, O_RDONLY
#ifdef CONFIG_LINUX
		| O_DIRECTORY
#endif
);
  if (fd < 0)
    goto err;
  int err = fsync(fd);
  close(fd);
  if (err >= 0)
    return;
 err:
  msg(L_ERROR, "Unable to sync directory %s: %m", name);
}
