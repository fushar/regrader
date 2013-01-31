/*
 *	UCW Library -- Printf on Fastbuf Streams
 *
 *	(c) 2002--2005 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"

#include <stdio.h>
#include <alloca.h>

int
vbprintf(struct fastbuf *b, const char *msg, va_list args)
{
  byte *buf;
  int len, r;
  va_list args2;

  len = bdirect_write_prepare(b, &buf);
  if (len >= 16)
    {
      va_copy(args2, args);
      r = vsnprintf(buf, len, msg, args2);
      va_end(args2);
      if (r < 0)
	len = 256;
      else if (r < len)
	{
	  bdirect_write_commit(b, buf+r);
	  return r;
	}
      else
	len = r+1;
    }
  else
    len = 256;

  while (1)
    {
      buf = alloca(len);
      va_copy(args2, args);
      r = vsnprintf(buf, len, msg, args2);
      va_end(args2);
      if (r < 0)
	len += len;
      else if (r < len)
	{
	  bwrite(b, buf, r);
	  return r;
	}
      else
	len = r+1;
    }
}

int
bprintf(struct fastbuf *b, const char *msg, ...)
{
  va_list args;
  int res;

  va_start(args, msg);
  res = vbprintf(b, msg, args);
  va_end(args);
  return res;
}

#ifdef TEST

int main(void)
{
  struct fastbuf *b = bfdopen_shared(1, 65536);
  for (int i=0; i<10000; i++)
    bprintf(b, "13=%d str=<%s> msg=%m\n", 13, "str");
  bclose(b);
  return 0;
}

#endif
