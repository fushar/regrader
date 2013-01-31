/*
 *	UCW Library -- Base 224 Encoding & Decoding
 *
 *	(c) 2002 Martin Mares <mj@ucw.cz>
 *
 *	The `base-224' encoding transforms general sequences of bytes
 *	to sequences of non-control 8-bit characters (0x20-0xff). Since
 *	224 and 256 are incompatible bases (there is no k,l: 224^k=256^l)
 *	and we want to avoid lengthy calculations, we cheat a bit:
 *
 *	Each base-224 digit can be represented as a (base-7 digit, base-32 digit)
 *	pair, so we pass the lower 5 bits directly and use a base-7 encoder
 *	for the upper part. We process blocks of 39 bits and encode them
 *	to 5 base-224 digits: we take 5x5 bits as the lower halves and convert
 *	the remaining 14 bits in base-7 (2^14 = 16384 < 16807 = 7^5) to get
 *	the 7 upper parts we need (with a little redundancy). Little endian
 *	ordering is used to make handling of partial blocks easy.
 *
 *	We transform 39 source bits to 40 destination bits, stretching the data
 *	by 1/39 = approx. 2.56%.
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#undef LOCAL_DEBUG

#include "ucw/lib.h"
#include "ucw/base224.h"

static void
encode_block(byte *w, u32 hi, u32 lo)
{
  uns x, y;

  /*
   *   Splitting of the 39-bit block: [a-e][0-5] are the base-32 digits, *'s are used for base-7.
   *   +----------------+----------------+----------------+----------------+----------------+
   *   +00******e4e3e2e1|e0******d4d3d2d1|d0******c4c3c2c1|c0******b4b3b2b1|b0****a4a3a2a1a0|
   *   +----------------+----------------+----------------+----------------+----------------+
   */

  w[0] = lo & 0x1f;
  w[1] = (lo >> 7) & 0x1f;
  w[2] = (lo >> 15) & 0x1f;
  w[3] = (lo >> 23) & 0x1f;
  w[4] = (lo >> 31) | ((hi << 1) & 0x1e);
  x = (lo >> 5)  & 0x0003
    | (lo >> 10) & 0x001c
    | (lo >> 15) & 0x00e0
    | (lo >> 20) & 0x0700
    | (hi << 7)  & 0x3800;
  DBG("<<< h=%08x l=%08x x=%d", hi, lo, x);
  for (y=0; y<5; y++)
    {
      w[y] += 0x20 + ((x % 7) << 5);
      x /= 7;
    }
}

uns
base224_encode(byte *dest, const byte *src, uns len)
{
  u32 lo=0, hi=0;			/* 64-bit buffer accumulating input bits */
  uns i=0;				/* How many source bits do we have buffered */
  u32 x;
  byte *w=dest;

  while (len--)
    {
      x = *src++;
      if (i < 32)
	{
	  lo |= x << i;
	  if (i > 24)
	    hi |= x >> (32-i);
	}
      else
	hi |= x << (i-32);
      i += 8;
      if (i >= 39)
	{
	  encode_block(w, hi, lo);
	  w += 5;
	  lo = hi >> 7;
	  hi = 0;
	  i -= 39;
	}
    }
  if (i)				/* Partial block */
    {
      encode_block(w, hi, lo);
      w += (i+8)/8;			/* Just check logarithms if you want to understand */
    }
  return w - dest;
}

uns
base224_decode(byte *dest, const byte *src, uns len)
{
  u32 hi=0, lo=0;			/* 64-bit buffer accumulating output bits */
  uns i=0;				/* How many bits do we have accumulated */
  u32 h, l;				/* Decoding of the current block */
  uns x;				/* base-7 part of the current block */
  uns len0;
  byte *start = dest;

  do
    {
      if (!len)
	break;
      len0 = len;

      ASSERT(*src >= 0x20);		/* byte 0 */
      h = 0;
      l = *src & 0x1f;
      x = (*src++ >> 5) - 1;
      if (!--len)
	goto blockend;

      ASSERT(*src >= 0x20);		/* byte 1 */
      l |= (*src & 0x1f) << 7;
      x += ((*src++ >> 5) - 1) * 7;
      if (!--len)
	goto blockend;

      ASSERT(*src >= 0x20);		/* byte 2 */
      l |= (*src & 0x1f) << 15;
      x += ((*src++ >> 5) - 1) * 7*7;
      if (!--len)
	goto blockend;

      ASSERT(*src >= 0x20);		/* byte 3 */
      l |= (*src & 0x1f) << 23;
      x += ((*src++ >> 5) - 1) * 7*7*7;
      if (!--len)
	goto blockend;

      ASSERT(*src >= 0x20);		/* byte 4 */
      l |= *src << 31;
      h = (*src & 0x1f) >> 1;
      x += ((*src++ >> 5) - 1) * 7*7*7*7;
      --len;

    blockend:
      len0 -= len;
      l |= ((x & 0x0003) << 5)		/* Decode base-7 */
	|  ((x & 0x001c) << 10)
	|  ((x & 0x00e0) << 15)
	|  ((x & 0x0700) << 20);
      h |=  (x & 0x3800) >> 7;

      DBG("<<< i=%d h=%08x l=%08x x=%d len0=%d", i, h, l, x, len0);
      lo |= l << i;
      hi |= h << i;
      if (i)
	hi |= l >> (32-i);
      i += len0*8 - 1;

      while (i >= 8)
	{
	  *dest++ = lo;
	  lo = (lo >> 8U) | (hi << 24);
	  hi >>= 8;
	  i -= 8;
	}
    }
  while (len0 == 5);
  return dest-start;
}
