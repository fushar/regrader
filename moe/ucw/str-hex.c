/*
 *	UCW Library -- Hexdumping and Unhexdumping
 *
 *	(c) 2008 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/string.h"
#include "ucw/chartype.h"

static uns
hex_make(uns x)
{
  return (x < 10) ? (x + '0') : (x - 10 + 'a');
}

void
mem_to_hex(char *dest, const byte *src, uns bytes, uns flags)
{
  uns sep = flags & 0xff;

  while (bytes--)
    {
      dest[0] = hex_make(*src >> 4);
      dest[1] = hex_make(*src & 0x0f);
      if (flags & MEM_TO_HEX_UPCASE)
	{
	  dest[0] = Cupcase(dest[0]);
	  dest[1] = Cupcase(dest[1]);
	}
      dest += 2;
      if (sep && bytes)
	*dest++ = sep;
      src++;
    }
  *dest = 0;
}

static uns
hex_parse(uns c)
{
  c = Cupcase(c);
  c -= '0';
  return (c < 10) ? c : (c - 7);
}

const char *
hex_to_mem(byte *dest, const char *src, uns max_bytes, uns flags)
{
  uns sep = flags & 0xff;
  while (max_bytes-- && Cxdigit(src[0]) && Cxdigit(src[1]))
    {
      *dest++ = (hex_parse(src[0]) << 4) | hex_parse(src[1]);
      src += 2;
      if (sep && *src && max_bytes)
	{
	  if (*src != (char)sep)
	    return src;
	  src++;
	}
    }
  return src;
}

#ifdef TEST

#include <stdio.h>

int main(void)
{
  byte x[4] = { 0xfe, 0xed, 0xf0, 0x0d };
  byte y[4];
  char a[16];

  mem_to_hex(a, x, 4, MEM_TO_HEX_UPCASE);
  puts(a);
  mem_to_hex(a, x, 4, ':');
  puts(a);
  const char *z = hex_to_mem(y, a, 4, ':');
  if (*z)
    puts("BAD");
  else
    printf("%02x%02x%02x%02x\n", y[0], y[1], y[2], y[3]);

  return 0;
}

#endif
