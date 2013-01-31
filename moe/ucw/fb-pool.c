/*
 *	UCW Library -- Fast Buffered I/O on Memory Pools
 *
 *	(c) 2007 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/mempool.h"
#include "ucw/fastbuf.h"

#include <stdio.h>
#include <stdlib.h>

#define FB_POOL(f) ((struct fbpool *)(f)->is_fastbuf)

static void
fbpool_spout(struct fastbuf *b)
{
  if (b->bptr >= b->bufend)
    {
      uns len = b->bufend - b->buffer;
      b->buffer = mp_expand(FB_POOL(b)->mp);
      b->bufend = b->buffer + mp_avail(FB_POOL(b)->mp);
      b->bstop = b->buffer;
      b->bptr = b->buffer + len;
    }
}

void
fbpool_start(struct fbpool *b, struct mempool *mp, uns init_size)
{
  b->mp = mp;
  b->fb.buffer = b->fb.bstop = b->fb.bptr = mp_start(mp, init_size);
  b->fb.bufend = b->fb.buffer + mp_avail(mp);
}

void *
fbpool_end(struct fbpool *b)
{
  return mp_end(b->mp, b->fb.bptr); 
}

void
fbpool_init(struct fbpool *b)
{
  bzero(b, sizeof(*b));
  b->fb.name = "<fbpool>";
  b->fb.spout = fbpool_spout;
  b->fb.can_overwrite_buffer = 1;
}

#ifdef TEST

int main(void)
{
  struct mempool *mp;
  struct fbpool fb;
  byte *p;
  uns l;
  
  mp = mp_new(64);
  fbpool_init(&fb);
  fbpool_start(&fb, mp, 16);
  for (uns i = 0; i < 1024; i++)
    bprintf(&fb.fb, "<hello>");
  p = fbpool_end(&fb);
  l = mp_size(mp, p);
  if (l != 1024 * 7)
    ASSERT(0);
  for (uns i = 0; i < 1024; i++)
    if (memcmp(p + i * 7, "<hello>", 7))
      ASSERT(0);
  mp_delete(mp);
  
  return 0;
}

#endif
