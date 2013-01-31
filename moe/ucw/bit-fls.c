/*
 *	UCW Library -- Find Highest Set Bit
 *
 *	(c) 1997-2005 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/bitops.h"

int
bit_fls(u32 x)
{
  uns l;

  if (!x)
	return -1;

  l = 0;
  if (x & 0xffff0000) { l += 16; x &= 0xffff0000; }
  if (x & 0xff00ff00) { l += 8;  x &= 0xff00ff00; }
  if (x & 0xf0f0f0f0) { l += 4;  x &= 0xf0f0f0f0; }
  if (x & 0xcccccccc) { l += 2;  x &= 0xcccccccc; }
  if (x & 0xaaaaaaaa) l++;
  return l;
}

#ifdef TEST

#include <stdio.h>

int main(void)
{
  uns i;
  while (scanf("%x", &i) == 1)
    printf("%d\n", bit_fls(i));
  return 0;
}

#endif
