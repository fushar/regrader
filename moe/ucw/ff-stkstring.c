/*
 *	UCW Library -- Fast Buffered I/O: Strings on stack
 *
 *	(c) 2008 Michal Vaner <vorner@ucw.cz>
 *
 *	Code taken from ff-string.c by:
 *
 *	(c) 1997--2006 Martin Mares <mj@ucw.cz>
 *	(c) 2006 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"

void
bgets_stk_init(struct bgets_stk_struct *s)
{
  s->src_len = bdirect_read_prepare(s->f, &s->src);
  if (!s->src_len)
    {
      s->cur_buf = NULL;
      s->cur_len = 0;
    }
  else
    {
      s->old_buf = NULL;
      s->cur_len = 256;
    }
}

void
bgets_stk_step(struct bgets_stk_struct *s)
{
  byte *buf = s->cur_buf;
  uns buf_len = s->cur_len;
  if (s->old_buf)
    {
      memcpy( s->cur_buf, s->old_buf, s->old_len);
      buf += s->old_len;
      buf_len -= s->old_len;
    }
  do
    {
      uns cnt = MIN(s->src_len, buf_len);
      for (uns i = cnt; i--;)
        {
	  byte v = *s->src++;
	  if (v == '\n')
	    {
              bdirect_read_commit(s->f, s->src);
	      goto exit;
	    }
	  *buf++ = v;
	}
      if (cnt == s->src_len)
        {
	  bdirect_read_commit(s->f, s->src);
	  s->src_len = bdirect_read_prepare(s->f, &s->src);
	}
      else
	s->src_len -= cnt;
      if (cnt == buf_len)
        {
	  s->old_len = s->cur_len;
	  s->old_buf = s->cur_buf;
	  s->cur_len *= 2;
	  return;
	}
      else
	buf_len -= cnt;
    }
  while (s->src_len);
exit:
  *buf = 0;
  s->cur_len = 0;
}

