/*
 *	UCW Library -- Fast Buffered I/O on Memory-Mapped Files
 *
 *	(c) 2002 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/fastbuf.h"
#include "ucw/lfs.h"
#include "ucw/conf.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

static uns mmap_window_size = 16*CPU_PAGE_SIZE;
static uns mmap_extend_size = 4*CPU_PAGE_SIZE;

#ifndef TEST
static struct cf_section fbmm_config = {
  CF_ITEMS {
    CF_UNS("WindowSize", &mmap_window_size),
    CF_UNS("ExtendSize", &mmap_extend_size),
    CF_END
  }
};

static void CONSTRUCTOR fbmm_init_config(void)
{
  cf_declare_section("FBMMap", &fbmm_config, 0);
}
#endif

struct fb_mmap {
  struct fastbuf fb;
  int fd;
  int is_temp_file;
  ucw_off_t file_size;
  ucw_off_t file_extend;
  ucw_off_t window_pos;
  uns window_size;
  int mode;
};
#define FB_MMAP(f) ((struct fb_mmap *)(f)->is_fastbuf)

static void
bfmm_map_window(struct fastbuf *f)
{
  struct fb_mmap *F = FB_MMAP(f);
  ucw_off_t pos0 = f->pos & ~(ucw_off_t)(CPU_PAGE_SIZE-1);
  int l = MIN((ucw_off_t)mmap_window_size, F->file_extend - pos0);
  uns ll = ALIGN_TO(l, CPU_PAGE_SIZE);
  int prot = ((F->mode & O_ACCMODE) == O_RDONLY) ? PROT_READ : (PROT_READ | PROT_WRITE);

  DBG(" ... Mapping %x(%x)+%x(%x) len=%x extend=%x", (int)pos0, (int)f->pos, ll, l, (int)F->file_size, (int)F->file_extend);
  if (ll != F->window_size && f->buffer)
    {
      munmap(f->buffer, F->window_size);
      f->buffer = NULL;
    }
  F->window_size = ll;
  if (!f->buffer)
    f->buffer = ucw_mmap(NULL, ll, prot, MAP_SHARED, F->fd, pos0);
  else
    f->buffer = ucw_mmap(f->buffer, ll, prot, MAP_SHARED | MAP_FIXED, F->fd, pos0);
  if (f->buffer == (byte *) MAP_FAILED)
    die("mmap(%s): %m", f->name);
#ifdef MADV_SEQUENTIAL
  if (ll > CPU_PAGE_SIZE)
    madvise(f->buffer, ll, MADV_SEQUENTIAL);
#endif
  f->bufend = f->buffer + l;
  f->bptr = f->buffer + (f->pos - pos0);
  F->window_pos = pos0;
}

static int
bfmm_refill(struct fastbuf *f)
{
  struct fb_mmap *F = FB_MMAP(f);

  DBG("Refill <- %p %p %p %p", f->buffer, f->bptr, f->bstop, f->bufend);
  if (f->pos >= F->file_size)
    return 0;
  if (f->bstop >= f->bufend)
    bfmm_map_window(f);
  if (F->window_pos + (f->bufend - f->buffer) > F->file_size)
    f->bstop = f->buffer + (F->file_size - F->window_pos);
  else
    f->bstop = f->bufend;
  f->pos = F->window_pos + (f->bstop - f->buffer);
  DBG(" -> %p %p %p(%x) %p", f->buffer, f->bptr, f->bstop, (int)f->pos, f->bufend);
  return 1;
}

static void
bfmm_spout(struct fastbuf *f)
{
  struct fb_mmap *F = FB_MMAP(f);
  ucw_off_t end = f->pos + (f->bptr - f->bstop);

  DBG("Spout <- %p %p %p %p", f->buffer, f->bptr, f->bstop, f->bufend);
  if (end > F->file_size)
    F->file_size = end;
  if (f->bptr < f->bufend)
    return;
  f->pos = end;
  if (f->pos >= F->file_extend)
    {
      F->file_extend = ALIGN_TO(F->file_extend + mmap_extend_size, (ucw_off_t)CPU_PAGE_SIZE);
      if (ucw_ftruncate(F->fd, F->file_extend))
	die("ftruncate(%s): %m", f->name);
    }
  bfmm_map_window(f);
  f->bstop = f->bptr;
  DBG(" -> %p %p %p(%x) %p", f->buffer, f->bptr, f->bstop, (int)f->pos, f->bufend);
}

static int
bfmm_seek(struct fastbuf *f, ucw_off_t pos, int whence)
{
  if (whence == SEEK_END)
    pos += FB_MMAP(f)->file_size;
  else
    ASSERT(whence == SEEK_SET);
  ASSERT(pos >= 0 && pos <= FB_MMAP(f)->file_size);
  f->pos = pos;
  f->bptr = f->bstop = f->bufend = f->buffer;	/* force refill/spout call */
  DBG("Seek -> %p %p %p(%x) %p", f->buffer, f->bptr, f->bstop, (int)f->pos, f->bufend);
  return 1;
}

static void
bfmm_close(struct fastbuf *f)
{
  struct fb_mmap *F = FB_MMAP(f);

  if (f->buffer)
    munmap(f->buffer, F->window_size);
  if (F->file_extend > F->file_size &&
      ucw_ftruncate(F->fd, F->file_size))
    die("ftruncate(%s): %m", f->name);
  bclose_file_helper(f, F->fd, F->is_temp_file);
  xfree(f);
}

static int
bfmm_config(struct fastbuf *f, uns item, int value)
{
  int orig;

  switch (item)
    {
    case BCONFIG_IS_TEMP_FILE:
      orig = FB_MMAP(f)->is_temp_file;
      FB_MMAP(f)->is_temp_file = value;
      return orig;
    default:
      return -1;
    }
}

struct fastbuf *
bfmmopen_internal(int fd, const char *name, uns mode)
{
  int namelen = strlen(name) + 1;
  struct fb_mmap *F = xmalloc(sizeof(struct fb_mmap) + namelen);
  struct fastbuf *f = &F->fb;

  bzero(F, sizeof(*F));
  f->name = (byte *)(F+1);
  memcpy(f->name, name, namelen);
  F->fd = fd;
  F->file_extend = F->file_size = ucw_seek(fd, 0, SEEK_END);
  if (F->file_size < 0)
    die("seek(%s): %m", name);
  if (mode & O_APPEND)
    f->pos = F->file_size;
  F->mode = mode;

  f->refill = bfmm_refill;
  f->spout = bfmm_spout;
  f->seek = bfmm_seek;
  f->close = bfmm_close;
  f->config = bfmm_config;
  return f;
}

#ifdef TEST

int main(int UNUSED argc, char **argv)
{
  struct fb_params par = { .type = FB_MMAP };
  struct fastbuf *f = bopen_file(argv[1], O_RDONLY, &par);
  struct fastbuf *g = bopen_file(argv[2], O_RDWR | O_CREAT | O_TRUNC, &par);
  int c;

  DBG("Copying");
  while ((c = bgetc(f)) >= 0)
    bputc(g, c);
  bclose(f);
  DBG("Seek inside last block");
  bsetpos(g, btell(g)-1333);
  bputc(g, 13);
  DBG("Seek to the beginning & write");
  bsetpos(g, 1333);
  bputc(g, 13);
  DBG("flush");
  bflush(g);
  bputc(g, 13);
  bflush(g);
  DBG("Seek nearby & read");
  bsetpos(g, 133);
  bgetc(g);
  DBG("Seek far & read");
  bsetpos(g, 133333);
  bgetc(g);
  DBG("Closing");
  bclose(g);

  return 0;
}

#endif
