/*
 *	UCW Library -- Atomic Buffered Write to Files
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"
#include "ucw/lfs.h"
#include "ucw/conf.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static uns trace;

#ifndef TEST

static struct cf_section fbatomic_config = {
  CF_ITEMS {
    CF_UNS("Trace", &trace)
  }
};

static void CONSTRUCTOR fbatomic_init_config(void)
{
  cf_declare_section("FBAtomic", &fbatomic_config, 1);
}

#endif

#define TRACE(m...) do { if(trace) msg(L_DEBUG, "FB_ATOMIC: " m); } while(0)

struct fb_atomic_file {
  int fd;
  int use_count;
  int record_len;
  uns locked;
  byte name[1];
};

void
fbatomic_internal_write(struct fastbuf *f)
{
  struct fb_atomic_file *af = FB_ATOMIC(f)->af;
  int size = f->bptr - f->buffer;
  if (size)
    {
      ASSERT(af->record_len < 0 || !(size % af->record_len));
      int res = write(af->fd, f->buffer, size);
      if (res < 0)
	die("Error writing %s: %m", f->name);
      if (res != size)
	die("Unexpected partial write to %s: written only %d bytes of %d", f->name, res, size);
      f->bptr = f->buffer;
    }
}

static void
fbatomic_spout(struct fastbuf *f)
{
  if (f->bptr < f->bufend)		/* Explicit flushes should be ignored */
    return;

  struct fb_atomic *F = FB_ATOMIC(f);
  if (F->af->locked)
    {
      uns written = f->bptr - f->buffer;
      uns size = f->bufend - f->buffer + F->slack_size;
      F->slack_size *= 2;
      TRACE("Reallocating buffer for atomic file %s with slack %d", f->name, F->slack_size);
      f->buffer = xrealloc(f->buffer, size);
      f->bufend = f->buffer + size;
      f->bptr = f->buffer + written;
      F->expected_max_bptr = f->bufend - F->slack_size;
    }
  else
    fbatomic_internal_write(f);
}

static void
fbatomic_close(struct fastbuf *f)
{
  struct fb_atomic_file *af = FB_ATOMIC(f)->af;
  fbatomic_internal_write(f);	/* Need to flush explicitly, because the file can be locked */
  if (!--af->use_count)
    {
      close(af->fd);
      xfree(af);
    }
  xfree(f);
}

struct fastbuf *
fbatomic_open(const char *name, struct fastbuf *master, uns bufsize, int record_len)
{
  struct fb_atomic *F = xmalloc_zero(sizeof(*F));
  struct fastbuf *f = &F->fb;
  struct fb_atomic_file *af;
  if (master)
    {
      af = FB_ATOMIC(master)->af;
      af->use_count++;
      ASSERT(af->record_len == record_len);
    }
  else
    {
      af = xmalloc_zero(sizeof(*af) + strlen(name));
      if ((af->fd = ucw_open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0666)) < 0)
	die("Cannot create %s: %m", name);
      af->use_count = 1;
      af->record_len = record_len;
      af->locked = (record_len < 0);
      strcpy(af->name, name);
    }
  F->af = af;
  if (record_len > 0 && bufsize % record_len)
    bufsize += record_len - (bufsize % record_len);
  f->buffer = xmalloc(bufsize);
  f->bufend = f->buffer + bufsize;
  F->slack_size = (record_len < 0) ? -record_len : 0;
  ASSERT(bufsize > F->slack_size);
  F->expected_max_bptr = f->bufend - F->slack_size;
  f->bptr = f->bstop = f->buffer;
  f->name = af->name;
  f->spout = fbatomic_spout;
  f->close = fbatomic_close;
  return f;
}

#ifdef TEST

int main(int argc UNUSED, char **argv UNUSED)
{
  struct fastbuf *f, *g;

  // Always trace in the test
  trace = 1;

  msg(L_INFO, "Testing block writes");
  f = fbatomic_open("test", NULL, 16, 4);
  for (u32 i=0; i<17; i++)
    bwrite(f, &i, 4);
  bclose(f);

  msg(L_INFO, "Testing interleaved var-size writes");
  f = fbatomic_open("test2", NULL, 23, -5);
  g = fbatomic_open("test2", f, 23, -5);
  for (int i=0; i<100; i++)
    {
      struct fastbuf *x = (i%2) ? g : f;
      bprintf(x, "%c<%d>\n", "fg"[i%2], ((259309*i) % 1000000) >> (i % 8));
      fbatomic_commit(x);
    }
  bclose(f);
  bclose(g);

  return 0;
}

#endif
