/*
 *	UCW Library -- Unicode Characters
 *
 *	(c) 1997--2004 Martin Mares <mj@ucw.cz>
 *	(c) 2004 Robert Spalek <robert@ucw.cz>
 *	(c) 2007 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_UNICODE_H
#define _UCW_UNICODE_H

#include "ucw/unaligned.h"

/* Macros for handling UTF-8 */

#define UNI_REPLACEMENT 0xfffc	/** Unicode value used as a default replacement of invalid characters. **/

/**
 * Encode a value from the range `[0, 0xFFFF]`
 * (basic multilingual plane); up to 3 bytes needed (RFC2279).
 **/
static inline byte *utf8_put(byte *p, uns u)
{
  if (u < 0x80)
    *p++ = u;
  else if (u < 0x800)
    {
      *p++ = 0xc0 | (u >> 6);
      *p++ = 0x80 | (u & 0x3f);
    }
  else
    {
      ASSERT(u < 0x10000);
      *p++ = 0xe0 | (u >> 12);
      *p++ = 0x80 | ((u >> 6) & 0x3f);
      *p++ = 0x80 | (u & 0x3f);
    }
  return p;
}

/**
 * Encode a value from the range `[0, 0x7FFFFFFF]`;
 * (superset of Unicode 4.0) up to 6 bytes needed (RFC2279).
 **/
static inline byte *utf8_32_put(byte *p, uns u)
{
  if (u < 0x80)
    *p++ = u;
  else if (u < 0x800)
    {
      *p++ = 0xc0 | (u >> 6);
      goto put1;
    }
  else if (u < (1<<16))
    {
      *p++ = 0xe0 | (u >> 12);
      goto put2;
    }
  else if (u < (1<<21))
    {
      *p++ = 0xf0 | (u >> 18);
      goto put3;
    }
  else if (u < (1<<26))
    {
      *p++ = 0xf8 | (u >> 24);
      goto put4;
    }
  else if (u < (1U<<31))
    {
      *p++ = 0xfc | (u >> 30);
      *p++ = 0x80 | ((u >> 24) & 0x3f);
put4: *p++ = 0x80 | ((u >> 18) & 0x3f);
put3: *p++ = 0x80 | ((u >> 12) & 0x3f);
put2: *p++ = 0x80 | ((u >> 6) & 0x3f);
put1: *p++ = 0x80 | (u & 0x3f);
    }
  else
    ASSERT(0);
  return p;
}

#define UTF8_GET_NEXT if (unlikely((*p & 0xc0) != 0x80)) goto bad; u = (u << 6) | (*p++ & 0x3f)

/**
 * Decode a value from the range `[0, 0xFFFF]` (basic multilingual plane)
 * or return @repl if the encoding has been corrupted.
 **/
static inline byte *utf8_get_repl(const byte *p, uns *uu, uns repl)
{
  uns u = *p++;
  if (u < 0x80)
    ;
  else if (unlikely(u < 0xc0))
    {
      /* Incorrect byte sequence */
    bad:
      u = repl;
    }
  else if (u < 0xe0)
    {
      u &= 0x1f;
      UTF8_GET_NEXT;
    }
  else if (likely(u < 0xf0))
    {
      u &= 0x0f;
      UTF8_GET_NEXT;
      UTF8_GET_NEXT;
    }
  else
    goto bad;
  *uu = u;
  return (byte *)p;
}

/**
 * Decode a value from the range `[0, 0x7FFFFFFF]`
 * or return @repl if the encoding has been corrupted.
 **/
static inline byte *utf8_32_get_repl(const byte *p, uns *uu, uns repl)
{
  uns u = *p++;
  if (u < 0x80)
    ;
  else if (unlikely(u < 0xc0))
    {
      /* Incorrect byte sequence */
    bad:
      u = repl;
    }
  else if (u < 0xe0)
    {
      u &= 0x1f;
      goto get1;
    }
  else if (u < 0xf0)
    {
      u &= 0x0f;
      goto get2;
    }
  else if (u < 0xf8)
    {
      u &= 0x07;
      goto get3;
    }
  else if (u < 0xfc)
    {
      u &= 0x03;
      goto get4;
    }
  else if (u < 0xfe)
    {
      u &= 0x01;
      UTF8_GET_NEXT;
get4: UTF8_GET_NEXT;
get3: UTF8_GET_NEXT;
get2: UTF8_GET_NEXT;
get1: UTF8_GET_NEXT;
    }
  else
    goto bad;
  *uu = u;
  return (byte *)p;
}

/**
 * Decode a value from the range `[0, 0xFFFF]` (basic multilignual plane)
 * or return `UNI_REPLACEMENT` if the encoding has been corrupted.
 **/
static inline byte *utf8_get(const byte *p, uns *uu)
{
  return utf8_get_repl(p, uu, UNI_REPLACEMENT);
}

/**
 * Decode a value from the range `[0, 0x7FFFFFFF]`
 * or return `UNI_REPLACEMENT` if the encoding has been corrupted.
 **/
static inline byte *utf8_32_get(const byte *p, uns *uu)
{
  return utf8_32_get_repl(p, uu, UNI_REPLACEMENT);
}

#define UTF8_SKIP(p) do {				\
    uns c = *p++;					\
    if (c >= 0xc0)					\
      while (c & 0x40 && *p >= 0x80 && *p < 0xc0)	\
        p++, c <<= 1;					\
  } while (0)

