/*
 *	UCW Library -- Fast Buffered I/O on Sockets with Timeouts
 *
 *	(c) 2008 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"
#include "ucw/fb-socket.h"

#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

struct fb_sock {
  struct fastbuf fb;
  struct fbsock_params par;
  byte buf[0];
};

#define FB_SOCK(f) ((struct fb_sock *)(f)->is_fastbuf)

static int
fbs_refill(struct fastbuf *f)
{
  struct fbsock_params *p = &FB_SOCK(f)->par;
  struct pollfd pf = {
      .fd = p->fd,
      .events = POLLIN
  };

  for (;;)
    {
      int e = poll(&pf, 1, p->timeout_ms);
      if (e < 0)
	{
	  p->err(p->data, FBSOCK_READ, "read error");
	  return 0;
	}
      if (!e)
	{
	  p->err(p->data, FBSOCK_READ | FBSOCK_TIMEOUT, "read timeout");
	  return 0;
	}

      f->bptr = f->buffer;
      int l = read(p->fd, f->buffer, f->bufend-f->buffer);
      if (l < 0)
	{
	  if (errno == EINTR || errno == EAGAIN)
	    continue;
	  p->err(p->data, FBSOCK_READ, "read error");
	  return 0;
	}
      f->bstop = f->buffer + l;
      f->pos += l;
      return l;
    }
}

static void
fbs_spout(struct fastbuf *f)
{
  struct fbsock_params *p = &FB_SOCK(f)->par;
  struct pollfd pf = {
      .fd = p->fd,
      .events = POLLOUT,
  };

  int l = f->bptr - f->buffer;
  f->bptr = f->buffer;
  char *buf = f->buffer;

  while (l)
    {
      int e = poll(&pf, 1, p->timeout_ms);
      if (e < 0)
	{
	  p->err(p->data, FBSOCK_WRITE, "write error");
	  return;
	}
      if (!e)
	{
	  p->err(p->data, FBSOCK_WRITE | FBSOCK_TIMEOUT, "write timeout");
	  return;
	}

      e = write(p->fd, buf, l);
      if (e < 0)
	{
	  if (errno == EINTR || errno == EAGAIN)
	    continue;
	  p->err(p->data, FBSOCK_WRITE, "write error");
	  return;
	}
      buf += e;
      l -= e;
    }
}

static void
fbs_close(struct fastbuf *f)
{
  close(FB_SOCK(f)->par.fd);
  xfree(f);
}

struct fastbuf *
fbsock_create(struct fbsock_params *p)
{
  struct fb_sock *F = xmalloc(sizeof(*F) + p->bufsize);
  struct fastbuf *f = &F->fb;

  bzero(F, sizeof(*F));
  F->par = *p;
  f->buffer = F->buf;
  f->bptr = f->bstop = f->buffer;
  f->bufend = f->buffer + p->bufsize;
  f->name = "<socket>";
  f->refill = fbs_refill;
  f->spout = fbs_spout;
  f->close = fbs_close;
  f->can_overwrite_buffer = 1;
  return f;
}

#ifdef TEST

#include <stdlib.h>

static void test_err(void *x UNUSED, uns flags, char *msg UNUSED)
{
  if (flags & FBSOCK_READ)
    printf("READ");
  else if (flags & FBSOCK_WRITE)
    printf("WRITE");
  if (flags & FBSOCK_TIMEOUT)
    printf(" TIMEOUT\n");
  else
    printf(" ERROR\n");
  exit(0);
}

int main(void)
{
  int fd[2];
  if (pipe(fd) < 0)
    ASSERT(0);

  struct fbsock_params p = {
      .fd = fd[0],
      .bufsize = 16,
      .timeout_ms = 100,
      .err = test_err
  };
  struct fastbuf *f = fbsock_create(&p);

  bputsn(f, "Oook!");		// This fits in PIPE_BUF
  bflush(f);

  char buf[256];
  if (!bgets(f, buf, sizeof(buf)))
    die("bgets failed");
  if (strcmp(buf, "Oook!"))
    die("Misread input");

  bgets(f, buf, sizeof(buf));
  puts("WRONG");
  exit(0);
}

#endif
