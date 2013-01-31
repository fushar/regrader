/*
 *	UCW Library -- Fast Buffered I/O on O_DIRECT Files
 *
 *	(c) 2006--2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/*
 *	This is a fastbuf backend for fast streaming I/O using O_DIRECT and
 *	the asynchronous I/O module. It's designed for use on large files
 *	which don't fit in the disk cache.
 *
 *	CAVEATS:
 *
 *	  - All operations with a single fbdirect handle must be done
 *	    within a single thread, unless you provide a custom I/O queue
 *	    and take care of locking.
 *
 *	FIXME: what if the OS doesn't support O_DIRECT?
 *	FIXME: unaligned seeks and partial writes?
 *	FIXME: append to unaligned file
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/fastbuf.h"
#include "ucw/lfs.h"
#include "ucw/asio.h"
#include "ucw/conf.h"
#include "ucw/threads.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define FBDIR_ALIGN 512

enum fbdir_mode {				// Current operating mode
    M_NULL,
    M_READ,
    M_WRITE
};

struct fb_direct {
  struct fastbuf fb;
  int fd;					// File descriptor
  int is_temp_file;
  struct asio_queue *io_queue;			// I/O queue to use
  struct asio_queue *user_queue;		// If io_queue was supplied by the user
  struct asio_request *pending_read;
  struct asio_request *done_read;
  struct asio_request *active_buffer;
  enum fbdir_mode mode;
  byte name[0];
};
#define FB_DIRECT(f) ((struct fb_direct *)(f)->is_fastbuf)

#ifndef TEST
uns fbdir_cheat;

static struct cf_section fbdir_cf = {
  CF_ITEMS {
    CF_UNS("Cheat", &fbdir_cheat),
    CF_END
  }
};

static void CONSTRUCTOR
fbdir_global_init(void)
{
  cf_declare_section("FBDirect", &fbdir_cf, 0);
}
#endif

static void
fbdir_read_sync(struct fb_direct *F)
{
  while (F->pending_read)
    {
      struct asio_request *r = asio_wait(F->io_queue);
      ASSERT(r);
      struct fb_direct *G = r->user_data;
      ASSERT(G);
      ASSERT(G->pending_read == r && !G->done_read);
      G->pending_read = NULL;
      G->done_read = r;
    }
}

static void
fbdir_change_mode(struct fb_direct *F, enum fbdir_mode mode)
{
  if (F->mode == mode)
    return;
  DBG("FB-DIRECT: Switching mode to %d", mode);
  switch (F->mode)
    {
    case M_NULL:
      break;
    case M_READ:
      fbdir_read_sync(F);			// Wait for read-ahead requests to finish
      if (F->done_read)				// Return read-ahead requests if any
	{
	  asio_put(F->done_read);
	  F->done_read = NULL;
	}
      break;
    case M_WRITE:
      asio_sync(F->io_queue);			// Wait for pending writebacks
      break;
    }
  if (F->active_buffer)
    {
      asio_put(F->active_buffer);
      F->active_buffer = NULL;
    }
  F->mode = mode;
}

static void
fbdir_submit_read(struct fb_direct *F)
{
  struct asio_request *r = asio_get(F->io_queue);
  r->fd = F->fd;
  r->op = ASIO_READ;
  r->len = F->io_queue->buffer_size;
  r->user_data = F;
  asio_submit(r);
  F->pending_read = r;
}

static int
fbdir_refill(struct fastbuf *f)
{
  struct fb_direct *F = FB_DIRECT(f);

  DBG("FB-DIRECT: Refill");

  if (!F->done_read)
    {
      if (!F->pending_read)
	{
	  fbdir_change_mode(F, M_READ);
	  fbdir_submit_read(F);
	}
      fbdir_read_sync(F);
      ASSERT(F->done_read);
    }

  struct asio_request *r = F->done_read;
  F->done_read = NULL;
  if (F->active_buffer)
    asio_put(F->active_buffer);
  F->active_buffer = r;
  if (!r->status)
    return 0;
  if (r->status < 0)
    die("Error reading %s: %s", f->name, strerror(r->returned_errno));
  f->bptr = f->buffer = r->buffer;
  f->bstop = f->bufend = f->buffer + r->status;
  f->pos += r->status;

  fbdir_submit_read(F);				// Read-ahead the next block

  return r->status;
}

static void
fbdir_spout(struct fastbuf *f)
{
  struct fb_direct *F = FB_DIRECT(f);
  struct asio_request *r;

  DBG("FB-DIRECT: Spout");

  fbdir_change_mode(F, M_WRITE);
  r = F->active_buffer;
  if (r && f->bptr > f->bstop)
    {
      r->op = ASIO_WRITE_BACK;
      r->fd = F->fd;
      r->len = f->bptr - f->bstop;
      ASSERT(!(f->pos % FBDIR_ALIGN) || fbdir_cheat);
      f->pos += r->len;
      if (!fbdir_cheat && r->len % FBDIR_ALIGN)			// Have to simulate incomplete writes
	{
	  r->len = ALIGN_TO(r->len, FBDIR_ALIGN);
	  asio_submit(r);
	  asio_sync(F->io_queue);
	  DBG("FB-DIRECT: Truncating at %llu", (long long)f->pos);
	  if (ucw_ftruncate(F->fd, f->pos) < 0)
	    die("Error truncating %s: %m", f->name);
	}
      else
	asio_submit(r);
      r = NULL;
    }
  if (!r)
    r = asio_get(F->io_queue);
  f->bstop = f->bptr = f->buffer = r->buffer;
  f->bufend = f->buffer + F->io_queue->buffer_size;
  F->active_buffer = r;
}

static int
fbdir_seek(struct fastbuf *f, ucw_off_t pos, int whence)
{
  DBG("FB-DIRECT: Seek %llu %d", (long long)pos, whence);

  if (whence == SEEK_SET && pos == f->pos)
    return 1;

  fbdir_change_mode(FB_DIRECT(f), M_NULL);			// Wait for all async requests to finish
  ucw_off_t l = ucw_seek(FB_DIRECT(f)->fd, pos, whence);
  if (l < 0)
    return 0;
  f->pos = l;
  return 1;
}

static struct asio_queue *
fbdir_get_io_queue(uns buffer_size, uns write_back)
{
  struct ucwlib_context *ctx = ucwlib_thread_context();
  struct asio_queue *q = ctx->io_queue;
  if (!q)
    {
      q = xmalloc_zero(sizeof(struct asio_queue));
      q->buffer_size = buffer_size;
      q->max_writebacks = write_back;
      asio_init_queue(q);
      ctx->io_queue = q;
    }
  q->use_count++;
  DBG("FB-DIRECT: Got I/O queue, uc=%d", q->use_count);
  return q;
}

static void
fbdir_put_io_queue(void)
{
  struct ucwlib_context *ctx = ucwlib_thread_context();
  struct asio_queue *q = ctx->io_queue;
  ASSERT(q);
  DBG("FB-DIRECT: Put I/O queue, uc=%d", q->use_count);
  if (!--q->use_count)
    {
      asio_cleanup_queue(q);
      xfree(q);
      ctx->io_queue = NULL;
    }
}

static void
fbdir_close(struct fastbuf *f)
{
  struct fb_direct *F = FB_DIRECT(f);

  DBG("FB-DIRECT: Close");

  fbdir_change_mode(F, M_NULL);
  if (!F->user_queue)
    fbdir_put_io_queue();

  bclose_file_helper(f, F->fd, F->is_temp_file);
  xfree(f);
}

static int
fbdir_config(struct fastbuf *f, uns item, int value)
{
  int orig;

  switch (item)
    {
    case BCONFIG_IS_TEMP_FILE:
      orig = FB_DIRECT(f)->is_temp_file;
      FB_DIRECT(f)->is_temp_file = value;
      return orig;
    default:
      return -1;
    }
}

struct fastbuf *
fbdir_open_fd_internal(int fd, const char *name, struct asio_queue *q, uns buffer_size, uns read_ahead UNUSED, uns write_back)
{
  int namelen = strlen(name) + 1;
  struct fb_direct *F = xmalloc(sizeof(struct fb_direct) + namelen);
  struct fastbuf *f = &F->fb;

  DBG("FB-DIRECT: Open");
  bzero(F, sizeof(*F));
  f->name = F->name;
  memcpy(f->name, name, namelen);
  F->fd = fd;
  if (q)
    F->io_queue = F->user_queue = q;
  else
    F->io_queue = fbdir_get_io_queue(buffer_size, write_back);
  f->refill = fbdir_refill;
  f->spout = fbdir_spout;
  f->seek = fbdir_seek;
  f->close = fbdir_close;
  f->config = fbdir_config;
  f->can_overwrite_buffer = 2;
  return f;
}

#ifdef TEST

#include "ucw/getopt.h"

int main(int argc, char **argv)
{
  struct fb_params par = { .type = FB_DIRECT };
  struct fastbuf *f, *t;

  log_init(NULL);
  if (cf_getopt(argc, argv, CF_SHORT_OPTS, CF_NO_LONG_OPTS, NULL) >= 0)
    die("Hey, whaddya want?");
  f = (optind < argc) ? bopen_file(argv[optind++], O_RDONLY, &par) : bopen_fd(0, &par);
  t = (optind < argc) ? bopen_file(argv[optind++], O_RDWR | O_CREAT | O_TRUNC, &par) : bopen_fd(1, &par);

  bbcopy(f, t, ~0U);
  ASSERT(btell(f) == btell(t));

#if 0		// This triggers unaligned write
  bflush(t);
  bputc(t, '\n');
#endif

  brewind(t);
  bgetc(t);
  ASSERT(btell(t) == 1);

  bclose(f);
  bclose(t);
  return 0;
}

#endif
