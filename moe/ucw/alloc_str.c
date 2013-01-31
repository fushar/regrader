/*
 *	UCW Library -- String Allocation
 *
 *	(c) 1997 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"

#include <string.h>

char *
xstrdup(const char *s)
{
  uns l = strlen(s) + 1;
  return memcpy(xmalloc(l), s, l);
}
