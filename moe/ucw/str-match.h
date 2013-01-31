/*
 *	UCW Library -- Generic Shell-Like Pattern Matching (currently only '?' and '*')
 *
 *	(c) 1997 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/string.h"

int
MATCH_FUNC_NAME(const char *p, const char *s)
{
  while (*p)
    {
      if (*p == '?' && *s)
	p++, s++;
      else if (*p == '*')
	{
	  int z = p[1];

	  if (!z)
	    return 1;
	  if (z == '\\' && p[2])
	    z = p[2];
	  z = Convert(z);
	  for(;;)
	    {
	      while (*s && Convert(*s) != z)
		s++;
	      if (!*s)
		return 0;
	      if (MATCH_FUNC_NAME(p+1, s))
		return 1;
	      s++;
	    }
	}
      else
	{
	  if (*p == '\\' && p[1])
	    p++;
	  if (Convert(*p++) != Convert(*s++))
	    return 0;
	}
    }
  return !*s;
}