#define UTF8_SKIP_BWD(p) while ((*--(p) & 0xc0) == 0x80)

/**
 * Return the number of bytes needed to encode a given value from the range `[0, 0x7FFFFFFF]` to UTF-8.
 **/
static inline uns utf8_space(uns u)
{
  if (u < 0x80)
    return 1;
  if (u < 0x800)
    return 2;
  if (u < (1<<16))
    return 3;
  if (u < (1<<21))
    return 4;
  if (u < (1<<26))
    return 5;
  return 6;
}

/**
 * Compute the length of a single UTF-8 character from it's first byte. The encoding must be valid.
 **/
static inline uns utf8_encoding_len(uns c)
{
  if (c < 0x80)
    return 1;
  ASSERT(c >= 0xc0 && c < 0xfe);
  if (c < 0xe0)
    return 2;
  if (c < 0xf0)
    return 3;
  if (c < 0xf8)
    return 4;
  if (c < 0xfc)
    return 5;
  return 6;
}

/**
 * Encode an UTF-16LE character from the range `[0, 0xD7FF]` or `[0xE000,0x11FFFF]`;
 * up to 4 bytes needed.
 **/
static inline void *utf16_le_put(void *p, uns u)
{
  if (u < 0xd800 || (u < 0x10000 && u >= 0xe000))
    {
      put_u16_le(p, u);
      return p + 2;
    }
  else if ((u -= 0x10000) < 0x100000)
    {
      put_u16_le(p, 0xd800 | (u >> 10));
      put_u16_le(p + 2, 0xdc00 | (u & 0x3ff));
      return p + 4;
    }
  else
    ASSERT(0);
}

/**
 * Encode an UTF-16BE character from the range `[0, 0xD7FF]` or `[0xE000,0x11FFFF]`;
 * up to 4 bytes needed.
 **/
static inline void *utf16_be_put(void *p, uns u)
{
  if (u < 0xd800 || (u < 0x10000 && u >= 0xe000))
    {
      put_u16_be(p, u);
      return p + 2;
    }
  else if ((u -= 0x10000) < 0x100000)
    {
      put_u16_be(p, 0xd800 | (u >> 10));
      put_u16_be(p + 2, 0xdc00 | (u & 0x3ff));
      return p + 4;
    }
  else
    ASSERT(0);
}

/**
 * Decode an UTF-16LE character from the range `[0, 0xD7FF]` or `[0xE000,11FFFF]`
 * or return @repl if the encoding has been corrupted.
 **/
static inline void *utf16_le_get_repl(const void *p, uns *uu, uns repl)
{
  uns u = get_u16_le(p), x, y;
  x = u - 0xd800;
  if (x < 0x800)
    if (x < 0x400 && (y = get_u16_le(p + 2) - 0xdc00) < 0x400)
      {
	u = 0x10000 + (x << 10) + y;
	p += 2;
      }
    else
      u = repl;
  *uu = u;
  return (void *)(p + 2);
}

/**
 * Decode an UTF-16BE character from the range `[0, 0xD7FF]` or `[0xE000,11FFFF]`
 * or return @repl if the encoding has been corrupted.
 **/
static inline void *utf16_be_get_repl(const void *p, uns *uu, uns repl)
{
  uns u = get_u16_be(p), x, y;
  x = u - 0xd800;
  if (x < 0x800)
    if (x < 0x400 && (y = get_u16_be(p + 2) - 0xdc00) < 0x400)
      {
	u = 0x10000 + (x << 10) + y;
	p += 2;
      }
    else
      u = repl;
  *uu = u;
  return (void *)(p + 2);
}

/**
 * Decode an UTF-16LE  character from the range `[0, 0xD7FF]` or `[0xE000,11FFFF]`
 * or return `UNI_REPLACEMENT` if the encoding has been corrupted.
 **/
static inline void *utf16_le_get(const void *p, uns *uu)
{
  return utf16_le_get_repl(p, uu, UNI_REPLACEMENT);
}

/**
 * Decode an UTF-16BE  character from the range `[0, 0xD7FF]` or `[0xE000,11FFFF]`
 * or return `UNI_REPLACEMENT` if the encoding has been corrupted.
 **/
static inline void *utf16_be_get(const void *p, uns *uu)
{
  return utf16_be_get_repl(p, uu, UNI_REPLACEMENT);
}

/**
 * Check an Unicode value and if it seems to be useless (defined by Ucwlib; it may change in future) return `UNI_REPLACEMENT` instead.
 **/
static inline uns unicode_sanitize_char(uns u)
{
  if (u >= 0x10000 ||			// We don't accept anything outside the basic plane
      u >= 0xd800 && u < 0xf900 ||	// neither we do surrogates
      u >= 0x80 && u < 0xa0 ||		// nor latin-1 control chars
      u < 0x20 && u != '\t')
    return UNI_REPLACEMENT;
  return u;
}

/* unicode-utf8.c */

/**
 * Count the number of Unicode character in a zero-terminated UTF-8 string.
 * Returned value for corrupted encoding is undefined, but is never greater than strlen().
 **/
uns utf8_strlen(const byte *str);

/**
 * Same as @utf8_strlen(), but returns at most @n characters.
 **/
uns utf8_strnlen(const byte *str, uns n);

#endif
