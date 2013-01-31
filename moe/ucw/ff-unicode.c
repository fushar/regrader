/*
 *	UCW Library: Reading and writing of UTF-8 on Fastbuf Streams
 *
 *	(c) 2001--2004 Martin Mares <mj@ucw.cz>
 *	(c) 2004 Robert Spalek <robert@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/fastbuf.h"
#include "ucw/unicode.h"
#include "ucw/ff-unicode.h"
#include "ucw/ff-binary.h"

/*** UTF-8 ***/

int
bget_utf8_slow(struct fastbuf *b, uns repl)
{
  int c = bgetc(b);
  int code;

  if (c < 0x80)				/* Includes EOF */
    return c;
  if (c < 0xc0)				/* Incorrect combination */
    return repl;
  if (c >= 0xf0)			/* Too large, skip it */
    {
      while ((c = bgetc(b)) >= 0x80 && c < 0xc0)
	;
      goto wrong;
    }
  if (c >= 0xe0)			/* 3 bytes */
    {
      code = c & 0x0f;
      if ((c = bgetc(b)) < 0x80 || c >= 0xc0)
	goto wrong;
      code = (code << 6) | (c & 0x3f);
      if ((c = bgetc(b)) < 0x80 || c >= 0xc0)
	goto wrong;
      code = (code << 6) | (c & 0x3f);
    }
  else					/* 2 bytes */
    {
      code = c & 0x1f;
      if ((c = bgetc(b)) < 0x80 || c >= 0xc0)
	goto wrong;
      code = (code << 6) | (c & 0x3f);
    }
  return code;

 wrong:
  if (c >= 0)
    bungetc(b);
  return repl;
}

int
bget_utf8_32_slow(struct fastbuf *b, uns repl)
{
  int c = bgetc(b);
  int code;
  int nr;

  if (c < 0x80)				/* Includes EOF */
    return c;
  if (c < 0xc0)				/* Incorrect combination */
    return repl;
  if (c < 0xe0)
    {
      code = c & 0x1f;
      nr = 1;
    }
  else if (c < 0xf0)
    {
      code = c & 0x0f;
      nr = 2;
    }
  else if (c < 0xf8)
    {
      code = c & 0x07;
      nr = 3;
    }
  else if (c < 0xfc)
    {
      code = c & 0x03;
      nr = 4;
    }
  else if (c < 0xfe)
    {
      code = c & 0x01;
      nr = 5;
    }
  else					/* Too large, skip it */
    {
      while ((c = bgetc(b)) >= 0x80 && c < 0xc0)
	;
      goto wrong;
    }
  while (nr-- > 0)
    {
      if ((c = bgetc(b)) < 0x80 || c >= 0xc0)
	goto wrong;
      code = (code << 6) | (c & 0x3f);
    }
  return code;

 wrong:
  if (c >= 0)
    bungetc(b);
  return repl;
}

void
bput_utf8_slow(struct fastbuf *b, uns u)
{
  ASSERT(u < 65536);
  if (u < 0x80)
    bputc(b, u);
  else
    {
      if (u < 0x800)
	bputc(b, 0xc0 | (u >> 6));
      else
	{
	  bputc(b, 0xe0 | (u >> 12));
	  bputc(b, 0x80 | ((u >> 6) & 0x3f));
	}
      bputc(b, 0x80 | (u & 0x3f));
    }
}

void
bput_utf8_32_slow(struct fastbuf *b, uns u)
{
  ASSERT(u < (1U<<31));
  if (u < 0x80)
    bputc(b, u);
  else
    {
      if (u < 0x800)
	bputc(b, 0xc0 | (u >> 6));
      else
	{
	  if (u < (1<<16))
	    bputc(b, 0xe0 | (u >> 12));
	  else
	    {
	      if (u < (1<<21))
		bputc(b, 0xf0 | (u >> 18));
	      else
		{
		  if (u < (1<<26))
		    bputc(b, 0xf8 | (u >> 24));
		  else
		    {
		      bputc(b, 0xfc | (u >> 30));
		      bputc(b, 0x80 | ((u >> 24) & 0x3f));
		    }
		  bputc(b, 0x80 | ((u >> 18) & 0x3f));
		}
	      bputc(b, 0x80 | ((u >> 12) & 0x3f));
	    }
	  bputc(b, 0x80 | ((u >> 6) & 0x3f));
	}
      bputc(b, 0x80 | (u & 0x3f));
    }
}

/*** UTF-16 ***/

