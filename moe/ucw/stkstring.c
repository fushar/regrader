/*
 *	UCW Library -- Strings Allocated on the Stack
 *
 *	(c) 2005--2007 Martin Mares <mj@ucw.cz>
 *	(c) 2005 Tomas Valla <tom@ucw.cz>
 *	(c) 2008 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/stkstring.h"
#include "ucw/string.h"

#include <stdio.h>

uns
stk_array_len(char **s, uns cnt)
{
  uns l = 1;
  while (cnt--)
    l += strlen(*s++);
  return l;
}

void
stk_array_join(char *x, char **s, uns cnt, uns sep)
{
  while (cnt--)
    {
      uns l = strlen(*s);
      memcpy(x, *s, l);
      x += l;
      s++;
      if (sep && cnt)
	*x++ = sep;
    }
  *x = 0;
}

uns
stk_printf_internal(const char *fmt, ...)
{
  uns len = 256;
  char *buf = alloca(len);
  va_list args, args2;
  va_start(args, fmt);
  for (;;)
    {
      va_copy(args2, args);
      int l = vsnprintf(buf, len, fmt, args2);
      va_end(args2);
      if (l < 0)
	len *= 2;
      else
	{
	  va_end(args);
	  return l+1;
	}
      buf = alloca(len);
    }
}

uns
stk_vprintf_internal(const char *fmt, va_list args)
{
  uns len = 256;
  char *buf = alloca(len);
  va_list args2;
  for (;;)
    {
      va_copy(args2, args);
      int l = vsnprintf(buf, len, fmt, args2);
      va_end(args2);
      if (l < 0)
	len *= 2;
      else
	{
	  va_end(args);
	  return l+1;
	}
      buf = alloca(len);
    }
}

void
stk_hexdump_internal(char *dst, const byte *src, uns n)
{
  mem_to_hex(dst, src, n, ' ');
}

void
stk_fsize_internal(char *buf, u64 x)
{
  if (x < 1<<10)
    sprintf(buf, "%dB", (int)x);
  else if (x < 10<<10)
    sprintf(buf, "%.1fK", (double)x/(1<<10));
  else if (x < 1<<20)
    sprintf(buf, "%dK", (int)(x/(1<<10)));
  else if (x < 10<<20)
    sprintf(buf, "%.1fM", (double)x/(1<<20));
  else if (x < 1<<30)
    sprintf(buf, "%dM", (int)(x/(1<<20)));
  else if (x < (u64)10<<30)
    sprintf(buf, "%.1fG", (double)x/(1<<30));
  else if (x != ~(u64)0)
    sprintf(buf, "%dG", (int)(x/(1<<30)));
  else
    strcpy(buf, "unknown");
}

#ifdef TEST

int main(void)
{
  char *a = stk_strndup("are!",3);
  a = stk_strcat(a, " the ");
  a = stk_strmulticat(a, stk_strdup("Jabberwock, "), "my", NULL);
  char *arr[] = { a, " son" };
  a = stk_strarraycat(arr, 2);
  a = stk_printf("Bew%s!", a);
  puts(a);
  puts(stk_hexdump(a, 3));
  char *ary[] = { "The", "jaws", "that", "bite" };
  puts(stk_strjoin(ary, 4, ' '));
  puts(stk_fsize(1234567));
  return 0;
}

#endif
