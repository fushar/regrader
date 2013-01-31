/*
 * PerlXS module for managing file locks
 *
 * (c) 2007 Pavel Charvat <pchar@ucw.cz>
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <unistd.h>
#include <fcntl.h>


MODULE = UCW::Filelock		PACKAGE = UCW::Filelock

PROTOTYPES: ENABLED

int
fcntl_lock(IN int fd, IN int cmd, IN int type, IN int whence, IN int start, IN int len)
CODE:
	struct flock fl;
	fl.l_type = type;
	fl.l_whence = whence;
	fl.l_start = start;
	fl.l_len = len;

	RETVAL = fcntl(fd, cmd, &fl);
OUTPUT:
	RETVAL
