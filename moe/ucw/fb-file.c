/*
 *	UCW Library -- Fast Buffered I/O on Files
 *
 *	(c) 1997--2007 Martin Mares <mj@ucw.cz>
 *	(c) 2007 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"
#include "ucw/lfs.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

struct fb_file {
  struct fastbuf fb;
  int fd;				/* File descriptor */
  int is_temp_file;
  int keep_back_buf;			/* Optimize for backwards reading */
  ucw_off_t wpos;			/* Real file position */
  uns wlen;				/* Window size */
};
#define FB_FILE(f) ((struct fb_file *)(f)->is_fastbuf)
#define FB_BUFFER(f) (byte *)(FB_FILE(f) + 1)

static int
bfd_refill(struct fastbuf *f)
{
  struct fb_file *F = FB_FILE(f);
  byte *read_ptr = (f->buffer = FB_BUFFER(f));
  uns blen = f->bufend - f->buffer, back = F->keep_back_buf ? blen >> 2 : 0, read_len = blen;
  /* Forward or no seek */
  if (F->wpos <= f->pos)
    {
      ucw_off_t diff = f->pos - F->wpos;
      /* Formula for long forward seeks (prefer lseek()) */
      if (diff > ((ucw_off_t)blen << 2))
        {
long_seek:
	  f->bptr = f->buffer + back;
	  f->bstop = f->buffer + blen;
	  goto seek;
	}
      /* Short forward seek (prefer read() to skip data )*/
      else if ((uns)diff >= back)
        {
	  uns skip = diff - back;
	  F->wpos += skip;
	  while (skip)
	    {
	      int l = read(F->fd, f->buffer, MIN(skip, blen));
	      if (unlikely(l <= 0))
		if (l < 0)
		  die("Error reading %s: %m", f->name);
		else
		  {
		    F->wpos -= skip;
		    goto eof;
		  }
	      skip -= l;
	    }
	}
      /* Reuse part of the previous window and append new data (also F->wpos == f->pos) */
      else
        {
	  uns keep = back - (uns)diff;
	  if (keep >= F->wlen)
	    back = diff + (keep = F->wlen);
	  else
	    memmove(f->buffer, f->buffer + F->wlen - keep, keep);
	  read_len -= keep;
	  read_ptr += keep;
	}
      f->bptr = f->buffer + back;
      f->bstop = f->buffer + blen;
    }
  /* Backwards seek */
  else
    {
      ucw_off_t diff = F->wpos - f->pos;
      /* Formula for long backwards seeks (keep smaller backbuffer than for shorter seeks ) */
      if (diff > ((ucw_off_t)blen << 1))
        {
	  if ((ucw_off_t)back > f->pos)
	    back = f->pos;
	  goto long_seek;
	}
      /* Seek into previous window (do nothing... for example brewind) */
      else if ((uns)diff <= F->wlen) 
        {
	  f->bstop = f->buffer + F->wlen;
	  f->bptr = f->bstop - diff;
	  f->pos = F->wpos;
	  return 1;
	}
      back *= 3;
      if ((ucw_off_t)back > f->pos)
	back = f->pos;
      f->bptr = f->buffer + back;
      read_len = blen;
      f->bstop = f->buffer + read_len;
      /* Reuse part of previous window */
      if (F->wlen && read_len <= back + diff && read_len > back + diff - F->wlen)
        {
	  uns keep = read_len + F->wlen - back - diff;
	  memmove(f->buffer + read_len - keep, f->buffer, keep);
	}
seek:
      /* Do lseek() */
      F->wpos = f->pos + (f->buffer - f->bptr);
      if (ucw_seek(F->fd, F->wpos, SEEK_SET) < 0)
	die("Error seeking %s: %m", f->name);
    }
  /* Read (part of) buffer */
  do
    {
      int l = read(F->fd, read_ptr, read_len);
      if (unlikely(l < 0))
	die("Error reading %s: %m", f->name);
      if (!l)
	if (unlikely(read_ptr < f->bptr))
	  goto eof;
	else
	  break; /* Incomplete read because of EOF */
      read_ptr += l;
      read_len -= l;
      F->wpos += l;
    }
  while (read_ptr <= f->bptr);
  if (read_len)
    f->bstop = read_ptr;
  f->pos += f->bstop - f->bptr;
  F->wlen = f->bstop - f->buffer;
  return f->bstop - f->bptr;
eof:
  /* Seeked behind EOF */
  f->bptr = f->bstop = f->buffer;
  F->wlen = 0;
  return 0;
}

