/*
 *	UCW Library -- String Routines
 *
 *	(c) 2006 Pavel Charvat <pchar@ucw.cz>
 *	(c) 2007--2008 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/string.h"

char *
str_format_flags(char *dest, const char *fmt, uns flags)
{
  char *start = dest;
  for (uns i=0; fmt[i]; i++)
    {
      if (flags & (1 << i))
	*dest++ = fmt[i];
      else
	*dest++ = '-';
    }
  *dest = 0;
  return start;
}

uns
str_count_char(const char *str, uns chr)
{
  const byte *s = str;
  uns i = 0;
  while (*s)
    if (*s++ == chr)
      i++;
  return i;
}
