/*
 *	UCW Library -- UTF-8 Functions
 *
 *	(c) 1997--2004 Martin Mares <mj@ucw.cz>
 *	(c) 2003 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/unicode.h"

uns
utf8_strlen(const byte *str)
{
  uns len = 0;
  while (*str)
    {
      UTF8_SKIP(str);
      len++;
    }
  return len;
}

uns
utf8_strnlen(const byte *str, uns n)
{
  uns len = 0;
  const byte *end = str + n;
  while (str < end)
    {
      UTF8_SKIP(str);
      len++;
    }
  return len;
}

#ifdef TEST

#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  byte buf[256];

#define FUNCS \
  F(UTF8_GET) F(UTF8_32_GET) F(UTF16_BE_GET) F(UTF16_LE_GET) \
  F(UTF8_PUT) F(UTF8_32_PUT) F(UTF16_BE_PUT) F(UTF16_LE_PUT)

  enum {
#define F(x) FUNC_##x,
    FUNCS
#undef F
  };
  char *names[] = {
#define F(x) [FUNC_##x] = #x,
    FUNCS
#undef F
  };

  uns func = ~0U;
  if (argc > 1)
    for (uns i = 0; i < ARRAY_SIZE(names); i++)
      if (!strcasecmp(names[i], argv[1]))
	func = i;
  if (!~func)
    {
      fprintf(stderr, "Invalid usage!\n");
      return 1;
    }

  if (func < FUNC_UTF8_PUT)
    {
      byte *p = buf, *q = buf, *last;
      uns u;
      bzero(buf, sizeof(buf));
      while (scanf("%x", &u) == 1)
	*q++ = u;
      while (p < q)
	{
	  last = p;
	  if (p != buf)
	    putchar(' ');
	  switch (func)
	    {
	      case FUNC_UTF8_GET:
		p = utf8_get(p, &u);
		break;
	      case FUNC_UTF8_32_GET:
		p = utf8_32_get(p, &u);
		break;
	      case FUNC_UTF16_BE_GET:
		p = utf16_be_get(p, &u);
		break;
	      case FUNC_UTF16_LE_GET:
		p = utf16_le_get(p, &u);
		break;
	      default:
		ASSERT(0);
	    }
	  printf("%04x", u);
	  ASSERT(last < p && p <= q);
	}
      putchar('\n');
    }
  else
    {
      uns u, i=0;
      while (scanf("%x", &u) == 1)
	{
	  byte *p = buf, *q = buf;
	  switch (func)
	    {
	      case FUNC_UTF8_PUT:
		p = utf8_put(p, u);
		break;
	      case FUNC_UTF8_32_PUT:
		p = utf8_32_put(p, u);
		break;
	      case FUNC_UTF16_BE_PUT:
		p = utf16_be_put(p, u);
		break;
	      case FUNC_UTF16_LE_PUT:
		p = utf16_le_put(p, u);
		break;
	      default:
		ASSERT(0);
	    }
	  while (q < p)
	    {
	      if (i++)
		putchar(' ');
	      printf("%02x", *q++);
	    }
	}
      putchar('\n');
    }
  return 0;
}

#endif