static void
bfd_spout(struct fastbuf *f)
{
  /* Do delayed lseek() if needed */
  if (FB_FILE(f)->wpos != f->pos && ucw_seek(FB_FILE(f)->fd, f->pos, SEEK_SET) < 0)
    die("Error seeking %s: %m", f->name);

  int l = f->bptr - f->buffer;
  byte *c = f->buffer;

  /* Write the buffer */
  FB_FILE(f)->wpos = (f->pos += l);
  FB_FILE(f)->wlen = 0;
  while (l)
    {
      int z = write(FB_FILE(f)->fd, c, l);
      if (z <= 0)
	die("Error writing %s: %m", f->name);
      l -= z;
      c += z;
    }
  f->bptr = f->buffer = FB_BUFFER(f);
}

static int
bfd_seek(struct fastbuf *f, ucw_off_t pos, int whence)
{
  /* Delay the seek for the next refill() or spout() call (if whence != SEEK_END). */
  ucw_off_t l;
  switch (whence)
    {
      case SEEK_SET:
	f->pos = pos;
	return 1;
      case SEEK_CUR:
	l = f->pos + pos;
	if ((pos > 0) ^ (l > f->pos))
	  return 0;
	f->pos = l;
	return 1;
      case SEEK_END:
	l = ucw_seek(FB_FILE(f)->fd, pos, SEEK_END);
	if (l < 0)
	  return 0;
	FB_FILE(f)->wpos = f->pos = l;
	FB_FILE(f)->wlen = 0;
	return 1;
      default:
	ASSERT(0);
    }
}

static void
bfd_close(struct fastbuf *f)
{
  bclose_file_helper(f, FB_FILE(f)->fd, FB_FILE(f)->is_temp_file);
  xfree(f);
}

static int
bfd_config(struct fastbuf *f, uns item, int value)
{
  int orig;

  switch (item)
    {
      case BCONFIG_IS_TEMP_FILE:
	orig = FB_FILE(f)->is_temp_file;
	FB_FILE(f)->is_temp_file = value;
	return orig;
      case BCONFIG_KEEP_BACK_BUF:
        orig = FB_FILE(f)->keep_back_buf;
	FB_FILE(f)->keep_back_buf = value;
	return orig;
      default:
	return -1;
    }
}

struct fastbuf *
bfdopen_internal(int fd, const char *name, uns buflen)
{
  ASSERT(buflen);
  int namelen = strlen(name) + 1;
  struct fb_file *F = xmalloc_zero(sizeof(struct fb_file) + buflen + namelen);
  struct fastbuf *f = &F->fb;

  bzero(F, sizeof(*F));
  f->buffer = (byte *)(F+1);
  f->bptr = f->bstop = f->buffer;
  f->bufend = f->buffer + buflen;
  f->name = f->bufend;
  memcpy(f->name, name, namelen);
  F->fd = fd;
  f->refill = bfd_refill;
  f->spout = bfd_spout;
  f->seek = bfd_seek;
  f->close = bfd_close;
  f->config = bfd_config;
  f->can_overwrite_buffer = 2;
  return f;
}

void
bfilesync(struct fastbuf *b)
{
  bflush(b);
  if (fsync(FB_FILE(b)->fd) < 0)
    msg(L_ERROR, "fsync(%s) failed: %m", b->name);
}

#ifdef TEST

int main(void)
{
  struct fastbuf *f, *t;
  f = bopen_tmp(16);
  t = bfdopen_shared(1, 13);
  for (uns i = 0; i < 16; i++)
    bwrite(f, "<hello>", 7);
  bprintf(t, "%d\n", (int)btell(f));
  brewind(f);
  bbcopy(f, t, ~0U);
  bprintf(t, "\n%d %d\n", (int)btell(f), (int)btell(t));
  bclose(f);
  bclose(t);
  return 0;
}

#endif
