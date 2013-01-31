/*
 *	UCW Library -- Memory Pools (String Operations)
 *
 *	(c) 2004 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/mempool.h"

#include <alloca.h>
#include <string.h>

char *
mp_strdup(struct mempool *p, const char *s)
{
  uns l = strlen(s) + 1;
  char *t = mp_alloc_fast_noalign(p, l);
  memcpy(t, s, l);
  return t;
}

void *
mp_memdup(struct mempool *p, const void *s, uns len)
{
  void *t = mp_alloc_fast(p, len);
  memcpy(t, s, len);
  return t;
}

char *
mp_multicat(struct mempool *p, ...)
{
  va_list args, a;
  va_start(args, p);
  char *x, *y;
  uns cnt = 0;
  va_copy(a, args);
  while (x = va_arg(a, char *))
    cnt++;
  uns *sizes = alloca(cnt * sizeof(uns));
  uns len = 1;
  cnt = 0;
  va_end(a);
  va_copy(a, args);
  while (x = va_arg(a, char *))
    len += sizes[cnt++] = strlen(x);
  char *buf = mp_alloc_fast_noalign(p, len);
  y = buf;
  va_end(a);
  cnt = 0;
  while (x = va_arg(args, char *))
    {
      memcpy(y, x, sizes[cnt]);
      y += sizes[cnt++];
    }
  *y = 0;
  va_end(args);
  return buf;
}

char *
mp_strjoin(struct mempool *p, char **a, uns n, uns sep)
{
  uns sizes[n];
  uns len = 1;
  for (uns i=0; i<n; i++)
    len += sizes[i] = strlen(a[i]);
  if (sep && n)
    len += n-1;
  char *dest = mp_alloc_fast_noalign(p, len);
  char *d = dest;
  for (uns i=0; i<n; i++)
    {
      if (sep && i)
	*d++ = sep;
      memcpy(d, a[i], sizes[i]);
      d += sizes[i];
    }
  *d = 0;
  return dest;
}

#ifdef TEST

#include <stdio.h>

int main(void)
{
  struct mempool *p = mp_new(64);
  char *s = mp_strdup(p, "12345");
  char *c = mp_multicat(p, "<<", s, ">>", NULL);
  puts(c);
  char *a[] = { "bugs", "gnats", "insects" };
  puts(mp_strjoin(p, a, 3, '.'));
  puts(mp_strjoin(p, a, 3, 0));
  return 0;
}

#endif
