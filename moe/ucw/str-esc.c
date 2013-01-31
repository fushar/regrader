/*
 *	UCW Library -- String Unescaping
 *
 *	(c) 2006 Pavel Charvat <pchar@ucw.cz>
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/string.h"
#include "ucw/chartype.h"
#include <stdlib.h>

/* Expands C99-like escape sequences.
 * It is safe to use the same buffer for both input and output. */
char *
str_unesc(char *d, const char *s)
{
  while (*s)
    {
      if (*s == '\\')
	switch (s[1])
	  {
	    case 'a': *d++ = '\a'; s += 2; break;
	    case 'b': *d++ = '\b'; s += 2; break;
	    case 'f': *d++ = '\f'; s += 2; break;
	    case 'n': *d++ = '\n'; s += 2; break;
	    case 'r': *d++ = '\r'; s += 2; break;
	    case 't': *d++ = '\t'; s += 2; break;
	    case 'v': *d++ = '\v'; s += 2; break;
	    case '\?': *d++ = '\?'; s += 2; break;
	    case '\'': *d++ = '\''; s += 2; break;
	    case '\"': *d++ = '\"'; s += 2; break;
	    case '\\': *d++ = '\\'; s += 2; break;
	    case 'x':
	      if (!Cxdigit(s[2]))
	        {
		  s++;
		  DBG("\\x used with no following hex digits");
		}
	      else
	        {
		  char *p;
		  uns v = strtoul(s + 2, &p, 16);
		  if (v <= 255)
		    *d++ = v;
		  else
		    DBG("hex escape sequence out of range");
                  s = (char *)p;
		}
	      break;
            default:
	      if (s[1] >= '0' && s[1] <= '7')
	        {
		  uns v = s[1] - '0';
		  s += 2;
		  for (uns i = 0; i < 2 && *s >= '0' && *s <= '7'; s++, i++)
		    v = (v << 3) + *s - '0';
		  if (v <= 255)
		    *d++ = v;
		  else
		    DBG("octal escape sequence out of range");
	        }
	      else
		*d++ = *s++;
	      break;
	  }
      else
	*d++ = *s++;
    }
  *d = 0;
  return d;
}

#ifdef TEST

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  if (argc < 2)
    return 1;

  char tmp[strlen(argv[1]) + 1];
  int len = str_unesc(tmp, argv[1]) - tmp;

  char hex[2*len + 1];
  mem_to_hex(hex, tmp, len, ' ');
  puts(hex);

  return 0;
}

#endif
