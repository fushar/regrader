/*
 *	UCW Library -- Fast Buffered I/O on Static Buffers
 *
 *	(c) 2003--2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"

#include <stdio.h>
#include <stdlib.h>

static int
fbbuf_refill(struct fastbuf *f UNUSED)
{
  return 0;
}

static int
fbbuf_seek(struct fastbuf *f, ucw_off_t pos, int whence)
{
  /* Somebody might want to seek to the end of buffer, try to be nice to him. */
  ucw_off_t len = f->bufend - f->buffer;
  if (whence == SEEK_END)
    pos += len;
  ASSERT(pos >= 0 && pos <= len);
  f->bptr = f->buffer + pos;
  f->bstop = f->bufend;
  f->pos = len;
  return 1;
}

void
fbbuf_init_read(struct fastbuf *f, byte *buf, uns size, uns can_overwrite)
{
  f->buffer = f->bptr = buf;
  f->bstop = f->bufend = buf + size;
  f->name = "fbbuf-read";
  f->pos = size;
  f->refill = fbbuf_refill;
  f->spout = NULL;
  f->seek = fbbuf_seek;
  f->close = NULL;
  f->config = NULL;
  f->can_overwrite_buffer = can_overwrite;
}

static void
fbbuf_spout(struct fastbuf *f UNUSED)
{
  die("fbbuf: buffer overflow on write");
}

void
fbbuf_init_write(struct fastbuf *f, byte *buf, uns size)
{
  f->buffer = f->bstop = f->bptr = buf;
  f->bufend = buf + size;
  f->name = "fbbuf-write";
  f->pos = size;
  f->refill = NULL;
  f->spout = fbbuf_spout;
  f->seek = NULL;
  f->close = NULL;
  f->config = NULL;
  f->can_overwrite_buffer = 0;
}

#ifdef TEST

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
      fprintf(stderr, "You must specify a test (r, w, o)\n");
      return 1;
    }
  switch (*argv[1])
    {
      case 'r':
        {
          struct fastbuf fb;
          char *data = "Two\nlines\n";
          fbbuf_init_read(&fb, data, strlen(data), 0);
          char buffer[10];
          while (bgets(&fb, buffer, 10))
            puts(buffer);
          bclose(&fb);
          break;
        }
      case 'w':
        {
          struct fastbuf fb;
          char buff[20];
          fbbuf_init_write(&fb, buff, 20);
          bputs(&fb, "Hello world\n");
          bputc(&fb, 0);
          fputs(buff, stdout);
          break;
        }
      case 'o':
        {
          struct fastbuf fb;
          char buff[4];
          fbbuf_init_write(&fb, buff, 4);
          bputs(&fb, "Hello");
          bputc(&fb, 0);
          fputs(buff, stdout);
          break;
        }
    }
  return 0;
}

#endif