int
bget_utf16_be_slow(struct fastbuf *b, uns repl)
{
  if (bpeekc(b) < 0)
    return -1;
  uns u = bgetw_be(b), x, y;
  if ((int)u < 0)
    return repl;
  if ((x = u - 0xd800) >= 0x800)
    return u;
  if (x >= 0x400 || bpeekc(b) < 0 || (y = bgetw_be(b) - 0xdc00) >= 0x400)
    return repl;
  return 0x10000 + (x << 10) + y;
}

int
bget_utf16_le_slow(struct fastbuf *b, uns repl)
{
  if (bpeekc(b) < 0)
    return -1;
  uns u = bgetw_le(b), x, y;
  if ((int)u < 0)
    return repl;
  if ((x = u - 0xd800) >= 0x800)
    return u;
  if (x >= 0x400 || bpeekc(b) < 0 || (y = bgetw_le(b) - 0xdc00) >= 0x400)
    return repl;
  return 0x10000 + (x << 10) + y;
}

void
bput_utf16_be_slow(struct fastbuf *b, uns u)
{
  if (u < 0xd800 || (u < 0x10000 && u >= 0xe000))
    {
      bputc(b, u >> 8);
      bputc(b, u & 0xff);
    }
  else if ((u -= 0x10000) < 0x100000)
    {
      bputc(b, 0xd8 | (u >> 18));
      bputc(b, (u >> 10) & 0xff);
      bputc(b, 0xdc | ((u >> 8) & 0x3));
      bputc(b, u & 0xff);
    }
  else
    ASSERT(0);
}

void
bput_utf16_le_slow(struct fastbuf *b, uns u)
{
  if (u < 0xd800 || (u < 0x10000 && u >= 0xe000))
    {
      bputc(b, u & 0xff);
      bputc(b, u >> 8);
    }
  else if ((u -= 0x10000) < 0x100000)
    {
      bputc(b, (u >> 10) & 0xff);
      bputc(b, 0xd8 | (u >> 18));
      bputc(b, u & 0xff);
      bputc(b, 0xdc | ((u >> 8) & 0x3));
    }
  else
    ASSERT(0);
}

#ifdef TEST

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
#define FUNCS \
  F(BGET_UTF8) F(BGET_UTF8_32) F(BGET_UTF16_BE) F(BGET_UTF16_LE) \
  F(BPUT_UTF8) F(BPUT_UTF8_32) F(BPUT_UTF16_BE) F(BPUT_UTF16_LE)

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

  struct fastbuf *b = fbgrow_create(8);
  if (func < FUNC_BPUT_UTF8)
    {
      uns u;
      while (scanf("%x", &u) == 1)
	bputc(b, u);
      fbgrow_rewind(b);
      while (bpeekc(b) >= 0)
        {
	  if (btell(b))
	    putchar(' ');
	  switch (func)
	    {
	      case FUNC_BGET_UTF8:
		u = bget_utf8_slow(b, UNI_REPLACEMENT);
		break;
	      case FUNC_BGET_UTF8_32:
		u = bget_utf8_32_slow(b, UNI_REPLACEMENT);
		break;
	      case FUNC_BGET_UTF16_BE:
		u = bget_utf16_be_slow(b, UNI_REPLACEMENT);
		break;
	      case FUNC_BGET_UTF16_LE:
		u = bget_utf16_le_slow(b, UNI_REPLACEMENT);
		break;
	      default:
		ASSERT(0);
	    }
	  printf("%04x", u);
	}
      putchar('\n');
    }
  else
    {
      uns u, i = 0;
      while (scanf("%x", &u) == 1)
        {
	  switch (func)
	    {
	      case FUNC_BPUT_UTF8:
		bput_utf8_slow(b, u);
		break;
	      case FUNC_BPUT_UTF8_32:
		bput_utf8_32_slow(b, u);
		break;
	      case FUNC_BPUT_UTF16_BE:
                bput_utf16_be_slow(b, u);
		break;
	      case FUNC_BPUT_UTF16_LE:
		bput_utf16_le_slow(b, u);
		break;
	      default:
		ASSERT(0);
	    }
	  fbgrow_rewind(b);
	  u = 0;
	  while (bpeekc(b) >= 0)
	    {
	      if (i++)
		putchar(' ');
	      printf("%02x", bgetc(b));
	    }
	  fbgrow_reset(b);
	}
      putchar('\n');
    }
  bclose(b);

  return 0;
}

#endif
