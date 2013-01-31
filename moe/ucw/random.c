/*
 *	UCW Library -- Unbiased Random Numbers
 *
 *	(c) 1998--2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <stdlib.h>

/* We expect the random generator in libc to give at least 30 bits of randomness */
COMPILE_ASSERT(RAND_MAX_RANGE_TEST, RAND_MAX >= (1 << 30)-1);

uns
random_u32(void)
{
  return (random() & 0xffff) | ((random() & 0xffff) << 16);
}

uns
random_max(uns max)
{
  uns r, l;

  ASSERT(max <= (1 << 30));
  l = (RAND_MAX + 1U) - ((RAND_MAX + 1U) % max);
  do
    r = random();
  while (r >= l);
  return r % max;
}

u64
random_u64(void)
{
  return
    ((u64)(random() & 0xffff) << 48) |
    ((u64)(random() & 0xffffff) << 24) |
    (random() & 0xffffff);
}

u64
random_max_u64(u64 max)
{
  if (max < (1 << 30))
    return random_max(max);

  u64 r, l, m;
  m = 0xffffffffffffffff;
  l = m - (m % max);
  do
    r = random_u64();
  while (r >= l);
  return r % max;
}
