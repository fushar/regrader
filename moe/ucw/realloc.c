/*
 *	UCW Library -- Memory Re-allocation
 *
 *	(c) 1997 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <stdlib.h>

void *
xrealloc(void *old, uns size)
{
  /* We assume that realloc(NULL, x) works like malloc(x), which is true with the glibc. */
  void *x = realloc(old, size);
  if (!x)
    die("Cannot reallocate %d bytes of memory", size);
  return x;
}
