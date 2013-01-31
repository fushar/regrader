/*
 *	UCW Library -- Fast Buffered I/O on Sockets with Timeouts
 *
 *	(c) 2008 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_FB_SOCKET_H
#define _UCW_FB_SOCKET_H

#include "ucw/fastbuf.h"

struct fbsock_params {	/** Configuration of socket fastbuf. **/
  int fd;
  uns bufsize;
  uns timeout_ms;
  void (*err)(void *data, uns flags, char *msg);
  void *data;			// Passed to the err callback
};

enum fbsock_err_flags {	/** Description of a socket error **/
  FBSOCK_READ = 1,		// Happened during read
  FBSOCK_WRITE = 2,		// Happened during write
  FBSOCK_TIMEOUT = 4,		// The error is a timeout
};

/**
 * Create a new socket fastbuf.
 * All information is passed by @par.
 **/
struct fastbuf *fbsock_create(struct fbsock_params *par);

#endif
