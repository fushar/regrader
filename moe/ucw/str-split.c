/*
 *	UCW Library -- Word Splitting
 *
 *	(c) 1997 Martin Mares <mj@ucw.cz>
 *	(c) 2004 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/chartype.h"
#include "ucw/string.h"

#include <string.h>

int
str_sepsplit(char *str, uns sep, char **rec, uns max)
{
  uns cnt = 0;
  while (1)
  {
    rec[cnt++] = str;
    str = strchr(str, sep);
    if (!str)
      return cnt;
    if (cnt >= max)
      return -1;
    *str++ = 0;
  }
}

int
str_wordsplit(char *src, char **dst, uns max)
{
  uns cnt = 0;

  for(;;)
    {
      while (Cspace(*src))
	*src++ = 0;
      if (!*src)
	break;
      if (cnt >= max)
	return -1;
      if (*src == '"')
	{
	  src++;
	  dst[cnt++] = src;
	  while (*src && *src != '"')
	    src++;
	  if (*src)
	    *src++ = 0;
	}
      else
	{
	  dst[cnt++] = src;
	  while (*src && !Cspace(*src))
	    src++;
	}
    }
  return cnt;
}
